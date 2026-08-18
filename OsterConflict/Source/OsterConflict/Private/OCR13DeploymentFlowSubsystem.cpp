#include "OCR13DeploymentFlowSubsystem.h"

#include "OCCapturePoint.h"
#include "OCGameState.h"
#include "OCGameUIRootWidget.h"
#include "OCPlayerController.h"
#include "OCPlayerState.h"
#include "OCTeamSpawnPoint.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/PanelWidget.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Widget.h"
#include "Components/WidgetSwitcher.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerState.h"
#include "Styling/SlateTypes.h"
#include "UObject/UObjectIterator.h"

namespace
{
    const FLinearColor FlowPanelColor(0.010f, 0.014f, 0.018f, 0.985f);
    const FLinearColor FlowSectionColor(0.026f, 0.032f, 0.038f, 0.985f);
    const FLinearColor FlowTextColor(0.94f, 0.95f, 0.95f, 1.0f);
    const FLinearColor FlowMutedColor(0.62f, 0.66f, 0.69f, 1.0f);
    const FLinearColor FlowButtonColor(0.075f, 0.085f, 0.095f, 0.96f);
    const FLinearColor FlowButtonHover(0.20f, 0.23f, 0.25f, 1.0f);
    const FLinearColor FlowButtonPressed(0.30f, 0.26f, 0.17f, 1.0f);

    FString TeamLabel(const EOCTeam Team)
    {
        if (Team == EOCTeam::TeamOne) return TEXT("КОМАНДА 1");
        if (Team == EOCTeam::TeamTwo) return TEXT("КОМАНДА 2");
        return TEXT("НЕ ВИБРАНО");
    }

    FString SquadLabel(const int32 Squad)
    {
        static const TCHAR* Names[] = { TEXT("АЛЬФА"), TEXT("БРАВО"), TEXT("ЧАРЛІ"), TEXT("ДЕЛЬТА") };
        return Squad >= 0 && Squad < UE_ARRAY_COUNT(Names) ? FString(Names[Squad]) : FString(TEXT("НЕ ВИБРАНО"));
    }

    FString RoleLabel(const EOCPlayerRole Role)
    {
        switch (Role)
        {
        case EOCPlayerRole::Medic: return TEXT("МЕДИК");
        case EOCPlayerRole::Engineer: return TEXT("ІНЖЕНЕР");
        case EOCPlayerRole::Support: return TEXT("ПІДТРИМКА");
        default: return TEXT("ШТУРМОВИК");
        }
    }

    FString SpawnLabel(const FName Spawn)
    {
        if (Spawn == TEXT("A")) return TEXT("ТОЧКА A");
        if (Spawn == TEXT("B")) return TEXT("ТОЧКА B");
        if (Spawn == TEXT("C")) return TEXT("ТОЧКА C");
        if (Spawn == TEXT("BASE")) return TEXT("БАЗА");
        return TEXT("НЕ ВИБРАНО");
    }

    UTextBlock* MakeFlowText(UObject* Owner, const FString& Text, const int32 Size, const bool bStrong = false)
    {
        UTextBlock* Block = NewObject<UTextBlock>(Owner);
        if (!Block) return nullptr;
        Block->SetText(FText::FromString(Text));
        Block->SetColorAndOpacity(FSlateColor(bStrong ? FlowTextColor : FlowMutedColor));
        Block->SetAutoWrapText(true);
        FSlateFontInfo Font = Block->GetFont();
        Font.Size = Size;
        Block->SetFont(Font);
        return Block;
    }

    void SetFill(UHorizontalBoxSlot* Slot, const float Weight)
    {
        if (!Slot) return;
        FSlateChildSize Size;
        Size.SizeRule = ESlateSizeRule::Fill;
        Size.Value = Weight;
        Slot->SetSize(Size);
    }
}

bool UOCR13DeploymentFlowSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

UButton* UOCR13DeploymentFlowSubsystem::MakeActionButton(UObject* Owner, const FString& Label, UTextBlock*& OutLabel)
{
    UButton* Button = NewObject<UButton>(Owner);
    OutLabel = MakeFlowText(Owner, Label, 18, true);
    if (!Button || !OutLabel) return Button;

    FButtonStyle Style = Button->GetStyle();
    Style.Normal.TintColor = FSlateColor(FlowButtonColor);
    Style.Hovered.TintColor = FSlateColor(FlowButtonHover);
    Style.Pressed.TintColor = FSlateColor(FlowButtonPressed);
    Style.Disabled.TintColor = FSlateColor(FLinearColor(0.035f, 0.040f, 0.045f, 0.75f));
    Style.NormalPadding = FMargin(1.0f);
    Style.PressedPadding = FMargin(1.0f, 2.0f, 1.0f, 0.0f);
    Button->SetStyle(Style);
    Button->SetBackgroundColor(FLinearColor::White);
    Button->AddChild(OutLabel);
    return Button;
}

