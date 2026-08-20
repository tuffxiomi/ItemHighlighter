#include <pl/Mod.hpp>
#include <pl/memory/Hook.hpp>
#include <pl/memory/Signature.hpp>
#include <pl/memory/Vtable.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace item_highlighter {

namespace {

constexpr std::string_view MinecraftLibrary = "libminecraftpe.so";
constexpr std::string_view UiContextTypeInfo =
    "24MinecraftUIRenderContext";

constexpr std::size_t DrawTextVtableSlot = 6;
constexpr std::size_t FillRectangleVtableSlot = 16;

constexpr std::chrono::milliseconds MarkerLifetime{1600};

// Exact signatures from the supplied BedrockTools source.
constexpr std::string_view ItemRendererRenderGuiItemNewSignature =
    "? ? ? D1 ? ? ? FD ? ? ? 6D ? ? ? 6D "
    "? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 "
    "? ? ? A9 ? ? ? A9 ? ? ? 91 5B D0 3B D5 "
    "F4 03 00 AA";

constexpr std::string_view ScreenViewRenderSignature =
    "? ? ? FC ? ? ? 6D ? ? ? 6D "
    "? ? ? A9 ? ? ? A9 ? ? ? A9 "
    "? ? ? A9 ? ? ? A9 ? ? ? A9 "
    "? ? ? 91 ? ? ? D1 48 D0 3B D5 "
    "FC 03 00 AA";

// ItemStackBase -> SharedCounter pointer.
constexpr std::size_t ItemStackBaseItemOffset = 0x8;

// SharedCounter -> Item pointer.
constexpr std::size_t SharedCounterPointerOffset = 0x0;

// Item -> mId.
constexpr std::size_t ItemIdOffset = 0x8A;

struct Color {
    float r;
    float g;
    float b;
    float a;
};

struct RectangleArea {
    float left;
    float right;
    float top;
    float bottom;
};

struct Font {};
struct TextMeasureData {};
struct CaretMeasureData {};

enum class TextAlignment : std::uint8_t {
    Left,
    Right,
    Center
};

using ItemRendererRenderGuiItemNewFn =
    std::uint64_t (*)(
        void*,
        void*,
        void*,
        unsigned int,
        unsigned char,
        std::uint64_t,
        float,
        float,
        float,
        float,
        float
    );

using ScreenViewRenderFn =
    void (*)(
        void*,
        void*,
        void*,
        void*,
        void*,
        void*,
        void*,
        void*
    );

using DrawTextFn =
    void (*)(
        void*,
        Font&,
        const RectangleArea&,
        const std::string&,
        const Color&,
        TextAlignment,
        float,
        const TextMeasureData&,
        const CaretMeasureData&
    );

using FillRectangleFn =
    void (*)(
        void*,
        const RectangleArea&,
        const Color&,
        float
    );

struct RenderSlot {
    bool initialized = false;

    std::uint16_t itemId = 0;
    unsigned int aux = 0;

    float x = 0.0f;
    float y = 0.0f;
};

struct Marker {
    float x = 0.0f;
    float y = 0.0f;

