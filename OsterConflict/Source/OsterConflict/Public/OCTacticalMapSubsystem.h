#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCTacticalMapProjection.h"
#include "OCTacticalMapSubsystem.generated.h"

class AOCCharacter;
class AOCPlayerController;
class AOCWorldSectorOster;
class UCanvasPanel;
class UEnhancedInputComponent;
class UInputAction;
class UInputMappingContext;
class UTextBlock;

/**
 * Source-driven Tactical Map 2.0 presentation.
 *
 * The accepted concept image is a style reference only. Geometry and marker placement are derived from
 * the actual Oster world sector and its world-space coordinates.
 */
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

    TWeakObjectPtr<AOCWorldSectorOster> WorldSector;
    FOCTacticalMapProjection Projection;

    bool ResolveProjectionFromWorld();
    FVector ResolveSectorWorldLocation(const FVector& SectorLocalLocation) const;
    FVector2D WorldToMap(const FVector& WorldLocation) const;
    void AddLandmarkMarker(const FString& Label, const FVector& WorldLocation);
    void AddGrid();
};

/**
 * Owns the Tactical Map 2.0 runtime contract.
 *
 * M is registered through Enhanced Input instead of being polled every 25 ms. The subsystem keeps the map
 * transient and local, while the world itself remains the source of truth for map bounds and marker positions.
 */
UCLASS()
class OSTERCONFLICT_API UOCTacticalMapSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

    bool IsMapOpen() const { return bMapOpen; }
    void ToggleMap(AOCPlayerController& PlayerController);

private:
    FTimerHandle InputSetupTimer;
    TWeakObjectPtr<AOCPlayerController> BoundPlayerController;
    TWeakObjectPtr<UEnhancedInputComponent> BoundInputComponent;
    TWeakObjectPtr<AOCCharacter> RemappedCharacter;

    UPROPERTY() TObjectPtr<UInputMappingContext> MapMappingContext;
    UPROPERTY() TObjectPtr<UInputAction> MapToggleAction;
    UPROPERTY() TObjectPtr<UOCTacticalMapWidget> MapWidget;

    bool bMapOpen = false;

    void EnsureEnhancedInputBinding();
    void HandleMapToggleAction();
    void EnsureExclusiveMapBinding(AOCCharacter& Character);
    bool CanOpenMap(const AOCPlayerController& PlayerController) const;
    bool HasBlockingUI(const AOCPlayerController& PlayerController) const;
    void OpenMap(AOCPlayerController& PlayerController);
    void CloseMap(AOCPlayerController& PlayerController, bool bRestoreGameplayInput);
};
