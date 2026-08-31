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
    'Runtime map is NOT loaded before the menu.',
    'Startup bootstrap and map loading are rendered INSIDE Unreal.',
    'Frontend bootstrap stays visible until the actual R13 menu widget is visible.',
    'PREVIEW ONLY',
]:
    assert needle in launcher, f'missing Fast Preview contract: {needle}'

for forbidden in [
    'PASS45_FAST_PREVIEW_PROGRESS.ps1',
    'PASS45_FAST_PREVIEW_PROGRESS.state',
    'powershell.exe',
    'System.Windows.Forms',
    '"/Game/Maps/OsterConflict_Runtime"',
    ' -OCFastPreview',
    ' -NoFrontend',
    ' -log ',
]:
    assert forbidden not in launcher, f'old Fast Preview path returned: {forbidden}'

for needle in [
    '#include "MoviePlayer.h"',
    'FCoreUObjectDelegates::PreLoadMap.AddUObject',
    'FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject',
    'FLoadingScreenAttributes LoadingScreen',
    'LoadingScreen.bWaitForManualStop = true',
    'MoviePlayer->SetupLoadingScreen(LoadingScreen)',
    'MoviePlayer->PlayMovie()',
    'MoviePlayer->StopMovie()',
    'PASS45_INGAME_LOADING_BEGIN',
    'PASS45_INGAME_LOADING_MAP_COMPLETE',
    'PASS45_INGAME_LOADING_READY',
]:
    assert needle in game_instance, f'missing map loading contract: {needle}'

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
    'GI->CompleteRuntimeLoading(',
]:
    assert needle in runtime_safe_cpp, f'missing frontend handoff contract: {needle}'

assert '"MoviePlayer"' in build_rules
assert not legacy_progress.exists()
assert 'System.Windows.Forms.ProgressBar' not in game_instance
assert 'GetAsyncLoadPercentage' not in game_instance

print('PASS45 Fast Preview in-game bootstrap and map loading: PASS')
print('- Engine Entry starts with a viewport-owned frontend bootstrap')
print('- frontend bootstrap hands off only to a visible menu widget')
print('- runtime map loading remains engine-native and milestone based')
print('- external progress helper remains retired')
print('- runtime acceptance remains pending until local UE 5.8 acceptance')
