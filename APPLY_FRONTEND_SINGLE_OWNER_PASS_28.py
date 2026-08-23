from pathlib import Path

root = Path(__file__).resolve().parent
ui = root / "OsterConflict/Source/OsterConflict/Private/OCGameUIRootWidget.cpp"
ledger = root / "OSTER_CONFLICT_WORK_LEDGER.md"

text = ui.read_text(encoding="utf-8")
old = '''    const bool bSettings=PC->IsSettingsVisible(); const bool bFrontend=PC->IsFrontendMenuVisible()&&!bSettings;\n    SettingsPanel->SetVisibility(bSettings?ESlateVisibility::Visible:ESlateVisibility::Collapsed);\n    FrontendPanel->SetVisibility(bFrontend?ESlateVisibility::Visible:ESlateVisibility::Collapsed);\n    DeploymentPanel->SetVisibility(!bFrontend&&!bSettings&&PC->IsDeploymentPanelVisible()?ESlateVisibility::Visible:ESlateVisibility::Collapsed);'''
new = '''    const bool bSettings=PC->IsSettingsVisible(); const bool bFrontend=PC->IsFrontendMenuVisible()&&!bSettings;\n    // Pass 28: -Frontend is owned exclusively by OCR13FrontendMenuSubsystem. Pass 27 kept the legacy\n    // FrontendPanel attached to its WidgetTree, but this 0.20s RefreshAll loop immediately made it\n    // visible again after the subsystem collapsed it. That produced the exact double-menu screenshot\n    // and reintroduced two simultaneously live frontend widget paths. Keep the legacy panel structurally\n    // attached but permanently collapsed/disabled while the R13 frontend shell owns presentation/input.\n    const bool bR13OwnsFrontend = FParse::Param(FCommandLine::Get(), TEXT("Frontend"));\n    const bool bShowLegacyFrontend = bFrontend && !bR13OwnsFrontend;\n    SettingsPanel->SetVisibility(bSettings?ESlateVisibility::Visible:ESlateVisibility::Collapsed);\n    if (FrontendPanel)\n    {\n        FrontendPanel->SetVisibility(bShowLegacyFrontend?ESlateVisibility::Visible:ESlateVisibility::Collapsed);\n        FrontendPanel->SetIsEnabled(bShowLegacyFrontend);\n    }\n    DeploymentPanel->SetVisibility(!bFrontend&&!bSettings&&PC->IsDeploymentPanelVisible()?ESlateVisibility::Visible:ESlateVisibility::Collapsed);'''
if old not in text:
    raise SystemExit("Pass 28 source anchor not found")
text = text.replace(old, new, 1)
text = text.replace('    UpdateFocusForVisibleContext(PC, bFrontend, bSettings);', '    UpdateFocusForVisibleContext(PC, bShowLegacyFrontend, bSettings);', 1)
ui.write_text(text, encoding="utf-8")

lt = ledger.read_text(encoding="utf-8")
lt = lt.replace('Active correction branch: `fix/frontend-widgettree-pass-27-20260823` → `main`', 'Active correction branch: `fix/frontend-single-owner-pass-28-20260823` → `main`')
lt = lt.replace('Pass 27 source hardening coded; потрібен новий UE 5.8 build/runtime.', 'Pass 27 runtime прибрав попередній blind spot, але user screenshot показав конкретний double-menu: native legacy FrontendPanel знову ставав Visible через `UOCGameUIRootWidget::RefreshAll()` кожні 0.20 s поверх R13 frontend. Pass 28 робить `-Frontend` single-owner: legacy panel лишається в WidgetTree, але завжди Collapsed+Disabled; focus також не йде в legacy frontend. Новий UE 5.8 runtime обов’язковий.')
lt = lt.replace('| CRASH-FRONTEND-SLATE-20260823 | Pass 25 frontend click → Slate/SlateCore array assertion | CODED_UNTESTED | Pass 26: усі `OnClicked` лише ставлять action у чергу; виконання fenced мінімум на наступний engine frame; legacy suppression більше не кожен Tick; presentation/pause invalidation dedupe. Runtime acceptance pending. |', '| CRASH-FRONTEND-SLATE-20260823 | Pass 25/26/27 frontend click → Slate/SlateCore array assertion / double frontend | CODED_UNTESTED | Pass 28: `-Frontend` має одного presentation owner. `RefreshAll()` більше не resurrect-ить legacy `FrontendPanel`; він лишається attached, але Collapsed+Disabled, а focus context не переходить у legacy frontend. Runtime acceptance pending. |')
ledger.write_text(lt, encoding="utf-8")
