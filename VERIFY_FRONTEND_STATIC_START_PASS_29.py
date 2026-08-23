from pathlib import Path

ROOT = Path(__file__).resolve().parent
MENU = (ROOT / "OsterConflict/Source/OsterConflict/Private/OCR13FrontendMenuSubsystem.cpp").read_text(encoding="utf-8")
ROOT_UI = (ROOT / "OsterConflict/Source/OsterConflict/Private/OCGameUIRootWidget.cpp").read_text(encoding="utf-8")
LAUNCH = (ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd").read_text(encoding="utf-8")


def fail(msg: str) -> None:
    raise SystemExit("FRONTEND STATIC START PASS 29 FAIL: " + msg)

required_menu = [
    'PASS29_MAIN_START_DIRECT_HOST_QUEUED',
    'PASS29_STATIC_FRONTEND_HOST_TRAVEL_EXECUTE',
    'PASS29_NETWORK_DIRECT_CONNECT_QUEUED',
    'PASS29_UNSAFE_FRONTEND_PAGE_TRANSITION_BLOCKED',
    'bPendingHostedStart = true;',
    'bPendingNetworkConnect = true;',
    'Root->WidgetTree',
    'PASS27_FRONTEND_WIDGETTREE_OWNED',
    'PASS26_FRONTEND_ACTION_FENCE',
]
for token in required_menu:
    if token not in MENU:
        fail(f"missing menu contract: {token}")

for forbidden in [
    'PendingPage = 1;',
    'PendingPage = 2;',
    'PASS24_FRONTEND_PAGE_TRANSITION_QUEUED page=1',
    'PASS24_FRONTEND_PAGE_TRANSITION_QUEUED page=2',
]:
    if forbidden in MENU:
        fail(f"unsafe runtime page transition returned: {forbidden}")

required_root = [
    'if (bR13OwnsFrontend && bFrontend)',
    'FreezeLegacyPanel(SettingsPanel);',
    'FreezeLegacyPanel(FrontendPanel);',
    'FreezeLegacyPanel(DeploymentPanel);',
    'FreezeLegacyPanel(ScoreboardPanel);',
    'FreezeLegacyPanel(ChatPanel);',
    'FreezeLegacyPanel(AdminPanel);',
    'LastFocusContext = 0;',
]
for token in required_root:
    if token not in ROOT_UI:
        fail(f"missing root freeze contract: {token}")

if '/C:"PASS29_"' not in LAUNCH:
    fail("launcher does not surface Pass 29 crash breadcrumbs")

print("FRONTEND STATIC START PASS 29: PASS")
print("START and NETWORK no longer perform live frontend page-tree transitions; legacy root churn is frozen while R13 owns the startup shell.")
