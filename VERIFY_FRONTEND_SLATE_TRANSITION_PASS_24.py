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
    ('int32 PendingPage = INDEX_NONE', 'deferred page state'),
    ('int32 LastAppliedPage = INDEX_NONE', 'page application cache'),
    ('uint64 PendingActionEarliestFrame = 0', 'next-frame action fence'),
    ('bool bPendingHostedStart = false', 'deferred host state'),
    ('bool bPendingNetworkConnect = false', 'deferred connect state'),
    ('IsTickableWhenPaused() const override { return true; }', 'paused-menu tick contract'),
]:
    need(hdr, token, label)

for token, label in [
    ('Primary->OnClicked.AddDynamic', 'primary OnClicked binding'),
    ('Secondary->OnClicked.AddDynamic', 'secondary OnClicked binding'),
    ('Network->OnClicked.AddDynamic', 'network OnClicked binding'),
    ('Settings->OnClicked.AddDynamic', 'settings OnClicked binding'),
    ('Quit->OnClicked.AddDynamic', 'quit OnClicked binding'),
    ('PendingActionEarliestFrame = GFrameCounter + 1', 'next-frame fence arm'),
    ('PendingPage = 1;', 'host setup page queue'),
    ('PendingPage = 2;', 'network setup page queue'),
    ('PendingPage = 0;', 'back-to-main page queue'),
    ('PASS24_FRONTEND_PAGE_TRANSITION_QUEUED page=1', 'host page queued marker'),
    ('PASS24_FRONTEND_PAGE_TRANSITION_QUEUED page=2', 'network page queued marker'),
    ('PASS24_FRONTEND_PAGE_TRANSITION_BEGIN', 'deferred transition begin marker'),
    ('PASS24_FRONTEND_PAGE_TRANSITION_READY', 'deferred transition ready marker'),
    ('PASS45_FRONTEND_PAGE_APPLIED', 'stable tree/backdrop marker'),
    ('PASS24_HOST_START_DEFERRED_QUEUED', 'deferred host queue'),
    ('PASS24_HOST_START_DEFERRED_EXECUTE', 'deferred host execute'),
    ('PASS24_NETWORK_CONNECT_DEFERRED_QUEUED', 'deferred connect queue'),
    ('PASS24_NETWORK_CONNECT_DEFERRED_EXECUTE', 'deferred connect execute'),
    ('PASS14_MAIN_START_OPENS_SERVER_SETUP', 'server setup compatibility marker'),
    ('SetPresentationVisibility(true, true, false);', 'secondary menu backdrop ownership'),
    ('Root->WidgetTree', 'stable WidgetTree ownership'),
]:
    need(cpp, token, label)

for token, label in [
    ('->OnPressed.AddDynamic', 'input mutation from OnPressed'),
    ('Page = 1;\n        ApplyPage();', 'immediate page mutation in click callback'),
    ('PASS29_UNSAFE_FRONTEND_PAGE_TRANSITION_BLOCKED', 'obsolete transition blocker'),
    ('PASS29_SECONDARY_IGNORED_STATIC_FRONTEND', 'obsolete inert BACK button'),
]:
    forbid(cpp, token, label)

if errors:
    print('FRONTEND SLATE TRANSITION PASS 24: FAIL')
    for error in errors:
        print('[FAIL]', error)
    raise SystemExit(1)

print('FRONTEND SLATE TRANSITION PASS 24: SUCCESS')
print('- START, NETWORK and BACK are deferred beyond the Slate click frame')
print('- secondary pages reuse the already-built WidgetTree and retain the authored background')
print('- host/network travel remains deferred and menu Tick continues while the world is paused')
print('STATUS: SOURCE CONTRACT ONLY; local UE 5.8 click-through remains authoritative')
