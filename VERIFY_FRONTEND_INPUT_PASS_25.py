from pathlib import Path

root = Path(__file__).resolve().parent
cpp = (root / 'OsterConflict/Source/OsterConflict/Private/OCR13FrontendMenuSubsystem.cpp').read_text(encoding='utf-8')
hdr = (root / 'OsterConflict/Source/OsterConflict/Public/OCR13FrontendMenuSubsystem.h').read_text(encoding='utf-8')

errors = []

def need(text: str, token: str, label: str):
    if token not in text:
        errors.append(f'missing {label}: {token}')

def forbid(text: str, token: str, label: str):
    if token in text:
        errors.append(f'forbidden {label}: {token}')

need(hdr, 'bool bMenuInputArmed = false', 'menu input lifecycle state')
need(cpp, 'if (ActiveController.Get() != PC) bMenuInputArmed = false;', 'controller-change reset')
need(cpp, 'if (bMenuInputArmed) return;', 'per-tick input-mode dedupe')
need(cpp, 'bMenuInputArmed = true;', 'input armed state')
need(cpp, 'PASS25_MENU_INPUT_ARMED', 'runtime input marker')
need(cpp, 'void UOCR13FrontendMenuSubsystem::ReleaseMenuInput()', 'release helper')
need(cpp, 'Primary->OnClicked.AddDynamic', 'stable OnClicked primary binding')

pass29_static = 'PASS29_MAIN_START_DIRECT_HOST_QUEUED' in cpp
if pass29_static:
    need(cpp, 'PASS29_MAIN_START_DIRECT_HOST_QUEUED', 'Pass 29 static START compatibility')
    need(cpp, 'PASS29_UNSAFE_FRONTEND_PAGE_TRANSITION_BLOCKED', 'Pass 29 page-mutation block')
    forbid(cpp, 'PendingPage = 1;', 'removed crash-prone START page transition')
else:
    need(cpp, 'PASS24_FRONTEND_PAGE_TRANSITION_QUEUED', 'Pass 24 deferred transition compatibility')

forbid(cpp, '->OnPressed.AddDynamic', 'OnPressed frontend mutation path')
forbid(cpp, 'Mode.SetWidgetToFocus(PrimaryButton->TakeWidget())', 'per-tick TakeWidget focus churn')

force_start = cpp.find('void UOCR13FrontendMenuSubsystem::ForceMenuInput()')
release_start = cpp.find('void UOCR13FrontendMenuSubsystem::ReleaseMenuInput()')
if force_start < 0 or release_start < 0 or release_start <= force_start:
    errors.append('cannot isolate ForceMenuInput body')
else:
    force = cpp[force_start:release_start]
    if force.count('PC->SetInputMode(Mode);') != 1:
        errors.append(f'ForceMenuInput must contain exactly one UI SetInputMode call, found {force.count("PC->SetInputMode(Mode);")}')
    guard = force.find('if (bMenuInputArmed) return;')
    set_mode = force.find('PC->SetInputMode(Mode);')
    armed = force.find('bMenuInputArmed = true;')
    if not (0 <= guard < set_mode < armed):
        errors.append('menu input guard must execute before SetInputMode, then arm state after it')

release_end = cpp.find('TStatId UOCR13FrontendMenuSubsystem::GetStatId()', release_start)
if release_start >= 0 and release_end > release_start:
    release = cpp[release_start:release_end]
    if 'bMenuInputArmed = false;' not in release:
        errors.append('ReleaseMenuInput must reset bMenuInputArmed')

if errors:
    print('FRONTEND INPUT PASS 25: FAIL')
    for error in errors:
        print('[FAIL]', error)
    raise SystemExit(1)

print('FRONTEND INPUT PASS 25: SUCCESS')
print('- UI input mode is armed once per menu/controller lifecycle instead of every Tick')
print('- OnClicked mouse release can survive without per-frame capture reset')
if pass29_static:
    print('- Pass 29 replaces the unsafe page transition with a static later-frame START path')
else:
    print('- Pass 24 deferred Slate transition behavior remains intact')
print('STATUS: SOURCE CONTRACT ONLY; local UE 5.8 button click confirmation still required')
