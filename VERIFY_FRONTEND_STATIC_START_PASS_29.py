from pathlib import Path

ROOT = Path(__file__).resolve().parent
MENU = (ROOT / "OsterConflict/Source/OsterConflict/Private/OCR13FrontendMenuSubsystem.cpp").read_text(encoding="utf-8")
HEADER = (ROOT / "OsterConflict/Source/OsterConflict/Public/OCR13FrontendMenuSubsystem.h").read_text(encoding="utf-8")
ROOT_UI = (ROOT / "OsterConflict/Source/OsterConflict/Private/OCGameUIRootWidget.cpp").read_text(encoding="utf-8")


def fail(msg: str) -> None:
    raise SystemExit("FRONTEND DEFERRED MENU CONTRACT FAIL: " + msg)

required_menu = [
    'PendingPage = 1;',
    'PendingPage = 2;',
    'PendingPage = 0;',
    'PASS24_FRONTEND_PAGE_TRANSITION_QUEUED page=1',
    'PASS24_FRONTEND_PAGE_TRANSITION_QUEUED page=2',
    'PASS24_FRONTEND_PAGE_TRANSITION_BEGIN',
    'PASS24_FRONTEND_PAGE_TRANSITION_READY',
    'PASS45_FRONTEND_PAGE_APPLIED',
    'PASS45_SECONDARY_MENU_BACK_QUEUED',
    'PASS45_SECONDARY_MENU_HOST_TRAVEL_EXECUTE',
    'PASS24_HOST_START_DEFERRED_QUEUED',
    'PASS24_NETWORK_CONNECT_DEFERRED_QUEUED',
    'SetPresentationVisibility(true, true, false);',
    'Root->WidgetTree',
    'PASS27_FRONTEND_WIDGETTREE_OWNED',
    'PASS26_FRONTEND_ACTION_FENCE',
]
for token in required_menu:
    if token not in MENU:
        fail(f"missing menu contract: {token}")

for forbidden in [
    'PASS29_UNSAFE_FRONTEND_PAGE_TRANSITION_BLOCKED',
    'PASS29_SECONDARY_IGNORED_STATIC_FRONTEND',
    'PASS29_MAIN_START_DIRECT_HOST_QUEUED',
    'PASS29_NETWORK_DIRECT_CONNECT_QUEUED',
    'Page = 1;\n        ApplyPage();',
]:
    if forbidden in MENU:
        fail(f"obsolete/in-frame frontend behavior returned: {forbidden}")

if 'IsTickableWhenPaused() const override { return true; }' not in HEADER:
    fail("frontend subsystem must keep processing deferred clicks while the menu world is paused")

for token in [
    'if (bR13OwnsFrontend && bFrontend)',
    'FreezeLegacyPanel(FrontendPanel);',
    'FreezeLegacyPanel(DeploymentPanel);',
    'LastFocusContext = 0;',
]:
    if token not in ROOT_UI:
        fail(f"missing legacy-root suppression contract: {token}")

print("FRONTEND DEFERRED SECONDARY MENU CONTRACT: PASS")
print("START, NETWORK and BACK use a next-frame action fence, keep one stable WidgetTree and preserve the authored backdrop.")
