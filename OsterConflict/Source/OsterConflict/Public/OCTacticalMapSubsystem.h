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
class ASceneCapture2D;
class UCanvasPanel;
class UEnhancedInputComponent;
class UInputAction;
class UInputMappingContext;
class UTextBlock;
class UTextureRenderTarget2D;

/**
 * Source-driven Tactical Map 2.0 presentation.
 * The accepted concept image is style-only; world geometry owns the geography.
 */
UCLASS()
class OSTERCONFLICT_API UOCTacticalMapWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent) override;

    void ConfigureWorldMap(const FOCTacticalMapProjection& InProjection, AOCWorldSectorOster* InWorldSector,
        UTextureRenderTarget2D* InWorldMapTexture);

private:
    UPROPERTY() TObjectPtr<UCanvasPanel> MapCanvas;
    UPROPERTY() TObjectPtr<UCanvasPanel> MapContentCanvas;
    UPROPERTY() TObjectPtr<UTextBlock> PlayerMarker;
    UPROPERTY() TObjectPtr<UTextBlock> PlayerCoordinates;
    UPROPERTY() TObjectPtr<UTextBlock> LocalPingMarker;
    UPROPERTY() TObjectPtr<UTextureRenderTarget2D> WorldMapTexture;

    TWeakObjectPtr<AOCWorldSectorOster> WorldSector;
    FOCTacticalMapProjection Projection;
    bool bConfiguredFromSubsystem = false;

    float MapZoom = 1.0f;
    FVector2D MapPan = FVector2D::ZeroVector;
    bool bDraggingMap = false;
    FVector2D LastDragLocalPosition = FVector2D::ZeroVector;

    bool ResolveProjectionFromWorld();
    FVector ResolveSectorWorldLocation(const FVector& SectorLocalLocation) const;
    FVector2D WorldToMap(const FVector& WorldLocation) const;
    void AddLandmarkMarker(const FString& Label, const FVector& WorldLocation);
    void AddGrid();

    bool PointerToMapLocal(const FPointerEvent& InMouseEvent, FVector2D& OutLocal) const;
    FVector2D ViewportToContent(const FVector2D& ViewportLocal) const;
    void ApplyMapViewTransform();
    void ClampMapPan();
    void PlaceLocalPing(const FVector2D& ViewportLocal);
};

/**
 * Owns the Tactical Map 2.0 runtime contract.
 * M is event-driven through Enhanced Input; the level/world remains the source of truth.
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
    TWeakObjectPtr<AOCWorldSectorOster> WorldSector;

    UPROPERTY() TObjectPtr<UInputMappingContext> MapMappingContext;
    UPROPERTY() TObjectPtr<UInputAction> MapToggleAction;
    UPROPERTY() TObjectPtr<UOCTacticalMapWidget> MapWidget;
    UPROPERTY() TObjectPtr<UTextureRenderTarget2D> MapRenderTarget;
    UPROPERTY() TObjectPtr<ASceneCapture2D> MapCaptureActor;

    FOCTacticalMapProjection MapProjection;
    bool bMapOpen = false;

    void EnsureEnhancedInputBinding();
    void HandleMapToggleAction();
    void EnsureExclusiveMapBinding(AOCCharacter& Character);
    bool CanOpenMap(const AOCPlayerController& PlayerController) const;
    bool HasBlockingUI(const AOCPlayerController& PlayerController) const;
    bool ResolveWorldMapSource();
    bool CaptureWorldMap();
    void ReleaseCaptureResources();
    void OpenMap(AOCPlayerController& PlayerController);
    void CloseMap(AOCPlayerController& PlayerController, bool bRestoreGameplayInput);
};
