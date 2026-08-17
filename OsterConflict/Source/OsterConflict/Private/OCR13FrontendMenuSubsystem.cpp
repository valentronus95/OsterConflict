#include "OCR13FrontendMenuSubsystem.h"

#include "OCGameUIRootWidget.h"
#include "OCPlayerController.h"
#include "OCPlayerUserSettings.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"

namespace
{
    UTextBlock* R13FrontendMakeMenuText(UObject* Outer, const FText& Text, int32 FontSize, bool bBright = true)
    {
        UTextBlock* Block = NewObject<UTextBlock>(Outer);
        if (!Block) return nullptr;
        Block->SetText(Text);
        Block->SetColorAndOpacity(FSlateColor(bBright
            ? FLinearColor(0.97f, 0.98f, 1.0f, 1.0f)
            : FLinearColor(0.72f, 0.78f, 0.84f, 1.0f)));
        FSlateFontInfo Font = Block->GetFont();
        Font.Size = FontSize;
        Block->SetFont(Font);
        return Block;
    }

    UButton* R13FrontendMakeMenuButton(UObject* Outer, UVerticalBox* Parent, const FText& Label)
    {
        if (!Outer || !Parent) return nullptr;
        UButton* Button = NewObject<UButton>(Outer);
        UTextBlock* Text = R13FrontendMakeMenuText(Outer, Label, 18, true);
        if (!Button || !Text) return nullptr;
        Text->SetJustification(ETextJustify::Center);
        Button->SetBackgroundColor(FLinearColor(0.018f, 0.032f, 0.047f, 0.98f));
        Button->AddChild(Text);
        if (UVerticalBoxSlot* Slot = Parent->AddChildToVerticalBox(Button))
        {
            Slot->SetPadding(FMargin(0.0f, 5.0f, 0.0f, 5.0f));
        }
        return Button;
    }

    void R13FrontendSetButtonLabel(UButton* Button, const FText& Label)
    {
        if (!Button) return;
        if (UTextBlock* Text = Cast<UTextBlock>(Button->GetContent())) Text->SetText(Label);
    }

