#include "OCGameUIRootWidget.h"
#include "OCGameInstance.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/ScaleBox.h"
#include "Components/SizeBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Widget.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/GameUserSettings.h"
#include "InputCoreTypes.h"
#include "Input/Events.h"
#include "Input/Reply.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "OCAudioTypes.h"
#include "OCAudioUserSettings.h"
#include "OCCharacterVisualTypes.h"
#include "OCGameState.h"
#include "OCPlayerController.h"
#include "OCPlayerState.h"
#include "OCPlayerUserSettings.h"

#define LOCTEXT_NAMESPACE "OCGameUIRootWidget"

namespace
{
    const FLinearColor OCPanel(0.025f, 0.032f, 0.041f, 0.94f);
    const FLinearColor OCPanelAlt(0.045f, 0.055f, 0.066f, 0.96f);

    FString L(const FText& Text) { return Text.ToString(); }

    TArray<FString> QualityOptions(bool bAllowCustom = false)
    {
        TArray<FString> Out{ L(LOCTEXT("QualityLow", "Низька")), L(LOCTEXT("QualityMedium", "Середня")), L(LOCTEXT("QualityHigh", "Висока")), L(LOCTEXT("QualityEpic", "Епічна")), L(LOCTEXT("QualityCinematic", "Кінематографічна")) };
        if (bAllowCustom) Out.Add(L(LOCTEXT("QualityCustom", "Власна")));
        return Out;
    }

    int32 QualityFromString(const FString& Value)
    {
        if (Value == L(LOCTEXT("QualityLow", "Низька"))) return 0;
        if (Value == L(LOCTEXT("QualityMedium", "Середня"))) return 1;
        if (Value == L(LOCTEXT("QualityHigh", "Висока"))) return 2;
        if (Value == L(LOCTEXT("QualityEpic", "Епічна"))) return 3;
        if (Value == L(LOCTEXT("QualityCinematic", "Кінематографічна"))) return 4;
        return -1;
    }

    FString QualityToString(int32 Value)
    {
        switch (Value)
        {
            case 0: return L(LOCTEXT("QualityLow", "Низька"));
            case 1: return L(LOCTEXT("QualityMedium", "Середня"));
            case 2: return L(LOCTEXT("QualityHigh", "Висока"));
            case 3: return L(LOCTEXT("QualityEpic", "Епічна"));
            case 4: return L(LOCTEXT("QualityCinematic", "Кінематографічна"));
            default: return L(LOCTEXT("QualityCustom", "Власна"));
        }
    }

    FString WindowModeToString(EWindowMode::Type Mode)
    {
        if (Mode == EWindowMode::Fullscreen) return L(LOCTEXT("WindowFullscreen", "На весь екран"));
        if (Mode == EWindowMode::WindowedFullscreen) return L(LOCTEXT("WindowBorderless", "Без рамки"));
        return L(LOCTEXT("WindowWindowed", "У вікні"));
    }

    EWindowMode::Type WindowModeFromString(const FString& Mode)
    {
        if (Mode == L(LOCTEXT("WindowFullscreen", "На весь екран"))) return EWindowMode::Fullscreen;
        if (Mode == L(LOCTEXT("WindowBorderless", "Без рамки"))) return EWindowMode::WindowedFullscreen;
        return EWindowMode::Windowed;
    }

    FString DynamicRangeToString(EOCDynamicRange Range)
    {
        if (Range == EOCDynamicRange::Night) return L(LOCTEXT("DynamicNight", "Нічний / стиснений"));
        if (Range == EOCDynamicRange::High) return L(LOCTEXT("DynamicHigh", "Високий динамічний діапазон"));
        return L(LOCTEXT("DynamicStandard", "Стандартний"));
    }

    FString OutputModeToString(EOCAudioOutputMode Mode)
    {
        if (Mode == EOCAudioOutputMode::StereoSpeakers) return L(LOCTEXT("OutputStereo", "Стереодинаміки"));
        if (Mode == EOCAudioOutputMode::SpatialHeadphones) return L(LOCTEXT("OutputSpatial", "3D / просторові навушники"));
        return L(LOCTEXT("OutputHeadphones", "Навушники"));
    }

    EOCDynamicRange DynamicRangeFromString(const FString& Range)
    {
        if (Range == L(LOCTEXT("DynamicNight", "Нічний / стиснений"))) return EOCDynamicRange::Night;
        if (Range == L(LOCTEXT("DynamicHigh", "Високий динамічний діапазон"))) return EOCDynamicRange::High;
        return EOCDynamicRange::Standard;
    }

    EOCAudioOutputMode OutputModeFromString(const FString& Mode)
    {
        if (Mode == L(LOCTEXT("OutputStereo", "Стереодинаміки"))) return EOCAudioOutputMode::StereoSpeakers;
        if (Mode == L(LOCTEXT("OutputSpatial", "3D / просторові навушники"))) return EOCAudioOutputMode::SpatialHeadphones;
        return EOCAudioOutputMode::Headphones;
    }

    FText TeamText(EOCTeam Team)
    {
        switch (Team)
        {
            case EOCTeam::TeamOne: return LOCTEXT("TeamOne", "КОМАНДА 1");
            case EOCTeam::TeamTwo: return LOCTEXT("TeamTwo", "КОМАНДА 2");
            default: return LOCTEXT("TeamNeutral", "НЕЙТРАЛЬНО");
        }
    }

    FText RoleText(EOCPlayerRole Role)
    {
        switch (Role)
        {
            case EOCPlayerRole::Medic: return LOCTEXT("RoleMedic", "МЕДИК");
            case EOCPlayerRole::Engineer: return LOCTEXT("RoleEngineer", "ІНЖЕНЕР");
            case EOCPlayerRole::Support: return LOCTEXT("RoleSupport", "ПІДТРИМКА");
            default: return LOCTEXT("RoleRifleman", "ШТУРМОВИК");
        }
    }

    FText FactionText(EOCFactionArchetype Faction)
    {
        switch (Faction)
        {
            case EOCFactionArchetype::UASpecialUnit: return LOCTEXT("FactionUA", "УКРАЇНСЬКИЙ СПЕЦПІДРОЗДІЛ");
            case EOCFactionArchetype::MaskedFighters: return LOCTEXT("FactionMasked", "БІЙЦІ В МАСКАХ");
            case EOCFactionArchetype::USRangers: return LOCTEXT("FactionRangers", "РЕЙНДЖЕРИ");
            case EOCFactionArchetype::Insurgents: return LOCTEXT("FactionInsurgents", "ПОВСТАНЦІ");
            default: return LOCTEXT("FactionUnknown", "НЕВІДОМО");
        }
    }

    FText SquadText(int32 SquadId)
    {
        static const FText Names[] = {
            LOCTEXT("SquadAlpha", "АЛЬФА"), LOCTEXT("SquadBravo", "БРАВО"), LOCTEXT("SquadCharlie", "ЧАРЛІ"), LOCTEXT("SquadDelta", "ДЕЛЬТА"),
            LOCTEXT("SquadEcho", "ЕХО"), LOCTEXT("SquadFoxtrot", "ФОКСТРОТ"), LOCTEXT("SquadGolf", "ГОЛЬФ"), LOCTEXT("SquadHotel", "ХОТЕЛ")
        };
        return (SquadId >= 0 && SquadId < UE_ARRAY_COUNT(Names)) ? Names[SquadId] : LOCTEXT("SquadAuto", "АВТО");
    }

    FText ChatChannelTextValue(EOCChatChannel Channel)
    {
        switch (Channel)
        {
            case EOCChatChannel::Team: return LOCTEXT("ChatTeam", "КОМАНДА");
            case EOCChatChannel::Squad: return LOCTEXT("ChatSquad", "ГРУПА");
            default: return LOCTEXT("ChatAll", "УСІ");
        }
    }


    FText ActionText(FName ActionId)
    {
        if (ActionId == TEXT("MoveForward")) return LOCTEXT("ActionMoveForward", "Рух уперед");
        if (ActionId == TEXT("MoveBackward")) return LOCTEXT("ActionMoveBackward", "Рух назад");
        if (ActionId == TEXT("MoveLeft")) return LOCTEXT("ActionMoveLeft", "Рух ліворуч");
        if (ActionId == TEXT("MoveRight")) return LOCTEXT("ActionMoveRight", "Рух праворуч");
        if (ActionId == TEXT("Jump")) return LOCTEXT("ActionJump", "Стрибок");
        if (ActionId == TEXT("Sprint")) return LOCTEXT("ActionSprint", "Спринт");
        if (ActionId == TEXT("Crouch")) return LOCTEXT("ActionCrouch", "Присісти");
        if (ActionId == TEXT("Fire")) return LOCTEXT("ActionFire", "Вогонь");
        if (ActionId == TEXT("Aim")) return LOCTEXT("ActionAim", "Прицілювання");
        if (ActionId == TEXT("Reload")) return LOCTEXT("ActionReload", "Перезарядити");
        if (ActionId == TEXT("Interact")) return LOCTEXT("ActionInteract", "Взаємодія");
        if (ActionId == TEXT("ThrowGrenade")) return LOCTEXT("ActionGrenade", "Кинути гранату");
        if (ActionId == TEXT("Scoreboard")) return LOCTEXT("ActionScoreboard", "Таблиця рахунку");
        if (ActionId == TEXT("Chat")) return LOCTEXT("ActionChat", "Чат");
        return FText::FromName(ActionId);
    }

    TArray<FName> RebindActionIds()
    {
        return {
            TEXT("MoveForward"), TEXT("MoveBackward"), TEXT("MoveLeft"), TEXT("MoveRight"),
            TEXT("Jump"), TEXT("Sprint"), TEXT("Crouch"), TEXT("Fire"), TEXT("Aim"),
            TEXT("Reload"), TEXT("Interact"), TEXT("ThrowGrenade"), TEXT("Scoreboard"), TEXT("Chat")
        };
    }
}

TSharedRef<SWidget> UOCGameUIRootWidget::RebuildWidget()
{
    // R6: native C++ UMG must populate WidgetTree before UUserWidget builds the underlying Slate widget.
    // NativeConstruct runs after that Slate construction, so building the root there can leave a blank cached widget.
    if (!RootCanvas)
    {
        BuildWidgetTree();
    }
    return Super::RebuildWidget();
}

void UOCGameUIRootWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetIsFocusable(true);
    SyncSettingsWidgetsFromBackend();
    RefreshAll();
}

void UOCGameUIRootWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    RefreshAccumulator += InDeltaTime;
    if (RefreshAccumulator >= 0.20f)
    {
        RefreshAccumulator = 0.0f;
        RefreshAll();
    }
}

