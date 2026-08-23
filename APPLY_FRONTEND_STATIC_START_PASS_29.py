from pathlib import Path

ROOT = Path(__file__).resolve().parent
MENU = ROOT / "OsterConflict/Source/OsterConflict/Private/OCR13FrontendMenuSubsystem.cpp"
ROOT_UI = ROOT / "OsterConflict/Source/OsterConflict/Private/OCGameUIRootWidget.cpp"
LAUNCH = ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd"
LEDGER = ROOT / "OSTER_CONFLICT_WORK_LEDGER.md"

menu = MENU.read_text(encoding="utf-8")
root_ui = ROOT_UI.read_text(encoding="utf-8")
launch = LAUNCH.read_text(encoding="utf-8")
ledger = LEDGER.read_text(encoding="utf-8")

old_pending = '''        if (PendingPage != INDEX_NONE)\n        {\n            const int32 NewPage = PendingPage;\n            PendingPage = INDEX_NONE;\n            Page = NewPage;\n            LastAppliedPage = INDEX_NONE;\n            bPausePageApplied = false;\n            UE_LOG(LogTemp, Display, TEXT("PASS24_FRONTEND_PAGE_TRANSITION_BEGIN page=%d"), Page);\n            UE_LOG(LogTemp, Display, TEXT("PASS26_FRONTEND_ACTION_EXECUTE action=page_%d"), Page);\n            ApplyPage();\n            ForceMenuInput();\n            if (Page == 1)\n            {\n                UE_LOG(LogTemp, Display, TEXT("PASS14_MAIN_START_OPENS_SERVER_SETUP"));\n            }\n            UE_LOG(LogTemp, Display, TEXT("PASS24_FRONTEND_PAGE_TRANSITION_READY page=%d"), Page);\n        }\n        else if (bPendingHostedStart)\n'''
new_pending = '''        if (PendingPage != INDEX_NONE)\n        {\n            // Pass 29: runtime repeatedly crashed inside Slate immediately after the main-menu START\n            // path changed the live widget hierarchy. Page transitions are now forbidden in the R13\n            // startup shell. Keep the frontend structurally static and route actions directly instead.\n            const int32 BlockedPage = PendingPage;\n            PendingPage = INDEX_NONE;\n            UE_LOG(LogTemp, Error, TEXT("PASS29_UNSAFE_FRONTEND_PAGE_TRANSITION_BLOCKED page=%d"), BlockedPage);\n        }\n        if (bPendingHostedStart)\n'''
if old_pending not in menu:
    raise SystemExit("Pass 29 pending-page anchor not found")
menu = menu.replace(old_pending, new_pending, 1)

old_primary = '''    if (Page == 0)\n    {\n        PendingPage = 1;\n        ArmDeferredActionFence();\n        UE_LOG(LogTemp, Display, TEXT("PASS24_FRONTEND_PAGE_TRANSITION_QUEUED page=1"));\n        UE_LOG(LogTemp, Display, TEXT("PASS26_FRONTEND_ACTION_QUEUED action=page_1"));\n        return;\n    }\n'''
new_primary = '''    if (Page == 0)\n    {\n        // Pass 29: START no longer turns the already-live Slate tree into a different page. The\n        // repeated crash is synchronized with that structural transition, not with compilation.\n        // Start the local hosted session directly from the stable main menu using the already-created\n        // default/saved fields (16 max, 0 bots, Normal unless later changed by a dedicated safe UI).\n        bPendingHostedStart = true;\n        ArmDeferredActionFence();\n        UE_LOG(LogTemp, Display, TEXT("PASS29_MAIN_START_DIRECT_HOST_QUEUED"));\n        UE_LOG(LogTemp, Display, TEXT("PASS26_FRONTEND_ACTION_QUEUED action=start_host"));\n        return;\n    }\n'''
if old_primary not in menu:
    raise SystemExit("Pass 29 primary anchor not found")
menu = menu.replace(old_primary, new_primary, 1)

