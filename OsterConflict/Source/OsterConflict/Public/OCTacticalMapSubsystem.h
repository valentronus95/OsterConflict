#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCTacticalMapSubsystem.generated.h"

class AOCCharacter;
class AOCPlayerController;
class UCanvasPanel;
class UTextBlock;

/** Source-only tactical map used until final authored map art/UI assets are supplied. */
UCLASS()
class OSTERCONFLICT_API UOCTacticalMapWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    UPROPERTY() TObjectPtr<UCanvasPanel> MapCanvas;
    UPROPERTY() TObjectPtr<UTextBlock> PlayerMarker;
    UPROPERTY() TObjectPtr<UTextBlock> PlayerCoordinates;

    FVector2D WorldMin = FVector2D::ZeroVector;
    FVector2D WorldMax = FVector2D(1.0f, 1.0f);

    FVector2D WorldToMap(const FVector& WorldLocation) const;
    void AddLandmarkMarker(const FString& Label, const FVector& WorldLocation);
};

/**
 * Owns the M-key tactical-map contract without coupling it to trap deployment.
 * It also removes the legacy IA_DeployTrap -> M mapping from the current character and rehomes
 * that diagnostic gameplay action to V, keeping M exclusive to the map.
 */
UCLASS()
class OSTERCONFLICT_API UOCTacticalMapSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    FTimerHandle InputPollTimer;
    TWeakObjectPtr<AOCCharacter> RemappedCharacter;
    UPROPERTY() TObjectPtr<UOCTacticalMapWidget> MapWidget;
    bool bPreviousMDown = false;
    bool bPreviousEscapeDown = false;
    bool bMapOpen = false;

    void PollInput();
    void EnsureExclusiveMapBinding(AOCCharacter& Character);
    bool CanOpenMap(const AOCPlayerController& PlayerController) const;
    void OpenMap(AOCPlayerController& PlayerController);
    void CloseMap(AOCPlayerController& PlayerController);
};
