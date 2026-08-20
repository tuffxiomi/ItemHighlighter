# BedrockTools research used by Item Highlighter

This document records the source-level audit performed on the supplied BedrockTools archive. The new Item Highlighter project does not include BedrockTools files.

## Signature system

- Signature IDs: **104** (`Count` excluded).
- Resolution model: masked AArch64 byte-pattern signatures resolved against `libminecraftpe.so`, then used as native hook/call addresses.

### All signature IDs

000. `VersionString`
001. `Nametag`
002. `Fullbright`
003. `SetupFogPlayer`
004. `RaknetUpdate`
005. `NormalTick`
006. `Time`
007. `SetTime`
008. `EduMultiplayer`
009. `HudCursor`
010. `LevelInit`
011. `LevelDtor`
012. `ActorManagerList`
013. `DimensionTick`
014. `WeatherTick`
015. `WeatherGetRainLevel`
016. `WeatherGetLightningLevel`
017. `WeatherIsRaining`
018. `WeatherIsLightning`
019. `ActorShaderManagerSetEntityConstants`
020. `ActorShaderManagerSetupShaderParametersActorGlint`
021. `ActorShaderManagerSetupFoilShaderParameters`
022. `ActorShaderManagerSetupShaderParametersGlint`
023. `RenderItem`
024. `GetFov`
025. `GetPerspective`
026. `ClientInstanceUpdate`
027. `ClientInstanceGetLocalPlayer`
028. `ContainerScreenControllerDtor`
029. `ContainerScreenControllerOpen`
030. `ChatScreenDtor`
031. `ChatScreenOpen`
032. `BiomeGetTemperature`
033. `GetDestroyProgress`
034. `RenderLevel`
035. `TessellatorBegin`
036. `TessellatorColor`
037. `TessellatorVertex`
038. `MeshHelpersRenderMeshImmediately`
039. `MeshHelpersRenderMeshImmediately2`
040. `RenderMaterialGroupCommon`
041. `SurvivalModeAttack`
042. `GameModeAttack`
043. `LevelGetHitResult`
044. `BlockSourceGetBiome`
045. `BlockSourceGetBlock`
046. `BlockSourceGetBrightness`
047. `BlockSourceIsSolidBlockingBlock`
048. `LocalPlayerApplyTurnDelta`
049. `BaseOptionRegistryGetHideItemInHand`
050. `HitResultGetEntity`
051. `ActorIsPlayer`
052. `ActorIsInvisible`
053. `ActorFetchNearbyActorsSorted`
054. `ActorGetNameTag`
055. `ActorSetNameTag`
056. `SynchedActorDataEnsureIndex`
057. `ActorSynchedDataUpdateAlwaysShowNameTag`
058. `PrimedTntNormalTick`
059. `MinecraftUIRenderContextDrawText`
060. `ScreenViewRender`
061. `ContainerScreenControllerOnContainerSlotSelected`
062. `ContainerScreenControllerGetItemStack`
063. `ClientNetworkHandlerHandleSetTitle`
064. `ClientNetworkHandlerHandleText`
065. `LoopbackPacketSenderSendToServer`
066. `ClientInstanceGetPacketSender`
067. `MinecraftPacketsCreatePacket`
068. `LocalPlayerChangeDimension`
069. `NbtTreeFind`
070. `ItemStackBaseLoadItem`
071. `ItemStackBaseGetDamageValue`
072. `BaseActorRenderContextCtor`
073. `ItemRendererRenderGuiItemNew`
074. `ControlOptionEditorTick`
075. `ControlOptionEditorRender`
076. `BlockTessellatorTessellateFaceDown`
077. `BlockTessellatorTessellateFaceUp`
078. `BlockTessellatorTessellateFaceNorth`
079. `BlockTessellatorTessellateFaceSouth`
080. `BlockTessellatorTessellateFaceWest`
081. `BlockTessellatorTessellateFaceEast`
082. `BlockTessellatorTessellatePane`
083. `BlockSourceGetBlockForTessellation`
084. `TextureUVCoordinateSetCopyCtor`
085. `TextureUVCoordinateSetDtor`
086. `RenderChunkCoordinatorSetAllDirty`
087. `GuiDataDisplayAnnouncementMessage`
088. `GuiDataDisplayChatMessage`
089. `GuiDataDisplayClientMessage`
090. `GuiDataDisplayDevConsoleMessage`
091. `GuiDataDisplayLocalizableMessage`
092. `GuiDataDisplayLocalizedMessage`
093. `GuiDataDisplaySystemMessage`
094. `GuiDataDisplayTextObjectMessage`
095. `GuiDataDisplayTextObjectWhisperMessageText`
096. `GuiDataDisplayTextObjectWhisperMessageObject`
097. `GuiDataDisplayWhisperMessage`
098. `GuiDataAddMessage`
099. `ResourcePacksInfoPacketHandle`
100. `ResourcePackStackPacketHandle`
101. `BlockTessellatorTessellateDoubleThinFenceInWorld`
102. `BlockGraphicsGetTexture`
103. `BlockOccluderUpdateRenderFace`