old_secondary = '''void UOCR13FrontendMenuSubsystem::OnSecondaryClicked()\n{\n    if (bPauseMenuActive || bLocalTravelPending || Page == 0 || HasPendingFrontendAction()) return;\n    PendingPage = 0;\n    ArmDeferredActionFence();\n    UE_LOG(LogTemp, Display, TEXT("PASS24_FRONTEND_PAGE_TRANSITION_QUEUED page=0"));\n    UE_LOG(LogTemp, Display, TEXT("PASS26_FRONTEND_ACTION_QUEUED action=page_0"));\n}\n'''
new_secondary = '''void UOCR13FrontendMenuSubsystem::OnSecondaryClicked()\n{\n    // Pass 29: no runtime page mutation exists in the startup shell anymore. Secondary is retained\n    // only for compatibility with old constructed widgets and must never alter the live Slate tree.\n    UE_LOG(LogTemp, Display, TEXT("PASS29_SECONDARY_IGNORED_STATIC_FRONTEND"));\n}\n'''
if old_secondary not in menu:
    raise SystemExit("Pass 29 secondary anchor not found")
menu = menu.replace(old_secondary, new_secondary, 1)

old_network = '''void UOCR13FrontendMenuSubsystem::OnNetworkClicked()\n{\n    if (bPauseMenuActive || bLocalTravelPending || HasPendingFrontendAction()) return;\n    PendingPage = 2;\n    ArmDeferredActionFence();\n    UE_LOG(LogTemp, Display, TEXT("PASS24_FRONTEND_PAGE_TRANSITION_QUEUED page=2"));\n    UE_LOG(LogTemp, Display, TEXT("PASS26_FRONTEND_ACTION_QUEUED action=page_2"));\n}\n'''
new_network = '''void UOCR13FrontendMenuSubsystem::OnNetworkClicked()\n{\n    if (bPauseMenuActive || bLocalTravelPending || HasPendingFrontendAction()) return;\n    // Pass 29: keep the startup Slate hierarchy immutable. Network uses the saved/default address\n    // already present in AddressEntry instead of opening the crash-prone structural page transition.\n    bPendingNetworkConnect = true;\n    ArmDeferredActionFence();\n    UE_LOG(LogTemp, Display, TEXT("PASS29_NETWORK_DIRECT_CONNECT_QUEUED"));\n    UE_LOG(LogTemp, Display, TEXT("PASS26_FRONTEND_ACTION_QUEUED action=network_connect"));\n}\n'''
if old_network not in menu:
    raise SystemExit("Pass 29 network anchor not found")
menu = menu.replace(old_network, new_network, 1)

old_host_marker = '''    UE_LOG(LogTemp, Display,\n        TEXT("PASS14_HOST_TRAVEL_BEGIN max_players=%d bots=%d difficulty=%s"), MaxPlayers, Bots, *Difficulty);\n    PC->ConsoleCommand(Travel);\n'''
new_host_marker = '''    UE_LOG(LogTemp, Display,\n        TEXT("PASS14_HOST_TRAVEL_BEGIN max_players=%d bots=%d difficulty=%s"), MaxPlayers, Bots, *Difficulty);\n    UE_LOG(LogTemp, Display, TEXT("PASS29_STATIC_FRONTEND_HOST_TRAVEL_EXECUTE"));\n    PC->ConsoleCommand(Travel);\n'''
if old_host_marker not in menu:
    raise SystemExit("Pass 29 host marker anchor not found")
menu = menu.replace(old_host_marker, new_host_marker, 1)

