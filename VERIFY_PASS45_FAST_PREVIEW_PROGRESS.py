from pathlib import Path

ROOT = Path(__file__).resolve().parent
launcher = (ROOT / 'RUN_PASS45_FAST_PREVIEW.cmd').read_text(encoding='utf-8')
game_instance = (ROOT / 'OsterConflict/Source/OsterConflict/Private/OCGameInstance.cpp').read_text(encoding='utf-8')
game_instance_h = (ROOT / 'OsterConflict/Source/OsterConflict/Public/OCGameInstance.h').read_text(encoding='utf-8')
runtime_safe_h = (ROOT / 'OsterConflict/Source/OsterConflict/Public/OCGameModeRuntimeSafe.h').read_text(encoding='utf-8')
runtime_safe_cpp = (ROOT / 'OsterConflict/Source/OsterConflict/Private/OCGameModeRuntimeSafe.cpp').read_text(encoding='utf-8')
build_rules = (ROOT / 'OsterConflict/Source/OsterConflict/OsterConflict.Build.cs').read_text(encoding='utf-8')
legacy_progress = ROOT / 'OsterConflict/Scripts/PASS45_FAST_PREVIEW_PROGRESS.ps1'

required_launcher = [
    '-abslog="%PREVIEW_LOG%"',
    '"/Engine/Maps/Entry?game=/Script/OsterConflict.OCGameModeRuntimeSafe"',
    '-game -OCFastPreview',
    'Runtime map is NOT loaded before the menu.',
    'Loading presentation is rendered INSIDE Unreal with real lifecycle milestones.',
    'Percentage means UE startup milestones, not guessed byte/shader completion.',
    'Standalone frontend fallback remains visible until the R13 menu takes ownership.',
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
    ' -Frontend',
    ' -NoFrontend',
    ' -log ',
]:
    assert forbidden not in launcher, f'heavy/external/blank-frontend startup regressed into Fast Preview: {forbidden}'

required_game_instance = [
    '#include "MoviePlayer.h"',
    '#include "Misc/CommandLine.h"',
    '#include "Misc/Parse.h"',
    '#include "UObject/UObjectGlobals.h"',
    'FCoreUObjectDelegates::PreLoadMap.AddUObject',
    'FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject',
    'FCoreUObjectDelegates::PreLoadMap.RemoveAll(this)',
    'FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this)',
    'FParse::Param(FCommandLine::Get(), TEXT("OCFastPreview"))',
    'PrepareRuntimeLoadingScreen(TEXT("FrontendBootstrap"), 5, 1)',
    'FLoadingScreenAttributes LoadingScreen',
    'LoadingScreen.bAutoCompleteWhenLoadingCompletes = false',
    'LoadingScreen.bWaitForManualStop = true',
    'LoadingScreen.bAllowEngineTick = false',
    'LoadingScreen.WidgetLoadingScreen',
    'SNew(SThrobber)',
    '.Text_Lambda([]() { return Pass45LoadingPercentText(); })',
    '.Text_Lambda([]() { return Pass45LoadingPhaseText(); })',
    'MoviePlayer->SetupLoadingScreen(LoadingScreen)',
    'MoviePlayer->PlayMovie()',
    'MoviePlayer->StopMovie()',
    'PASS45_INGAME_LOADING_BEGIN',
    'PASS45_INGAME_LOADING_MAP_COMPLETE',
    'PASS45_INGAME_LOADING_READY',
    'milestone_percent=70',
    'percent=100',
    'Не оцінка байтів/шейдерів.',
]
for needle in required_game_instance:
    assert needle in game_instance, f'missing engine-native milestone loading screen contract: {needle}'

for needle in [
    'void CompleteRuntimeLoading(const TCHAR* Reason);',
    'void HandlePreLoadMap(const FString& MapName);',
    'void HandlePostLoadMap(UWorld* LoadedWorld);',
    'void PrepareRuntimeLoadingScreen(const FString& Context, int32 MilestonePercent, int32 Phase);',
    'double ActiveMapLoadStartedAtSeconds = 0.0;',
]:
    assert needle in game_instance_h, f'missing loading lifecycle declaration: {needle}'

assert 'virtual void BeginPlay() override;' in runtime_safe_h, 'runtime-safe GameMode must own final loading milestone'
for needle in [
    'void AOCGameModeRuntimeSafe::BeginPlay()',
    'Super::BeginPlay();',
    'GI->CompleteRuntimeLoading(',
    'frontend_beginplay_ready',
    'runtime_beginplay_ready',
]:
    assert needle in runtime_safe_cpp, f'loading screen can disappear before runtime BeginPlay completes: {needle}'

assert '"MoviePlayer"' in build_rules, 'MoviePlayer module dependency must remain available'
assert not legacy_progress.exists(), 'legacy external WinForms loading script must stay deleted'
assert 'System.Windows.Forms.ProgressBar' not in game_instance, 'fake WinForms progress must never move into runtime code'
assert 'GetAsyncLoadPercentage' not in game_instance, 'slow/blocking package percentage polling must not run on the loading surface'

print('PASS45 Fast Preview engine-native milestone loading presentation: PASS')
print('- initial Fast Preview opens a lightweight Engine Entry frontend under OCGameModeRuntimeSafe')
print('- standalone fallback is not prematurely hidden by the old -Frontend command-line ownership switch')
print('- OsterConflict_Runtime is deferred until the user actually starts/joins gameplay')
print('- external PowerShell/WinForms progress window remains retired')
print('- MoviePlayer is manual-stop and remains visible through OCGameModeRuntimeSafe::BeginPlay')
print('- visible percent is factual lifecycle milestone progress: boot/map/world/ready, never guessed byte/shader completion')
print('- runtime acceptance remains unchanged until the actual UE 5.8 acceptance route passes')
