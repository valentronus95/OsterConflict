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


for token, label in [
    ('bool bPendingSettingsOpen = false', 'deferred settings state'),
    ('bool bPendingQuit = false', 'deferred quit state'),
    ('bool bPendingPauseResume = false', 'deferred pause-resume state'),
    ('uint64 PendingActionEarliestFrame = 0', 'engine-frame fence state'),
    ('bool bPresentationStateValid = false', 'presentation dedupe state'),
    ('bool bPausePageApplied = false', 'pause-page dedupe state'),
    ('bool HasPendingFrontendAction() const;', 'pending-action guard helper'),
    ('void ArmDeferredActionFence();', 'frame fence helper'),
]:
    need(hdr, token, label)

for token, label in [
    ('GFrameCounter >= PendingActionEarliestFrame', 'next-engine-frame execution gate'),
    ('PendingActionEarliestFrame = GFrameCounter + 1;', 'one-frame action fence'),
    ('PASS26_FRONTEND_ACTION_FENCE', 'runtime fence marker'),
    ('PASS26_FRONTEND_ACTION_QUEUED', 'runtime queued marker'),
    ('PASS26_FRONTEND_ACTION_EXECUTE', 'runtime execute marker'),
    ('PASS26_LEGACY_FRONTEND_SUPPRESSED_ONCE', 'one-shot legacy suppression marker'),
    ('if (bPresentationStateValid && bLastShowMenu == bShowMenu', 'presentation invalidation dedupe'),
    ('if (!MenuBox.IsValid() || bPausePageApplied) return;', 'pause page dedupe'),
]:
    need(cpp, token, label)

if cpp.count('SuppressLegacyFrontendLayers(Root);') != 1:
    errors.append(f'legacy suppression must be invoked exactly once per new root source path, found {cpp.count("SuppressLegacyFrontendLayers(Root);")} calls')

callbacks = [
    ('OnPrimaryClicked', 'OnSecondaryClicked'),
    ('OnSecondaryClicked', 'OnNetworkClicked'),
    ('OnNetworkClicked', 'OnSettingsClicked'),
    ('OnSettingsClicked', 'OnQuitClicked'),
    ('OnQuitClicked', 'StartNetworkGameplay'),
]
unsafe = [
    'SetPresentationVisibility(',
    'UIOpenSettings(',
    'QuitGame(',
    'DisconnectFromServer(',
    'ReleaseMenuInput(',
    'ApplyPage(',
    'StartHostedGameplay(',
    'StartNetworkGameplay(',
    'UIToggleFrontend(',
    'SetInputMode(',
    'ConsoleCommand(',
    'UIConnect(',
]

for name, next_name in callbacks:
    start = cpp.find(f'void UOCR13FrontendMenuSubsystem::{name}()')
    end = cpp.find(f'void UOCR13FrontendMenuSubsystem::{next_name}()', start + 1)
    if start < 0 or end <= start:
        errors.append(f'cannot isolate callback {name}')
        continue
    body = cpp[start:end]
    if 'ArmDeferredActionFence();' not in body:
        errors.append(f'{name} must only queue a fenced action')
    for token in unsafe:
        if token in body:
            errors.append(f'{name} performs unsafe Slate/input/travel work inside OnClicked: {token}')

# Pass 24/25 contracts must remain present while Pass 26 strengthens the timing boundary.
for token, label in [
    ('Primary->OnClicked.AddDynamic', 'OnClicked binding'),
    ('PASS24_FRONTEND_PAGE_TRANSITION_QUEUED', 'Pass 24 navigation marker'),
    ('PASS24_HOST_START_DEFERRED_QUEUED', 'Pass 24 host marker'),
    ('PASS25_MENU_INPUT_ARMED', 'Pass 25 input marker'),
    ('if (bMenuInputArmed) return;', 'Pass 25 input dedupe'),
]:
    need(cpp, token, label)

forbid(cpp, '->OnPressed.AddDynamic', 'OnPressed mutation path')
forbid(cpp, 'Mode.SetWidgetToFocus(PrimaryButton->TakeWidget())', 'TakeWidget focus churn')

if errors:
    print('FRONTEND SLATE ACTION PASS 26: FAIL')
    for error in errors:
        print('[FAIL]', error)
    raise SystemExit(1)

print('FRONTEND SLATE ACTION PASS 26: SUCCESS')
print('- every frontend OnClicked callback only queues work')
print('- widget/input/travel/settings/quit mutations wait for a later engine frame')
print('- repeated presentation/pause Slate invalidations are deduplicated')
print('- legacy frontend suppression is no longer executed every Tick')
print('STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime confirmation is still required')
