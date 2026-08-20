# Item Highlighter

Standalone Levi Launchroid / Preloader Android visual mod inspired by the Java Edition Item Highlighter mod.

## Behavior

Item Highlighter shows a small animated gold star over newly detected items in the Minecraft GUI.

It is designed to work with:

- Hotbar
- Player inventory
- Container inventories
- Other GUI item-rendering locations

The mod uses the native `ItemRendererRenderGuiItemNew` path to observe rendered items.

The marker itself is rendered after the complete `ScreenViewRender` pass using the active `MinecraftUIRenderContext`.

## How it works

The implementation uses the following native paths:

- `ItemRendererRenderGuiItemNew`
- `ScreenViewRender`
- `MinecraftUIRenderContext::DrawText`
- `MinecraftUIRenderContext::FillRectangle`

The supplied BedrockTools source uses the same rendering-context technique:

1. `ScreenViewRender` defines the UI rendering pass.
2. `DrawText` provides the active `MinecraftUIRenderContext`.
3. `FillRectangle` is used to draw UI rectangles.

This project independently implements only the required functionality and does not include BedrockTools files.

## Highlight detection

The mod establishes an initial baseline so existing hotbar/inventory items are not immediately highlighted.

After the baseline:

- An empty slot becoming an item can be highlighted.
- An item changing to another item can be highlighted.
- An item auxiliary value changing can be highlighted.

The implementation intentionally does not rely on the `ItemStack` pointer because temporary ItemStack objects may be used during GUI rendering.

## Limitations

A pickup that only changes the count of an existing stack while keeping the same item ID and auxiliary value may not be detected by this minimal renderer-based implementation.

Detecting every stack-count change reliably would require an additional inventory-state hook or a verified ItemStack count field for the exact Minecraft build.

## Stability

The mod does not perform signature scanning from `enable()`.

Native targets are resolved during `load()`.

The mod also does not depend on:

- `ContainerScreenControllerOpen`
- `ContainerScreenControllerDtor`
- inventory-wide polling

This avoids the expensive main-thread work that caused the previous ANR.

## Compatibility

Target ABI:

`arm64-v8a`

Target library:

`libminecraftpe.so`

No BedrockTools source or `libminecraftpe.so` is bundled with this project.
