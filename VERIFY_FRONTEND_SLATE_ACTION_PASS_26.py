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

pass29_static = 'PASS29_MAIN_START_DIRECT_HOST_QUEUED' in cpp

for name, next_name in callbacks:
    start = cpp.find(f'void UOCR13FrontendMenuSubsystem::{name}()')
    end = cpp.find(f'void UOCR13FrontendMenuSubsystem::{next_name}()', start + 1)
    if start < 0 or end <= start:
        errors.append(f'cannot isolate callback {name}')
        continue
    body = cpp[start:end]

    # Pass 29 intentionally turns Secondary into a no-op because there is no longer a mutable
    # server-setup page to navigate back from. Every callback that still performs an action must
    # retain the Pass 26 engine-frame fence; a no-op callback is safer than queuing fake work.
    is_pass29_noop = name == 'OnSecondaryClicked' and 'PASS29_SECONDARY_IGNORED_STATIC_FRONTEND' in body
    if not is_pass29_noop and 'ArmDeferredActionFence();' not in body:
        errors.append(f'{name} must only queue a fenced action')

    for token in unsafe:
        if token in body:
            errors.append(f'{name} performs unsafe Slate/input/travel work inside OnClicked: {token}')

# Pass 25 input protection remains mandatory. Pass 24's original page mutation contract is superseded
# by Pass 29 when the static frontend is present: START/NETWORK still queue work and still wait for
# Pass 26's later-frame execution fence, but they no longer mutate the live Slate page hierarchy.
for token, label in [
    ('Primary->OnClicked.AddDynamic', 'OnClicked binding'),
    ('PASS25_MENU_INPUT_ARMED', 'Pass 25 input marker'),
    ('if (bMenuInputArmed) return;', 'Pass 25 input dedupe'),
]:
    need(cpp, token, label)

if pass29_static:
    for token, label in [
        ('PASS29_MAIN_START_DIRECT_HOST_QUEUED', 'Pass 29 direct host queue marker'),
        ('PASS29_NETWORK_DIRECT_CONNECT_QUEUED', 'Pass 29 direct network queue marker'),
        ('PASS29_UNSAFE_FRONTEND_PAGE_TRANSITION_BLOCKED', 'Pass 29 fail-closed page guard'),
        ('PASS24_HOST_START_DEFERRED_EXECUTE', 'deferred host execution compatibility'),
        ('PASS24_NETWORK_CONNECT_DEFERRED_EXECUTE', 'deferred network execution compatibility'),
    ]:
        need(cpp, token, label)
    forbid(cpp, 'PendingPage = 1;', 'removed main START page mutation')
    forbid(cpp, 'PendingPage = 2;', 'removed network page mutation')
else:
    for token, label in [
        ('PASS24_FRONTEND_PAGE_TRANSITION_QUEUED', 'Pass 24 navigation marker'),
        ('PASS24_HOST_START_DEFERRED_QUEUED', 'Pass 24 host marker'),
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
print('- every actionable frontend OnClicked callback only queues work; Pass 29 Secondary is an inert no-op')
print('- widget/input/travel/settings/quit mutations wait for a later engine frame')
print('- repeated presentation/pause Slate invalidations are deduplicated')
print('- legacy frontend suppression is no longer executed every Tick')
if pass29_static:
    print('- Pass 29 supersedes crash-prone page mutation while preserving the Pass 26 action fence')
print('STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime confirmation is still required')
