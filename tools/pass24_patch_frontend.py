from pathlib import Path

HEADER = Path('OsterConflict/Source/OsterConflict/Public/OCR13FrontendMenuSubsystem.h')
CPP = Path('OsterConflict/Source/OsterConflict/Private/OCR13FrontendMenuSubsystem.cpp')


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise SystemExit(f'{label}: expected 1 match, found {count}')
    return text.replace(old, new, 1)

h = HEADER.read_text(encoding='utf-8')
h = replace_once(
    h,
    '    void StartHostedGameplay();\n    void ForceMenuInput();',
    '    void StartHostedGameplay();\n    void StartNetworkGameplay();\n    void ForceMenuInput();',
    'header helper declaration')
h = replace_once(
    h,
    '    int32 Page = 0; // 0 main, 1 create server, 2 join server\n    bool bGameplayStarted = false;\n    bool bPauseMenuActive = false;\n    bool bLocalTravelPending = false; // keep approved frontend intact until the server world replaces it',
    '    int32 Page = 0; // 0 main, 1 create server, 2 join server\n    int32 PendingPage = INDEX_NONE; // Pass 24: structural Slate changes are applied from Tick, never inside input callbacks\n    int32 LastAppliedPage = INDEX_NONE;\n    bool bPendingHostedStart = false;\n    bool bPendingNetworkConnect = false;\n    bool bGameplayStarted = false;\n    bool bPauseMenuActive = false;\n    bool bLocalTravelPending = false; // keep approved frontend intact until the server world replaces it',
    'header pending state')
HEADER.write_text(h, encoding='utf-8')

c = CPP.read_text(encoding='utf-8')

c = replace_once(
    c,
    '        Field->SetHintText(Hint);\n        Field->SetText(FText::FromString(Value));\n        if (UVerticalBoxSlot* Slot = Parent->AddChildToVerticalBox(Field))',
    '''        Field->SetHintText(Hint);\n        Field->SetText(FText::FromString(Value));\n\n        // Pass 24: use an explicit game-owned dark style instead of the editor/default white field look.\n        // Keeping all brushes resource-stable while tinting them avoids constructing transient Slate resources\n        // during the main-menu -> server-setup transition.\n        FEditableTextBoxStyle FieldStyle = Field->GetWidgetStyle();\n        FieldStyle.BackgroundColor = FSlateColor(FLinearColor(0.045f, 0.052f, 0.061f, 1.0f));\n        FieldStyle.ForegroundColor = FSlateColor(FLinearColor(0.93f, 0.93f, 0.91f, 1.0f));\n        FieldStyle.FocusedForegroundColor = FSlateColor(FLinearColor::White);\n        FieldStyle.ReadOnlyForegroundColor = FSlateColor(FLinearColor(0.62f, 0.62f, 0.60f, 1.0f));\n        FieldStyle.BackgroundImageNormal.TintColor = FSlateColor(FLinearColor(0.055f, 0.062f, 0.072f, 1.0f));\n        FieldStyle.BackgroundImageHovered.TintColor = FSlateColor(FLinearColor(0.075f, 0.085f, 0.098f, 1.0f));\n        FieldStyle.BackgroundImageFocused.TintColor = FSlateColor(FLinearColor(0.085f, 0.098f, 0.115f, 1.0f));\n        FieldStyle.BackgroundImageReadOnly.TintColor = FSlateColor(FLinearColor(0.040f, 0.046f, 0.054f, 1.0f));\n        FieldStyle.Padding = FMargin(14.0f, 10.0f);\n        Field->SetWidgetStyle(FieldStyle);\n        Field->SetMinimumDesiredWidth(420.0f);\n\n        if (UVerticalBoxSlot* Slot = Parent->AddChildToVerticalBox(Field))''',
    'dark fields')

c = replace_once(
    c,
    '    EnsureFrontend(Root, PC);\n    SuppressLegacyFrontendLayers(Root);\n\n    const bool bSettingsVisible = PC->IsSettingsVisible();',
    '''    EnsureFrontend(Root, PC);\n    SuppressLegacyFrontendLayers(Root);\n\n    // Pass 24: never rebuild Slate hierarchy from UButton input delegates. Apply queued navigation\n    // on the next world Tick, after Slate has finished routing the click that requested it.\n    if (PendingPage != INDEX_NONE)\n    {\n        const int32 NewPage = PendingPage;\n        PendingPage = INDEX_NONE;\n        Page = NewPage;\n        LastAppliedPage = INDEX_NONE;\n        UE_LOG(LogTemp, Display, TEXT("PASS24_FRONTEND_PAGE_TRANSITION_BEGIN page=%d"), Page);\n        ApplyPage();\n        ForceMenuInput();\n        if (Page == 1)\n        {\n            UE_LOG(LogTemp, Display, TEXT("PASS14_MAIN_START_OPENS_SERVER_SETUP"));\n        }\n        UE_LOG(LogTemp, Display, TEXT("PASS24_FRONTEND_PAGE_TRANSITION_READY page=%d"), Page);\n    }\n\n    if (bPendingHostedStart)\n    {\n        bPendingHostedStart = false;\n        UE_LOG(LogTemp, Display, TEXT("PASS24_HOST_START_DEFERRED_EXECUTE"));\n        StartHostedGameplay();\n        return;\n    }\n\n    if (bPendingNetworkConnect)\n    {\n        bPendingNetworkConnect = false;\n        UE_LOG(LogTemp, Display, TEXT("PASS24_NETWORK_CONNECT_DEFERRED_EXECUTE"));\n        StartNetworkGameplay();\n        return;\n    }\n\n    const bool bSettingsVisible = PC->IsSettingsVisible();''',
    'deferred tick actions')