UBorder* UOCR13DeploymentFlowSubsystem::MakeSection(UObject* Owner, const FString& Title, UTextBlock*& OutBody)
{
    UBorder* Border = NewObject<UBorder>(Owner);
    if (!Border) return nullptr;
    Border->SetBrushColor(FlowSectionColor);
    Border->SetPadding(FMargin(16.0f));

    UVerticalBox* Box = NewObject<UVerticalBox>(Owner);
    Border->SetContent(Box);
    if (UTextBlock* Header = MakeFlowText(Owner, Title, 15, true))
    {
        Box->AddChildToVerticalBox(Header)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
    }
    OutBody = MakeFlowText(Owner, TEXT(""), 14, false);
    if (OutBody) Box->AddChildToVerticalBox(OutBody);
    return Border;
}

void UOCR13DeploymentFlowSubsystem::EnsureBuilt(UOCGameUIRootWidget* Root, AOCPlayerController* PC)
{
    if (!Root || !PC) return;
    if (ActiveRoot.Get() == Root && FlowPanel.IsValid()) return;

    ActiveRoot = Root;
    FlowPanel.Reset();
    PageSwitcher.Reset();
    SquadButtons.Reset();
    SquadButtonTexts.Reset();
    RoleButtons.Reset();
    RoleButtonTexts.Reset();
    SpawnButtons.Reset();
    SpawnButtonTexts.Reset();

    UCanvasPanel* Canvas = Cast<UCanvasPanel>(Root->GetWidgetFromName(TEXT("OC_UI_Root")));
    if (!Canvas) return;

    // The old panel is deliberately kept for source compatibility, but it must never flash through the new flow.
    if (UWidget* Legacy = Root->GetWidgetFromName(TEXT("DeploymentPanel")))
    {
        Legacy->SetRenderOpacity(0.0f);
        Legacy->SetIsEnabled(false);
    }

    UBorder* Panel = NewObject<UBorder>(Root, TEXT("R13_DeploymentFlowPanel"));
    if (!Panel) return;
    Panel->SetBrushColor(FlowPanelColor);
    Panel->SetPadding(FMargin(28.0f));
    Panel->SetVisibility(ESlateVisibility::Collapsed);

    UHorizontalBox* Columns = NewObject<UHorizontalBox>(Root);
    Panel->SetContent(Columns);

    UVerticalBox* Main = NewObject<UVerticalBox>(Root);
    UHorizontalBoxSlot* MainSlot = Columns->AddChildToHorizontalBox(Main);
    SetFill(MainSlot, 0.72f);
    MainSlot->SetPadding(FMargin(0.0f, 0.0f, 24.0f, 0.0f));

    if (UTextBlock* Header = MakeFlowText(Root, TEXT("РОЗГОРТАННЯ"), 32, true))
        Main->AddChildToVerticalBox(Header);
    StepText = MakeFlowText(Root, TEXT("КРОК 1 / 4 · КОМАНДА"), 14, false);
    if (StepText.IsValid()) Main->AddChildToVerticalBox(StepText.Get())->SetPadding(FMargin(0.0f, 5.0f, 0.0f, 18.0f));

    UWidgetSwitcher* Switcher = NewObject<UWidgetSwitcher>(Root, TEXT("R13_DeploymentStepSwitcher"));
    PageSwitcher = Switcher;
    if (UVerticalBoxSlot* SwitcherSlot = Main->AddChildToVerticalBox(Switcher))
    {
        FSlateChildSize FillSize;
        FillSize.SizeRule = ESlateSizeRule::Fill;
        FillSize.Value = 1.0f;
        SwitcherSlot->SetSize(FillSize);
    }

    auto NewPage = [Root, Switcher](const FString& Title, const FString& Subtitle) -> UVerticalBox*
    {
        UVerticalBox* Page = NewObject<UVerticalBox>(Root);
        Switcher->AddChild(Page);
        if (UTextBlock* T = MakeFlowText(Root, Title, 24, true))
            Page->AddChildToVerticalBox(T)->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 6.0f));
        if (UTextBlock* S = MakeFlowText(Root, Subtitle, 14, false))
            Page->AddChildToVerticalBox(S)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 18.0f));
        return Page;
    };

    UVerticalBox* TeamPage = NewPage(
        TEXT("ОБЕРІТЬ КОМАНДУ"),
        TEXT("Спочатку оберіть сторону. Склад груп, ролі та точки появи відкриються на наступних кроках."));
    UTextBlock* TeamOneText = nullptr;
    UButton* TeamOne = MakeActionButton(Root, TEXT("КОМАНДА 1"), TeamOneText);
    TeamOne->OnClicked.AddDynamic(this, &UOCR13DeploymentFlowSubsystem::OnTeamOne);
    TeamPage->AddChildToVerticalBox(TeamOne)->SetPadding(FMargin(0.0f, 3.0f, 0.0f, 8.0f));
    UTextBlock* TeamTwoText = nullptr;
    UButton* TeamTwo = MakeActionButton(Root, TEXT("КОМАНДА 2"), TeamTwoText);
    TeamTwo->OnClicked.AddDynamic(this, &UOCR13DeploymentFlowSubsystem::OnTeamTwo);
    TeamPage->AddChildToVerticalBox(TeamTwo)->SetPadding(FMargin(0.0f, 3.0f, 0.0f, 8.0f));

    UVerticalBox* SquadPage = NewPage(
        TEXT("ОБЕРІТЬ ГРУПУ"),
        TEXT("У групі максимум 4 бійці. Заповнені групи недоступні."));
    const TCHAR* SquadNames[] = { TEXT("АЛЬФА"), TEXT("БРАВО"), TEXT("ЧАРЛІ"), TEXT("ДЕЛЬТА") };
    for (int32 Index = 0; Index < 4; ++Index)
    {
        UTextBlock* Label = nullptr;
        UButton* Button = MakeActionButton(Root, SquadNames[Index], Label);
        SquadButtons.Add(Button);
        SquadButtonTexts.Add(Label);
        SquadPage->AddChildToVerticalBox(Button)->SetPadding(FMargin(0.0f, 2.0f, 0.0f, 7.0f));
    }
    SquadButtons[0]->OnClicked.AddDynamic(this, &UOCR13DeploymentFlowSubsystem::OnSquadAlpha);
    SquadButtons[1]->OnClicked.AddDynamic(this, &UOCR13DeploymentFlowSubsystem::OnSquadBravo);
    SquadButtons[2]->OnClicked.AddDynamic(this, &UOCR13DeploymentFlowSubsystem::OnSquadCharlie);
    SquadButtons[3]->OnClicked.AddDynamic(this, &UOCR13DeploymentFlowSubsystem::OnSquadDelta);

    UVerticalBox* RolePage = NewPage(
        TEXT("ОБЕРІТЬ РОЛЬ"),
        TEXT("Штурмовик є універсальним слотом. Медик, інженер і підтримка мають по одному слоту в групі."));
    const TCHAR* RoleNames[] = { TEXT("ШТУРМОВИК"), TEXT("МЕДИК"), TEXT("ІНЖЕНЕР"), TEXT("ПІДТРИМКА") };
    for (int32 Index = 0; Index < 4; ++Index)
    {
        UTextBlock* Label = nullptr;
        UButton* Button = MakeActionButton(Root, RoleNames[Index], Label);
        RoleButtons.Add(Button);
        RoleButtonTexts.Add(Label);
        RolePage->AddChildToVerticalBox(Button)->SetPadding(FMargin(0.0f, 2.0f, 0.0f, 7.0f));
    }
    RoleButtons[0]->OnClicked.AddDynamic(this, &UOCR13DeploymentFlowSubsystem::OnRoleRifleman);
    RoleButtons[1]->OnClicked.AddDynamic(this, &UOCR13DeploymentFlowSubsystem::OnRoleMedic);
    RoleButtons[2]->OnClicked.AddDynamic(this, &UOCR13DeploymentFlowSubsystem::OnRoleEngineer);
    RoleButtons[3]->OnClicked.AddDynamic(this, &UOCR13DeploymentFlowSubsystem::OnRoleSupport);

    UVerticalBox* SpawnPage = NewPage(
        TEXT("ОБЕРІТЬ ТОЧКУ ПОЯВИ"),
        TEXT("База доступна завжди. Передові точки доступні лише коли їх контролює ваша команда і вони не оспорюються."));
    SpawnSelectionText = MakeFlowText(Root, TEXT("ТОЧКА: НЕ ВИБРАНО"), 15, true);
    if (SpawnSelectionText.IsValid()) SpawnPage->AddChildToVerticalBox(SpawnSelectionText.Get())->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 12.0f));
    const TCHAR* SpawnNames[] = { TEXT("БАЗА"), TEXT("ТОЧКА A"), TEXT("ТОЧКА B"), TEXT("ТОЧКА C") };
    for (int32 Index = 0; Index < 4; ++Index)
    {
        UTextBlock* Label = nullptr;
        UButton* Button = MakeActionButton(Root, SpawnNames[Index], Label);
        SpawnButtons.Add(Button);
        SpawnButtonTexts.Add(Label);
        SpawnPage->AddChildToVerticalBox(Button)->SetPadding(FMargin(0.0f, 2.0f, 0.0f, 7.0f));
    }
    SpawnButtons[0]->OnClicked.AddDynamic(this, &UOCR13DeploymentFlowSubsystem::OnSpawnBase);
    SpawnButtons[1]->OnClicked.AddDynamic(this, &UOCR13DeploymentFlowSubsystem::OnSpawnA);
    SpawnButtons[2]->OnClicked.AddDynamic(this, &UOCR13DeploymentFlowSubsystem::OnSpawnB);
    SpawnButtons[3]->OnClicked.AddDynamic(this, &UOCR13DeploymentFlowSubsystem::OnSpawnC);

    StatusText = MakeFlowText(Root, TEXT("Оберіть команду."), 13, false);
    if (StatusText.IsValid()) Main->AddChildToVerticalBox(StatusText.Get())->SetPadding(FMargin(0.0f, 14.0f, 0.0f, 8.0f));

    UHorizontalBox* Footer = NewObject<UHorizontalBox>(Root);
    Main->AddChildToVerticalBox(Footer)->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 0.0f));
    UTextBlock* BackLabel = nullptr;
    UButton* Back = MakeActionButton(Root, TEXT("НАЗАД"), BackLabel);
    Back->OnClicked.AddDynamic(this, &UOCR13DeploymentFlowSubsystem::OnBack);
    BackButton = Back;
    UHorizontalBoxSlot* BackSlot = Footer->AddChildToHorizontalBox(Back);
    SetFill(BackSlot, 0.35f);
    BackSlot->SetPadding(FMargin(0.0f, 0.0f, 10.0f, 0.0f));

    UTextBlock* DeployLabel = nullptr;
    UButton* Deploy = MakeActionButton(Root, TEXT("ПОЯВИТИСЯ"), DeployLabel);
    Deploy->OnClicked.AddDynamic(this, &UOCR13DeploymentFlowSubsystem::OnDeploy);
    DeployButton = Deploy;
    UHorizontalBoxSlot* DeploySlot = Footer->AddChildToHorizontalBox(Deploy);
    SetFill(DeploySlot, 0.65f);

    UVerticalBox* Info = NewObject<UVerticalBox>(Root);
    UHorizontalBoxSlot* InfoSlot = Columns->AddChildToHorizontalBox(Info);
    SetFill(InfoSlot, 0.28f);

    UTextBlock* MatchBody = nullptr;
    if (UBorder* Section = MakeSection(Root, TEXT("МАТЧ"), MatchBody))
    {
        MatchText = MatchBody;
        Info->AddChildToVerticalBox(Section)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
    }
    UTextBlock* SelectionBody = nullptr;
    if (UBorder* Section = MakeSection(Root, TEXT("ВАШ ВИБІР"), SelectionBody))
    {
        SelectionText = SelectionBody;
        Info->AddChildToVerticalBox(Section)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
    }
    UTextBlock* SquadBody = nullptr;
    if (UBorder* Section = MakeSection(Root, TEXT("СКЛАД ГРУПИ"), SquadBody))
    {
        SquadRosterText = SquadBody;
        Info->AddChildToVerticalBox(Section);
    }

    if (UCanvasPanelSlot* Slot = Canvas->AddChildToCanvas(Panel))
    {
        Slot->SetPosition(FVector2D(90.0f, 70.0f));
        Slot->SetSize(FVector2D(1420.0f, 760.0f));
        Slot->SetZOrder(9200);
    }

    FlowPanel = Panel;
    ResetFlow();
}

