#include <pl/Mod.hpp>
#include <pl/memory/Hook.hpp>
#include <pl/memory/Signature.hpp>
#include <pl/memory/Vtable.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <memory>
#include <cmath>
#include <vector>

namespace item_highlighter {

namespace {

constexpr std::string_view kMinecraftLibrary = "libminecraftpe.so";
constexpr std::size_t kInventorySlots = 36;
constexpr double kHighlightDurationMs = 1600.0;
constexpr std::size_t kUiFillRectangleVtableSlot = 16;
constexpr std::string_view kUiContextTypeInfo = "24MinecraftUIRenderContext";

// These are the BedrockTools signatures used by this standalone mod.
// The project intentionally defines only the signatures it needs.
constexpr std::string_view kSigContainerOpen =
    "? ? ? A9 ? ? ? F9 FD 03 00 91 F3 03 00 AA ? ? ? 94 ? ? ? F9 E1 03 1F 2A ? ? ? 94";
constexpr std::string_view kSigContainerDtor =
    "? ? ? D1 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? 91 56 D0 3B D5 F3 03 00 AA ? ? ? F9 ? ? ? F9 ? ? ? 90 ? ? ? 91 ? ? ? F9 ? ? ? F9 ? ? ? 91 ? ? ? F9 ? ? ? 94";
constexpr std::string_view kSigContainerGetItemStack =
    "? ? ? D1 ? ? ? A9 ? ? ? A9 ? ? ? 91 54 D0 3B D5 F3 03 00 AA ? ? ? 91 ? ? ? F9 ? ? ? F8 ? ? ? 95 ? ? ? F9 ? ? ? F9 ? ? ? 91";
constexpr std::string_view kSigScreenViewRender =
    "? ? ? FC ? ? ? 6D ? ? ? 6D ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? 91 ? ? ? D1 48 D0 3B D5 FC 03 00 AA";
constexpr std::string_view kSigItemRendererGui =
    "? ? ? D1 ? ? ? FD ? ? ? 6D ? ? ? 6D ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? 91 5B D0 3B D5 F4 03 00 AA";
constexpr std::string_view kSigItemDamage =
    "? ? ? D1 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? 91 55 D0 3B D5 ? ? ? F9 ? ? ? F8 ? ? ? F9 ? ? ? B4 ? ? ? F9 ? ? ? B4 E8 03 00 AA";

constexpr std::size_t kItemStackItemOffset = 0x8;
constexpr std::size_t kSharedCounterPointerOffset = 0x0;
constexpr std::size_t kItemIdOffset = 0x8A;

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

using ContainerScreenFn = void* (*)(void*, void*, void*, void*, void*, void*, void*, void*);
using ContainerGetItemStackFn = void* (*)(void*, const std::string&, int);
using ScreenViewRenderFn = void (*)(void*, void*, void*, void*, void*, void*, void*, void*);
using ItemRendererRenderGuiItemNewFn = std::uint64_t (*)(void*, void*, void*, unsigned int, unsigned char, std::uint64_t, float, float, float, float, float);
using ItemStackBaseGetDamageValueFn = short (*)(void*);
using DrawTextFn = void (*)(void*, Font&, const RectangleArea&, const std::string&, const Color&, TextAlignment, float, const TextMeasureData&, const CaretMeasureData&);
using FillRectangleFn = void (*)(void*, const RectangleArea&, const Color&, float);

struct SlotState {
    std::uint16_t itemId = 0;
    short damage = 0;
    bool valid = false;
};

struct Marker {
    void* stack = nullptr;
    float x = 0.0f;
    float y = 0.0f;
    std::chrono::steady_clock::time_point expires{};
};

ContainerScreenFn gContainerOpenOriginal = nullptr;
ContainerScreenFn gContainerDtorOriginal = nullptr;
ScreenViewRenderFn gScreenViewRenderOriginal = nullptr;
ItemRendererRenderGuiItemNewFn gItemRendererOriginal = nullptr;
DrawTextFn gDrawTextOriginal = nullptr;
ContainerGetItemStackFn gContainerGetItemStack = nullptr;
ItemStackBaseGetDamageValueFn gItemDamage = nullptr;
FillRectangleFn gFillRectangle = nullptr;

void* gController = nullptr;
void* gUiContext = nullptr;
std::array<SlotState, kInventorySlots> gSlots{};
bool gBaselineReady = false;
bool gEnabled = false;
std::vector<Marker> gMarkers;
std::vector<std::unique_ptr<pl::memory::HookHandle>> gHooks;

std::uint16_t readItemId(void* stack) {
    if (!stack) return 0;
    auto* stackBytes = static_cast<std::byte*>(stack);
    void* counter = *reinterpret_cast<void**>(stackBytes + kItemStackItemOffset);
    if (!counter) return 0;
    void* item = *reinterpret_cast<void**>(static_cast<std::byte*>(counter) + kSharedCounterPointerOffset);
    if (!item) return 0;
    return *reinterpret_cast<std::uint16_t*>(static_cast<std::byte*>(item) + kItemIdOffset);
}

short readDamage(void* stack) {
    if (!stack || !gItemDamage) return 0;
    return gItemDamage(stack);
}

bool validStack(void* stack) {
    return stack != nullptr && readItemId(stack) != 0;
}

void clearInventoryState() {
    gController = nullptr;
    gBaselineReady = false;
    gSlots.fill({});
    gMarkers.clear();
}

void purgeMarkers() {
    const auto now = std::chrono::steady_clock::now();
    gMarkers.erase(
        std::remove_if(gMarkers.begin(), gMarkers.end(), [&](const Marker& marker) {
            return marker.stack == nullptr || marker.expires <= now;
        }),
        gMarkers.end());
}

void markStack(void* stack) {
    if (!validStack(stack)) return;
    const auto now = std::chrono::steady_clock::now();
    for (auto& marker : gMarkers) {
        if (marker.stack == stack) {
            marker.expires = now + std::chrono::milliseconds(static_cast<int>(kHighlightDurationMs));
            return;
        }
    }
    gMarkers.push_back(Marker{stack, 0.0f, 0.0f, now + std::chrono::milliseconds(static_cast<int>(kHighlightDurationMs))});
}

void pollInventory() {
    if (!gController || !gContainerGetItemStack) return;

    std::array<SlotState, kInventorySlots> current{};
    for (std::size_t i = 0; i < kInventorySlots; ++i) {
        void* stack = gContainerGetItemStack(gController, "inventory_items", static_cast<int>(i));
        current[i].itemId = readItemId(stack);
        current[i].damage = readDamage(stack);
        current[i].valid = validStack(stack);

        if (gBaselineReady) {
            const bool changed = (!gSlots[i].valid && current[i].valid) ||
                                 (gSlots[i].valid && !current[i].valid) ||
                                 (gSlots[i].valid && current[i].valid &&
                                  (gSlots[i].itemId != current[i].itemId || gSlots[i].damage != current[i].damage));
            if (changed && current[i].valid) {
                markStack(stack);
            }
        }
    }

    gSlots = current;
    gBaselineReady = true;
    purgeMarkers();
}

bool isMarked(void* stack) {
    const auto now = std::chrono::steady_clock::now();
    for (const auto& marker : gMarkers) {
        if (marker.stack == stack && marker.expires > now) return true;
    }
    return false;
}

void rememberRender(void* stack, float x, float y) {
    if (!stack || !isMarked(stack)) return;
    for (auto& marker : gMarkers) {
        if (marker.stack == stack) {
            marker.x = x;
            marker.y = y;
            break;
        }
    }
}

void drawStar(void* context, float x, float y) {
    if (!context || !gFillRectangle) return;
    const float phase = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
    const float pulse = 0.70f + 0.30f * (0.5f + 0.5f * std::sin(phase * 0.010f));
    const Color color{1.0f, 0.86f, 0.20f, pulse};

    // Small four-point star near the upper-right corner of the item.
    const float cx = x + 13.0f;
    const float cy = y + 2.0f;
    gFillRectangle(context, {cx - 1.0f, cx + 1.0f, cy - 4.0f, cy + 4.0f}, color, 1.0f);
    gFillRectangle(context, {cx - 4.0f, cx + 4.0f, cy - 1.0f, cy + 1.0f}, color, 1.0f);
    gFillRectangle(context, {cx - 0.75f, cx + 0.75f, cy - 6.0f, cy - 3.0f}, color, 1.0f);
    gFillRectangle(context, {cx + 3.0f, cx + 4.5f, cy - 0.75f, cy + 0.75f}, color, 1.0f);
}

void renderMarkers() {
    if (!gUiContext) return;
    purgeMarkers();
    for (const auto& marker : gMarkers) {
        if (marker.stack && marker.x != 0.0f && marker.y != 0.0f && marker.expires > std::chrono::steady_clock::now()) {
            drawStar(gUiContext, marker.x, marker.y);
        }
    }
}

void* containerOpenHook(void* a0, void* a1, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7) {
    void* result = gContainerOpenOriginal ? gContainerOpenOriginal(a0, a1, a2, a3, a4, a5, a6, a7) : nullptr;
    if (!gEnabled) return result;
    gController = a0;
    gBaselineReady = false;
    gSlots.fill({});
    return result;
}

void* containerDtorHook(void* a0, void* a1, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7) {
    void* result = gContainerDtorOriginal ? gContainerDtorOriginal(a0, a1, a2, a3, a4, a5, a6, a7) : nullptr;
    if (gEnabled && gController == a0) clearInventoryState();
    return result;
}

void drawTextHook(void* self, Font& font, const RectangleArea& rectangle, const std::string& text,
                  const Color& color, TextAlignment alignment, float alpha,
                  const TextMeasureData& measure, const CaretMeasureData& caret) {
    if (gEnabled) gUiContext = self;
    if (gDrawTextOriginal) {
        gDrawTextOriginal(self, font, rectangle, text, color, alignment, alpha, measure, caret);
    }
}

void screenViewRenderHook(void* self, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7, void* a8) {
    if (!gEnabled) {
        if (gScreenViewRenderOriginal) gScreenViewRenderOriginal(self, a2, a3, a4, a5, a6, a7, a8);
        return;
    }
    gUiContext = nullptr;
    pollInventory();
    if (gScreenViewRenderOriginal) {
        gScreenViewRenderOriginal(self, a2, a3, a4, a5, a6, a7, a8);
    }
    renderMarkers();
}

std::uint64_t itemRendererHook(void* itemRenderer, void* baseActorRenderContext, void* stack,
                               unsigned int aux, unsigned char mode, std::uint64_t a6,
                               float x, float y, float sx, float sy, float sz) {
    std::uint64_t result = gItemRendererOriginal
        ? gItemRendererOriginal(itemRenderer, baseActorRenderContext, stack, aux, mode, a6, x, y, sx, sy, sz)
        : 0;
    if (gEnabled) rememberRender(stack, x, y);
    return result;
}

bool installHook(std::string_view signature, void* detour, void** original, const char* label, pl::log::Logger& logger) {
    const auto address = pl::memory::resolveSignature(signature, kMinecraftLibrary);
    if (!address) {
        logger.warn("{} signature not found", label);
        return false;
    }
    auto hook = std::make_unique<pl::memory::HookHandle>(reinterpret_cast<void*>(address), detour, original);
    if (!hook->installed()) {
        logger.warn("{} hook installation failed", label);
        return false;
    }
    gHooks.push_back(std::move(hook));
    return true;
}


} // namespace

class ItemHighlighterMod {
public:
    static ItemHighlighterMod& instance() {
        static ItemHighlighterMod mod;
        return mod;
    }