c = replace_once(
    c,
    '    Page = 0;\n    bGameplayStarted = false;\n    bPauseMenuActive = false;\n    bLocalTravelPending = false;\n    BuildFrontend(Root, PC);',
    '    Page = 0;\n    PendingPage = INDEX_NONE;\n    LastAppliedPage = INDEX_NONE;\n    bPendingHostedStart = false;\n    bPendingNetworkConnect = false;\n    bGameplayStarted = false;\n    bPauseMenuActive = false;\n    bLocalTravelPending = false;\n    BuildFrontend(Root, PC);',
    'frontend reset')

c = replace_once(
    c,
    '    Primary->OnPressed.AddDynamic(this, &UOCR13FrontendMenuSubsystem::OnPrimaryClicked);\n    Secondary->OnPressed.AddDynamic(this, &UOCR13FrontendMenuSubsystem::OnSecondaryClicked);\n    Network->OnPressed.AddDynamic(this, &UOCR13FrontendMenuSubsystem::OnNetworkClicked);\n    Settings->OnPressed.AddDynamic(this, &UOCR13FrontendMenuSubsystem::OnSettingsClicked);\n    Quit->OnPressed.AddDynamic(this, &UOCR13FrontendMenuSubsystem::OnQuitClicked);',
    '    Primary->OnClicked.AddDynamic(this, &UOCR13FrontendMenuSubsystem::OnPrimaryClicked);\n    Secondary->OnClicked.AddDynamic(this, &UOCR13FrontendMenuSubsystem::OnSecondaryClicked);\n    Network->OnClicked.AddDynamic(this, &UOCR13FrontendMenuSubsystem::OnNetworkClicked);\n    Settings->OnClicked.AddDynamic(this, &UOCR13FrontendMenuSubsystem::OnSettingsClicked);\n    Quit->OnClicked.AddDynamic(this, &UOCR13FrontendMenuSubsystem::OnQuitClicked);',
    'clicked bindings')

c = replace_once(
    c,
    'void UOCR13FrontendMenuSubsystem::ApplyPage()\n{\n    if (!MenuBox.IsValid()) return;\n    bPauseMenuActive = false;',
    'void UOCR13FrontendMenuSubsystem::ApplyPage()\n{\n    if (!MenuBox.IsValid() || LastAppliedPage == Page) return;\n    bPauseMenuActive = false;',
    'apply page dedupe')

c = replace_once(
    c,
    '            ? FLinearColor::Transparent\n            : FLinearColor(0.0f, 0.0f, 0.0f, 0.36f));',
    '            ? FLinearColor::Transparent\n            : FLinearColor(0.025f, 0.030f, 0.036f, 0.96f));',
    'opaque setup panel')

c = replace_once(
    c,
    '        R13FrontendSetButtonState(SettingsButton.Get(), true);\n        R13FrontendSetButtonState(QuitButton.Get(), true);\n        return;',
    '        R13FrontendSetButtonState(SettingsButton.Get(), true);\n        R13FrontendSetButtonState(QuitButton.Get(), true);\n        LastAppliedPage = Page;\n        return;',
    'main page applied marker')

c = replace_once(
    c,
    '        R13FrontendSetButtonLabel(SecondaryButton.Get(), NSLOCTEXT("OCR13Frontend", "BackNetwork", "НАЗАД"));\n    }\n}\n\nvoid UOCR13FrontendMenuSubsystem::ApplyPausePage()',
    '        R13FrontendSetButtonLabel(SecondaryButton.Get(), NSLOCTEXT("OCR13Frontend", "BackNetwork", "НАЗАД"));\n    }\n\n    LastAppliedPage = Page;\n}\n\nvoid UOCR13FrontendMenuSubsystem::ApplyPausePage()',
    'non-main applied marker')

c = replace_once(
    c,
    'void UOCR13FrontendMenuSubsystem::ApplyPausePage()\n{\n    if (!MenuBox.IsValid()) return;\n    bPauseMenuActive = true;',
    'void UOCR13FrontendMenuSubsystem::ApplyPausePage()\n{\n    if (!MenuBox.IsValid()) return;\n    LastAppliedPage = INDEX_NONE;\n    bPauseMenuActive = true;',
    'pause invalidates normal page cache')