    std::chrono::steady_clock::time_point expires{};
};

ItemRendererRenderGuiItemNewFn gItemRendererOriginal = nullptr;
ScreenViewRenderFn gScreenViewRenderOriginal = nullptr;
DrawTextFn gDrawTextOriginal = nullptr;
FillRectangleFn gFillRectangle = nullptr;

void* gActiveUiContext = nullptr;

bool gEnabled = false;
bool gBaselineReady = false;

std::array<RenderSlot, 128> gRenderSlots{};
std::vector<Marker> gMarkers;

std::vector<std::unique_ptr<pl::memory::HookHandle>> gHooks;

bool samePosition(float a, float b) {
    return std::fabs(a - b) < 0.75f;
}

std::uint16_t getItemId(void* stack) {
    if (!stack) {
        return 0;
    }

    auto* stackBytes =
        static_cast<std::byte*>(stack);

    void* counter =
        *reinterpret_cast<void**>(
            stackBytes + ItemStackBaseItemOffset
        );

    if (!counter) {
        return 0;
    }

    void* item =
        *reinterpret_cast<void**>(
            static_cast<std::byte*>(counter) +
            SharedCounterPointerOffset
        );

    if (!item) {
        return 0;
    }

    return *reinterpret_cast<std::uint16_t*>(
        static_cast<std::byte*>(item) +
        ItemIdOffset
    );
}

RenderSlot* findRenderSlot(float x, float y) {
    /*
     * GUI positions are much more stable than ItemStack pointers.
     *
     * We intentionally do NOT use the stack pointer as the identity
     * because Minecraft can use temporary/copy ItemStack objects while
     * rendering.
     */
    for (auto& slot : gRenderSlots) {
        if (!slot.initialized) {
            continue;
        }

        if (samePosition(slot.x, x) &&
            samePosition(slot.y, y)) {
            return &slot;
        }
    }

    for (auto& slot : gRenderSlots) {
        if (!slot.initialized) {
            return &slot;
        }
    }

    return nullptr;
}

void addMarker(float x, float y) {
    const auto expires =
        std::chrono::steady_clock::now() +
        MarkerLifetime;

    for (auto& marker : gMarkers) {
        if (samePosition(marker.x, x) &&
            samePosition(marker.y, y)) {
            marker.expires = expires;
            return;
        }
    }

    gMarkers.push_back(
        Marker{
            x,
            y,
            expires
        }
    );
}

void purgeExpiredMarkers() {
    const auto now =
        std::chrono::steady_clock::now();

    gMarkers.erase(
        std::remove_if(
            gMarkers.begin(),
            gMarkers.end(),
            [&](const Marker& marker) {
                return marker.expires <= now;
            }
        ),
        gMarkers.end()
    );
}

void drawStar(
    void* context,
    float x,
    float y
) {
    if (!context || !gFillRectangle) {
        return;
    }

    const auto milliseconds =
        std::chrono::duration_cast<
            std::chrono::milliseconds
        >(
            std::chrono::steady_clock::now()
                .time_since_epoch()
        ).count();

    const float time =
        static_cast<float>(milliseconds);

    const float pulse =
        0.75f +
        0.25f *
            (
                0.5f +
                0.5f *
                    std::sin(
                        time * 0.012f
                    )
            );

    const Color gold{
        1.0f,
        0.86f,
        0.18f,
        pulse
    };

    /*
     * The marker is deliberately small so it behaves like
     * the Java Item Highlighter's star rather than covering
     * the entire item.
     */
    const float centerX = x + 13.0f;
    const float centerY = y + 1.0f;

    // Vertical beam.
    gFillRectangle(
        context,
        {
            centerX - 1.0f,
            centerX + 1.0f,
            centerY - 5.0f,
            centerY + 5.0f
        },
        gold,
        1.0f
    );

    // Horizontal beam.
    gFillRectangle(
        context,
        {
            centerX - 5.0f,
            centerX + 5.0f,
            centerY - 1.0f,
            centerY + 1.0f
        },
        gold,
        1.0f
    );

    // Center.
    gFillRectangle(
        context,
        {
            centerX - 2.0f,
            centerX + 2.0f,
            centerY - 2.0f,
            centerY + 2.0f
        },
        gold,
        1.0f
    );
}

void drawAllMarkers(void* context) {
    if (!context || !gFillRectangle) {
        return;
    }

    purgeExpiredMarkers();

    for (const auto& marker : gMarkers) {
        drawStar(
            context,
            marker.x,
            marker.y
        );
    }
}

void observeItem(
    void* stack,
    unsigned int aux,
    float x,
    float y
) {
    if (!gEnabled || !stack) {
        return;
    }

    const std::uint16_t itemId =
        getItemId(stack);

    if (itemId == 0) {
        return;
    }

    RenderSlot* slot =
        findRenderSlot(x, y);

    if (!slot) {
        return;
    }

    if (!gBaselineReady) {
        slot->initialized = true;
        slot->itemId = itemId;
        slot->aux = aux;
        slot->x = x;
        slot->y = y;
        return;
    }

    const bool changed =
        !slot->initialized ||
        slot->itemId != itemId ||
        slot->aux != aux;

    /*
     * Do not highlight the initial inventory/hotbar contents.
     * Only changes after the initial baseline are marked.
     */
    if (slot->initialized && changed) {
        addMarker(x, y);
    }

    slot->initialized = true;
    slot->itemId = itemId;
    slot->aux = aux;
    slot->x = x;
    slot->y = y;
}

std::uint64_t itemRendererHook(
    void* itemRenderer,
    void* baseActorRenderContext,
    void* stack,
    unsigned int aux,
    unsigned char mode,
    std::uint64_t a6,
    float x,
    float y,
    float sx,
    float sy,
    float sz
) {
    const std::uint64_t result =
        gItemRendererOriginal
            ? gItemRendererOriginal(
                itemRenderer,
                baseActorRenderContext,
                stack,
                aux,
                mode,
                a6,
                x,
                y,
                sx,
                sy,
                sz
            )
            : 0;

    if (gEnabled) {
        observeItem(
            stack,
            aux,
            x,
            y
        );
    }

    return result;
}

void drawTextHook(
    void* self,
    Font& font,
    const RectangleArea& rectangle,
    const std::string& text,
    const Color& color,
    TextAlignment alignment,
    float alpha,
    const TextMeasureData& measure,
    const CaretMeasureData& caret
) {
    /*
     * This is the important part:
     *
     * BedrockTools captures the active MinecraftUIRenderContext
     * from DrawText. We do the same thing.
     */
    if (gEnabled) {
        gActiveUiContext = self;
    }

    if (gDrawTextOriginal) {
        gDrawTextOriginal(
            self,
            font,
            rectangle,
            text,
            color,
            alignment,
            alpha,
            measure,
            caret
        );
    }
}

void screenViewRenderHook(
    void* self,
    void* a2,
    void* a3,
    void* a4,
    void* a5,
    void* a6,
    void* a7,
    void* a8
) {
    /*
     * Reset the context at the beginning of each UI render pass.
     * DrawTextHook will set it again while the original render
     * function executes.
     */
    gActiveUiContext = nullptr;

    if (gScreenViewRenderOriginal) {
        gScreenViewRenderOriginal(
            self,
            a2,
            a3,
            a4,
            a5,
            a6,
            a7,
            a8
        );
    }

    if (!gEnabled) {
        return;
    }

    /*
     * ItemRendererRenderGuiItemNew ran during the original
     * ScreenViewRender call, so the slot positions have now
     * been observed.
     *
     * Draw after the vanilla UI render pass so the star is
     * visible above the item.
     */
    if (gActiveUiContext) {
        drawAllMarkers(
            gActiveUiContext
        );
    }

    purgeExpiredMarkers();
}

void clearState() {
    gActiveUiContext = nullptr;

    gBaselineReady = false;

    gRenderSlots.fill({});

    gMarkers.clear();
}

} // namespace

