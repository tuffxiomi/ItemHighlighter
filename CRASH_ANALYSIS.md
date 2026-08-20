# Item Highlighter Functional Fix

## Problem

The previous v2 implementation hooked:

- ItemRendererRenderGuiItemNew
- MinecraftUIRenderContext::DrawText

However, it did not hook ScreenViewRender.

The mod attempted to draw markers directly from ItemRendererRenderGuiItemNew.

At that point there was no guaranteed active MinecraftUIRenderContext, because the UI context was only captured when DrawText happened to execute.

As a result, the marker renderer could receive:

`nullptr`

and silently do nothing.

This explains why the mod could load successfully but show no visible Item Highlighter marker.

## Fix

The implementation now follows the rendering structure used by the supplied BedrockTools ShulkerPreview module.

### ScreenViewRender

The mod hooks ScreenViewRender and creates a clear UI render boundary.

Before the original render:

`gActiveUiContext = nullptr`

During the original render:

`DrawText` captures the active UI context.

After the original render:

`drawAllMarkers(gActiveUiContext)`

This means the marker is drawn after the normal UI items have been rendered.

## Item rendering

ItemRendererRenderGuiItemNew is now used only for observation.

It records:

- item ID
- auxiliary value
- GUI X position
- GUI Y position

The marker is not drawn from inside the item-renderer hook.

## Stability

The following dependencies were removed:

- ContainerScreenControllerOpen
- ContainerScreenControllerDtor
- inventory polling

Only the required GUI rendering signatures are resolved.

No BedrockTools files are bundled.