    void R13FrontendSetButtonState(UButton* Button, bool bVisible)
    {
        if (!Button) return;
        Button->SetIsEnabled(bVisible);
        Button->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

bool UOCR13FrontendMenuSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13FrontendMenuSubsystem::Tick(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World) return;

    AOCPlayerController* PC = Cast<AOCPlayerController>(World->GetFirstPlayerController());
    if (!PC || !PC->IsLocalController()) return;

    UOCGameUIRootWidget* Root = nullptr;
    for (TObjectIterator<UOCGameUIRootWidget> It; It; ++It)
    {
        if (IsValid(*It) && It->GetWorld() == World && It->GetOwningPlayer() == PC)
        {
            Root = *It;
            break;
        }
    }
    if (!Root) return;

    EnsureFrontend(Root, PC);

    if (PC->IsFrontendMenuVisible() && !PC->IsSettingsVisible())
    {
        if (bGameplayStarted) ApplyPausePage();
        else ApplyPage();
        ForceMenuInput();
    }
}

void UOCR13FrontendMenuSubsystem::EnsureFrontend(UOCGameUIRootWidget* Root, AOCPlayerController* PC)
{
    if (!Root || !PC) return;

    UBorder* Panel = Cast<UBorder>(Root->GetWidgetFromName(TEXT("FrontendPanel")));
    if (!Panel) return;

    if (ActiveRoot.Get() == Root && ActivePanel.Get() == Panel && MenuBox.IsValid())
    {
        ActiveController = PC;
        return;
    }

    ActiveRoot = Root;
    ActiveController = PC;
    ActivePanel = Panel;
    Page = 0;
    bGameplayStarted = false;
    BuildFrontend(Root, Panel, PC);
}

void UOCR13FrontendMenuSubsystem::BuildFrontend(UOCGameUIRootWidget* Root, UBorder* Panel, AOCPlayerController* PC)
{
    if (!Root || !Panel || !PC) return;

    UVerticalBox* Box = NewObject<UVerticalBox>(Root, TEXT("R13_PlayerFrontend"));
    if (!Box) return;

    // IMPORTANT: keep exactly eight direct children. OCUIRuntimePolishSubsystem only rewrites the old nine-child
    // debug frontend, so this dedicated menu remains stable instead of having its labels/delegates cleared every tick.
    UTextBlock* Title = R13FrontendMakeMenuText(Root, NSLOCTEXT("OCR13Frontend", "Title", "OSTER CONFLICT"), 34, true);
    UTextBlock* Subtitle = R13FrontendMakeMenuText(Root, NSLOCTEXT("OCR13Frontend", "Subtitle", "ОСТЕР  •  ГОЛОВНЕ МЕНЮ"), 16, false);
    if (!Title || !Subtitle) return;
    Box->AddChildToVerticalBox(Title)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));
    Box->AddChildToVerticalBox(Subtitle)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 12.0f));

    UButton* Primary = R13FrontendMakeMenuButton(Root, Box, NSLOCTEXT("OCR13Frontend", "Start", "СТАРТ"));
    UButton* Secondary = R13FrontendMakeMenuButton(Root, Box, NSLOCTEXT("OCR13Frontend", "Local", "ЛОКАЛЬНА ГРА"));
    UButton* Network = R13FrontendMakeMenuButton(Root, Box, NSLOCTEXT("OCR13Frontend", "Network", "МЕРЕЖЕВА ГРА"));

    UVerticalBox* Fields = NewObject<UVerticalBox>(Root, TEXT("R13_FrontendFields"));
    UEditableTextBox* Username = NewObject<UEditableTextBox>(Root, TEXT("R13_Username"));
    UEditableTextBox* Address = NewObject<UEditableTextBox>(Root, TEXT("R13_ServerAddress"));
    UTextBlock* Status = R13FrontendMakeMenuText(Root, FText::GetEmpty(), 12, false);
    if (!Fields || !Username || !Address || !Status) return;

    const UOCPlayerUserSettings* Prefs = UOCPlayerUserSettings::Get();
    Username->SetHintText(NSLOCTEXT("OCR13Frontend", "UsernameHint", "Ім'я гравця"));
    Username->SetText(FText::FromString(Prefs ? Prefs->GetSavedUsername() : FString(TEXT("Player"))));
    Address->SetHintText(NSLOCTEXT("OCR13Frontend", "AddressHint", "IP:порт сервера"));
    Address->SetText(FText::FromString(Prefs ? Prefs->GetLastServerAddress() : FString(TEXT("127.0.0.1:7777"))));
    Fields->AddChildToVerticalBox(Username)->SetPadding(FMargin(0.0f, 4.0f));
    Fields->AddChildToVerticalBox(Address)->SetPadding(FMargin(0.0f, 4.0f));
    Fields->AddChildToVerticalBox(Status)->SetPadding(FMargin(0.0f, 7.0f, 0.0f, 2.0f));
    Box->AddChildToVerticalBox(Fields)->SetPadding(FMargin(0.0f, 4.0f));

    UButton* Settings = R13FrontendMakeMenuButton(Root, Box, NSLOCTEXT("OCR13Frontend", "Settings", "НАЛАШТУВАННЯ"));
    UButton* Quit = R13FrontendMakeMenuButton(Root, Box, NSLOCTEXT("OCR13Frontend", "Quit", "ВИЙТИ З ГРИ"));
    if (!Primary || !Secondary || !Network || !Settings || !Quit) return;

    Primary->OnClicked.AddDynamic(this, &UOCR13FrontendMenuSubsystem::OnPrimaryClicked);
    Secondary->OnClicked.AddDynamic(this, &UOCR13FrontendMenuSubsystem::OnSecondaryClicked);
    Network->OnClicked.AddDynamic(this, &UOCR13FrontendMenuSubsystem::OnNetworkClicked);
    Settings->OnClicked.AddDynamic(this, &UOCR13FrontendMenuSubsystem::OnSettingsClicked);
    Quit->OnClicked.AddDynamic(this, &UOCR13FrontendMenuSubsystem::OnQuitClicked);

    Panel->SetContent(Box);
    Panel->SetIsEnabled(true);
    Panel->SetVisibility(ESlateVisibility::Visible);
    Panel->SetBrushColor(FLinearColor(0.006f, 0.012f, 0.020f, 0.94f));
    Panel->SetPadding(FMargin(30.0f));

    MenuBox = Box;
    TitleText = Title;
    SubtitleText = Subtitle;
    FieldsBox = Fields;
    UsernameEntry = Username;
    AddressEntry = Address;
    StatusText = Status;
    PrimaryButton = Primary;
    SecondaryButton = Secondary;
    NetworkButton = Network;
    SettingsButton = Settings;
    QuitButton = Quit;

    ApplyPage();
}