class ItemHighlighterMod {
public:
    static ItemHighlighterMod& instance() {
        static ItemHighlighterMod mod;
        return mod;
    }

    bool load() {
        auto& logger =
            loggerRef();

        logger.info(
            "Loading Item Highlighter..."
        );

        /*
         * Resolve BOTH render signatures before enable().
         *
         * No signature scanning occurs from enable(), which avoids
         * repeating expensive libminecraftpe.so scans on the
         * Android main thread.
         */
        std::vector<std::string> signatures{
            std::string(
                ItemRendererRenderGuiItemNewSignature
            ),
            std::string(
                ScreenViewRenderSignature
            )
        };

        const auto resolved =
            pl::memory::resolveSignatures(
                std::span<const std::string>(
                    signatures.data(),
                    signatures.size()
                ),
                MinecraftLibrary
            );

        const auto itemIt =
            resolved.find(
                std::string(
                    ItemRendererRenderGuiItemNewSignature
                )
            );

        if (itemIt != resolved.end()) {
            gItemRendererOriginal =
                reinterpret_cast<
                    ItemRendererRenderGuiItemNewFn
                >(
                    itemIt->second
                );
        }

        const auto screenIt =
            resolved.find(
                std::string(
                    ScreenViewRenderSignature
                )
            );

        if (screenIt != resolved.end()) {
            gScreenViewRenderOriginal =
                reinterpret_cast<
                    ScreenViewRenderFn
                >(
                    screenIt->second
                );
        }

        if (!gItemRendererOriginal) {
            logger.error(
                "ItemRendererRenderGuiItemNew "
                "signature not found"
            );

            return false;
        }

        if (!gScreenViewRenderOriginal) {
            logger.error(
                "ScreenViewRender signature not found"
            );

            return false;
        }

        /*
         * Resolve UI vtable functions.
         *
         * These are the same vtable slots used by the supplied
         * BedrockTools ShulkerPreview implementation.
         */
        const uintptr_t drawTextAddress =
            pl::memory::resolveVtableFunction(
                UiContextTypeInfo,
                DrawTextVtableSlot,
                MinecraftLibrary
            );

        if (!drawTextAddress) {
            logger.error(
                "MinecraftUIRenderContext DrawText "
                "vtable slot not found"
            );

            return false;
        }

        const uintptr_t fillRectangleAddress =
            pl::memory::resolveVtableFunction(
                UiContextTypeInfo,
                FillRectangleVtableSlot,
                MinecraftLibrary
            );

        if (!fillRectangleAddress) {
            logger.error(
                "MinecraftUIRenderContext FillRectangle "
                "vtable slot not found"
            );

            return false;
        }

        gDrawTextOriginal =
            reinterpret_cast<DrawTextFn>(
                drawTextAddress
            );

        gFillRectangle =
            reinterpret_cast<FillRectangleFn>(
                fillRectangleAddress
            );

        logger.info(
            "Item Highlighter native targets resolved"
        );

        return true;
    }