FReply UOCGameUIRootWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    const FKey NewKey = InKeyEvent.GetKey();
    if (!PendingRebindAction.IsNone())
    {
        if (NewKey == EKeys::Escape || NewKey == EKeys::Gamepad_FaceButton_Right)
        {
            PendingRebindAction = NAME_None;
            if (SettingsStatusText) SettingsStatusText->SetText(LOCTEXT("RebindCancelled", "Зміну клавіші скасовано."));
            return FReply::Handled();
        }
        if (NewKey.IsValid() && !NewKey.IsGamepadKey())
        {
            UOCPlayerUserSettings* Prefs = UOCPlayerUserSettings::Get();
            const FName OldKey = Prefs->GetKeyName(PendingRebindAction);
            const FName NewName = NewKey.GetFName();
            for (const FName OtherAction : RebindActionIds())
            {
                if (OtherAction != PendingRebindAction && Prefs->GetKeyName(OtherAction) == NewName)
                {
                    Prefs->SetKeyName(OtherAction, OldKey);
                    break;
                }
            }
            Prefs->SetKeyName(PendingRebindAction, NewName);
            if (SettingsStatusText)
            {
                FFormatNamedArguments Args;
                Args.Add(TEXT("Action"), ActionText(PendingRebindAction));
                Args.Add(TEXT("Key"), NewKey.GetDisplayName());
                SettingsStatusText->SetText(FText::Format(LOCTEXT("RebindPendingFmt", "{Action} → {Key} (очікує Застосувати/Зберегти)"), Args));
            }
            PendingRebindAction = NAME_None;
            RefreshKeyBindingLabels();
            return FReply::Handled();
        }
    }

    if (NewKey == EKeys::Escape || NewKey == EKeys::Gamepad_FaceButton_Right)
    {
        if (AOCPlayerController* PC = Cast<AOCPlayerController>(GetOwningPlayer()))
        {
            if (PC->IsChatInputActive()) PC->UIEndChatInput();
            else if (PC->IsSettingsVisible()) CancelPendingSettings();
            else if (PC->IsAdminPanelVisible()) PC->UICloseAdmin();
            else if (PC->IsDeploymentPanelVisible() && !PC->IsFrontendMenuVisible()) PC->UIToggleFrontend();
            else if (PC->IsFrontendMenuVisible()) PC->UIToggleFrontend();
            return FReply::Handled();
        }
    }
    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

UTextBlock* UOCGameUIRootWidget::MakeText(const FText& Text, int32 Size, bool bBoldHint)
{
    UTextBlock* Block = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    Block->SetText(Text);
    Block->SetColorAndOpacity(FSlateColor(bBoldHint ? FLinearColor::White : FLinearColor(0.78f, 0.82f, 0.86f, 1.0f)));
    FSlateFontInfo Font = Block->GetFont(); Font.Size = Size; Block->SetFont(Font);
    return Block;
}

UTextBlock* UOCGameUIRootWidget::MakeText(const FString& Text, int32 Size, bool bBoldHint)
{
    return MakeText(FText::FromString(Text), Size, bBoldHint);
}

UButton* UOCGameUIRootWidget::MakeButton(UVerticalBox* Parent, const FText& Label)
{
    UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
    Button->SetIsEnabled(true);
    Button->AddChild(MakeText(Label, 15, true));
    if (Parent)
        if (UVerticalBoxSlot* ButtonSlot = Parent->AddChildToVerticalBox(Button)) ButtonSlot->SetPadding(FMargin(0.0f, 3.0f));
    return Button;
}

UButton* UOCGameUIRootWidget::MakeButton(UVerticalBox* Parent, const FString& Label)
{
    return MakeButton(Parent, FText::FromString(Label));
}

UBorder* UOCGameUIRootWidget::MakePanel(const FString& DebugName, const FLinearColor& Color)
{
    UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), FName(*DebugName));
    Panel->SetBrushColor(Color); Panel->SetPadding(FMargin(18.0f)); return Panel;
}

void UOCGameUIRootWidget::PlacePanel(UBorder* Panel, const FVector2D& Position, const FVector2D& Size, int32 ZOrder)
{
    if (!RootCanvas || !Panel) return;
    if (UCanvasPanelSlot* CanvasSlot = RootCanvas->AddChildToCanvas(Panel))
    { CanvasSlot->SetPosition(Position); CanvasSlot->SetSize(Size); CanvasSlot->SetZOrder(ZOrder); }
}

USlider* UOCGameUIRootWidget::MakeSliderRow(UVerticalBox* Parent, const FText& Label, UTextBlock*& OutValueText)
{
    UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
    Parent->AddChildToVerticalBox(Row)->SetPadding(FMargin(0, 3));
    UTextBlock* L = MakeText(Label, 14, false); Row->AddChildToHorizontalBox(L)->SetPadding(FMargin(0, 6, 18, 0));
    USlider* Slider = WidgetTree->ConstructWidget<USlider>(USlider::StaticClass());
    if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(Slider)) { S->SetPadding(FMargin(0, 5, 12, 0)); S->SetSize(FSlateChildSize(ESlateSizeRule::Fill)); }
    OutValueText = MakeText(TEXT("0%"), 13, true); Row->AddChildToHorizontalBox(OutValueText)->SetPadding(FMargin(0, 6, 0, 0));
    return Slider;
}

UCheckBox* UOCGameUIRootWidget::MakeCheckRow(UVerticalBox* Parent, const FText& Label)
{
    UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
    Parent->AddChildToVerticalBox(Row)->SetPadding(FMargin(0, 2));
    UCheckBox* Check = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass());
    Row->AddChildToHorizontalBox(Check)->SetPadding(FMargin(0, 5, 12, 0));
    Row->AddChildToHorizontalBox(MakeText(Label, 14, false))->SetPadding(FMargin(0, 3, 0, 0));
    return Check;
}

UComboBoxString* UOCGameUIRootWidget::MakeComboRow(UVerticalBox* Parent, const FText& Label, const TArray<FString>& Options)
{
    UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
    Parent->AddChildToVerticalBox(Row)->SetPadding(FMargin(0, 3));
    Row->AddChildToHorizontalBox(MakeText(Label, 14, false))->SetPadding(FMargin(0, 5, 20, 0));
    UComboBoxString* Combo = WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass());
    for (const FString& Option : Options) Combo->AddOption(Option);
    if (Options.Num() > 0) Combo->SetSelectedOption(Options[0]);
    if (UHorizontalBoxSlot* ComboSlot = Row->AddChildToHorizontalBox(Combo)) ComboSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    return Combo;
}

UButton* UOCGameUIRootWidget::MakeRebindRow(UVerticalBox* Parent, FName ActionId, const FText& Label)
{
    UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
    Parent->AddChildToVerticalBox(Row)->SetPadding(FMargin(0, 2));
    Row->AddChildToHorizontalBox(MakeText(Label, 14, false))->SetPadding(FMargin(0, 5, 18, 0));
    UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
    UTextBlock* KeyText = MakeText(LOCTEXT("Unbound", "НЕ ПРИЗНАЧЕНО"), 14, true); Button->AddChild(KeyText);
    Row->AddChildToHorizontalBox(Button);
    KeyBindingTexts.Add(ActionId, KeyText);
    return Button;
}