void UOCR13FrontendMenuSubsystem::ApplyPage()
{
    if (!MenuBox.IsValid()) return;

    if (Page == 0)
    {
        TitleText->SetText(NSLOCTEXT("OCR13Frontend", "MainTitle", "OSTER CONFLICT"));
        SubtitleText->SetText(NSLOCTEXT("OCR13Frontend", "MainSubtitle", "ОСТЕР  •  ГОЛОВНЕ МЕНЮ"));
        SubtitleText->SetVisibility(ESlateVisibility::Visible);
        FieldsBox->SetVisibility(ESlateVisibility::Collapsed);
        R13FrontendSetButtonLabel(PrimaryButton.Get(), NSLOCTEXT("OCR13Frontend", "MainStart", "СТАРТ"));
        R13FrontendSetButtonLabel(SecondaryButton.Get(), NSLOCTEXT("OCR13Frontend", "MainLocal", "ЛОКАЛЬНА ГРА"));
        R13FrontendSetButtonLabel(NetworkButton.Get(), NSLOCTEXT("OCR13Frontend", "MainNetwork", "МЕРЕЖЕВА ГРА"));
        R13FrontendSetButtonLabel(SettingsButton.Get(), NSLOCTEXT("OCR13Frontend", "MainSettings", "НАЛАШТУВАННЯ"));
        R13FrontendSetButtonLabel(QuitButton.Get(), NSLOCTEXT("OCR13Frontend", "MainQuit", "ВИЙТИ З ГРИ"));
        R13FrontendSetButtonState(PrimaryButton.Get(), true);
        R13FrontendSetButtonState(SecondaryButton.Get(), true);
        R13FrontendSetButtonState(NetworkButton.Get(), true);
        R13FrontendSetButtonState(SettingsButton.Get(), true);
        R13FrontendSetButtonState(QuitButton.Get(), true);
    }
    else if (Page == 1)
    {
        TitleText->SetText(NSLOCTEXT("OCR13Frontend", "LocalTitle", "ЛОКАЛЬНА ГРА"));
        SubtitleText->SetText(NSLOCTEXT("OCR13Frontend", "LocalSubtitle", "ВКАЖІТЬ ІМ'Я ТА ЗАПУСТІТЬ МАТЧ"));
        SubtitleText->SetVisibility(ESlateVisibility::Visible);
        FieldsBox->SetVisibility(ESlateVisibility::Visible);
        UsernameEntry->SetVisibility(ESlateVisibility::Visible);
        AddressEntry->SetVisibility(ESlateVisibility::Collapsed);
        StatusText->SetText(NSLOCTEXT("OCR13Frontend", "LocalStatus", "Conquest • 15 ботів • локальний сервер"));
        R13FrontendSetButtonLabel(PrimaryButton.Get(), NSLOCTEXT("OCR13Frontend", "StartLocal", "ПОЧАТИ ЛОКАЛЬНУ ГРУ"));
        R13FrontendSetButtonLabel(SecondaryButton.Get(), NSLOCTEXT("OCR13Frontend", "BackLocal", "НАЗАД"));
        R13FrontendSetButtonState(PrimaryButton.Get(), true);
        R13FrontendSetButtonState(SecondaryButton.Get(), true);
        R13FrontendSetButtonState(NetworkButton.Get(), false);
        R13FrontendSetButtonState(SettingsButton.Get(), false);
        R13FrontendSetButtonState(QuitButton.Get(), false);
    }
    else
    {
        TitleText->SetText(NSLOCTEXT("OCR13Frontend", "NetworkTitle", "МЕРЕЖЕВА ГРА"));
        SubtitleText->SetText(NSLOCTEXT("OCR13Frontend", "NetworkSubtitle", "ПРЯМЕ ПІДКЛЮЧЕННЯ ДО СЕРВЕРА"));
        SubtitleText->SetVisibility(ESlateVisibility::Visible);
        FieldsBox->SetVisibility(ESlateVisibility::Visible);
        UsernameEntry->SetVisibility(ESlateVisibility::Visible);
        AddressEntry->SetVisibility(ESlateVisibility::Visible);
        StatusText->SetText(NSLOCTEXT("OCR13Frontend", "NetworkStatus", "Формат адреси: 127.0.0.1:7777"));
        R13FrontendSetButtonLabel(PrimaryButton.Get(), NSLOCTEXT("OCR13Frontend", "Connect", "ПІДКЛЮЧИТИСЯ"));
        R13FrontendSetButtonLabel(SecondaryButton.Get(), NSLOCTEXT("OCR13Frontend", "BackNetwork", "НАЗАД"));
        R13FrontendSetButtonState(PrimaryButton.Get(), true);
        R13FrontendSetButtonState(SecondaryButton.Get(), true);
        R13FrontendSetButtonState(NetworkButton.Get(), false);
        R13FrontendSetButtonState(SettingsButton.Get(), false);
        R13FrontendSetButtonState(QuitButton.Get(), false);
    }
}

