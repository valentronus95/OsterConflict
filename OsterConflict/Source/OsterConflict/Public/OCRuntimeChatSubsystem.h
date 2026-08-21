#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCLobbyTypes.h"
#include "OCRuntimeChatSubsystem.generated.h"

class AOCPlayerController;
class UBorder;
class UCanvasPanel;
class UEditableTextBox;
class UTextBlock;
class UOCGameUIRootWidget;

/**
 * Small gameplay-only chat surface used by the runtime Y/U contract.
 * The legacy source-built ChatPanel is kept hidden so it cannot remain permanently on screen.
 */
UCLASS()
class OSTERCONFLICT_API UOCRuntimeChatWidget final : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual TSharedRef<SWidget> RebuildWidget() override;

    void OpenChannel(EOCChatChannel Channel);
    void CloseChat();
    void RefreshMessages();
    bool IsChatOpen() const { return bOpen; }

private:
    UPROPERTY() TObjectPtr<UCanvasPanel> RootCanvas;
    UPROPERTY() TObjectPtr<UBorder> ChatPanel;
    UPROPERTY() TObjectPtr<UTextBlock> ChannelText;
    UPROPERTY() TObjectPtr<UTextBlock> ChatLogText;
    UPROPERTY() TObjectPtr<UEditableTextBox> ChatEntry;

    EOCChatChannel SelectedChannel = EOCChatChannel::Team;
    bool bOpen = false;

    void BuildWidgetTree();
    void ApplyGameplayInputMode(bool bChatOpen);

    UFUNCTION()
    void HandleChatCommitted(const FText& Text, ETextCommit::Type CommitMethod);
};

/**
 * Runtime input owner for gameplay chat.
 * Y = Team, U = Global. The subsystem deliberately ignores those keys while another UI is open.
 */
UCLASS()
class OSTERCONFLICT_API UOCRuntimeChatSubsystem final : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

private:
    TWeakObjectPtr<UOCRuntimeChatWidget> ChatWidget;
    TWeakObjectPtr<UOCGameUIRootWidget> LegacyRoot;
    bool bYWasDown = false;
    bool bUWasDown = false;
    bool bEscapeWasDown = false;

    void EnsureWidget(AOCPlayerController& PlayerController);
    void HideLegacyChat(AOCPlayerController& PlayerController);
};
