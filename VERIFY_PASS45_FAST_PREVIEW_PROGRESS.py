from pathlib import Path

ROOT = Path(__file__).resolve().parent
launcher = (ROOT / 'RUN_PASS45_FAST_PREVIEW.cmd').read_text(encoding='utf-8')
game_instance = (ROOT / 'OsterConflict/Source/OsterConflict/Private/OCGameInstance.cpp').read_text(encoding='utf-8')
game_instance_h = (ROOT / 'OsterConflict/Source/OsterConflict/Public/OCGameInstance.h').read_text(encoding='utf-8')
runtime_safe_h = (ROOT / 'OsterConflict/Source/OsterConflict/Public/OCGameModeRuntimeSafe.h').read_text(encoding='utf-8')
runtime_safe_cpp = (ROOT / 'OsterConflict/Source/OsterConflict/Private/OCGameModeRuntimeSafe.cpp').read_text(encoding='utf-8')
build_rules = (ROOT / 'OsterConflict/Source/OsterConflict/OsterConflict.Build.cs').read_text(encoding='utf-8')
legacy_progress = ROOT / 'OsterConflict/Scripts/PASS45_FAST_PREVIEW_PROGRESS.ps1'

for needle in [
    '-abslog="%PREVIEW_LOG%"',
    '"/Engine/Maps/Entry"',
    '-game -Frontend',
    '-windowed',
    'Runtime map is NOT loaded before the menu.',
    'Engine Entry startup is owned by the responsive in-game viewport bootstrap.',
    'Frontend bootstrap stays visible until the actual R13 menu widget is visible.',
    'Windowed mode is intentional so a broken frontend cannot trap desktop focus or hide the cursor.',
    'PREVIEW ONLY',
]:
    assert needle in launcher, f'missing Fast Preview recovery contract: {needle}'

for forbidden in [
    'PASS45_FAST_PREVIEW_PROGRESS.ps1',
    'PASS45_FAST_PREVIEW_PROGRESS.state',
    'powershell.exe',
    'System.Windows.Forms',
    '"/Game/Maps/OsterConflict_Runtime"',
    ' -OCFastPreview',
    ' -NoFrontend',
    ' -fullscreen',
    ' -NoScreenMessages',
    ' -log ',
]:
    assert forbidden not in launcher, f'unsafe/old Fast Preview path returned: {forbidden}'

for needle in [
    '#include "MoviePlayer.h"',
    'FCoreUObjectDelegates::PreLoadMap.AddUObject',
    'FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject',
    'IsFrontendEntryMap(MapName)',
    'PASS45_FRONTEND_STARTUP_MOVIEPLAYER_SKIPPED',
    'deadlock_guard=1',
    'FLoadingScreenAttributes LoadingScreen',
    'LoadingScreen.bAutoCompleteWhenLoadingCompletes = true',
    'LoadingScreen.bWaitForManualStop = false',
    'MoviePlayer->SetupLoadingScreen(LoadingScreen)',
    'MoviePlayer->PlayMovie()',
    'auto_complete=1 manual_stop=0',
    'PASS45_INGAME_LOADING_BEGIN',
    'PASS45_INGAME_LOADING_MAP_COMPLETE',
    'PASS45_INGAME_LOADING_READY',
]:
    assert needle in game_instance, f'missing deadlock-free map loading contract: {needle}'

for forbidden in [
    'PrepareRuntimeLoadingScreen(TEXT("FrontendBootstrap")',
    'LoadingScreen.bWaitForManualStop = true',
    'MoviePlayer->StopMovie()',
]:
    assert forbidden not in game_instance, f'frontend MoviePlayer deadlock contract returned: {forbidden}'

for needle in [
    'void CompleteRuntimeLoading(const TCHAR* Reason);',
    'void HandlePreLoadMap(const FString& MapName);',
    'void HandlePostLoadMap(UWorld* LoadedWorld);',
]:
    assert needle in game_instance_h, f'missing loading declaration: {needle}'

for needle in [
    'virtual void BeginPlay() override;',
    'virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;',
    'void ShowFrontendBootstrapOverlay();',
    'void PollFrontendBootstrapReady();',
    'TSharedPtr<SWidget> FrontendBootstrapOverlay;',
]:
    assert needle in runtime_safe_h, f'missing frontend bootstrap declaration: {needle}'

for needle in [
    'GEngine->GameViewport->AddViewportWidgetContent',
    'SNew(SProgressBar)',
    '.Percent(0.90f)',
    'ПІДГОТОВКА ГОЛОВНОГО МЕНЮ',
    'PASS45_FRONTEND_BOOTSTRAP_OVERLAY_READY',
    'R13_MenuPanel',
    'FrontendPanel',
    'PASS45_FRONTEND_BOOTSTRAP_HANDOFF_READY',
    'PASS45_FRONTEND_BOOTSTRAP_STALLED',
    'KeepFrontendInputRecoverable',
    'bShowMouseCursor = true',
    'InputMode.SetHideCursorDuringCapture(false)',
    'PASS45_FRONTEND_INPUT_RECOVERY_READY',
    'GI->CompleteRuntimeLoading(',
]:
    assert needle in runtime_safe_cpp, f'missing responsive frontend handoff contract: {needle}'

assert '"MoviePlayer"' in build_rules
assert not legacy_progress.exists()
assert 'System.Windows.Forms.ProgressBar' not in game_instance
assert 'GetAsyncLoadPercentage' not in game_instance

print('PASS45 Fast Preview deadlock-free in-game bootstrap: PASS')
print('- Engine Entry explicitly skips MoviePlayer and reaches GameMode/PlayerController startup')
print('- responsive viewport bootstrap owns the initial frontend wait and keeps the cursor visible')
print('- recovery preview is windowed so desktop focus is not trapped on a failed frontend')
print('- MoviePlayer is reserved for actual runtime-map travel and auto-completes without manual-stop deadlock')
print('- external progress helper remains retired')
print('- runtime acceptance remains pending until local UE 5.8 acceptance')