    bool enable() {
        if (gEnabled) {
            return true;
        }

        auto& logger =
            loggerRef();

        logger.info(
            "Enabling Item Highlighter..."
        );

        clearState();

        /*
         * Hook ScreenViewRender first.
         *
         * This gives us a reliable UI-render boundary.
         */
        auto screenHook =
            std::make_unique<
                pl::memory::HookHandle
            >(
                reinterpret_cast<void*>(
                    gScreenViewRenderOriginal
                ),
                reinterpret_cast<void*>(
                    screenViewRenderHook
                ),
                reinterpret_cast<void**>(
                    &gScreenViewRenderOriginal
                )
            );

        if (!screenHook->installed()) {
            logger.error(
                "Failed to hook ScreenViewRender"
            );

            return false;
        }

        gHooks.push_back(
            std::move(screenHook)
        );

        /*
         * Hook the GUI item renderer.
         *
         * This observes hotbar and inventory items.
         */
        auto itemHook =
            std::make_unique<
                pl::memory::HookHandle
            >(
                reinterpret_cast<void*>(
                    gItemRendererOriginal
                ),
                reinterpret_cast<void*>(
                    itemRendererHook
                ),
                reinterpret_cast<void**>(
                    &gItemRendererOriginal
                )
            );

        if (!itemHook->installed()) {
            gHooks.clear();

            logger.error(
                "Failed to hook "
                "ItemRendererRenderGuiItemNew"
            );

            return false;
        }

        gHooks.push_back(
            std::move(itemHook)
        );

        /*
         * Finally capture MinecraftUIRenderContext from DrawText.
         */
        auto drawTextHookHandle =
            std::make_unique<
                pl::memory::HookHandle
            >(
                reinterpret_cast<void*>(
                    gDrawTextOriginal
                ),
                reinterpret_cast<void*>(
                    drawTextHook
                ),
                reinterpret_cast<void**>(
                    &gDrawTextOriginal
                )
            );

        if (!drawTextHookHandle->installed()) {
            gHooks.clear();

            logger.error(
                "Failed to hook "
                "MinecraftUIRenderContext::DrawText"
            );

            return false;
        }

        gHooks.push_back(
            std::move(drawTextHookHandle)
        );

        gEnabled = true;

        logger.info(
            "Item Highlighter enabled"
        );

        return true;
    }

    bool disable() {
        gEnabled = false;

        clearState();

        return true;
    }

    bool unload() {
        gEnabled = false;

        gHooks.clear();

        clearState();

        gItemRendererOriginal = nullptr;
        gScreenViewRenderOriginal = nullptr;
        gDrawTextOriginal = nullptr;
        gFillRectangle = nullptr;

        return true;
    }

private:
    static pl::log::Logger& loggerRef() {
        static pl::log::Logger& logger =
            pl::log::Logger::getOrCreate(
                "Item Highlighter"
            );

        return logger;
    }
};

} // namespace item_highlighter

PL_REGISTER_MOD(
    item_highlighter::ItemHighlighterMod,
    item_highlighter::ItemHighlighterMod::instance()
);
