# Item Highlighter

Standalone Levi Launchroid / Preloader Android visual mod inspired by the Modrinth **Item Highlighter** concept.

## What it does

- Watches the player inventory while a container/inventory screen is open.
- Establishes a baseline when the screen opens so old items are not highlighted immediately.
- Detects inventory slot changes during the open screen.
- Newly detected item stacks receive a small animated gold star for about 1.6 seconds.
- Rendering is client-side only. It does not edit inventory data, send packets, or change gameplay state.

The detection method is intentionally conservative: it observes normal inventory-slot changes rather than trying to patch a network pickup packet. A stack count increase inside the same unchanged `ItemStackBase` object is not guaranteed to be detected.

## BedrockTools research used

This project does **not** include BedrockTools source files, headers, `libminecraftpe.so`, or its module framework.

The implementation independently uses the same native concepts observed in the supplied BedrockTools source:

- masked ARM64 signature resolution
- Preloader detours
- `ContainerScreenControllerOpen` / destructor lifecycle
- `ContainerScreenControllerGetItemStack`
- `ScreenViewRender`
- `ItemRendererRenderGuiItemNew`
- `MinecraftUIRenderContext::DrawText` vtable slot 6
- `MinecraftUIRenderContext::FillRectangle` vtable slot 16

Relevant object offsets reused from the audit/source analysis:

- `ItemStackBase` item pointer: `0x8`
- shared-counter item pointer: `0x0`
- `Item::mId`: `0x8A`

The original BedrockTools audit found 104 signature IDs and 37 module implementation units. The new mod intentionally reimplements only the tiny subset needed for this feature.

## Build

GitHub Actions builds with Android NDK 28 and CMake. The supplied reverse-engineering fixture is AArch64, so the workflow targets `arm64-v8a`.

Local build prerequisites:

- Android NDK 28.x
- CMake 3.22+
- Ninja

The GitHub workflow also packages a `.levipack` artifact.