void UOCGameUIRootWidget::BuildWidgetTree()
{
    // R6: all source-built panels are authored against a 1600x900 reference canvas.
    // Scale the whole logical canvas to the actual viewport so 720p/1080p/1440p/4K
    // do not clip the deployment/settings panels merely because their coordinates are fixed.
    UScaleBox* ViewportScale = WidgetTree->ConstructWidget<UScaleBox>(UScaleBox::StaticClass(), TEXT("OC_UI_ViewportScale"));
    ViewportScale->SetStretch(EStretch::ScaleToFit);

    USizeBox* ReferenceFrame = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("OC_UI_Reference1600x900"));
    ReferenceFrame->SetWidthOverride(1600.0f);
    ReferenceFrame->SetHeightOverride(900.0f);

    RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("OC_UI_Root"));
    ReferenceFrame->SetContent(RootCanvas);
    ViewportScale->SetContent(ReferenceFrame);
    WidgetTree->RootWidget = ViewportScale;

    FrontendPanel = MakePanel(TEXT("FrontendPanel"), OCPanel);
    UVerticalBox* Frontend = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
    FrontendPanel->SetContent(Frontend);
    Frontend->AddChildToVerticalBox(MakeText(LOCTEXT("Title", "OSTER CONFLICT"), 32, true));
    Frontend->AddChildToVerticalBox(MakeText(LOCTEXT("DirectConnectSubtitle", "МЕРЕЖЕВА ГРА / ПРЯМЕ ПІДКЛЮЧЕННЯ"), 15, false));
    const UOCPlayerUserSettings* FrontendPrefs = UOCPlayerUserSettings::Get();
    UsernameEntry = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass());
    UsernameEntry->SetHintText(LOCTEXT("UsernameHint", "Ім'я гравця (1–24)"));
    UsernameEntry->SetText(FText::FromString(FrontendPrefs->GetSavedUsername()));
    Frontend->AddChildToVerticalBox(UsernameEntry)->SetPadding(FMargin(0,18,0,5));
    AddressEntry = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass());
    AddressEntry->SetHintText(LOCTEXT("ServerAddressHint", "IP:порт сервера"));
    AddressEntry->SetText(FText::FromString(FrontendPrefs->GetLastServerAddress()));
    Frontend->AddChildToVerticalBox(AddressEntry)->SetPadding(FMargin(0,3,0,8));
    FrontendConnectButton = MakeButton(Frontend, LOCTEXT("Connect", "ПІДКЛЮЧИТИСЯ"));
    FrontendConnectButton->OnClicked.AddDynamic(this, &UOCGameUIRootWidget::OnConnectClicked);
    FrontendLocalButton = MakeButton(Frontend, LOCTEXT("Localhost", "ЛОКАЛЬНИЙ СЕРВЕР 127.0.0.1:7777"));
    FrontendLocalButton->OnClicked.AddDynamic(this, &UOCGameUIRootWidget::OnLocalhostClicked);
    FrontendSettingsButton = MakeButton(Frontend, LOCTEXT("Settings", "НАЛАШТУВАННЯ"));
    FrontendSettingsButton->OnClicked.AddDynamic(this, &UOCGameUIRootWidget::OnOpenSettingsClicked);
    FrontendCloseButton = MakeButton(Frontend, LOCTEXT("CloseMenu", "ЗАКРИТИ МЕНЮ"));
    FrontendCloseButton->OnClicked.AddDynamic(this, &UOCGameUIRootWidget::OnCloseFrontendClicked);
    FrontendStatusText = MakeText(LOCTEXT("DirectConnectAuthority", "Пряме IP-підключення; сервер залишається авторитетним."), 12, false);
    Frontend->AddChildToVerticalBox(FrontendStatusText)->SetPadding(FMargin(0,12,0,0));
    PlacePanel(FrontendPanel, FVector2D(500, 120), FVector2D(620, 560), 100);
    FrontendFocusOrder = { FrontendConnectButton, FrontendLocalButton, FrontendSettingsButton };
    WireLinearNavigation(FrontendFocusOrder, false);

    DeploymentPanel = MakePanel(TEXT("DeploymentPanel"), OCPanel);
    UHorizontalBox* DeployColumns = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
    DeploymentPanel->SetContent(DeployColumns);
    UVerticalBox* DeployLeft = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
    DeployColumns->AddChildToHorizontalBox(DeployLeft)->SetPadding(FMargin(0,0,24,0));
    DeployLeft->AddChildToVerticalBox(MakeText(LOCTEXT("DeploymentTitle", "РОЗГОРТАННЯ"), 28, true));
    DeploymentIdentityText = MakeText(LOCTEXT("PlayerPlaceholder", "ГРАВЕЦЬ"), 15, false);
    DeployLeft->AddChildToVerticalBox(DeploymentIdentityText)->SetPadding(FMargin(0,8,0,10));
    DeploymentTeamOneButton = MakeButton(DeployLeft, LOCTEXT("JoinTeamOne", "ПРИЄДНАТИСЯ ДО КОМАНДИ 1"));
    DeploymentTeamOneButton->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnTeamOneClicked);
    DeploymentTeamTwoButton = MakeButton(DeployLeft, LOCTEXT("JoinTeamTwo", "ПРИЄДНАТИСЯ ДО КОМАНДИ 2"));
    DeploymentTeamTwoButton->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnTeamTwoClicked);
    DeploymentRoleButton = MakeButton(DeployLeft, LOCTEXT("ChangeRole", "ЗМІНИТИ РОЛЬ"));
    DeploymentRoleButton->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnRoleClicked);
    DeploymentSquadButton = MakeButton(DeployLeft, LOCTEXT("NextSquad", "НАСТУПНА ГРУПА"));
    DeploymentSquadButton->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnSquadClicked);

    UVerticalBox* SpawnColumn = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
    DeployColumns->AddChildToHorizontalBox(SpawnColumn)->SetPadding(FMargin(0,0,24,0));
    SpawnColumn->AddChildToVerticalBox(MakeText(LOCTEXT("SpawnTitle", "ТОЧКА ПОЯВИ"),22,true));
    DeploymentSpawnText=MakeText(LOCTEXT("SelectedBase", "ОБРАНО: БАЗА"),14,false);
    SpawnColumn->AddChildToVerticalBox(DeploymentSpawnText)->SetPadding(FMargin(0,6,0,8));
    DeploymentBaseButton=MakeButton(SpawnColumn,LOCTEXT("SpawnBase", "БАЗА"));
    DeploymentBaseButton->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnSpawnBaseClicked);
    DeploymentAButton=MakeButton(SpawnColumn,LOCTEXT("SpawnA", "ТОЧКА A"));
    DeploymentAButton->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnSpawnAClicked);
    DeploymentBButton=MakeButton(SpawnColumn,LOCTEXT("SpawnB", "ТОЧКА B"));
    DeploymentBButton->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnSpawnBClicked);
    DeploymentCButton=MakeButton(SpawnColumn,LOCTEXT("SpawnC", "ТОЧКА C"));
    DeploymentCButton->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnSpawnCClicked);
    DeploymentReadyButton=MakeButton(SpawnColumn,LOCTEXT("ReadyDeploy", "ГОТОВИЙ / У БІЙ"));
    DeploymentReadyButton->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnDeployClicked);

    UVerticalBox* DeployRight=WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
    DeployColumns->AddChildToHorizontalBox(DeployRight);
    DeploymentPopulationText=MakeText(LOCTEXT("HumansBots", "ЛЮДИ / БОТИ"),14,true);
    DeployRight->AddChildToVerticalBox(DeploymentPopulationText);
    DeploymentRosterText=MakeText(LOCTEXT("Roster", "СКЛАД"),13,false);
    DeploymentRosterText->SetAutoWrapText(true);
    DeployRight->AddChildToVerticalBox(DeploymentRosterText)->SetPadding(FMargin(0,10,0,0));
    PlacePanel(DeploymentPanel,FVector2D(120,80),FVector2D(1360,690),80);
    DeploymentFocusOrder = { DeploymentTeamOneButton, DeploymentTeamTwoButton, DeploymentRoleButton, DeploymentSquadButton, DeploymentBaseButton, DeploymentAButton, DeploymentBButton, DeploymentCButton, DeploymentReadyButton };
    WireLinearNavigation(DeploymentFocusOrder, false);

    ScoreboardPanel=MakePanel(TEXT("ScoreboardPanel"),OCPanel);
    UVerticalBox* ScoreRoot=WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
    ScoreboardPanel->SetContent(ScoreRoot);
    ScoreRoot->AddChildToVerticalBox(MakeText(LOCTEXT("Scoreboard", "ТАБЛИЦЯ РАХУНКУ"),28,true));
    ScoreboardText=MakeText(LOCTEXT("NoPlayers", "Немає гравців"),13,false);
    ScoreRoot->AddChildToVerticalBox(ScoreboardText)->SetPadding(FMargin(0,10,0,0));
    PlacePanel(ScoreboardPanel,FVector2D(250,80),FVector2D(1100,690),85);

    ChatPanel=MakePanel(TEXT("ChatPanel"),FLinearColor(0.02f,0.025f,0.03f,0.84f));
    UVerticalBox* ChatRoot=WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
    ChatPanel->SetContent(ChatRoot);
    ChatLogText=MakeText(FText::GetEmpty(),12,false);
    ChatLogText->SetAutoWrapText(true);
    ChatRoot->AddChildToVerticalBox(ChatLogText);
    FFormatNamedArguments InitialChatArgs; InitialChatArgs.Add(TEXT("Channel"), ChatChannelTextValue(EOCChatChannel::Team));
    ChatChannelText=MakeText(FText::Format(LOCTEXT("ChannelFmt", "КАНАЛ: {Channel}"), InitialChatArgs),12,true);
    ChatRoot->AddChildToVerticalBox(ChatChannelText)->SetPadding(FMargin(0,5,0,3));
    ChatEntry=WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass());
    ChatEntry->SetHintText(LOCTEXT("ChatHint", "Введіть повідомлення…"));
    ChatEntry->OnTextCommitted.AddDynamic(this,&UOCGameUIRootWidget::OnChatCommitted);
    ChatRoot->AddChildToVerticalBox(ChatEntry);
    ChatChannelButton=MakeButton(ChatRoot,LOCTEXT("ChangeChannel", "ЗМІНИТИ КАНАЛ"));
    ChatChannelButton->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnChatChannelClicked);
    ChatSendButton=MakeButton(ChatRoot,LOCTEXT("Send", "НАДІСЛАТИ"));
    ChatSendButton->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnChatSendClicked);
    PlacePanel(ChatPanel,FVector2D(30,500),FVector2D(430,330),70);

    AdminPanel=MakePanel(TEXT("AdminPanel"),OCPanelAlt);
    UVerticalBox* AdminRoot=WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
    AdminPanel->SetContent(AdminRoot);
    AdminRoot->AddChildToVerticalBox(MakeText(LOCTEXT("SandboxAdmin", "АДМІНІСТРАТОР SANDBOX"),24,true));
    AdminActionText=MakeText(LOCTEXT("AdminAction", "Дія"),15,false);
    AdminRoot->AddChildToVerticalBox(AdminActionText)->SetPadding(FMargin(0,12,0,12));
    AdminPrevButton=MakeButton(AdminRoot,LOCTEXT("Previous", "ПОПЕРЕДНЯ"));
    AdminPrevButton->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnAdminPrevClicked);
    AdminNextButton=MakeButton(AdminRoot,LOCTEXT("Next", "НАСТУПНА"));
    AdminNextButton->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnAdminNextClicked);
    AdminExecuteButton=MakeButton(AdminRoot,LOCTEXT("Execute", "ВИКОНАТИ"));
    AdminExecuteButton->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnAdminExecuteClicked);
    PlacePanel(AdminPanel,FVector2D(1120,150),FVector2D(430,420),90);
    AdminFocusOrder = { AdminPrevButton, AdminNextButton, AdminExecuteButton };
    WireLinearNavigation(AdminFocusOrder, false);

    SettingsPanel = MakePanel(TEXT("SettingsPanel"), OCPanelAlt);
    UVerticalBox* SettingsRoot = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
    SettingsPanel->SetContent(SettingsRoot);
    BuildSettingsTree(SettingsRoot);
    PlacePanel(SettingsPanel, FVector2D(110, 45), FVector2D(1380, 790), 130);
}

