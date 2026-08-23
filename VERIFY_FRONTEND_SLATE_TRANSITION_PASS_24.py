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
    ('int32 PendingPage = INDEX_NONE', 'fail-closed page state'),
    ('int32 LastAppliedPage = INDEX_NONE', 'legacy page application cache'),
    ('bool bPendingHostedStart = false', 'deferred host state'),
    ('bool bPendingNetworkConnect = false', 'deferred connect state'),
    ('void StartNetworkGameplay();', 'deferred network helper'),
]:
    need(hdr, token, label)

for token, label in [
    ('Primary->OnClicked.AddDynamic', 'primary OnClicked binding'),
    ('Secondary->OnClicked.AddDynamic', 'secondary OnClicked binding'),
    ('Network->OnClicked.AddDynamic', 'network OnClicked binding'),
    ('Settings->OnClicked.AddDynamic', 'settings OnClicked binding'),
    ('Quit->OnClicked.AddDynamic', 'quit OnClicked binding'),
    ('PASS24_HOST_START_DEFERRED_EXECUTE', 'deferred host execute marker'),
    ('PASS24_NETWORK_CONNECT_DEFERRED_EXECUTE', 'deferred connect execute marker'),
    ('if (!MenuBox.IsValid() || LastAppliedPage == Page) return;', 'legacy page dedupe guard'),
    ('LastAppliedPage = Page;', 'legacy page applied state'),
    ('FLinearColor(0.025f, 0.030f, 0.036f, 0.96f)', 'opaque server/network panel style retained'),
    ('FieldStyle.BackgroundColor = FSlateColor(FLinearColor(0.045f, 0.052f, 0.061f, 1.0f))', 'dark field background retained'),
    ('PASS14_HOST_TRAVEL_BEGIN', 'host travel compatibility marker'),
]:
    need(cpp, token, label)

pass29_static = 'PASS29_MAIN_START_DIRECT_HOST_QUEUED' in cpp

if pass29_static:
    # Pass 24's deferred concept remains valid, but its Page 0 -> Page 1 live Slate mutation was
    # disproven by repeated runtime crashes. Pass 29 is the stronger contract: defer the action,
    # keep the startup hierarchy static, then enter host/network directly on the later frame.
    for token, label in [
        ('PASS29_MAIN_START_DIRECT_HOST_QUEUED', 'static START queue marker'),
        ('PASS29_NETWORK_DIRECT_CONNECT_QUEUED', 'static NETWORK queue marker'),
        ('PASS29_UNSAFE_FRONTEND_PAGE_TRANSITION_BLOCKED', 'fail-closed page transition guard'),
        ('PASS29_STATIC_FRONTEND_HOST_TRAVEL_EXECUTE', 'static host travel marker'),
    ]:
        need(cpp, token, label)
    for token, label in [
        ('PendingPage = 1;', 'main START page mutation'),
        ('PendingPage = 2;', 'network page mutation'),
        ('PASS24_FRONTEND_PAGE_TRANSITION_QUEUED page=1', 'obsolete main page transition marker'),
        ('PASS24_FRONTEND_PAGE_TRANSITION_QUEUED page=2', 'obsolete network page transition marker'),
        ('PASS14_MAIN_START_OPENS_SERVER_SETUP', 'obsolete server-setup page marker'),
    ]:
        forbid(cpp, token, label)
else:
    for token, label in [
        ('PASS24_FRONTEND_PAGE_TRANSITION_QUEUED', 'queued page marker'),
        ('PASS24_FRONTEND_PAGE_TRANSITION_BEGIN', 'transition begin marker'),
        ('PASS24_FRONTEND_PAGE_TRANSITION_READY', 'transition ready marker'),
        ('PASS24_HOST_START_DEFERRED_QUEUED', 'deferred host queued marker'),
        ('PASS24_NETWORK_CONNECT_DEFERRED_QUEUED', 'deferred connect queued marker'),
        ('PASS14_MAIN_START_OPENS_SERVER_SETUP', 'Pass 14 compatibility marker'),
    ]:
        need(cpp, token, label)

for token, label in [
    ('->OnPressed.AddDynamic', 'input mutation from OnPressed'),
    ('Mode.SetWidgetToFocus(PrimaryButton->TakeWidget())', 'per-tick TakeWidget focus'),
    ('Page = 1;\n        ApplyPage();', 'immediate page mutation in click callback'),
    ('FLinearColor(0.0f, 0.0f, 0.0f, 0.36f)', 'old translucent setup panel'),
    ('Field->SetMinimumDesiredWidth(', 'unsupported UEditableTextBox width API'),
]:
    forbid(cpp, token, label)

if errors:
    print('FRONTEND SLATE TRANSITION PASS 24: FAIL')
    for error in errors:
        print('[FAIL]', error)
    raise SystemExit(1)

print('FRONTEND SLATE TRANSITION PASS 24: SUCCESS')
if pass29_static:
    print('- Pass 24 deferred action boundary is retained, but crash-prone live page transitions are retired by Pass 29')
    print('- START/NETWORK queue direct later-frame actions against a structurally static startup menu')
else:
    print('- frontend navigation is deferred out of Slate input callbacks')
print('- server host/network travel remains deferred to world Tick')
print('- TakeWidget focus churn remains removed and field styling contracts remain intact')
print('STATUS: SOURCE CONTRACT ONLY; local UE 5.8 click-through runtime confirmation is still required')