void UOCR13DeploymentFlowSubsystem::ResetFlow()
{
    CurrentStep = 0;
    SelectedTeam = EOCTeam::None;
    SelectedSquad = INDEX_NONE;
    SelectedRole = EOCPlayerRole::Rifleman;
    bRoleSelected = false;
    SelectedSpawn = NAME_None;
    SetStep(0);
    if (StatusText.IsValid()) StatusText->SetText(FText::FromString(TEXT("Оберіть команду.")));
}

void UOCR13DeploymentFlowSubsystem::SetStep(const int32 NewStep)
{
    CurrentStep = FMath::Clamp(NewStep, 0, 3);
    if (PageSwitcher.IsValid()) PageSwitcher->SetActiveWidgetIndex(CurrentStep);

    static const TCHAR* StepNames[] = { TEXT("КОМАНДА"), TEXT("ГРУПА"), TEXT("РОЛЬ"), TEXT("ТОЧКА ПОЯВИ") };
    if (StepText.IsValid())
    {
        StepText->SetText(FText::FromString(FString::Printf(TEXT("КРОК %d / 4 · %s"), CurrentStep + 1, StepNames[CurrentStep])));
    }
    if (DeployButton.IsValid())
    {
        DeployButton->SetVisibility(CurrentStep == 3 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

int32 UOCR13DeploymentFlowSubsystem::CountSquadMembers(
    const EOCTeam Team, const int32 SquadId, const AOCPlayerController* PC) const
{
    if (Team == EOCTeam::None || SquadId < 0 || !GetWorld()) return 0;
    const AOCPlayerState* Local = PC ? PC->GetPlayerState<AOCPlayerState>() : nullptr;
    const AOCGameState* GameState = GetWorld()->GetGameState<AOCGameState>();
    if (!GameState) return 0;

    int32 Count = 0;
    for (APlayerState* RawState : GameState->PlayerArray)
    {
        const AOCPlayerState* State = Cast<AOCPlayerState>(RawState);
        if (!State || State == Local) continue;
        if (State->GetTeamId() == Team && State->GetSquadId() == SquadId) ++Count;
    }
    return Count;
}

bool UOCR13DeploymentFlowSubsystem::IsRoleAvailable(
    const EOCTeam Team, const int32 SquadId, const EOCPlayerRole Role, const AOCPlayerController* PC) const
{
    if (Team == EOCTeam::None || SquadId < 0 || !GetWorld()) return false;
    if (Role == EOCPlayerRole::Rifleman) return true;

    const AOCPlayerState* Local = PC ? PC->GetPlayerState<AOCPlayerState>() : nullptr;
    const AOCGameState* GameState = GetWorld()->GetGameState<AOCGameState>();
    if (!GameState) return false;
    for (APlayerState* RawState : GameState->PlayerArray)
    {
        const AOCPlayerState* State = Cast<AOCPlayerState>(RawState);
        if (!State || State == Local) continue;
        if (State->GetTeamId() == Team && State->GetSquadId() == SquadId && State->GetPlayerRole() == Role)
            return false;
    }
    return true;
}

bool UOCR13DeploymentFlowSubsystem::IsSpawnAvailable(const EOCTeam Team, const FName SpawnId) const
{
    if (Team == EOCTeam::None || SpawnId.IsNone() || !GetWorld()) return false;
    if (SpawnId == TEXT("BASE")) return true;

    bool bFoundTeamSpawn = false;
    for (TActorIterator<AOCTeamSpawnPoint> It(GetWorld()); It; ++It)
    {
        const AOCTeamSpawnPoint* Spawn = *It;
        if (!Spawn || Spawn->GetTeamId() != Team || Spawn->IsBaseSpawn()) continue;
        if (Spawn->GetLinkedCapturePointId() == SpawnId)
        {
            bFoundTeamSpawn = true;
            if (Spawn->IsAvailableForTeam(Team)) return true;
        }
    }
    if (bFoundTeamSpawn) return false;

    // Remote clients may not own server-only PlayerStart actors, so capture ownership is the replication-safe fallback.
    for (TActorIterator<AOCCapturePoint> It(GetWorld()); It; ++It)
    {
        const AOCCapturePoint* Point = *It;
        if (Point && Point->GetPointId() == SpawnId)
            return Point->GetOwnerTeam() == Team && !Point->IsContested();
    }
    return false;
}

FString UOCR13DeploymentFlowSubsystem::BuildSelectedSquadRoster(
    const EOCTeam Team, const int32 SquadId, const AOCPlayerController* PC) const
{
    if (Team == EOCTeam::None || SquadId < 0 || !GetWorld()) return TEXT("Оберіть групу.");

    FString Roster;
    if (bRoleSelected)
        Roster += FString::Printf(TEXT("ВИ · %s\n"), *RoleLabel(SelectedRole));
    else
        Roster += TEXT("ВИ · роль не вибрана\n");

    const AOCPlayerState* Local = PC ? PC->GetPlayerState<AOCPlayerState>() : nullptr;
    const AOCGameState* GameState = GetWorld()->GetGameState<AOCGameState>();
    if (!GameState) return Roster;

    int32 Added = 0;
    for (APlayerState* RawState : GameState->PlayerArray)
    {
        const AOCPlayerState* State = Cast<AOCPlayerState>(RawState);
        if (!State || State == Local || State->GetTeamId() != Team || State->GetSquadId() != SquadId) continue;
        Roster += FString::Printf(TEXT("%s%s · %s\n"),
            State->IsBotPlayer() ? TEXT("[БОТ] ") : TEXT(""),
            *State->GetPlayerName(), *RoleLabel(State->GetPlayerRole()));
        if (++Added >= 3) break;
    }
    return Roster;
}

void UOCR13DeploymentFlowSubsystem::RefreshState(AOCPlayerController* PC)
{
    if (!PC) return;
    const AOCGameState* GameState = GetWorld() ? GetWorld()->GetGameState<AOCGameState>() : nullptr;

    if (MatchText.IsValid())
    {
        if (GameState)
        {
            MatchText->SetText(FText::FromString(FString::Printf(
                TEXT("ГРАВЦІ: %d\nБОТИ: %d\nЗАГАЛОМ: %d / %d"),
                GameState->GetHumanPlayerCount(), GameState->GetBotPlayerCount(),
                GameState->GetHumanPlayerCount() + GameState->GetBotPlayerCount(),
                GameState->GetTargetPopulation())));
        }
        else
        {
            MatchText->SetText(FText::FromString(TEXT("Очікування стану матчу…")));
        }
    }

    if (SelectionText.IsValid())
    {
        SelectionText->SetText(FText::FromString(FString::Printf(
            TEXT("КОМАНДА: %s\nГРУПА: %s\nРОЛЬ: %s\nПОЯВА: %s"),
            *TeamLabel(SelectedTeam),
            *SquadLabel(SelectedSquad),
            bRoleSelected ? *RoleLabel(SelectedRole) : TEXT("НЕ ВИБРАНО"),
            *SpawnLabel(SelectedSpawn))));
    }

    if (SquadRosterText.IsValid())
    {
        SquadRosterText->SetText(FText::FromString(BuildSelectedSquadRoster(SelectedTeam, SelectedSquad, PC)));
    }

    for (int32 Index = 0; Index < SquadButtons.Num() && Index < 4; ++Index)
    {
        const int32 OtherMembers = CountSquadMembers(SelectedTeam, Index, PC);
        const bool bSelected = SelectedSquad == Index;
        const int32 DisplayMembers = FMath::Clamp(OtherMembers + (bSelected ? 1 : 0), 0, 4);
        const bool bAvailable = OtherMembers < 4 || bSelected;
        if (SquadButtons[Index].IsValid()) SquadButtons[Index]->SetIsEnabled(bAvailable);
        if (SquadButtonTexts.IsValidIndex(Index) && SquadButtonTexts[Index].IsValid())
        {
            SquadButtonTexts[Index]->SetText(FText::FromString(FString::Printf(
                TEXT("%s    %d / 4    %s"), *SquadLabel(Index), DisplayMembers,
                bAvailable ? TEXT("Є МІСЦЕ") : TEXT("ЗАПОВНЕНО"))));
        }
    }

    const EOCPlayerRole Roles[] = {
        EOCPlayerRole::Rifleman, EOCPlayerRole::Medic, EOCPlayerRole::Engineer, EOCPlayerRole::Support
    };
    for (int32 Index = 0; Index < RoleButtons.Num() && Index < UE_ARRAY_COUNT(Roles); ++Index)
    {
        const bool bAvailable = IsRoleAvailable(SelectedTeam, SelectedSquad, Roles[Index], PC) ||
            (bRoleSelected && SelectedRole == Roles[Index]);
        if (RoleButtons[Index].IsValid()) RoleButtons[Index]->SetIsEnabled(bAvailable);
        if (RoleButtonTexts.IsValidIndex(Index) && RoleButtonTexts[Index].IsValid())
        {
            RoleButtonTexts[Index]->SetText(FText::FromString(FString::Printf(
                TEXT("%s    %s"), *RoleLabel(Roles[Index]), bAvailable ? TEXT("ВІЛЬНО") : TEXT("ЗАЙНЯТО"))));
        }
    }

    const FName SpawnIds[] = { TEXT("BASE"), TEXT("A"), TEXT("B"), TEXT("C") };
    for (int32 Index = 0; Index < SpawnButtons.Num() && Index < UE_ARRAY_COUNT(SpawnIds); ++Index)
    {
        const bool bAvailable = IsSpawnAvailable(SelectedTeam, SpawnIds[Index]);
        if (SpawnButtons[Index].IsValid()) SpawnButtons[Index]->SetIsEnabled(bAvailable);
        if (SpawnButtonTexts.IsValidIndex(Index) && SpawnButtonTexts[Index].IsValid())
        {
            SpawnButtonTexts[Index]->SetText(FText::FromString(FString::Printf(
                TEXT("%s    %s"), *SpawnLabel(SpawnIds[Index]), bAvailable ? TEXT("ДОСТУПНА") : TEXT("НЕДОСТУПНА"))));
        }
    }

    if (SpawnSelectionText.IsValid())
        SpawnSelectionText->SetText(FText::FromString(TEXT("ТОЧКА: ") + SpawnLabel(SelectedSpawn)));

    if (DeployButton.IsValid())
    {
        DeployButton->SetIsEnabled(CurrentStep == 3 && SelectedTeam != EOCTeam::None &&
            SelectedSquad >= 0 && bRoleSelected && !SelectedSpawn.IsNone() && IsSpawnAvailable(SelectedTeam, SelectedSpawn));
    }
}

void UOCR13DeploymentFlowSubsystem::Tick(float DeltaTime)
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

    EnsureBuilt(Root, PC);
    if (!FlowPanel.IsValid()) return;

    if (UWidget* Legacy = Root->GetWidgetFromName(TEXT("DeploymentPanel")))
    {
        Legacy->SetRenderOpacity(0.0f);
        Legacy->SetIsEnabled(false);
    }

    const bool bVisible = PC->IsDeploymentPanelVisible() &&
        !PC->IsFrontendMenuVisible() && !PC->IsSettingsVisible();

    if (bVisible && !bWasVisible)
    {
        ResetFlow();
    }
    bWasVisible = bVisible;
    FlowPanel->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    if (!bVisible) return;

    RefreshAccumulator += DeltaTime;
    if (RefreshAccumulator >= 0.10f)
    {
        RefreshAccumulator = 0.0f;
        RefreshState(PC);
    }
}

void UOCR13DeploymentFlowSubsystem::OnTeamOne()
{
    AOCPlayerController* PC = GetWorld() ? Cast<AOCPlayerController>(GetWorld()->GetFirstPlayerController()) : nullptr;
    if (!PC) return;
    SelectedTeam = EOCTeam::TeamOne;
    SelectedSquad = INDEX_NONE;
    bRoleSelected = false;
    SelectedSpawn = NAME_None;
    PC->UIRequestTeam(SelectedTeam);
    SetStep(1);
    if (StatusText.IsValid()) StatusText->SetText(FText::FromString(TEXT("Команду вибрано. Оберіть групу.")));
}

void UOCR13DeploymentFlowSubsystem::OnTeamTwo()
{
    AOCPlayerController* PC = GetWorld() ? Cast<AOCPlayerController>(GetWorld()->GetFirstPlayerController()) : nullptr;
    if (!PC) return;
    SelectedTeam = EOCTeam::TeamTwo;
    SelectedSquad = INDEX_NONE;
    bRoleSelected = false;
    SelectedSpawn = NAME_None;
    PC->UIRequestTeam(SelectedTeam);
    SetStep(1);
    if (StatusText.IsValid()) StatusText->SetText(FText::FromString(TEXT("Команду вибрано. Оберіть групу.")));
}

void UOCR13DeploymentFlowSubsystem::OnSquadAlpha(){ AOCPlayerController* PC=GetWorld()?Cast<AOCPlayerController>(GetWorld()->GetFirstPlayerController()):nullptr; if(!PC)return; SelectedSquad=0; bRoleSelected=false; SelectedSpawn=NAME_None; PC->UIRequestSquad(0); SetStep(2); }
void UOCR13DeploymentFlowSubsystem::OnSquadBravo(){ AOCPlayerController* PC=GetWorld()?Cast<AOCPlayerController>(GetWorld()->GetFirstPlayerController()):nullptr; if(!PC)return; SelectedSquad=1; bRoleSelected=false; SelectedSpawn=NAME_None; PC->UIRequestSquad(1); SetStep(2); }
void UOCR13DeploymentFlowSubsystem::OnSquadCharlie(){ AOCPlayerController* PC=GetWorld()?Cast<AOCPlayerController>(GetWorld()->GetFirstPlayerController()):nullptr; if(!PC)return; SelectedSquad=2; bRoleSelected=false; SelectedSpawn=NAME_None; PC->UIRequestSquad(2); SetStep(2); }
void UOCR13DeploymentFlowSubsystem::OnSquadDelta(){ AOCPlayerController* PC=GetWorld()?Cast<AOCPlayerController>(GetWorld()->GetFirstPlayerController()):nullptr; if(!PC)return; SelectedSquad=3; bRoleSelected=false; SelectedSpawn=NAME_None; PC->UIRequestSquad(3); SetStep(2); }

void UOCR13DeploymentFlowSubsystem::OnRoleRifleman(){ AOCPlayerController* PC=GetWorld()?Cast<AOCPlayerController>(GetWorld()->GetFirstPlayerController()):nullptr; if(!PC)return; SelectedRole=EOCPlayerRole::Rifleman; bRoleSelected=true; SelectedSpawn=NAME_None; PC->UIRequestRole(SelectedRole); SetStep(3); }
void UOCR13DeploymentFlowSubsystem::OnRoleMedic(){ AOCPlayerController* PC=GetWorld()?Cast<AOCPlayerController>(GetWorld()->GetFirstPlayerController()):nullptr; if(!PC||!IsRoleAvailable(SelectedTeam,SelectedSquad,EOCPlayerRole::Medic,PC))return; SelectedRole=EOCPlayerRole::Medic; bRoleSelected=true; SelectedSpawn=NAME_None; PC->UIRequestRole(SelectedRole); SetStep(3); }
void UOCR13DeploymentFlowSubsystem::OnRoleEngineer(){ AOCPlayerController* PC=GetWorld()?Cast<AOCPlayerController>(GetWorld()->GetFirstPlayerController()):nullptr; if(!PC||!IsRoleAvailable(SelectedTeam,SelectedSquad,EOCPlayerRole::Engineer,PC))return; SelectedRole=EOCPlayerRole::Engineer; bRoleSelected=true; SelectedSpawn=NAME_None; PC->UIRequestRole(SelectedRole); SetStep(3); }
void UOCR13DeploymentFlowSubsystem::OnRoleSupport(){ AOCPlayerController* PC=GetWorld()?Cast<AOCPlayerController>(GetWorld()->GetFirstPlayerController()):nullptr; if(!PC||!IsRoleAvailable(SelectedTeam,SelectedSquad,EOCPlayerRole::Support,PC))return; SelectedRole=EOCPlayerRole::Support; bRoleSelected=true; SelectedSpawn=NAME_None; PC->UIRequestRole(SelectedRole); SetStep(3); }

void UOCR13DeploymentFlowSubsystem::OnSpawnBase(){ AOCPlayerController* PC=GetWorld()?Cast<AOCPlayerController>(GetWorld()->GetFirstPlayerController()):nullptr; if(!PC)return; SelectedSpawn=TEXT("BASE"); PC->UISelectSpawn(SelectedSpawn); }
void UOCR13DeploymentFlowSubsystem::OnSpawnA(){ AOCPlayerController* PC=GetWorld()?Cast<AOCPlayerController>(GetWorld()->GetFirstPlayerController()):nullptr; if(!PC||!IsSpawnAvailable(SelectedTeam,TEXT("A")))return; SelectedSpawn=TEXT("A"); PC->UISelectSpawn(SelectedSpawn); }
void UOCR13DeploymentFlowSubsystem::OnSpawnB(){ AOCPlayerController* PC=GetWorld()?Cast<AOCPlayerController>(GetWorld()->GetFirstPlayerController()):nullptr; if(!PC||!IsSpawnAvailable(SelectedTeam,TEXT("B")))return; SelectedSpawn=TEXT("B"); PC->UISelectSpawn(SelectedSpawn); }
void UOCR13DeploymentFlowSubsystem::OnSpawnC(){ AOCPlayerController* PC=GetWorld()?Cast<AOCPlayerController>(GetWorld()->GetFirstPlayerController()):nullptr; if(!PC||!IsSpawnAvailable(SelectedTeam,TEXT("C")))return; SelectedSpawn=TEXT("C"); PC->UISelectSpawn(SelectedSpawn); }

void UOCR13DeploymentFlowSubsystem::OnBack()
{
    AOCPlayerController* PC = GetWorld() ? Cast<AOCPlayerController>(GetWorld()->GetFirstPlayerController()) : nullptr;
    if (!PC) return;
    if (CurrentStep <= 0)
    {
        PC->UIToggleFrontend();
        return;
    }

    if (CurrentStep == 3) SelectedSpawn = NAME_None;
    else if (CurrentStep == 2) bRoleSelected = false;
    else if (CurrentStep == 1) SelectedSquad = INDEX_NONE;
    SetStep(CurrentStep - 1);
}

void UOCR13DeploymentFlowSubsystem::OnDeploy()
{
    AOCPlayerController* PC = GetWorld() ? Cast<AOCPlayerController>(GetWorld()->GetFirstPlayerController()) : nullptr;
    if (!PC || CurrentStep != 3 || SelectedTeam == EOCTeam::None || SelectedSquad < 0 ||
        !bRoleSelected || SelectedSpawn.IsNone() || !IsSpawnAvailable(SelectedTeam, SelectedSpawn)) return;

    if (StatusText.IsValid())
        StatusText->SetText(FText::FromString(TEXT("ПЕРЕВІРКА ТОЧКИ ПОЯВИ…")));
    PC->UISelectSpawn(SelectedSpawn);
    PC->UIReadyDeploy();
}

TStatId UOCR13DeploymentFlowSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13DeploymentFlowSubsystem, STATGROUP_Tickables);
}