old_root = '''    const bool bR13OwnsFrontend = FParse::Param(FCommandLine::Get(), TEXT("Frontend"));\n    const bool bShowLegacyFrontend = bFrontend && !bR13OwnsFrontend;\n    SettingsPanel->SetVisibility(bSettings?ESlateVisibility::Visible:ESlateVisibility::Collapsed);\n'''
new_root = '''    const bool bR13OwnsFrontend = FParse::Param(FCommandLine::Get(), TEXT("Frontend"));\n    const bool bShowLegacyFrontend = bFrontend && !bR13OwnsFrontend;\n\n    // Pass 29: while the R13 startup shell owns the frontend, the native root must not mutate hidden\n    // legacy Slate subtrees every 0.20 s. Pass 28 hid the duplicate panel, but RefreshAll still touched\n    // settings/deployment/scoreboard/chat/admin and refreshed their text while the R13 click transition\n    // was running in the same UUserWidget. Freeze that entire legacy branch until settings/travel takes\n    // ownership again. Only collapse a panel if its state actually differs, then leave Slate alone.\n    if (bR13OwnsFrontend && bFrontend)\n    {\n        auto FreezeLegacyPanel = [](UWidget* Widget)\n        {\n            if (!Widget) return;\n            if (Widget->GetVisibility() != ESlateVisibility::Collapsed)\n                Widget->SetVisibility(ESlateVisibility::Collapsed);\n            if (Widget->GetIsEnabled())\n                Widget->SetIsEnabled(false);\n        };\n        FreezeLegacyPanel(SettingsPanel);\n        FreezeLegacyPanel(FrontendPanel);\n        FreezeLegacyPanel(DeploymentPanel);\n        FreezeLegacyPanel(ScoreboardPanel);\n        FreezeLegacyPanel(ChatPanel);\n        FreezeLegacyPanel(AdminPanel);\n        LastFocusContext = 0;\n        return;\n    }\n\n    SettingsPanel->SetVisibility(bSettings?ESlateVisibility::Visible:ESlateVisibility::Collapsed);\n'''
if old_root not in root_ui:
    raise SystemExit("Pass 29 root freeze anchor not found")
root_ui = root_ui.replace(old_root, new_root, 1)

old_diag = 'findstr /C:"PASS27_" /C:"PASS26_" /C:"PASS25_" /C:"PASS24_" /C:"R13 frontend:" "%PLAYTEST_LOG%"'
new_diag = 'findstr /C:"PASS29_" /C:"PASS28_" /C:"PASS27_" /C:"PASS26_" /C:"PASS25_" /C:"PASS24_" /C:"R13 frontend:" "%PLAYTEST_LOG%"'
if old_diag not in launch:
    raise SystemExit("Pass 29 launcher diagnostic anchor not found")
launch = launch.replace(old_diag, new_diag, 1)

ledger = ledger.replace(
    '- Active correction branch: `fix/frontend-single-owner-pass-28-20260823` → `main`',
    '- Active correction branch: `fix/frontend-static-start-pass-29-20260823` → `main`', 1)
ledger = ledger.replace(
    '| UI-MENU-001 | Головне меню стабільне | ≥5 | IN_PROGRESS |',
    '| UI-MENU-001 | Головне меню стабільне | ≥7 | IN_PROGRESS |', 1)
ledger += '''\n\n## 2026-08-23 — Pass 29 frontend crash localization\n\n- Pass 28 runtime again reproduced the exact Slate/SlateCore `Array index out of bounds: -808103970 into an array of size 0` immediately after pressing main-menu START.\n- The repeat now isolates the failing pre-travel path to R13 `Page 0 -> Page 1`: `ApplyPage()` performs many live `SetVisibility`, parent `Collapsed`, editable-field activation, panel padding/geometry and label mutations inside one already-built Slate hierarchy. No gameplay/server travel has started yet at that point.\n- Pass 29 removes runtime page transitions from the startup shell. START queues hosted travel directly from the static main menu; NETWORK queues direct connect from the saved/default address. `PendingPage` execution is fail-closed and logs `PASS29_UNSAFE_FRONTEND_PAGE_TRANSITION_BLOCKED` instead of mutating Slate.\n- `UOCGameUIRootWidget::RefreshAll()` now freezes every legacy panel while R13 owns the frontend, eliminating the remaining 0.20 s hidden-tree churn in the same UUserWidget.\n- Status: CODED_UNTESTED until UE 5.8 runtime confirms `PASS29_MAIN_START_DIRECT_HOST_QUEUED` -> `PASS29_STATIC_FRONTEND_HOST_TRAVEL_EXECUTE` without the Slate assertion.\n'''

MENU.write_text(menu, encoding="utf-8")
ROOT_UI.write_text(root_ui, encoding="utf-8")
LAUNCH.write_text(launch, encoding="utf-8")
LEDGER.write_text(ledger, encoding="utf-8")
