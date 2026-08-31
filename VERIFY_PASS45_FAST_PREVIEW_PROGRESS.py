from pathlib import Path

ROOT = Path(__file__).resolve().parent
start_here = (ROOT / 'START_HERE.cmd').read_text(encoding='utf-8')
launcher = (ROOT / 'RUN_PASS45_FAST_PREVIEW.cmd').read_text(encoding='utf-8')
game_instance = (ROOT / 'OsterConflict/Source/OsterConflict/Private/OCGameInstance.cpp').read_text(encoding='utf-8')
game_instance_h = (ROOT / 'OsterConflict/Source/OsterConflict/Public/OCGameInstance.h').read_text(encoding='utf-8')
runtime_safe_h = (ROOT / 'OsterConflict/Source/OsterConflict/Public/OCGameModeRuntimeSafe.h').read_text(encoding='utf-8')
runtime_safe_cpp = (ROOT / 'OsterConflict/Source/OsterConflict/Private/OCGameModeRuntimeSafe.cpp').read_text(encoding='utf-8')

# 2026-08-31 local UE 5.8 evidence rejected the experimental Entry/Fast Preview
# startup route: it produced an unresponsive black window and trapped desktop focus.
# Keep the diagnostic launcher in the tree for forensic comparison, but never wire it
# to the user-facing normal-game option again.
assert 'echo 1. ЗВИЧАЙНА ГРА\n' in start_here
assert 'call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"' in start_here
assert 'call "%~dp0RUN_PASS45_FAST_PREVIEW.cmd"' not in start_here
assert 'ЗВИЧАЙНА ГРА / ШВИДКИЙ ПЕРЕГЛЯД' not in start_here

# The rejected startup experiment must not own normal GameInstance/GameMode lifecycle.
for forbidden in [
    '#include "MoviePlayer.h"',
    'FCoreUObjectDelegates::PreLoadMap.AddUObject',
    'FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject',
    'FLoadingScreenAttributes LoadingScreen',
    'PASS45_INGAME_LOADING_',
    'PrepareRuntimeLoadingScreen',
    'CompleteRuntimeLoading',
]:
    assert forbidden not in game_instance, f'rejected startup loading owner returned: {forbidden}'

for forbidden in [
    'CompleteRuntimeLoading',
    'HandlePreLoadMap',
    'HandlePostLoadMap',
    'PrepareRuntimeLoadingScreen',
]:
    assert forbidden not in game_instance_h, f'rejected GameInstance declaration returned: {forbidden}'

for forbidden in [
    'ShowFrontendBootstrapOverlay',
    'PollFrontendBootstrapReady',
    'FrontendBootstrapOverlay',
    'virtual void BeginPlay() override;',
    'virtual void EndPlay(',
]:
    assert forbidden not in runtime_safe_h, f'rejected bootstrap declaration returned: {forbidden}'

for forbidden in [
    'PASS45_FRONTEND_BOOTSTRAP_',
    'GEngine->GameViewport->AddViewportWidgetContent',
    'SNew(SProgressBar)',
    'KeepFrontendInputRecoverable',
    'CompleteRuntimeLoading',
]:
    assert forbidden not in runtime_safe_cpp, f'rejected bootstrap implementation returned: {forbidden}'

# The diagnostic file can remain isolated until it is either repaired or removed in a
# separate root-hygiene pass. Its existence must never imply runtime acceptance.
assert 'PREVIEW ONLY' in launcher

print('PASS45 rejected Fast Preview startup is quarantined: PASS')
print('- START_HERE option 1 is restored to RUN_R14_CURRENT_GAMEPLAY.cmd')
print('- custom MoviePlayer startup ownership is retired from OCGameInstance')
print('- viewport bootstrap overlay ownership is retired from OCGameModeRuntimeSafe')
print('- RUN_PASS45_FAST_PREVIEW.cmd remains isolated diagnostic material only')
print('- local UE 5.8 runtime acceptance remains required')
