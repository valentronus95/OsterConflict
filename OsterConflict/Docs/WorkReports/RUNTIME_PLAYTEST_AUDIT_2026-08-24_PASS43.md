# Runtime playtest audit — 2026-08-24 — Pass 43

Status: **CODED_UNTESTED**. The user-provided UE 5.8 crash is authoritative; source/CI alone cannot mark the crash fixed.

## Reported runtime failure

Normal game was launched from `START_HERE.cmd` after Pass 42 was merged to `main`.

Crash Reporter assertion:

`Assertion failed: Texture [RenderCore/Private/RenderTargetPool.cpp] [Line: 95]`

The visible call stack is dominated by `UnrealEditor_RenderCore` and repeated `UnrealEditor_SlateRHIRenderer` frames.

The attached launch transcript proves the pre-launch editor build succeeded and the isolated required-weapon NullRHI preflight opened all 11 required weapon visuals successfully before the real normal-game frontend process started. The failure therefore belongs to the real frontend renderer lifecycle, not to UBT compilation or the weapon preflight commandlet.

## Source localization

Two independent early render-target hazards were active in current `main`:

1. `UOCGameUIRootWidget::NativeConstruct()` immediately calls `SyncSettingsWidgetsFromBackend()`, which reaches `UOCPlayerUserSettings::Get()` and `EnsureInitialGraphicsProfile()`. Pass 42 automatic migration then called `UGameUserSettings::ApplySettings(false)` while Slate was still constructing the frontend widget tree. A live settings apply may recreate the viewport/backbuffer under `SlateRHIRenderer`.
2. `UOCMinimapSubsystem::Tick()` called `EnsureMinimap()` before rejecting frontend/deployment/no-Pawn state. `EnsureMinimap()` calls `UOCTacticalMapSubsystem::EnsureMapSnapshot()`, which allocates a 1600×900 `UTextureRenderTarget2D`, performs a SceneCapture, and then publishes that render target as a Slate `UImage` brush. This hidden render-target path was therefore active before actual gameplay.

Both paths directly overlap the reported `RenderTargetPool` + `SlateRHIRenderer` failure class and should not exist during frontend construction.

## Pass 43 correction

- Automatic first-run / Pass 39 / Pass 42 graphics migrations still calculate and persist the intended settings but no longer call live `ApplySettings()` from `EnsureInitialGraphicsProfile()`.
- Explicit user action in the Settings UI remains the live `ApplySettings(false)` path, after the viewport exists.
- Minimap render-target creation is rejected while there is no gameplay Pawn or while frontend, deployment, settings, admin, chat or tactical-map UI blocks gameplay.
- Existing minimap widget is only collapsed while blocked; its SceneCapture/render target is not recreated.
- The first permitted minimap render target is now created only in stable unblocked gameplay.
- Pass 23 DX11 + SM5 + no-HDR + no-RHI-thread startup isolation remains unchanged.

Runtime evidence markers added:

- `PASS43_STARTUP_GRAPHICS_PERSIST_ONLY_READY`
- `PASS43_MINIMAP_RENDER_TARGET_GAMEPLAY_ONLY_READY`

## Acceptance

Source verification must prove:

- no `ApplySettings(` exists inside `EnsureInitialGraphicsProfile()`;
- manual Settings UI still owns live `ApplySettings(false)` + `ConfirmVideoMode()`;
- minimap blocked/no-Pawn checks occur before `EnsureMapSnapshot()` and before `EnsureMinimap()` in Tick;
- tactical-map scene capture remains one-shot;
- Pass 23 safe renderer flags remain intact.

Final status remains **CODED_UNTESTED** until the next local UE 5.8 normal-game launch confirms the `RenderTargetPool.cpp:95` crash does not recur.
