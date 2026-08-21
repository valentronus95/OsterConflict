#include "OCRuntimeChatSubsystem.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Widget.h"
#include "Engine/World.h"
#include "InputCoreTypes.h"
#include "OCGameUIRootWidget.h"
#include "OCPlayerController.h"

#define LOCTEXT_NAMESPACE "OCRuntimeChat"

TSharedRef<SWidget> UOCRuntimeChatWidget::RebuildWidget()
{
    if (!RootCanvas)
    {
        BuildWidgetTree();
    }
    return Super::RebuildWidget();
}

void UOCRuntimeChatWidget::BuildWidgetTree()
{
    RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RuntimeChatRoot"));
    WidgetTree->RootWidget = RootCanvas;

    ChatPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RuntimeChatPanel"));
    ChatPanel->SetBrushColor(FLinearColor(0.02f, 0.025f, 0.03f, 0.90f));
    ChatPanel->SetPadding(FMargin(12.0f));

    UVerticalBox* Content = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RuntimeChatContent"));
    ChatPanel->SetContent(Content);

    ChannelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RuntimeChatChannel"));
    ChannelText->SetText(LOCTEXT("TeamChannel", "КАНАЛ: КОМАНДА"));
    Content->AddChildToVerticalBox(ChannelText)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));

    ChatLogText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RuntimeChatLog"));
    ChatLogText->SetAutoWrapText(true);
    Content->AddChildToVerticalBox(ChatLogText)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));

    ChatEntry = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("RuntimeChatEntry"));
    ChatEntry->SetHintText(LOCTEXT("ChatHint", "Введіть повідомлення…"));
    ChatEntry->OnTextCommitted.AddDynamic(this, &UOCRuntimeChatWidget::HandleChatCommitted);
    Content->AddChildToVerticalBox(ChatEntry)->SetPadding(FMargin(0.0f, 2.0f, 0.0f, 7.0f));

    UTextBlock* HelpText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RuntimeChatHelp"));
    HelpText->SetText(LOCTEXT("ChatHelp", "ENTER — надіслати    ESC — закрити"));
    Content->AddChildToVerticalBox(HelpText);

    if (UCanvasPanelSlot* ChatCanvasSlot = RootCanvas->AddChildToCanvas(ChatPanel))
    {
        ChatCanvasSlot->SetAnchors(FAnchors(0.0f, 1.0f, 0.0f, 1.0f));
        ChatCanvasSlot->SetAlignment(FVector2D(0.0f, 1.0f));
        ChatCanvasSlot->SetPosition(FVector2D(28.0f, -28.0f));
        ChatCanvasSlot->SetSize(FVector2D(440.0f, 250.0f));
        ChatCanvasSlot->SetZOrder(0);
    }

    SetVisibility(ESlateVisibility::Collapsed);
}

void UOCRuntimeChatWidget::OpenChannel(EOCChatChannel Channel)
{
    SelectedChannel = Channel == EOCChatChannel::Global ? EOCChatChannel::Global : EOCChatChannel::Team;
    bOpen = true;

    if (ChannelText)
    {
        ChannelText->SetText(SelectedChannel == EOCChatChannel::Global
            ? LOCTEXT("GlobalChannel", "КАНАЛ: УСІ")
            : LOCTEXT("TeamChannelOpen", "КАНАЛ: КОМАНДА"));
    }
    if (ChatEntry)
    {
        ChatEntry->SetText(FText::GetEmpty());
    }

    RefreshMessages();
    SetVisibility(ESlateVisibility::Visible);
    ApplyGameplayInputMode(true);
    if (ChatEntry)
    {
        ChatEntry->SetKeyboardFocus();
    }
}

void UOCRuntimeChatWidget::CloseChat()
{
    const bool bWasOpen = bOpen;
    bOpen = false;
    if (ChatEntry)
    {
        ChatEntry->SetText(FText::GetEmpty());
    }
    SetVisibility(ESlateVisibility::Collapsed);
    if (bWasOpen)
    {
        ApplyGameplayInputMode(false);
    }
}

void UOCRuntimeChatWidget::ApplyGameplayInputMode(bool bChatOpen)
{
    AOCPlayerController* PC = Cast<AOCPlayerController>(GetOwningPlayer());
    if (!PC || !PC->IsLocalController())
    {
        return;
    }

    PC->ResetIgnoreMoveInput();
    PC->ResetIgnoreLookInput();

    if (bChatOpen)
    {
        PC->SetIgnoreMoveInput(true);
        PC->SetIgnoreLookInput(true);
        PC->bShowMouseCursor = false;

        FInputModeGameAndUI Mode;
        Mode.SetHideCursorDuringCapture(true);
        if (ChatEntry)
        {
            Mode.SetWidgetToFocus(ChatEntry->TakeWidget());
        }
        PC->SetInputMode(Mode);
        return;
    }

    // Do not override another UI mode if a menu became active while chat was open.
    if (PC->IsFrontendMenuVisible() || PC->IsSettingsVisible() || PC->IsDeploymentPanelVisible() || PC->IsAdminPanelVisible())
    {
        return;
    }

    PC->bShowMouseCursor = false;
    PC->SetInputMode(FInputModeGameOnly());
}