    bool load() {
        auto& logger = getLogger();
        logger.info("Loading Item Highlighter...");
        return true;
    }

    bool enable() {
        auto& logger = getLogger();
        if (gEnabled) return true;
        gEnabled = true;
        logger.info("Enabling Item Highlighter");

        if (!gContainerGetItemStack) {
            gContainerGetItemStack = reinterpret_cast<ContainerGetItemStackFn>(
                pl::memory::resolveSignature(kSigContainerGetItemStack, kMinecraftLibrary));
        }
        if (!gItemDamage) {
            gItemDamage = reinterpret_cast<ItemStackBaseGetDamageValueFn>(
                pl::memory::resolveSignature(kSigItemDamage, kMinecraftLibrary));
        }

        installHook(kSigContainerOpen, reinterpret_cast<void*>(containerOpenHook), reinterpret_cast<void**>(&gContainerOpenOriginal), "ContainerScreenControllerOpen", logger);
        installHook(kSigContainerDtor, reinterpret_cast<void*>(containerDtorHook), reinterpret_cast<void**>(&gContainerDtorOriginal), "ContainerScreenControllerDtor", logger);
        installHook(kSigScreenViewRender, reinterpret_cast<void*>(screenViewRenderHook), reinterpret_cast<void**>(&gScreenViewRenderOriginal), "ScreenViewRender", logger);
        installHook(kSigItemRendererGui, reinterpret_cast<void*>(itemRendererHook), reinterpret_cast<void**>(&gItemRendererOriginal), "ItemRendererRenderGuiItemNew", logger);

        const auto drawTextAddress = pl::memory::resolveVtableFunction(
            kUiContextTypeInfo, 6, kMinecraftLibrary);
        if (drawTextAddress) {
            auto hook = std::make_unique<pl::memory::HookHandle>(
                reinterpret_cast<void*>(drawTextAddress),
                reinterpret_cast<void*>(drawTextHook),
                reinterpret_cast<void**>(&gDrawTextOriginal));
            if (hook->installed()) gHooks.push_back(std::move(hook));
            else logger.warn("MinecraftUIRenderContext::DrawText hook installation failed");
        } else {
            logger.warn("MinecraftUIRenderContext::DrawText vtable slot could not be resolved");
        }

        const auto fillRectAddress = pl::memory::resolveVtableFunction(
            kUiContextTypeInfo, kUiFillRectangleVtableSlot, kMinecraftLibrary);
        if (fillRectAddress) {
            gFillRectangle = reinterpret_cast<FillRectangleFn>(fillRectAddress);
        } else {
            logger.warn("MinecraftUIRenderContext::FillRectangle vtable slot could not be resolved");
        }

        logger.info("Item Highlighter enabled; missing signatures are treated as optional for crash safety");
        return true;
    }

    bool disable() {
        gEnabled = false;
        clearInventoryState();
        gUiContext = nullptr;
        return true;
    }

    bool unload() {
        gEnabled = false;
        gHooks.clear();
        clearInventoryState();
        gUiContext = nullptr;
        gContainerGetItemStack = nullptr;
        gItemDamage = nullptr;
        gFillRectangle = nullptr;
        return true;
    }

private:
    pl::log::Logger& getLogger() {
        static pl::log::Logger& logger = pl::log::Logger::getOrCreate("Item Highlighter");
        return logger;
    }
};

} // namespace item_highlighter

PL_REGISTER_MOD(item_highlighter::ItemHighlighterMod, item_highlighter::ItemHighlighterMod::instance());
