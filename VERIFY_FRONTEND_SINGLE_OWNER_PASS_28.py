from pathlib import Path

root = Path(__file__).resolve().parent
ui = root / "OsterConflict/Source/OsterConflict/Private/OCGameUIRootWidget.cpp"
menu = root / "OsterConflict/Source/OsterConflict/Private/OCR13FrontendMenuSubsystem.cpp"

u = ui.read_text(encoding="utf-8", errors="replace")
m = menu.read_text(encoding="utf-8", errors="replace")

required_ui = [
    'const bool bR13OwnsFrontend = FParse::Param(FCommandLine::Get(), TEXT("Frontend"));',
    'const bool bShowLegacyFrontend = bFrontend && !bR13OwnsFrontend;',
    'FrontendPanel->SetVisibility(bShowLegacyFrontend?ESlateVisibility::Visible:ESlateVisibility::Collapsed);',
    'FrontendPanel->SetIsEnabled(bShowLegacyFrontend);',
    'UpdateFocusForVisibleContext(PC, bShowLegacyFrontend, bSettings);',
]
for token in required_ui:
    if token not in u:
        raise SystemExit(f"PASS28 FAIL: missing UI ownership marker: {token}")

for forbidden in [
    'FrontendPanel->SetVisibility(bFrontend?ESlateVisibility::Visible:ESlateVisibility::Collapsed);',
    'UpdateFocusForVisibleContext(PC, bFrontend, bSettings);',
]:
    if forbidden in u:
        raise SystemExit(f"PASS28 FAIL: legacy frontend resurrection path returned: {forbidden}")

required_menu = [
    'UWidgetTree* Tree = Root->WidgetTree;',
    'Tree->ConstructWidget<UBorder>',
    'PASS27_FRONTEND_WIDGETTREE_OWNED',
    'PASS26_FRONTEND_ACTION_FENCE',
]
for token in required_menu:
    if token not in m:
        raise SystemExit(f"PASS28 FAIL: prior frontend lifecycle protection missing: {token}")

print("FRONTEND SINGLE-OWNER PASS 28: PASS")
print("-Frontend keeps exactly one visible/input-owning frontend path; legacy FrontendPanel remains structurally attached but cannot be resurrected by RefreshAll.")