### Signatures used by this mod

- `ContainerScreenControllerOpen` — `? ? ? A9 ? ? ? F9 FD 03 00 91 F3 03 00 AA ? ? ? 94 ? ? ? F9 E1 03 1F 2A ? ? ? 94`
- `ContainerScreenControllerDtor` — `? ? ? D1 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? 91 56 D0 3B D5 F3 03 00 AA ? ? ? F9 ? ? ? F9 ? ? ? 90 ? ? ? 91 ? ? ? F9 ? ? ? F9 ? ? ? 91 ? ? ? F9 ? ? ? 94`
- `ContainerScreenControllerGetItemStack` — `? ? ? D1 ? ? ? A9 ? ? ? A9 ? ? ? 91 54 D0 3B D5 F3 03 00 AA ? ? ? 91 ? ? ? F9 ? ? ? F8 ? ? ? 95 ? ? ? F9 ? ? ? F9 ? ? ? 91`
- `ScreenViewRender` — `? ? ? FC ? ? ? 6D ? ? ? 6D ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? 91 ? ? ? D1 48 D0 3B D5 FC 03 00 AA`
- `ItemRendererRenderGuiItemNew` — `? ? ? D1 ? ? ? FD ? ? ? 6D ? ? ? 6D ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? 91 5B D0 3B D5 F4 03 00 AA`
- `MinecraftUIRenderContextDrawText` — `? ? ? D1 ? ? ? A9 ? ? ? F9 ? ? ? 91 ? ? ? A9 E8 03 05 2A`
- `ItemStackBaseGetDamageValue` — `? ? ? D1 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? 91 55 D0 3B D5 ? ? ? F9 ? ? ? F8 ? ? ? F9 ? ? ? B4 ? ? ? F9 ? ? ? B4 E8 03 00 AA`
- `ContainerScreenControllerOnContainerSlotSelected` — audited but intentionally **not used** by the standalone mod because the supplied fixture had no exact match.

## Vtable and field offsets

The source contains the following offset definitions. The highlighter only depends on the small subset listed after the full inventory.