c = replace_once(
    c,
    '''    if (Page == 0)\n    {\n        Page = 1;\n        ApplyPage();\n        ForceMenuInput();\n        UE_LOG(LogTemp, Display, TEXT("PASS14_MAIN_START_OPENS_SERVER_SETUP"));\n        return;\n    }\n\n    if (Page == 1)\n    {\n        StartHostedGameplay();\n        return;\n    }\n\n    AOCPlayerController* PC = ActiveController.Get();\n    if (!PC) return;\n    const FString Username = UsernameEntry.IsValid() ? UsernameEntry->GetText().ToString() : FString(TEXT("Player"));\n    const FString Address = AddressEntry.IsValid() ? AddressEntry->GetText().ToString() : FString(TEXT("127.0.0.1:7777"));\n    if (UOCPlayerUserSettings* Prefs = UOCPlayerUserSettings::Get()) Prefs->SetFrontendIdentity(Username, Address);\n    bGameplayStarted = true;\n    ReleaseMenuInput();\n    SetPresentationVisibility(false, false, false);\n    PC->UIConnect(Address, Username);''',
    '''    if (Page == 0)\n    {\n        PendingPage = 1;\n        UE_LOG(LogTemp, Display, TEXT("PASS24_FRONTEND_PAGE_TRANSITION_QUEUED page=1"));\n        return;\n    }\n\n    if (Page == 1)\n    {\n        bPendingHostedStart = true;\n        UE_LOG(LogTemp, Display, TEXT("PASS24_HOST_START_DEFERRED_QUEUED"));\n        return;\n    }\n\n    bPendingNetworkConnect = true;\n    UE_LOG(LogTemp, Display, TEXT("PASS24_NETWORK_CONNECT_DEFERRED_QUEUED"));''',
    'primary callback deferral')

c = replace_once(
    c,
    '''void UOCR13FrontendMenuSubsystem::OnSecondaryClicked()\n{\n    if (bPauseMenuActive || bLocalTravelPending || Page == 0) return;\n    Page = 0;\n    ApplyPage();\n    ForceMenuInput();\n}''',
    '''void UOCR13FrontendMenuSubsystem::OnSecondaryClicked()\n{\n    if (bPauseMenuActive || bLocalTravelPending || Page == 0) return;\n    PendingPage = 0;\n    UE_LOG(LogTemp, Display, TEXT("PASS24_FRONTEND_PAGE_TRANSITION_QUEUED page=0"));\n}''',
    'secondary callback deferral')

c = replace_once(
    c,
    '''void UOCR13FrontendMenuSubsystem::OnNetworkClicked()\n{\n    if (bPauseMenuActive || bLocalTravelPending) return;\n    Page = 2;\n    ApplyPage();\n    ForceMenuInput();\n}''',
    '''void UOCR13FrontendMenuSubsystem::OnNetworkClicked()\n{\n    if (bPauseMenuActive || bLocalTravelPending) return;\n    PendingPage = 2;\n    UE_LOG(LogTemp, Display, TEXT("PASS24_FRONTEND_PAGE_TRANSITION_QUEUED page=2"));\n}''',
    'network callback deferral')

c = replace_once(
    c,
    'void UOCR13FrontendMenuSubsystem::StartHostedGameplay()\n{',
    '''void UOCR13FrontendMenuSubsystem::StartNetworkGameplay()\n{\n    AOCPlayerController* PC = ActiveController.Get();\n    if (!PC || bLocalTravelPending) return;\n\n    const FString Username = UsernameEntry.IsValid() ? UsernameEntry->GetText().ToString() : FString(TEXT("Player"));\n    const FString Address = AddressEntry.IsValid() ? AddressEntry->GetText().ToString() : FString(TEXT("127.0.0.1:7777"));\n    if (UOCPlayerUserSettings* Prefs = UOCPlayerUserSettings::Get()) Prefs->SetFrontendIdentity(Username, Address);\n    bGameplayStarted = true;\n    ReleaseMenuInput();\n    SetPresentationVisibility(false, false, false);\n    PC->UIConnect(Address, Username);\n}\n\nvoid UOCR13FrontendMenuSubsystem::StartHostedGameplay()\n{''',
    'network helper')

c = replace_once(
    c,
    '    FInputModeUIOnly Mode;\n    Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);\n    if (PrimaryButton.IsValid()) Mode.SetWidgetToFocus(PrimaryButton->TakeWidget());\n    PC->SetInputMode(Mode);',
    '    FInputModeUIOnly Mode;\n    Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);\n    // Pass 24: do not recreate/focus an SWidget every Tick. Mouse input remains UI-only;\n    // focus is acquired naturally by the clicked control.\n    PC->SetInputMode(Mode);',
    'remove repeated TakeWidget focus')

CPP.write_text(c, encoding='utf-8')
print('PASS24 patch applied successfully')
