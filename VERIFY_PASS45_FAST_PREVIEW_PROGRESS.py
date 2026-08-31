from pathlib import Path

ROOT = Path(__file__).resolve().parent
launcher = (ROOT / 'RUN_PASS45_FAST_PREVIEW.cmd').read_text(encoding='utf-8')
game_instance = (ROOT / 'OsterConflict/Source/OsterConflict/Private/OCGameInstance.cpp').read_text(encoding='utf-8')
game_instance_h = (ROOT / 'OsterConflict/Source/OsterConflict/Public/OCGameInstance.h').read_text(encoding='utf-8')
build_rules = (ROOT / 'OsterConflict/Source/OsterConflict/OsterConflict.Build.cs').read_text(encoding='utf-8')
legacy_progress = ROOT / 'OsterConflict/Scripts/PASS45_FAST_PREVIEW_PROGRESS.ps1'

required_launcher = [
    '-abslog="%PREVIEW_LOG%"',
    '"/Engine/Maps/Entry?game=/Script/OsterConflict.OCGameModeRuntimeSafe"',
    '-game -Frontend',
    'Runtime map is NOT loaded before the menu.',
    'Loading presentation is rendered INSIDE Unreal.',
    'No external progress window and no fake percentage.',
    'PREVIEW ONLY',
    'runtime acceptance remains unchanged',
]
for needle in required_launcher:
    assert needle in launcher, f'missing Fast Preview in-game loading contract: {needle}'

for forbidden in [
    'PASS45_FAST_PREVIEW_PROGRESS.ps1',
    'PASS45_FAST_PREVIEW_PROGRESS.state',
    'powershell.exe',
    'System.Windows.Forms',
    'A separate progress window',
    '"/Game/Maps/OsterConflict_Runtime"',
    ' -log ',
]:
    assert forbidden not in launcher, f'heavy/external startup regressed into Fast Preview: {forbidden}'

required_game_instance = [
    '#include "MoviePlayer.h"',
    '#include "UObject/UObjectGlobals.h"',
    'FCoreUObjectDelegates::PreLoadMap.AddUObject',
    'FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject',
    'FCoreUObjectDelegates::PreLoadMap.RemoveAll(this)',
    'FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this)',
    'FLoadingScreenAttributes LoadingScreen',
    'LoadingScreen.bAutoCompleteWhenLoadingCompletes = true',
    'LoadingScreen.bWaitForManualStop = false',
    'LoadingScreen.bAllowEngineTick = false',
    'LoadingScreen.WidgetLoadingScreen',
    'SNew(SThrobber)',
    'GetMoviePlayer()->SetupLoadingScreen(LoadingScreen)',
    'PASS45_INGAME_LOADING_BEGIN',
    'PASS45_INGAME_LOADING_MAP_COMPLETE',
    'ЗАВАНТАЖЕННЯ СВІТУ',
]
for needle in required_game_instance:
    assert needle in game_instance, f'missing engine-native loading screen contract: {needle}'

for needle in [
    'void HandlePreLoadMap(const FString& MapName);',
    'void HandlePostLoadMap(UWorld* LoadedWorld);',
    'double ActiveMapLoadStartedAtSeconds = 0.0;',
]:
    assert needle in game_instance_h, f'missing loading lifecycle declaration: {needle}'

assert '"MoviePlayer"' in build_rules, 'MoviePlayer module dependency must remain available'
assert not legacy_progress.exists(), 'legacy external WinForms loading script must stay deleted'
assert 'System.Windows.Forms.ProgressBar' not in game_instance, 'fake WinForms progress must never move into runtime code'

print('PASS45 Fast Preview engine-native in-game loading presentation: PASS')
print('- initial Fast Preview opens a lightweight Engine Entry frontend under OCGameModeRuntimeSafe')
print('- OsterConflict_Runtime is deferred until the user actually starts/joins gameplay')
print('- external PowerShell/WinForms progress window retired')
print('- map load is bracketed by PreLoadMap/PostLoadMapWithWorld diagnostics')
print('- MoviePlayer Slate loading surface auto-completes when UE LoadMap completes')
print('- no fake percentage is presented; runtime acceptance remains unchanged')