void UOCR13FrontendMenuSubsystem::ApplyPausePage()
{
    if (!MenuBox.IsValid()) return;
    TitleText->SetText(NSLOCTEXT("OCR13Frontend", "PauseTitle", "МЕНЮ ГРИ"));
    SubtitleText->SetText(NSLOCTEXT("OCR13Frontend", "PauseSubtitle", "ESC  •  ПРОДОВЖИТИ ГРУ"));
    SubtitleText->SetVisibility(ESlateVisibility::Visible);
    FieldsBox->SetVisibility(ESlateVisibility::Collapsed);
    R13FrontendSetButtonState(PrimaryButton.Get(), false);
    R13FrontendSetButtonState(SecondaryButton.Get(), false);
    R13FrontendSetButtonState(NetworkButton.Get(), false);
    R13FrontendSetButtonLabel(SettingsButton.Get(), NSLOCTEXT("OCR13Frontend", "PauseSettings", "НАЛАШТУВАННЯ"));
    R13FrontendSetButtonLabel(QuitButton.Get(), NSLOCTEXT("OCR13Frontend", "PauseLeave", "ВИЙТИ В ГОЛОВНЕ МЕНЮ"));
    R13FrontendSetButtonState(SettingsButton.Get(), true);
    R13FrontendSetButtonState(QuitButton.Get(), true);
}

void UOCR13FrontendMenuSubsystem::OnPrimaryClicked()
{
    if (Page == 2)
    {
        AOCPlayerController* PC = ActiveController.Get();
        if (!PC) return;
        const FString Username = UsernameEntry.IsValid() ? UsernameEntry->GetText().ToString() : FString(TEXT("Player"));
        const FString Address = AddressEntry.IsValid() ? AddressEntry->GetText().ToString() : FString(TEXT("127.0.0.1:7777"));
        bGameplayStarted = true;
        PC->UIConnect(Address, Username);
        return;
    }

    StartLocalGameplay();
}

void UOCR13FrontendMenuSubsystem::OnSecondaryClicked()
{
    Page = (Page == 0) ? 1 : 0;
    ApplyPage();
    ForceMenuInput();
}

void UOCR13FrontendMenuSubsystem::OnNetworkClicked()
{
    Page = 2;
    ApplyPage();
    ForceMenuInput();
}

void UOCR13FrontendMenuSubsystem::OnSettingsClicked()
{
    if (AOCPlayerController* PC = ActiveController.Get()) PC->UIOpenSettings();
}

void UOCR13FrontendMenuSubsystem::OnQuitClicked()
{
    AOCPlayerController* PC = ActiveController.Get();
    if (!PC) return;

    if (bGameplayStarted)
    {
        bGameplayStarted = false;
        Page = 0;
        PC->DisconnectFromServer();
        return;
    }

    UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
}

void UOCR13FrontendMenuSubsystem::StartLocalGameplay()
{
    AOCPlayerController* PC = ActiveController.Get();
    if (!PC) return;

    if (UsernameEntry.IsValid()) PC->SetNickname(UsernameEntry->GetText().ToString());
    bGameplayStarted = true;

    if (PC->GetNetMode() != NM_Standalone)
    {
        if (PC->IsFrontendMenuVisible()) PC->UIToggleFrontend();
        return;
    }

    PC->ConsoleCommand(TEXT("open /Game/Maps/OsterConflict_Runtime?listen?Mode=Conquest?Bots=15?Population=16?BotFill=1?MaxPlayers=16"));
}

void UOCR13FrontendMenuSubsystem::ForceMenuInput()
{
    AOCPlayerController* PC = ActiveController.Get();
    if (!PC || !MenuBox.IsValid()) return;

    PC->SetIgnoreMoveInput(true);
    PC->SetIgnoreLookInput(true);
    PC->bShowMouseCursor = true;

    FInputModeGameAndUI Mode;
    Mode.SetHideCursorDuringCapture(false);
    Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    Mode.SetWidgetToFocus(MenuBox->TakeWidget());
    PC->SetInputMode(Mode);
}

TStatId UOCR13FrontendMenuSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13FrontendMenuSubsystem, STATGROUP_Tickables);
}