void UOCGameUIRootWidget::BuildSettingsTree(UVerticalBox* SettingsRoot)
{
    SettingsRoot->AddChildToVerticalBox(MakeText(LOCTEXT("SettingsTitle", "НАЛАШТУВАННЯ"), 30, true));
    SettingsRoot->AddChildToVerticalBox(MakeText(LOCTEXT("SettingsHelp", "Застосувати — зберегти й лишитися тут. Зберегти й назад — зберегти та закрити. Скасувати — відкинути зміни після останнього збереження."), 12, false));

    UHorizontalBox* Nav = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
    SettingsRoot->AddChildToVerticalBox(Nav)->SetPadding(FMargin(0,10,0,8));
    auto NavButton = [this, Nav](const FText& Label, void (UOCGameUIRootWidget::*Handler)()) -> UButton*
    {
        UButton* B = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
        B->AddChild(MakeText(Label, 14, true));
        Nav->AddChildToHorizontalBox(B)->SetPadding(FMargin(0,0,8,0));
        if (Handler == &UOCGameUIRootWidget::OnSettingsGraphicsClicked) B->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnSettingsGraphicsClicked);
        else if (Handler == &UOCGameUIRootWidget::OnSettingsAudioClicked) B->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnSettingsAudioClicked);
        else if (Handler == &UOCGameUIRootWidget::OnSettingsControlsClicked) B->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnSettingsControlsClicked);
        else if (Handler == &UOCGameUIRootWidget::OnSettingsInterfaceClicked) B->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnSettingsInterfaceClicked);
        else B->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnSettingsAccessibilityClicked);
        return B;
    };
    SettingsGraphicsButton=NavButton(LOCTEXT("TabGraphics", "ГРАФІКА"), &UOCGameUIRootWidget::OnSettingsGraphicsClicked);
    SettingsAudioButton=NavButton(LOCTEXT("TabAudio", "ЗВУК"), &UOCGameUIRootWidget::OnSettingsAudioClicked);
    SettingsControlsButton=NavButton(LOCTEXT("TabControls", "КЕРУВАННЯ"), &UOCGameUIRootWidget::OnSettingsControlsClicked);
    SettingsInterfaceButton=NavButton(LOCTEXT("TabInterface", "ІНТЕРФЕЙС"), &UOCGameUIRootWidget::OnSettingsInterfaceClicked);
    SettingsAccessibilityButton=NavButton(LOCTEXT("TabAccessibility", "ДОСТУПНІСТЬ"), &UOCGameUIRootWidget::OnSettingsAccessibilityClicked);
    SettingsTabFocusOrder={SettingsGraphicsButton,SettingsAudioButton,SettingsControlsButton,SettingsInterfaceButton,SettingsAccessibilityButton};
    WireLinearNavigation(SettingsTabFocusOrder, true);

    SettingsSwitcher = WidgetTree->ConstructWidget<UWidgetSwitcher>(UWidgetSwitcher::StaticClass());
    if (UVerticalBoxSlot* SwitchSlot = SettingsRoot->AddChildToVerticalBox(SettingsSwitcher)) SwitchSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    auto NewPage = [this]() -> UVerticalBox*
    {
        UScrollBox* Scroll = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass());
        SettingsSwitcher->AddChild(Scroll);
        UVerticalBox* Page = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
        Scroll->AddChild(Page);
        return Page;
    };

    UVerticalBox* Graphics = NewPage();
    Graphics->AddChildToVerticalBox(MakeText(LOCTEXT("GraphicsTitle", "ГРАФІКА"),22,true));
    ResolutionCombo=MakeComboRow(Graphics,LOCTEXT("Resolution", "Роздільна здатність"),{TEXT("1280x720"),TEXT("1600x900"),TEXT("1920x1080"),TEXT("2560x1440"),TEXT("3840x2160")});
    WindowModeCombo=MakeComboRow(Graphics,LOCTEXT("DisplayMode", "Режим екрана"),{WindowModeToString(EWindowMode::Fullscreen),WindowModeToString(EWindowMode::WindowedFullscreen),WindowModeToString(EWindowMode::Windowed)});
    QualityPresetCombo=MakeComboRow(Graphics,LOCTEXT("QualityPreset", "Профіль якості"),QualityOptions(true));
    QualityPresetCombo->OnSelectionChanged.AddDynamic(this,&UOCGameUIRootWidget::OnQualityPresetChanged);
    { UTextBlock* ValueText = nullptr; ResolutionScaleSlider=MakeSliderRow(Graphics,LOCTEXT("ResolutionScale", "Масштаб рендерингу"),ValueText); ResolutionScaleValue = ValueText; }
    FrameLimitCombo=MakeComboRow(Graphics,LOCTEXT("FrameLimit", "Ліміт FPS"),{TEXT("30"),TEXT("60"),TEXT("90"),TEXT("120"),TEXT("144"),TEXT("165"),TEXT("240"),L(LOCTEXT("Unlimited", "Без обмеження"))});
    VSyncCheck=MakeCheckRow(Graphics,LOCTEXT("VSync", "Вертикальна синхронізація"));
    DynamicResolutionCheck=MakeCheckRow(Graphics,LOCTEXT("DynamicResolution", "Динамічна роздільна здатність"));
    ViewDistanceCombo=MakeComboRow(Graphics,LOCTEXT("ViewDistance", "Дальність промальовування"),QualityOptions());
    ShadowCombo=MakeComboRow(Graphics,LOCTEXT("Shadows", "Тіні"),QualityOptions());
    TextureCombo=MakeComboRow(Graphics,LOCTEXT("Textures", "Текстури"),QualityOptions());
    EffectsCombo=MakeComboRow(Graphics,LOCTEXT("Effects", "Ефекти"),QualityOptions());
    FoliageCombo=MakeComboRow(Graphics,LOCTEXT("Foliage", "Рослинність"),QualityOptions());
    PostProcessCombo=MakeComboRow(Graphics,LOCTEXT("PostProcess", "Постобробка"),QualityOptions());
    AntiAliasingCombo=MakeComboRow(Graphics,LOCTEXT("AntiAliasing", "Згладжування"),QualityOptions());
    ShadingCombo=MakeComboRow(Graphics,LOCTEXT("Shading", "Шейдинг"),QualityOptions());
    GlobalIlluminationCombo=MakeComboRow(Graphics,LOCTEXT("GlobalIllumination", "Глобальне освітлення"),QualityOptions());
    ReflectionCombo=MakeComboRow(Graphics,LOCTEXT("Reflections", "Відбиття"),QualityOptions());
    LandscapeCombo=MakeComboRow(Graphics,LOCTEXT("Landscape", "Ландшафт"),QualityOptions());

    UVerticalBox* Audio=NewPage();
    Audio->AddChildToVerticalBox(MakeText(LOCTEXT("AudioMixerTitle", "ЗВУКОВИЙ МІКШЕР"),22,true));
    UTextBlock* V=nullptr;
    MasterVolumeSlider=MakeSliderRow(Audio,LOCTEXT("AudioMaster", "Загальна гучність"),V);AudioValueTexts.Add((int32)EOCAudioBus::Master,V);MasterAudioCheck=MakeCheckRow(Audio,LOCTEXT("AudioMasterEnabled", "Загальний звук увімкнено"));
    WeaponsVolumeSlider=MakeSliderRow(Audio,LOCTEXT("AudioWeapons", "Зброя"),V);AudioValueTexts.Add((int32)EOCAudioBus::Weapons,V);WeaponsAudioCheck=MakeCheckRow(Audio,LOCTEXT("AudioWeaponsEnabled", "Звуки зброї увімкнено"));
    VehiclesVolumeSlider=MakeSliderRow(Audio,LOCTEXT("AudioVehicles", "Техніка"),V);AudioValueTexts.Add((int32)EOCAudioBus::Vehicles,V);VehiclesAudioCheck=MakeCheckRow(Audio,LOCTEXT("AudioVehiclesEnabled", "Звуки техніки увімкнено"));
    CharactersVolumeSlider=MakeSliderRow(Audio,LOCTEXT("AudioCharacters", "Персонажі"),V);AudioValueTexts.Add((int32)EOCAudioBus::Characters,V);CharactersAudioCheck=MakeCheckRow(Audio,LOCTEXT("AudioCharactersEnabled", "Звуки персонажів увімкнено"));
    WorldSFXVolumeSlider=MakeSliderRow(Audio,LOCTEXT("AudioWorldSFX", "Звуки світу"),V);AudioValueTexts.Add((int32)EOCAudioBus::WorldSFX,V);WorldSFXAudioCheck=MakeCheckRow(Audio,LOCTEXT("AudioWorldSFXEnabled", "Звуки світу увімкнено"));
    AmbienceVolumeSlider=MakeSliderRow(Audio,LOCTEXT("AudioAmbience", "Атмосфера"),V);AudioValueTexts.Add((int32)EOCAudioBus::Ambience,V);AmbienceAudioCheck=MakeCheckRow(Audio,LOCTEXT("AudioAmbienceEnabled", "Атмосферні звуки увімкнено"));
    MusicVolumeSlider=MakeSliderRow(Audio,LOCTEXT("AudioMusic", "Музика"),V);AudioValueTexts.Add((int32)EOCAudioBus::Music,V);MusicAudioCheck=MakeCheckRow(Audio,LOCTEXT("AudioMusicEnabled", "Музику увімкнено"));MenuMusicCheck=MakeCheckRow(Audio,LOCTEXT("MenuMusicEnabled", "Музику меню увімкнено"));
    UIVolumeSlider=MakeSliderRow(Audio,LOCTEXT("AudioUI", "Інтерфейс"),V);AudioValueTexts.Add((int32)EOCAudioBus::UI,V);UIAudioCheck=MakeCheckRow(Audio,LOCTEXT("AudioUIEnabled", "Звуки інтерфейсу увімкнено"));
    // Voice Chat is P2: backend fields remain reserved, but P0/P1 UI must not present a placeholder control.
    DialogueVolumeSlider=MakeSliderRow(Audio,LOCTEXT("AudioDialogue", "Репліки"),V);AudioValueTexts.Add((int32)EOCAudioBus::Dialogue,V);DialogueAudioCheck=MakeCheckRow(Audio,LOCTEXT("AudioDialogueEnabled", "Репліки увімкнено"));
    DynamicRangeCombo=MakeComboRow(Audio,LOCTEXT("DynamicRange", "Динамічний діапазон"),{DynamicRangeToString(EOCDynamicRange::Night),DynamicRangeToString(EOCDynamicRange::Standard),DynamicRangeToString(EOCDynamicRange::High)});
    AudioOutputCombo=MakeComboRow(Audio,LOCTEXT("AudioOutput", "Вивід звуку"),{OutputModeToString(EOCAudioOutputMode::StereoSpeakers),OutputModeToString(EOCAudioOutputMode::Headphones),OutputModeToString(EOCAudioOutputMode::SpatialHeadphones)});

    UVerticalBox* Controls=NewPage();
    Controls->AddChildToVerticalBox(MakeText(LOCTEXT("ControlsTitle", "КЕРУВАННЯ"),22,true));
    { UTextBlock* ValueText = nullptr; MouseSensitivitySlider=MakeSliderRow(Controls,LOCTEXT("MouseSensitivity", "Чутливість миші"),ValueText); MouseSensitivityValue = ValueText; }
    { UTextBlock* ValueText = nullptr; AimSensitivitySlider=MakeSliderRow(Controls,LOCTEXT("ADSSensitivity", "Множник чутливості ADS"),ValueText); AimSensitivityValue = ValueText; }
    InvertYCheck=MakeCheckRow(Controls,LOCTEXT("InvertY", "Інвертувати вісь Y миші"));
    Controls->AddChildToVerticalBox(MakeText(LOCTEXT("RebindHelp", "Виберіть дію й натисніть нову клавішу. Якщо клавіша вже зайнята, основні прив'язки міняються місцями."),12,false))->SetPadding(FMargin(0,8,0,8));
    UButton* RB;
    RB=MakeRebindRow(Controls,TEXT("MoveForward"),LOCTEXT("BindMoveForward", "Рух уперед"));RB->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnBindMoveForward);
    RB=MakeRebindRow(Controls,TEXT("MoveBackward"),LOCTEXT("BindMoveBackward", "Рух назад"));RB->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnBindMoveBackward);
    RB=MakeRebindRow(Controls,TEXT("MoveLeft"),LOCTEXT("BindMoveLeft", "Рух ліворуч"));RB->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnBindMoveLeft);
    RB=MakeRebindRow(Controls,TEXT("MoveRight"),LOCTEXT("BindMoveRight", "Рух праворуч"));RB->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnBindMoveRight);
    RB=MakeRebindRow(Controls,TEXT("Jump"),LOCTEXT("BindJump", "Стрибок / утримання для здачі"));RB->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnBindJump);
    RB=MakeRebindRow(Controls,TEXT("Sprint"),LOCTEXT("BindSprint", "Спринт"));RB->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnBindSprint);
    RB=MakeRebindRow(Controls,TEXT("Crouch"),LOCTEXT("BindCrouch", "Присісти"));RB->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnBindCrouch);
    RB=MakeRebindRow(Controls,TEXT("Fire"),LOCTEXT("BindFire", "Вогонь"));RB->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnBindFire);
    RB=MakeRebindRow(Controls,TEXT("Aim"),LOCTEXT("BindAim", "Прицілювання / огляд у техніці"));RB->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnBindAim);
    RB=MakeRebindRow(Controls,TEXT("Reload"),LOCTEXT("BindReload", "Перезарядити"));RB->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnBindReload);
    RB=MakeRebindRow(Controls,TEXT("Interact"),LOCTEXT("BindInteract", "Взаємодія / реанімація / вхід-вихід"));RB->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnBindInteract);
    RB=MakeRebindRow(Controls,TEXT("ThrowGrenade"),LOCTEXT("BindGrenade", "Кинути гранату"));RB->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnBindGrenade);
    RB=MakeRebindRow(Controls,TEXT("Scoreboard"),LOCTEXT("BindScoreboard", "Таблиця рахунку"));RB->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnBindScoreboard);
    RB=MakeRebindRow(Controls,TEXT("Chat"),LOCTEXT("BindChat", "Чат"));RB->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnBindChat);

    UVerticalBox* Interface=NewPage();
    Interface->AddChildToVerticalBox(MakeText(LOCTEXT("InterfaceTitle", "ІНТЕРФЕЙС"),22,true));
    { UTextBlock* ValueText = nullptr; FOVSlider=MakeSliderRow(Interface,LOCTEXT("FieldOfView", "Поле зору"),ValueText); FOVValue = ValueText; }
    { UTextBlock* ValueText = nullptr; HUDScaleSlider=MakeSliderRow(Interface,LOCTEXT("HUDScale", "Масштаб HUD"),ValueText); HUDScaleValue = ValueText; }
    ShowFPSCheck=MakeCheckRow(Interface,LOCTEXT("ShowFPS", "Показувати FPS"));
    ShowPingCheck=MakeCheckRow(Interface,LOCTEXT("ShowPing", "Показувати ping"));
    ShowCrosshairCheck=MakeCheckRow(Interface,LOCTEXT("ShowCrosshair", "Показувати приціл"));
    ShowHitMarkerCheck=MakeCheckRow(Interface,LOCTEXT("ShowHitmarker", "Показувати підтвердження влучення"));

    UVerticalBox* Accessibility=NewPage();
    Accessibility->AddChildToVerticalBox(MakeText(LOCTEXT("AccessibilityTitle", "ДОСТУПНІСТЬ / КОНТЕНТ"),22,true));
    GoreCombo=MakeComboRow(Accessibility,LOCTEXT("Gore", "Кров і жорстокість"),{L(LOCTEXT("GoreOff", "Вимкнено")),L(LOCTEXT("GoreReduced", "Зменшено")),L(LOCTEXT("GoreFull", "Повністю"))});
    SubtitlesCheck=MakeCheckRow(Accessibility,LOCTEXT("Subtitles", "Субтитри"));
    ReduceFlashesCheck=MakeCheckRow(Accessibility,LOCTEXT("ReduceFlashes", "Зменшити інтенсивність спалахів"));
    { UTextBlock* ValueText = nullptr; CameraShakeSlider=MakeSliderRow(Accessibility,LOCTEXT("CameraShake", "Тряска камери"),ValueText); CameraShakeValue = ValueText; }
    ColorVisionCombo=MakeComboRow(Accessibility,LOCTEXT("ColorVisionMode", "Режим сприйняття кольорів"),{L(LOCTEXT("ColorOff", "Вимкнено")),L(LOCTEXT("ColorDeuteranopia", "Дейтеранопія")),L(LOCTEXT("ColorProtanopia", "Протанопія")),L(LOCTEXT("ColorTritanopia", "Тританопія"))});

    SettingsStatusText=MakeText(LOCTEXT("NoPendingChanges", "Немає незбережених змін."),12,false);
    SettingsRoot->AddChildToVerticalBox(SettingsStatusText)->SetPadding(FMargin(0,8,0,5));
    UHorizontalBox* Footer=WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
    SettingsRoot->AddChildToVerticalBox(Footer);
    auto FooterButton=[this,Footer](const FText& Label, void (UOCGameUIRootWidget::*Handler)()) -> UButton*
    {
        UButton* B=WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
        B->AddChild(MakeText(Label,14,true));
        Footer->AddChildToHorizontalBox(B)->SetPadding(FMargin(0,0,8,0));
        if(Handler==&UOCGameUIRootWidget::OnSettingsApplyClicked)B->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnSettingsApplyClicked);
        else if(Handler==&UOCGameUIRootWidget::OnSettingsSaveClicked)B->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnSettingsSaveClicked);
        else if(Handler==&UOCGameUIRootWidget::OnSettingsCancelClicked)B->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnSettingsCancelClicked);
        else B->OnClicked.AddDynamic(this,&UOCGameUIRootWidget::OnSettingsDefaultsClicked);
        return B;
    };
    SettingsApplyButton=FooterButton(LOCTEXT("Apply", "ЗАСТОСУВАТИ"),&UOCGameUIRootWidget::OnSettingsApplyClicked);
    SettingsSaveButton=FooterButton(LOCTEXT("SaveBack", "ЗБЕРЕГТИ Й НАЗАД"),&UOCGameUIRootWidget::OnSettingsSaveClicked);
    SettingsCancelButton=FooterButton(LOCTEXT("Cancel", "СКАСУВАТИ"),&UOCGameUIRootWidget::OnSettingsCancelClicked);
    SettingsDefaultsButton=FooterButton(LOCTEXT("ResetDefaults", "СКИНУТИ НАЛАШТУВАННЯ"),&UOCGameUIRootWidget::OnSettingsDefaultsClicked);
    SettingsFooterFocusOrder={SettingsApplyButton,SettingsSaveButton,SettingsCancelButton,SettingsDefaultsButton};
    WireLinearNavigation(SettingsFooterFocusOrder, true);
    SetSettingsPage(0);
}

