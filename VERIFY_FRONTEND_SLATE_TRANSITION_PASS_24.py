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
    ('PASS24_FRONTEND_PAGE_TRANSITION_QUEUED', 'queued page marker'),
    ('PASS24_FRONTEND_PAGE_TRANSITION_BEGIN', 'transition begin marker'),
    ('PASS24_FRONTEND_PAGE_TRANSITION_READY', 'transition ready marker'),
    ('PASS24_HOST_START_DEFERRED_QUEUED', 'deferred host queued marker'),
    ('PASS24_HOST_START_DEFERRED_EXECUTE', 'deferred host execute marker'),
    ('PASS24_NETWORK_CONNECT_DEFERRED_QUEUED', 'deferred connect queued marker'),
    ('PASS24_NETWORK_CONNECT_DEFERRED_EXECUTE', 'deferred connect execute marker'),
    ('if (!MenuBox.IsValid() || LastAppliedPage == Page) return;', 'page dedupe guard'),
    ('LastAppliedPage = Page;', 'page applied state'),
    ('FLinearColor(0.025f, 0.030f, 0.036f, 0.96f)', 'opaque server/network panel'),
    ('FieldStyle.BackgroundColor = FSlateColor(FLinearColor(0.045f, 0.052f, 0.061f, 1.0f))', 'dark field background'),
    ('PASS14_MAIN_START_OPENS_SERVER_SETUP', 'Pass 14 compatibility marker'),
    ('PASS14_HOST_TRAVEL_BEGIN', 'host travel compatibility marker'),
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
print('- frontend navigation is deferred out of Slate input callbacks')
print('- server host/network travel is deferred to world Tick')
print('- unchanged pages are not rebuilt every frame and TakeWidget focus churn is removed')
print('- server/network panel is opaque and editable fields use explicit dark styling')
print('STATUS: SOURCE CONTRACT ONLY; local UE 5.8 click-through runtime confirmation is still required')
