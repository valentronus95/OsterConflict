from pathlib import Path

ROOT = Path(__file__).resolve().parent
MENU = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCR13FrontendMenuSubsystem.cpp"
GUARD_H = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCR13FrontendShellGuardSubsystem.h"
GUARD_CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCR13FrontendShellGuardSubsystem.cpp"
GAME_MODE_H = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCGameMode.h"
GAME_MODE_CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCGameMode.cpp"
BACKGROUND = ROOT / "OsterConflict" / "Content" / "R13" / "UI" / "Oster_Menu_BG.uasset"


def fail(message: str) -> None:
    raise SystemExit(f"R13 FRONTEND MENU GUARD VERIFY FAIL: {message}")


for path in (MENU, GUARD_H, GUARD_CPP, GAME_MODE_H, GAME_MODE_CPP, BACKGROUND):
    if not path.is_file():
        fail(f"missing required file: {path.relative_to(ROOT)}")

menu = MENU.read_text(encoding="utf-8")
guard_h = GUARD_H.read_text(encoding="utf-8")
guard = GUARD_CPP.read_text(encoding="utf-8")
game_mode_h = GAME_MODE_H.read_text(encoding="utf-8")
game_mode = GAME_MODE_CPP.read_text(encoding="utf-8")

MENU_REQUIRED = [
    '/Game/R13/UI/Oster_Menu_BG.Oster_Menu_BG',
    'Background->SetColorAndOpacity(FLinearColor::White)',
    'Shade->SetVisibility(ESlateVisibility::Collapsed)',
    'const ESlateVisibility GradientVisibility = (bShowMenu && bShowBackdrop)',
    'SetPresentationVisibility(true, true, false)',
    'SetPresentationVisibility(true, false, true)',
    'PanelSlot->SetPosition(FVector2D(112.0f, 92.0f))',
    'PanelSlot->SetSize(FVector2D(440.0f, 760.0f))',
    'SuppressLegacyFrontendLayers(Root)',
    'const int32 ZOrder = Slot->GetZOrder()',
    'if (ZOrder == -100 || ZOrder == -99)',
    'LegacyFrontend->RemoveFromParent()',
    '"СТАРТ"',
    '"ЛОКАЛЬНА ГРА"',
    '"МЕРЕЖЕВА ГРА"',
]
for token in MENU_REQUIRED:
    if token not in menu:
        fail(f"missing approved frontend token: {token}")

background_tint_calls = menu.count('Background->SetColorAndOpacity(')
if background_tint_calls != 1:
    fail(f"expected exactly one background tint call, found {background_tint_calls}")
if menu.count('Background->SetColorAndOpacity(FLinearColor::White)') != 1:
    fail("the only menu-background tint must be neutral white")

if 'IsFrontendOnlySession() const { return bFrontendOnlySession; }' not in game_mode_h:
    fail("GameMode must expose read-only frontend-only session state")

GAME_MODE_REQUIRED = [
    'bFrontendOnlySession = (GetNetMode() == NM_Standalone)',
    'if (bFrontendOnlySession)',
    'Frontend-only standalone session: gameplay world, bots and match timers are suppressed.',
]
for token in GAME_MODE_REQUIRED:
    if token not in game_mode:
        fail(f"missing frontend-only GameMode guard: {token}")

GUARD_REQUIRED = [
    'public UTickableWorldSubsystem',
    'virtual void OnWorldBeginPlay(UWorld& InWorld) override;',
    'virtual void Tick(float DeltaTime) override;',
    'virtual TStatId GetStatId() const override;',
]
for token in GUARD_REQUIRED:
    if token not in guard_h:
        fail(f"missing frontend shell guard declaration: {token}")

GUARD_CPP_REQUIRED = [
    'GameMode->IsFrontendOnlySession()',
    'APawn* Pawn = PC->GetPawn()',
    'PC->UnPossess()',
    'Pawn->HasAuthority()',
    'Pawn->Destroy()',
    'RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13FrontendShellGuardSubsystem, STATGROUP_Tickables)',
    'static menu backdrop preserved',
]
for token in GUARD_CPP_REQUIRED:
    if token not in guard:
        fail(f"missing frontend shell runtime guard: {token}")

if 'SetPresentationVisibility(true, false, true)' not in menu:
    fail("pause presentation marker missing")
if 'SetPresentationVisibility(true, true, false)' not in menu:
    fail("main-menu static-backdrop presentation marker missing")

print("R13 FRONTEND MENU GUARD VERIFY: PASS")
print("Checks approved static backdrop, legacy-layer suppression, neutral-only background tint, local-only gradient/pause dimming and leaked-pawn protection for the UI-only shell.")