void UOCGameUIRootWidget::FocusWidget(UWidget* Widget)
{
    if (!Widget || !Widget->GetIsEnabled() || Widget->GetVisibility() == ESlateVisibility::Collapsed) return;
    Widget->SetFocus();
}

void UOCGameUIRootWidget::WireLinearNavigation(const TArray<TObjectPtr<UWidget>>& Widgets, bool bHorizontal)
{
    TArray<UWidget*> Valid;
    for (const TObjectPtr<UWidget>& Widget : Widgets)
        if (Widget) Valid.Add(Widget.Get());
    if (Valid.Num() < 2) return;

    for (int32 Index = 0; Index < Valid.Num(); ++Index)
    {
        UWidget* Current = Valid[Index];
        UWidget* Prev = Valid[(Index - 1 + Valid.Num()) % Valid.Num()];
        UWidget* Next = Valid[(Index + 1) % Valid.Num()];
        if (bHorizontal)
        {
            Current->SetNavigationRuleExplicit(EUINavigation::Left, Prev);
            Current->SetNavigationRuleExplicit(EUINavigation::Right, Next);
        }
        else
        {
            Current->SetNavigationRuleExplicit(EUINavigation::Up, Prev);
            Current->SetNavigationRuleExplicit(EUINavigation::Down, Next);
        }
    }
}

void UOCGameUIRootWidget::UpdateFocusForVisibleContext(AOCPlayerController* PC, bool bFrontend, bool bSettings)
{
    if (!PC) return;
    int32 Context = 0;
    UWidget* Target = nullptr;
    if (PC->IsChatInputActive()) { Context = 5; Target = ChatEntry; }
    else if (bSettings) { Context = 1; Target = SettingsGraphicsButton; }
    else if (bFrontend) { Context = 2; Target = FrontendConnectButton; }
    else if (PC->IsDeploymentPanelVisible()) { Context = 3; Target = DeploymentTeamOneButton; }
    else if (PC->IsAdminPanelVisible()) { Context = 4; Target = AdminPrevButton; }

    if (Context != LastFocusContext)
    {
        LastFocusContext = Context;
        if (Target) FocusWidget(Target);
    }
}

void UOCGameUIRootWidget::RefreshAll()
{
    AOCPlayerController* PC=Cast<AOCPlayerController>(GetOwningPlayer());if(!PC)return;
    const bool bSettings=PC->IsSettingsVisible(); const bool bFrontend=PC->IsFrontendMenuVisible()&&!bSettings;
    // Pass 28: -Frontend is owned exclusively by OCR13FrontendMenuSubsystem. Pass 27 kept the legacy
    // FrontendPanel attached to its WidgetTree, but this 0.20s RefreshAll loop immediately made it
    // visible again after the subsystem collapsed it. That produced the exact double-menu screenshot
    // and reintroduced two simultaneously live frontend widget paths. Keep the legacy panel structurally
    // attached but permanently collapsed/disabled while the R13 frontend shell owns presentation/input.
    const bool bR13OwnsFrontend = FParse::Param(FCommandLine::Get(), TEXT("Frontend"));
    const bool bShowLegacyFrontend = bFrontend && !bR13OwnsFrontend;

    // Pass 29: while the R13 startup shell owns the frontend, the native root must not mutate hidden
    // legacy Slate subtrees every 0.20 s. Pass 28 hid the duplicate panel, but RefreshAll still touched
    // settings/deployment/scoreboard/chat/admin and refreshed their text while the R13 click transition
    // was running in the same UUserWidget. Freeze that entire legacy branch until settings/travel takes
    // ownership again. Only collapse a panel if its state actually differs, then leave Slate alone.
    if (bR13OwnsFrontend && bFrontend)
    {
        auto FreezeLegacyPanel = [](UWidget* Widget)
        {
            if (!Widget) return;
            if (Widget->GetVisibility() != ESlateVisibility::Collapsed)
                Widget->SetVisibility(ESlateVisibility::Collapsed);
            if (Widget->GetIsEnabled())
                Widget->SetIsEnabled(false);
        };
        FreezeLegacyPanel(SettingsPanel);
        FreezeLegacyPanel(FrontendPanel);
        FreezeLegacyPanel(DeploymentPanel);
        FreezeLegacyPanel(ScoreboardPanel);
        FreezeLegacyPanel(ChatPanel);
        FreezeLegacyPanel(AdminPanel);
        LastFocusContext = 0;
        return;
    }

    SettingsPanel->SetVisibility(bSettings?ESlateVisibility::Visible:ESlateVisibility::Collapsed);
    if (FrontendPanel)
    {
        FrontendPanel->SetVisibility(bShowLegacyFrontend?ESlateVisibility::Visible:ESlateVisibility::Collapsed);
        FrontendPanel->SetIsEnabled(bShowLegacyFrontend);
    }
    DeploymentPanel->SetVisibility(!bFrontend&&!bSettings&&PC->IsDeploymentPanelVisible()?ESlateVisibility::Visible:ESlateVisibility::Collapsed);
    ScoreboardPanel->SetVisibility(!bFrontend&&!bSettings&&PC->IsScoreboardVisible()?ESlateVisibility::Visible:ESlateVisibility::Collapsed);
    ChatPanel->SetVisibility(!bFrontend&&!bSettings&&!PC->IsDeploymentPanelVisible()?(PC->IsChatInputActive()?ESlateVisibility::Visible:ESlateVisibility::SelfHitTestInvisible):ESlateVisibility::Collapsed);
    if(ChatEntry)ChatEntry->SetVisibility(PC->IsChatInputActive()?ESlateVisibility::Visible:ESlateVisibility::Collapsed);if(ChatChannelButton)ChatChannelButton->SetVisibility(PC->IsChatInputActive()?ESlateVisibility::Visible:ESlateVisibility::Collapsed);if(ChatSendButton)ChatSendButton->SetVisibility(PC->IsChatInputActive()?ESlateVisibility::Visible:ESlateVisibility::Collapsed);if(PC->IsChatInputActive()&&ChatEntry&&!ChatEntry->HasKeyboardFocus())ChatEntry->SetKeyboardFocus();
    AdminPanel->SetVisibility(!bFrontend&&!bSettings&&PC->IsAdminPanelVisible()?ESlateVisibility::Visible:ESlateVisibility::Collapsed);
    UpdateFocusForVisibleContext(PC, bShowLegacyFrontend, bSettings);
    RefreshFrontend(PC);RefreshDeployment(PC);RefreshScoreboard(PC);RefreshChat(PC);RefreshAdmin(PC);if(bSettings)RefreshSettingsLabels();
}