- `Core::ClientInstance_getRegion` = `31`
- `Core::BlockSource_getDimensionId` = `18`
- `Core::RenderMaterialGroup_getMaterial` = `2`
- `Core::HoverTextRendererRenderHoverBox` = `17`
- `Core::MinecraftUIRenderContextGetLineLength` = `2`
- `Core::MinecraftUIRenderContextDrawText` = `6`
- `Core::MinecraftUIRenderContextFlushText` = `7`
- `Core::MinecraftUIRenderContextDrawImage` = `8`
- `Core::MinecraftUIRenderContextFlushImages` = `10`
- `Core::MinecraftUIRenderContextFillRectangle` = `16`
- `Core::MinecraftUIRenderContextGetTexture` = `32`
- `Core::ClientInstanceGetMinecraftGame` = `83`
- `Core::ClientInstanceGetLocalPlayer` = `32`
- `Core::ItemGetMaxDamage` = `37`
- `Core::ItemGetAnimationFrameFor` = `120`
- `Core::mLevelRenderer` = `0x190`
- `Network::mAvgPing` = `0x108`
- `Network::Size` = `0x30`
- `Network::mResourcePackRequired` = `0x30`
- `Network::mForceDisableVibrantVisuals` = `0x33`
- `Network::mResourcePackRequired` = `0x68`
- `Network::mBody` = `0x58`
- `Network::mVariantIndex` = `0x90`
- `Network::mType` = `0x58`
- `Network::mAuthor` = `0x60`
- `Network::mMessage` = `0x78`
- `Network::mType` = `0x58`
- `Network::mMessage` = `0x60`
- `Network::mCommand` = `0`
- `Network::mOrigin` = `24`
- `Network::mInternalSource` = `84`
- `Network::mType` = `0`
- `Network::mDimensionId` = `0`
- `Network::mType` = `0`
- `Network::mTitleText` = `8`
- `Render::mFogColorRed` = `0x424`
- `Render::mFogColorGreen` = `0x428`
- `Render::mFogColorBlue` = `0x42C`
- `Render::mBaseFogStart` = `0x434`
- `Render::mBaseFogEnd` = `0x438`
- `Render::mCurrentFogDensityMax` = `0x464`
- `Render::mCamPos` = `0x61C`
- `Render::mSelectionOverlayMaterial` = `0x1030`
- `Render::mActorShaderConstants` = `0x20`
- `Render::mColorHolder` = `0x30`
- `Render::mTessellator` = `0xB8`
- `Render::mGlintColor` = `0xF8`
- `Render::mDirty` = `0x29`
- `Render::mData` = `0x30`
- `Render::mRenderMaterialGroupOffset` = `32`
- `Render::mMatrixStackWrapper` = `0x28`
- `Render::mMatrixStack` = `0x18`
- `Render::mBlocks` = `0x50`
- `Render::mStart` = `0x68`
- `Render::mSize` = `0x70`
- `Render::mExtractNameTagsPatchOffset` = `0x1A0`
- `Render::mRegion` = `0x8`
- `Render::mInternalTexture` = `0x18`
- `Render::mUseInternalTexture` = `0x71`
- `Render::mXFlipTexture` = `0x74`
- `Render::mFlipFace` = `0x174`
- `Render::mTextureOverride` = `0x17C`
- `Render::mCurrentShapeBB` = `0x5F0`
- `Render::mTextureU` = `0x17C`
- `Render::mTextureV` = `0x180`
- `Render::mBlockType` = `0x68`
- `Render::mNameInfo` = `0x88`
- `Render::mFullName` = `0x40`
- `Render::mString` = `0x8`
- `Render::mU0` = `0x4`
- `Render::mV0` = `0x8`
- `Render::mU1` = `0xC`
- `Render::mV1` = `0x10`
- `Render::mSizeW` = `0x14`
- `Render::mSizeH` = `0x16`
- `Render::mIsotropicFaceData` = `0x50`
- `Render::Size` = `0x58`
- `Render::Alignment` = `0x8`
- `Render::mTextureIsotropic` = `0x0`
- `Render::ElementSize` = `0x1`
- `Render::mRenderChunkCoordinators` = `0x28`
- `Render::mLevelRendererPlayer` = `0x420`
- `Render::mFirstNode` = `0x10`
- `Render::mNext` = `0x0`
- `Render::mValuePointer` = `0x18`
- `Render::MaxNodes` = `64`
- `Skin::mBytesOffset` = `0x18`
- `Skin::mSkinImpl` = `0`
- `Skin::mObject` = `0`
- `Skin::mSkinImage` = `120`
- `Skin::mIsPersona` = `442`
- `Skin::mSkinAnimatedImages` = `216`
- `Skin::mType` = `0`
- `Skin::mImage` = `8`
- `Skin::Size` = `64`
- `Skin::mWidth` = `4`
- `Skin::mHeight` = `8`
- `UI::mReservedAreas` = `0x120`
- `UI::ReservedAreaEntrySize` = `0x30`
- `UI::Type` = `0x0`
- `UI::FullString` = `0x70`
- `UI::FilteredFullString` = `0x88`
- `UI::FilteredFullStringPresent` = `0xA0`
- `UI::ItemStackBaseItem` = `0x8`
- `UI::ItemStackBaseUserData` = `0x10`
- `UI::SharedCounterPointer` = `0x0`
- `UI::ItemId` = `0x8A`
- `UI::CompoundTagTreeRoot` = `0x8`
- `UI::CompoundTagTreeEnd` = `0x10`
- `UI::NbtNodePayload` = `0x38`
- `UI::NbtNodeNumericValue` = `0x40`
- `UI::NbtNodeType` = `0x60`
- `UI::HoverRendererCursorX` = `0x40`
- `UI::HoverRendererCursorY` = `0x44`
- `UI::MinecraftUIRenderContextClient` = `0x8`
- `UI::MinecraftUIRenderContextScreenContext` = `0x10`
- `UI::ClientInstanceMinecraftGame` = `0xA8`
- `UI::BaseActorRenderContextItemRenderer` = `0x58`
- `UI::BaseActorRenderContextStorageSize` = `0x400`
- `UI::ItemStackStorageSize` = `0x800`
- `World::mName` = `2824`
- `World::mSkin` = `2552`
- `World::mEntityContext` = `0x8`
- `World::mEntityData` = `0x120`
- `World::mStateVectorComponent` = `0x208`
- `World::mActorRotationComponent` = `0x218`
- `World::mLevel` = `464`
- `World::mDimension` = `448`
- `World::mHurtTime` = `0x194`
- `World::mCategories` = `512`
- `World::mNameTagHash` = `384`
- `World::mFilteredNameTag` = `712`
- `World::FuseTime` = `55`
- `World::NametagAlwaysShow` = `81`
- `World::mType` = `0x8`
- `World::mId` = `0xA`
- `World::mValue` = `0xC`
- `World::mAABBShapeComponent` = `8`
- `World::mAABB` = `0`
- `World::mActorManager` = `0x470`
- `World::mHitResultWrapper` = `456`
- `World::mStartPos` = `0`
- `World::mType` = `24`
- `World::mPos` = `44`
- `World::mHitResult` = `0`
- `World::mBlockSource` = `208`
- `World::mWeather` = `0x1B8`
- `World::mHash` = `400`
- `World::mOldRainLevel` = `0x34`
- `World::mRainLevel` = `0x38`
- `World::mTargetRainLevel` = `0x3C`
- `World::mOldLightningLevel` = `0x40`
- `World::mLightningLevel` = `0x44`
- `World::mTargetLightningLevel` = `0x48`