void UOCRuntimeChatWidget::RefreshMessages()
{
    AOCPlayerController* PC = Cast<AOCPlayerController>(GetOwningPlayer());
    if (!PC || !ChatLogText)
    {
        return;
    }

    const TArray<FOCChatMessage>& Messages = PC->GetRecentChatMessages();
    FString Log;
    const int32 First = FMath::Max(0, Messages.Num() - 6);
    for (int32 Index = First; Index < Messages.Num(); ++Index)
    {
        const FOCChatMessage& Msg = Messages[Index];
        FString ChannelLabel = TEXT("УСІ");
        if (Msg.Channel == EOCChatChannel::Team) ChannelLabel = TEXT("КОМАНДА");
        else if (Msg.Channel == EOCChatChannel::Squad) ChannelLabel = TEXT("ГРУПА");
        Log += FString::Printf(TEXT("[%s] %s: %s\n"), *ChannelLabel, *Msg.SenderName, *Msg.Message);
    }
    ChatLogText->SetText(FText::FromString(Log));
}

void UOCRuntimeChatWidget::HandleChatCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    if (CommitMethod != ETextCommit::OnEnter)
    {
        return;
    }

    const FString Message = Text.ToString();
    if (AOCPlayerController* PC = Cast<AOCPlayerController>(GetOwningPlayer()))
    {
        if (SelectedChannel == EOCChatChannel::Global) PC->SayGlobal(Message);
        else PC->SayTeam(Message);
    }
    CloseChat();
}

void UOCRuntimeChatSubsystem::Tick(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld() || World->GetNetMode() == NM_DedicatedServer)
    {
        return;
    }

    AOCPlayerController* PC = Cast<AOCPlayerController>(World->GetFirstPlayerController());
    if (!PC || !PC->IsLocalController())
    {
        return;
    }

    EnsureWidget(*PC);
    HideLegacyChat(*PC);

    const bool bYDown = PC->IsInputKeyDown(EKeys::Y);
    const bool bUDown = PC->IsInputKeyDown(EKeys::U);
    const bool bEscapeDown = PC->IsInputKeyDown(EKeys::Escape);

    if (ChatWidget.IsValid() && ChatWidget->IsChatOpen())
    {
        ChatWidget->RefreshMessages();
        if ((bEscapeDown && !bEscapeWasDown) || PC->IsFrontendMenuVisible() || PC->IsSettingsVisible() ||
            PC->IsDeploymentPanelVisible() || PC->IsAdminPanelVisible())
        {
            ChatWidget->CloseChat();
        }
    }
    else
    {
        // Disable the obsolete T-toggle path if it is triggered by an old saved binding.
        if (PC->IsChatInputActive())
        {
            PC->UIEndChatInput();
        }

        const bool bOtherUIOpen = PC->IsFrontendMenuVisible() || PC->IsSettingsVisible() ||
            PC->IsDeploymentPanelVisible() || PC->IsAdminPanelVisible();
        if (!bOtherUIOpen && ChatWidget.IsValid())
        {
            if (bYDown && !bYWasDown)
            {
                ChatWidget->OpenChannel(EOCChatChannel::Team);
            }
            else if (bUDown && !bUWasDown)
            {
                ChatWidget->OpenChannel(EOCChatChannel::Global);
            }
        }
    }

    bYWasDown = bYDown;
    bUWasDown = bUDown;
    bEscapeWasDown = bEscapeDown;
}

TStatId UOCRuntimeChatSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCRuntimeChatSubsystem, STATGROUP_Tickables);
}

void UOCRuntimeChatSubsystem::EnsureWidget(AOCPlayerController& PlayerController)
{
    if (ChatWidget.IsValid())
    {
        return;
    }

    UOCRuntimeChatWidget* Widget = CreateWidget<UOCRuntimeChatWidget>(&PlayerController, UOCRuntimeChatWidget::StaticClass());
    if (!Widget)
    {
        return;
    }

    Widget->AddToViewport(520);
    Widget->SetVisibility(ESlateVisibility::Collapsed);
    ChatWidget = Widget;
}

void UOCRuntimeChatSubsystem::HideLegacyChat(AOCPlayerController& PlayerController)
{
    if (!LegacyRoot.IsValid())
    {
        TArray<UUserWidget*> Roots;
        UWidgetBlueprintLibrary::GetAllWidgetsOfClass(&PlayerController, Roots, UOCGameUIRootWidget::StaticClass(), false);
        for (UUserWidget* Candidate : Roots)
        {
            if (UOCGameUIRootWidget* Root = Cast<UOCGameUIRootWidget>(Candidate))
            {
                LegacyRoot = Root;
                break;
            }
        }
    }

    if (LegacyRoot.IsValid())
    {
        if (UWidget* LegacyPanel = LegacyRoot->GetWidgetFromName(TEXT("ChatPanel")))
        {
            LegacyPanel->SetRenderOpacity(0.0f);
            if (!PlayerController.IsChatInputActive())
            {
                LegacyPanel->SetVisibility(ESlateVisibility::Collapsed);
            }
        }
    }
}

#undef LOCTEXT_NAMESPACE