void UOCGameUIRootWidget::RefreshFrontend(AOCPlayerController* PC)
{
    if (!FrontendStatusText || !PC) return;
    // In the standalone Frontend shell there is intentionally no local gameplay session behind the menu.
    // Hide the mouse-oriented Close button there instead of presenting a control that cannot safely close.
    const bool bStandaloneShell = PC->GetNetMode() == NM_Standalone &&
        !FParse::Param(FCommandLine::Get(), TEXT("NoFrontend"));
    if (FrontendCloseButton)
    {
        FrontendCloseButton->SetVisibility(bStandaloneShell ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
        FrontendCloseButton->SetIsEnabled(!bStandaloneShell);
    }
    if (const UOCGameInstance* GI = Cast<UOCGameInstance>(PC->GetGameInstance()))
    {
        FrontendStatusText->SetText(GI->GetConnectionStatusText());
        return;
    }
    FrontendStatusText->SetText(NSLOCTEXT("OCFrontend", "DirectConnectReady", "Готово до прямого підключення."));
}

void UOCGameUIRootWidget::RefreshDeployment(AOCPlayerController* PC)
{
    if(!PC||!DeploymentIdentityText||!DeploymentPopulationText||!DeploymentRosterText)return;
    const AOCPlayerState* Local=PC->GetPlayerState<AOCPlayerState>();
    const AOCGameState* GS=GetWorld()?GetWorld()->GetGameState<AOCGameState>():nullptr;
    if(Local)
    {
        FFormatNamedArguments Args;
        Args.Add(TEXT("Name"), FText::FromString(Local->GetPlayerName()));
        Args.Add(TEXT("Team"), TeamText(Local->GetTeamId()));
        Args.Add(TEXT("Faction"), FactionText(Local->GetFactionArchetype()));
        Args.Add(TEXT("Role"), RoleText(Local->GetPlayerRole()));
        Args.Add(TEXT("Squad"), SquadText(Local->GetSquadId()));
        Args.Add(TEXT("Leader"), Local->IsSquadLeader()?LOCTEXT("LeaderSuffix", "  ЛІДЕР"):FText::GetEmpty());
        DeploymentIdentityText->SetText(FText::Format(LOCTEXT("DeploymentIdentityFmt", "{Name}  |  {Team}  |  {Faction}  |  {Role}  |  {Squad}{Leader}"), Args));
    }
    if(GS)
    {
        FFormatNamedArguments Pop;
        Pop.Add(TEXT("Humans"), GS->GetHumanPlayerCount());
        Pop.Add(TEXT("Bots"), GS->GetBotPlayerCount());
        Pop.Add(TEXT("Target"), GS->GetTargetPopulation());
        Pop.Add(TEXT("Max"), GS->GetMaxPlayerSlots());
        DeploymentPopulationText->SetText(FText::Format(LOCTEXT("PopulationFmt", "ЛЮДИ {Humans}  |  БОТИ {Bots}  |  ЦІЛЬ {Target}  |  МАКС. ЛЮДЕЙ {Max}"), Pop));

        FString Roster;
        for(APlayerState* PS:GS->PlayerArray)
        {
            const AOCPlayerState* S=Cast<AOCPlayerState>(PS);if(!S)continue;
            const FString Kind=(S->IsBotPlayer()?LOCTEXT("BotTag", "[БОТ]"):LOCTEXT("HumanTag", "[ЛЮД]")).ToString();
            const FString Ready=S->IsLobbyReady()?LOCTEXT("ReadyTag", "  ГОТОВИЙ").ToString():FString();
            Roster+=FString::Printf(TEXT("%s  %s  %s  %s  %s%s\n"),*Kind,*S->GetPlayerName(),*TeamText(S->GetTeamId()).ToString(),*SquadText(S->GetSquadId()).ToString(),*RoleText(S->GetPlayerRole()).ToString(),*Ready);
        }
        DeploymentRosterText->SetText(FText::FromString(Roster.Left(3500)));
    }
    if(DeploymentSpawnText)
    {
        FText Spawn=LOCTEXT("SpawnBase", "БАЗА");
        if(SelectedSpawnId==TEXT("A"))Spawn=LOCTEXT("SpawnA", "ТОЧКА A");
        else if(SelectedSpawnId==TEXT("B"))Spawn=LOCTEXT("SpawnB", "ТОЧКА B");
        else if(SelectedSpawnId==TEXT("C"))Spawn=LOCTEXT("SpawnC", "ТОЧКА C");
        FFormatNamedArguments Args; Args.Add(TEXT("Spawn"),Spawn);
        DeploymentSpawnText->SetText(FText::Format(LOCTEXT("SelectedSpawnFmt", "ОБРАНО: {Spawn}"),Args));
    }
}

void UOCGameUIRootWidget::RefreshScoreboard(AOCPlayerController* PC)
{
    if(!PC||!ScoreboardText)return;
    const AOCGameState* GS=GetWorld()?GetWorld()->GetGameState<AOCGameState>():nullptr;if(!GS)return;
    const FString Header=LOCTEXT("ScoreColumns", "ГРАВЕЦЬ                   ГР    РОЛЬ       K   D   R   РАХУНОК  PING").ToString();
    FString T1=TeamText(EOCTeam::TeamOne).ToString()+TEXT("\n")+Header+TEXT("\n");
    FString T2=TEXT("\n")+TeamText(EOCTeam::TeamTwo).ToString()+TEXT("\n")+Header+TEXT("\n");
    for(APlayerState* PS:GS->PlayerArray)
    {
        const AOCPlayerState* S=Cast<AOCPlayerState>(PS);if(!S)continue;
        const FString Name=(S->IsBotPlayer()?LOCTEXT("BotPrefix", "[БОТ] ").ToString():FString())+S->GetPlayerName();
        const FString Row=FString::Printf(TEXT("%-24s %-5s %-10s %2d  %2d  %2d  %5d  %4d\n"),*Name.Left(24),*SquadText(S->GetSquadId()).ToString().Left(5),*RoleText(S->GetPlayerRole()).ToString().Left(10),S->GetKills(),S->GetDeaths(),S->GetRevives(),FMath::RoundToInt(S->GetScore()),S->GetPingMs());
        if(S->GetTeamId()==EOCTeam::TeamTwo)T2+=Row;else T1+=Row;
    }
    ScoreboardText->SetText(FText::FromString((T1+T2).Left(6000)));
}

void UOCGameUIRootWidget::RefreshChat(AOCPlayerController* PC)
{
    if(!PC||!ChatLogText||!ChatChannelText)return;
    FString Log;
    for(const FOCChatMessage& Msg:PC->GetRecentChatMessages())
        Log+=FString::Printf(TEXT("[%s] %s: %s\n"),*ChatChannelTextValue(Msg.Channel).ToString(),*Msg.SenderName,*Msg.Message);
    ChatLogText->SetText(FText::FromString(Log.Right(2200)));
    FFormatNamedArguments Args; Args.Add(TEXT("Channel"),ChatChannelTextValue(SelectedChatChannel));
    ChatChannelText->SetText(FText::Format(LOCTEXT("ChannelFmt", "КАНАЛ: {Channel}"),Args));
}

void UOCGameUIRootWidget::RefreshAdmin(AOCPlayerController* PC)
{
    if(!PC||!AdminActionText)return;
    static const FText Actions[] = {
        LOCTEXT("AdminSpawnWeapons", "Створити всю зброю"), LOCTEXT("AdminRefillAmmo", "Поповнити боєприпаси"),
        LOCTEXT("AdminRestorePlayer", "Відновити гравця"), LOCTEXT("AdminSpawnCar", "Створити цивільне авто"),
        LOCTEXT("AdminSpawnGunTruck", "Створити озброєний пікап"), LOCTEXT("AdminSpawnBTR", "Створити БТР"),
        LOCTEXT("AdminGodMode", "Перемкнути god mode"), LOCTEXT("AdminResetWorld", "Скинути двері / ворота / світло"),
        LOCTEXT("AdminMuseum", "Телепорт: музей"), LOCTEXT("AdminStadium", "Телепорт: стадіон"),
        LOCTEXT("AdminPark", "Телепорт: парк"), LOCTEXT("AdminCollege", "Телепорт: коледж"),
        LOCTEXT("AdminSpawnBots", "Створити 4 AI-ботів"), LOCTEXT("AdminClearBots", "Прибрати AI-ботів") };
    const int32 Index=FMath::Clamp(PC->GetSelectedAdminActionIndex(),0,UE_ARRAY_COUNT(Actions)-1);
    FFormatNamedArguments Args; Args.Add(TEXT("Index"),Index+1); Args.Add(TEXT("Total"),UE_ARRAY_COUNT(Actions)); Args.Add(TEXT("Action"),Actions[Index]);
    AdminActionText->SetText(FText::Format(LOCTEXT("AdminActionFmt", "{Index} / {Total}   {Action}"),Args));
}

void UOCGameUIRootWidget::SetSettingsPage(int32 Index){if(SettingsSwitcher)SettingsSwitcher->SetActiveWidgetIndex(FMath::Clamp(Index,0,4));}

void UOCGameUIRootWidget::RefreshSettingsLabels()
{
    if(ResolutionScaleValue&&ResolutionScaleSlider)ResolutionScaleValue->SetText(FText::FromString(FString::Printf(TEXT("%d%%"),FMath::RoundToInt(50.0f+ResolutionScaleSlider->GetValue()*50.0f))));
    if(MouseSensitivityValue&&MouseSensitivitySlider)MouseSensitivityValue->SetText(FText::FromString(FString::Printf(TEXT("%.2f"),FMath::Lerp(0.10f,4.0f,MouseSensitivitySlider->GetValue()))));
    if(AimSensitivityValue&&AimSensitivitySlider)AimSensitivityValue->SetText(FText::FromString(FString::Printf(TEXT("%.2f"),FMath::Lerp(0.25f,1.50f,AimSensitivitySlider->GetValue()))));
    if(FOVValue&&FOVSlider)FOVValue->SetText(FText::FromString(FString::Printf(TEXT("%d°"),FMath::RoundToInt(FMath::Lerp(75.0f,120.0f,FOVSlider->GetValue())))));
    if(HUDScaleValue&&HUDScaleSlider)HUDScaleValue->SetText(FText::FromString(FString::Printf(TEXT("%d%%"),FMath::RoundToInt(FMath::Lerp(75.0f,125.0f,HUDScaleSlider->GetValue())))));
    if(CameraShakeValue&&CameraShakeSlider)CameraShakeValue->SetText(FText::FromString(FString::Printf(TEXT("%d%%"),FMath::RoundToInt(CameraShakeSlider->GetValue()*100.0f))));
    auto AudioLabel=[this](EOCAudioBus Bus,USlider* Slider){if(UTextBlock** Txt=AudioValueTexts.Find((int32)Bus);Txt&&*Txt&&Slider)(*Txt)->SetText(FText::FromString(FString::Printf(TEXT("%d%%"),FMath::RoundToInt(Slider->GetValue()*100.0f))));};
    AudioLabel(EOCAudioBus::Master,MasterVolumeSlider);AudioLabel(EOCAudioBus::Weapons,WeaponsVolumeSlider);AudioLabel(EOCAudioBus::Vehicles,VehiclesVolumeSlider);AudioLabel(EOCAudioBus::Characters,CharactersVolumeSlider);AudioLabel(EOCAudioBus::WorldSFX,WorldSFXVolumeSlider);AudioLabel(EOCAudioBus::Ambience,AmbienceVolumeSlider);AudioLabel(EOCAudioBus::Music,MusicVolumeSlider);AudioLabel(EOCAudioBus::UI,UIVolumeSlider);AudioLabel(EOCAudioBus::VoiceChat,VoiceChatVolumeSlider);AudioLabel(EOCAudioBus::Dialogue,DialogueVolumeSlider);
}

void UOCGameUIRootWidget::SyncSettingsWidgetsFromBackend()
{
    if(UGameUserSettings* GU=GEngine?GEngine->GetGameUserSettings():nullptr)
    {
        const FIntPoint Res=GU->GetScreenResolution();const FString ResString=FString::Printf(TEXT("%dx%d"),Res.X,Res.Y);if(ResolutionCombo->FindOptionIndex(ResString)<0)ResolutionCombo->AddOption(ResString);ResolutionCombo->SetSelectedOption(ResString);WindowModeCombo->SetSelectedOption(WindowModeToString(GU->GetFullscreenMode()));QualityPresetCombo->SetSelectedOption(QualityToString(GU->GetOverallScalabilityLevel()));
        ViewDistanceCombo->SetSelectedOption(QualityToString(GU->GetViewDistanceQuality()));ShadowCombo->SetSelectedOption(QualityToString(GU->GetShadowQuality()));TextureCombo->SetSelectedOption(QualityToString(GU->GetTextureQuality()));EffectsCombo->SetSelectedOption(QualityToString(GU->GetVisualEffectQuality()));FoliageCombo->SetSelectedOption(QualityToString(GU->GetFoliageQuality()));PostProcessCombo->SetSelectedOption(QualityToString(GU->GetPostProcessingQuality()));AntiAliasingCombo->SetSelectedOption(QualityToString(GU->GetAntiAliasingQuality()));ShadingCombo->SetSelectedOption(QualityToString(GU->GetShadingQuality()));GlobalIlluminationCombo->SetSelectedOption(QualityToString(GU->GetGlobalIlluminationQuality()));ReflectionCombo->SetSelectedOption(QualityToString(GU->GetReflectionQuality()));LandscapeCombo->SetSelectedOption(QualityToString(GU->GetLandscapeQuality()));
        float Norm=1,Value=100,Min=50,Max=100;GU->GetResolutionScaleInformationEx(Norm,Value,Min,Max);ResolutionScaleSlider->SetValue(FMath::Clamp((Value-50.0f)/50.0f,0.0f,1.0f));VSyncCheck->SetIsChecked(GU->IsVSyncEnabled());DynamicResolutionCheck->SetIsChecked(GU->IsDynamicResolutionEnabled());const int32 Limit=FMath::RoundToInt(GU->GetFrameRateLimit());const FString LimitOption=Limit<=0?L(LOCTEXT("Unlimited", "Без обмеження")):FString::FromInt(Limit);if(FrameLimitCombo->FindOptionIndex(LimitOption)<0)FrameLimitCombo->AddOption(LimitOption);FrameLimitCombo->SetSelectedOption(LimitOption);
    }
    UOCAudioUserSettings* A=UOCAudioUserSettings::Get();auto SetBus=[A](EOCAudioBus B,USlider* S,UCheckBox* C){if(S)S->SetValue(A->GetBusPercent(B)/100.0f);if(C)C->SetIsChecked(A->IsBusEnabled(B));};SetBus(EOCAudioBus::Master,MasterVolumeSlider,MasterAudioCheck);SetBus(EOCAudioBus::Weapons,WeaponsVolumeSlider,WeaponsAudioCheck);SetBus(EOCAudioBus::Vehicles,VehiclesVolumeSlider,VehiclesAudioCheck);SetBus(EOCAudioBus::Characters,CharactersVolumeSlider,CharactersAudioCheck);SetBus(EOCAudioBus::WorldSFX,WorldSFXVolumeSlider,WorldSFXAudioCheck);SetBus(EOCAudioBus::Ambience,AmbienceVolumeSlider,AmbienceAudioCheck);SetBus(EOCAudioBus::Music,MusicVolumeSlider,MusicAudioCheck);SetBus(EOCAudioBus::UI,UIVolumeSlider,UIAudioCheck);SetBus(EOCAudioBus::VoiceChat,VoiceChatVolumeSlider,VoiceChatAudioCheck);SetBus(EOCAudioBus::Dialogue,DialogueVolumeSlider,DialogueAudioCheck);MenuMusicCheck->SetIsChecked(A->bMenuMusicEnabled);DynamicRangeCombo->SetSelectedOption(DynamicRangeToString(A->DynamicRange));AudioOutputCombo->SetSelectedOption(OutputModeToString(A->OutputMode));
    UOCPlayerUserSettings* P=UOCPlayerUserSettings::Get();MouseSensitivitySlider->SetValue(FMath::GetRangePct(0.10f,4.0f,P->MouseSensitivity));AimSensitivitySlider->SetValue(FMath::GetRangePct(0.25f,1.50f,P->AimSensitivityMultiplier));InvertYCheck->SetIsChecked(P->bInvertMouseY);FOVSlider->SetValue(FMath::GetRangePct(75.0f,120.0f,P->FieldOfView));HUDScaleSlider->SetValue(FMath::GetRangePct(0.75f,1.25f,P->HUDScale));ShowFPSCheck->SetIsChecked(P->bShowFPS);ShowPingCheck->SetIsChecked(P->bShowPing);ShowCrosshairCheck->SetIsChecked(P->bShowCrosshair);ShowHitMarkerCheck->SetIsChecked(P->bShowHitMarker);GoreCombo->SetSelectedOption(P->GoreLevel<=0?L(LOCTEXT("GoreOff", "Вимкнено")):(P->GoreLevel==1?L(LOCTEXT("GoreReduced", "Зменшено")):L(LOCTEXT("GoreFull", "Повністю"))));SubtitlesCheck->SetIsChecked(P->bSubtitles);ReduceFlashesCheck->SetIsChecked(P->bReduceFlashes);CameraShakeSlider->SetValue(P->CameraShakeScale);FString Color=L(LOCTEXT("ColorOff", "Вимкнено"));if(P->ColorVisionMode==EOCColorVisionMode::Deuteranopia)Color=L(LOCTEXT("ColorDeuteranopia", "Дейтеранопія"));else if(P->ColorVisionMode==EOCColorVisionMode::Protanopia)Color=L(LOCTEXT("ColorProtanopia", "Протанопія"));else if(P->ColorVisionMode==EOCColorVisionMode::Tritanopia)Color=L(LOCTEXT("ColorTritanopia", "Тританопія"));ColorVisionCombo->SetSelectedOption(Color);RefreshKeyBindingLabels();RefreshSettingsLabels();
}

void UOCGameUIRootWidget::ApplySettingsWidgets(bool bCloseAfterApply)
{
    if(UGameUserSettings* GU=GEngine?GEngine->GetGameUserSettings():nullptr)
    {
        FString X,Y;if(ResolutionCombo->GetSelectedOption().Split(TEXT("x"),&X,&Y))GU->SetScreenResolution(FIntPoint(FCString::Atoi(*X),FCString::Atoi(*Y)));GU->SetFullscreenMode(WindowModeFromString(WindowModeCombo->GetSelectedOption()));GU->SetVSyncEnabled(VSyncCheck->IsChecked());GU->SetDynamicResolutionEnabled(DynamicResolutionCheck->IsChecked());const FString Limit=FrameLimitCombo->GetSelectedOption();GU->SetFrameRateLimit(Limit==L(LOCTEXT("Unlimited", "Без обмеження"))?0.0f:FCString::Atof(*Limit));GU->SetResolutionScaleValueEx(FMath::Lerp(50.0f,100.0f,ResolutionScaleSlider->GetValue()));
        const int32 Preset=QualityFromString(QualityPresetCombo->GetSelectedOption());if(Preset>=0)GU->SetOverallScalabilityLevel(Preset);GU->SetViewDistanceQuality(QualityFromString(ViewDistanceCombo->GetSelectedOption()));GU->SetShadowQuality(QualityFromString(ShadowCombo->GetSelectedOption()));GU->SetTextureQuality(QualityFromString(TextureCombo->GetSelectedOption()));GU->SetVisualEffectQuality(QualityFromString(EffectsCombo->GetSelectedOption()));GU->SetFoliageQuality(QualityFromString(FoliageCombo->GetSelectedOption()));GU->SetPostProcessingQuality(QualityFromString(PostProcessCombo->GetSelectedOption()));GU->SetAntiAliasingQuality(QualityFromString(AntiAliasingCombo->GetSelectedOption()));GU->SetShadingQuality(QualityFromString(ShadingCombo->GetSelectedOption()));GU->SetGlobalIlluminationQuality(QualityFromString(GlobalIlluminationCombo->GetSelectedOption()));GU->SetReflectionQuality(QualityFromString(ReflectionCombo->GetSelectedOption()));GU->SetLandscapeQuality(QualityFromString(LandscapeCombo->GetSelectedOption()));GU->ApplySettings(false);GU->ConfirmVideoMode();
    }
    UOCAudioUserSettings* A=UOCAudioUserSettings::Get();auto ApplyBus=[A](EOCAudioBus B,USlider* S,UCheckBox* C){if(!S||!C)return;A->SetBusPercent(B,FMath::RoundToInt(S->GetValue()*100));A->SetBusEnabled(B,C->IsChecked());};ApplyBus(EOCAudioBus::Master,MasterVolumeSlider,MasterAudioCheck);ApplyBus(EOCAudioBus::Weapons,WeaponsVolumeSlider,WeaponsAudioCheck);ApplyBus(EOCAudioBus::Vehicles,VehiclesVolumeSlider,VehiclesAudioCheck);ApplyBus(EOCAudioBus::Characters,CharactersVolumeSlider,CharactersAudioCheck);ApplyBus(EOCAudioBus::WorldSFX,WorldSFXVolumeSlider,WorldSFXAudioCheck);ApplyBus(EOCAudioBus::Ambience,AmbienceVolumeSlider,AmbienceAudioCheck);ApplyBus(EOCAudioBus::Music,MusicVolumeSlider,MusicAudioCheck);ApplyBus(EOCAudioBus::UI,UIVolumeSlider,UIAudioCheck);ApplyBus(EOCAudioBus::VoiceChat,VoiceChatVolumeSlider,VoiceChatAudioCheck);ApplyBus(EOCAudioBus::Dialogue,DialogueVolumeSlider,DialogueAudioCheck);A->bMenuMusicEnabled=MenuMusicCheck->IsChecked();A->DynamicRange=DynamicRangeFromString(DynamicRangeCombo->GetSelectedOption());A->OutputMode=OutputModeFromString(AudioOutputCombo->GetSelectedOption());A->SaveAudioSettings();
    UOCPlayerUserSettings* P=UOCPlayerUserSettings::Get();P->MouseSensitivity=FMath::Lerp(0.10f,4.0f,MouseSensitivitySlider->GetValue());P->AimSensitivityMultiplier=FMath::Lerp(0.25f,1.50f,AimSensitivitySlider->GetValue());P->bInvertMouseY=InvertYCheck->IsChecked();P->FieldOfView=FMath::Lerp(75.0f,120.0f,FOVSlider->GetValue());P->HUDScale=FMath::Lerp(0.75f,1.25f,HUDScaleSlider->GetValue());P->bShowFPS=ShowFPSCheck->IsChecked();P->bShowPing=ShowPingCheck->IsChecked();P->bShowCrosshair=ShowCrosshairCheck->IsChecked();P->bShowHitMarker=ShowHitMarkerCheck->IsChecked();const FString Gore=GoreCombo->GetSelectedOption();P->GoreLevel=Gore==L(LOCTEXT("GoreOff", "Вимкнено"))?0:(Gore==L(LOCTEXT("GoreReduced", "Зменшено"))?1:2);P->bSubtitles=SubtitlesCheck->IsChecked();P->bReduceFlashes=ReduceFlashesCheck->IsChecked();P->CameraShakeScale=CameraShakeSlider->GetValue();const FString CV=ColorVisionCombo->GetSelectedOption();P->ColorVisionMode=CV==L(LOCTEXT("ColorDeuteranopia", "Дейтеранопія"))?EOCColorVisionMode::Deuteranopia:(CV==L(LOCTEXT("ColorProtanopia", "Протанопія"))?EOCColorVisionMode::Protanopia:(CV==L(LOCTEXT("ColorTritanopia", "Тританопія"))?EOCColorVisionMode::Tritanopia:EOCColorVisionMode::Off));P->SavePlayerSettings();
    if(AOCPlayerController* PC=Cast<AOCPlayerController>(GetOwningPlayer())){PC->UIApplyLocalPreferences();if(bCloseAfterApply)PC->UICloseSettings();}
    if(SettingsStatusText)SettingsStatusText->SetText(LOCTEXT("SettingsSaved", "Налаштування застосовано та збережено."));
}

void UOCGameUIRootWidget::CancelPendingSettings()
{
    UOCPlayerUserSettings::Get()->ReloadConfig();UOCAudioUserSettings::Get()->ReloadConfig();if(UGameUserSettings* GU=GEngine?GEngine->GetGameUserSettings():nullptr)GU->LoadSettings(true);if(AOCPlayerController* PC=Cast<AOCPlayerController>(GetOwningPlayer())){PC->UIApplyLocalPreferences();PC->UICloseSettings();}PendingRebindAction=NAME_None;SyncSettingsWidgetsFromBackend();
}

void UOCGameUIRootWidget::ResetSettingsWidgetsToDefaults()
{
    if(UGameUserSettings* GU=GEngine?GEngine->GetGameUserSettings():nullptr)GU->SetToDefaults();UOCAudioUserSettings::Get()->ResetAudioDefaults();UOCPlayerUserSettings::Get()->ResetPlayerDefaults();SyncSettingsWidgetsFromBackend();if(SettingsStatusText)SettingsStatusText->SetText(LOCTEXT("DefaultsStaged", "Типові значення підготовлено. Натисніть Застосувати або Зберегти й назад."));
}

void UOCGameUIRootWidget::RefreshKeyBindingLabels(){const UOCPlayerUserSettings* P=UOCPlayerUserSettings::Get();for(const auto& Pair:KeyBindingTexts)if(Pair.Value)Pair.Value->SetText(FText::FromString(P->GetKey(Pair.Key).GetDisplayName().ToString()));}
void UOCGameUIRootWidget::BeginRebind(FName ActionId){PendingRebindAction=ActionId;SetKeyboardFocus();if(SettingsStatusText){FFormatNamedArguments Args;Args.Add(TEXT("Action"),ActionText(ActionId));SettingsStatusText->SetText(FText::Format(LOCTEXT("RebindPrompt", "Натисніть нову клавішу для {Action}. Escape / B — скасувати."),Args));}}

void UOCGameUIRootWidget::OnConnectClicked(){if(AOCPlayerController*PC=Cast<AOCPlayerController>(GetOwningPlayer()))PC->UIConnect(AddressEntry?AddressEntry->GetText().ToString():TEXT("127.0.0.1:7777"),UsernameEntry?UsernameEntry->GetText().ToString():TEXT("Player"));}
void UOCGameUIRootWidget::OnLocalhostClicked(){if(AddressEntry)AddressEntry->SetText(FText::FromString(TEXT("127.0.0.1:7777")));OnConnectClicked();}
void UOCGameUIRootWidget::OnCloseFrontendClicked(){if(AOCPlayerController*PC=Cast<AOCPlayerController>(GetOwningPlayer()))PC->UIToggleFrontend();}
void UOCGameUIRootWidget::OnOpenSettingsClicked(){SyncSettingsWidgetsFromBackend();if(AOCPlayerController*PC=Cast<AOCPlayerController>(GetOwningPlayer()))PC->UIOpenSettings();}
void UOCGameUIRootWidget::OnSettingsGraphicsClicked(){SetSettingsPage(0);}void UOCGameUIRootWidget::OnSettingsAudioClicked(){SetSettingsPage(1);}void UOCGameUIRootWidget::OnSettingsControlsClicked(){SetSettingsPage(2);}void UOCGameUIRootWidget::OnSettingsInterfaceClicked(){SetSettingsPage(3);}void UOCGameUIRootWidget::OnSettingsAccessibilityClicked(){SetSettingsPage(4);}
void UOCGameUIRootWidget::OnQualityPresetChanged(FString SelectedItem,ESelectInfo::Type SelectionType){const int32 Q=QualityFromString(SelectedItem);if(Q<0)return;const FString V=QualityToString(Q);ViewDistanceCombo->SetSelectedOption(V);ShadowCombo->SetSelectedOption(V);TextureCombo->SetSelectedOption(V);EffectsCombo->SetSelectedOption(V);FoliageCombo->SetSelectedOption(V);PostProcessCombo->SetSelectedOption(V);AntiAliasingCombo->SetSelectedOption(V);ShadingCombo->SetSelectedOption(V);GlobalIlluminationCombo->SetSelectedOption(V);ReflectionCombo->SetSelectedOption(V);LandscapeCombo->SetSelectedOption(V);}
void UOCGameUIRootWidget::OnSettingsApplyClicked(){ApplySettingsWidgets(false);}void UOCGameUIRootWidget::OnSettingsSaveClicked(){ApplySettingsWidgets(true);}void UOCGameUIRootWidget::OnSettingsCancelClicked(){CancelPendingSettings();}void UOCGameUIRootWidget::OnSettingsDefaultsClicked(){ResetSettingsWidgetsToDefaults();}
void UOCGameUIRootWidget::OnTeamOneClicked(){if(AOCPlayerController*PC=Cast<AOCPlayerController>(GetOwningPlayer()))PC->UIRequestTeam(EOCTeam::TeamOne);}void UOCGameUIRootWidget::OnTeamTwoClicked(){if(AOCPlayerController*PC=Cast<AOCPlayerController>(GetOwningPlayer()))PC->UIRequestTeam(EOCTeam::TeamTwo);}void UOCGameUIRootWidget::OnRoleClicked(){if(AOCPlayerController*PC=Cast<AOCPlayerController>(GetOwningPlayer()))PC->UICycleRole();}void UOCGameUIRootWidget::OnSquadClicked(){if(AOCPlayerController*PC=Cast<AOCPlayerController>(GetOwningPlayer()))PC->UICycleSquad();}
void UOCGameUIRootWidget::OnSpawnBaseClicked(){SelectedSpawnId=TEXT("BASE");if(AOCPlayerController*PC=Cast<AOCPlayerController>(GetOwningPlayer()))PC->UISelectSpawn(SelectedSpawnId);}void UOCGameUIRootWidget::OnSpawnAClicked(){SelectedSpawnId=TEXT("A");if(AOCPlayerController*PC=Cast<AOCPlayerController>(GetOwningPlayer()))PC->UISelectSpawn(SelectedSpawnId);}void UOCGameUIRootWidget::OnSpawnBClicked(){SelectedSpawnId=TEXT("B");if(AOCPlayerController*PC=Cast<AOCPlayerController>(GetOwningPlayer()))PC->UISelectSpawn(SelectedSpawnId);}void UOCGameUIRootWidget::OnSpawnCClicked(){SelectedSpawnId=TEXT("C");if(AOCPlayerController*PC=Cast<AOCPlayerController>(GetOwningPlayer()))PC->UISelectSpawn(SelectedSpawnId);}void UOCGameUIRootWidget::OnDeployClicked(){if(AOCPlayerController*PC=Cast<AOCPlayerController>(GetOwningPlayer()))PC->UIReadyDeploy();}
void UOCGameUIRootWidget::OnChatChannelClicked(){SelectedChatChannel=SelectedChatChannel==EOCChatChannel::Global?EOCChatChannel::Team:(SelectedChatChannel==EOCChatChannel::Team?EOCChatChannel::Squad:EOCChatChannel::Global);}void UOCGameUIRootWidget::OnChatSendClicked(){if(!ChatEntry)return;const FString Text=ChatEntry->GetText().ToString();if(AOCPlayerController*PC=Cast<AOCPlayerController>(GetOwningPlayer())){PC->UISendChat(SelectedChatChannel,Text);PC->UIEndChatInput();}ChatEntry->SetText(FText::GetEmpty());}void UOCGameUIRootWidget::OnChatCommitted(const FText& Text,ETextCommit::Type CommitMethod){if(CommitMethod==ETextCommit::OnEnter)OnChatSendClicked();}
void UOCGameUIRootWidget::OnAdminPrevClicked(){if(AOCPlayerController*PC=Cast<AOCPlayerController>(GetOwningPlayer()))PC->UIAdminPrevious();}void UOCGameUIRootWidget::OnAdminNextClicked(){if(AOCPlayerController*PC=Cast<AOCPlayerController>(GetOwningPlayer()))PC->UIAdminNext();}void UOCGameUIRootWidget::OnAdminExecuteClicked(){if(AOCPlayerController*PC=Cast<AOCPlayerController>(GetOwningPlayer()))PC->UIAdminExecute();}

void UOCGameUIRootWidget::OnBindMoveForward(){BeginRebind(TEXT("MoveForward"));}void UOCGameUIRootWidget::OnBindMoveBackward(){BeginRebind(TEXT("MoveBackward"));}void UOCGameUIRootWidget::OnBindMoveLeft(){BeginRebind(TEXT("MoveLeft"));}void UOCGameUIRootWidget::OnBindMoveRight(){BeginRebind(TEXT("MoveRight"));}void UOCGameUIRootWidget::OnBindJump(){BeginRebind(TEXT("Jump"));}void UOCGameUIRootWidget::OnBindSprint(){BeginRebind(TEXT("Sprint"));}void UOCGameUIRootWidget::OnBindCrouch(){BeginRebind(TEXT("Crouch"));}void UOCGameUIRootWidget::OnBindFire(){BeginRebind(TEXT("Fire"));}void UOCGameUIRootWidget::OnBindAim(){BeginRebind(TEXT("Aim"));}void UOCGameUIRootWidget::OnBindReload(){BeginRebind(TEXT("Reload"));}void UOCGameUIRootWidget::OnBindInteract(){BeginRebind(TEXT("Interact"));}void UOCGameUIRootWidget::OnBindGrenade(){BeginRebind(TEXT("ThrowGrenade"));}void UOCGameUIRootWidget::OnBindScoreboard(){BeginRebind(TEXT("Scoreboard"));}void UOCGameUIRootWidget::OnBindChat(){BeginRebind(TEXT("Chat"));}

#undef LOCTEXT_NAMESPACE