## Relevant offsets for Item Highlighter

- `Core::VTable::MinecraftUIRenderContextDrawText` = `6`
- `Core::VTable::MinecraftUIRenderContextFillRectangle` = `16`
- `UI::ShulkerPreview::ItemStackBaseItem` = `0x8`
- `UI::ShulkerPreview::SharedCounterPointer` = `0x0`
- `UI::ShulkerPreview::ItemId` = `0x8A`

## Modules checked

The archive contains **37 implementation module files** plus `ModuleRegistry.cpp`:

- `hud/breakindicator`
- `hud/combocounter`
- `hud/compass`
- `hud/debugmenu`
- `hud/keystrokes`
- `hud/pingcounter`
- `hud/playercoords`
- `hud/reachcounter`
- `hud/speeddisplay`
- `hud/tablist`
- `misc/chattimestamps`
- `misc/cpslimiter`
- `misc/forceglobalrp`
- `misc/nodisconnect`
- `misc/notouchborder`
- `player/autogg`
- `player/autoreq`
- `player/nick`
- `player/skinstealer`
- `player/timechanger`
- `player/weatherchanger`
- `visual/breadcrumbs`
- `visual/chunkborder`
- `visual/connectedglass`
- `visual/fogcolor`
- `visual/fpsunlocker`
- `visual/fullbright`
- `visual/glintcolor`
- `visual/hitbox`
- `visual/lightoverlay`
- `visual/motionblur`
- `visual/nofog`
- `visual/shulkerpreview`
- `visual/thirdpersonnametag`
- `visual/tnttimer`
- `visual/viewmodel`
- `visual/zoom`

## Compatibility finding

- The prior binary audit of the supplied `libminecraftpe.so` found exact matches for `ContainerScreenControllerGetItemStack`, `ScreenViewRender`, and `MinecraftUIRenderContextDrawText`, while `ContainerScreenControllerOnContainerSlotSelected` and `ClientInstanceUpdate` did not have exact matches in that fixture. The new highlighter therefore treats optional hook failures as non-fatal and does not depend on `ClientInstanceUpdate`.
- The supplied binary was AArch64 and the source audit recorded a `.text` start around `0x6121100`; it is not bundled with this project.
