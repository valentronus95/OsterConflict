#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OCR13TacticalMapWidget.generated.h"

class UBorder;
class UCanvasPanel;
class UTextBlock;

/** Source-only tactical map overlay for the compact Oster runtime map. */
UCLASS()
class OSTERCONFLICT_API UOCR13TacticalMapWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    void BuildWidgetTree();
    void BuildRoadSchematic();
    void UpdateRoadSchematic(const FVector2D& MapSize);
    void UpdatePlayerMarker(const FVector2D& MapSize);
    void UpdateObjectiveMarkers(const FVector2D& MapSize);
    void UpdateLandmarkMarkers(const FVector2D& MapSize);
    FVector2D WorldToMap(const FVector& WorldLocation, const FVector2D& MapSize) const;
    UTextBlock* MakeMarkerText(const FString& Label, int32 FontSize);

    UPROPERTY() TObjectPtr<UCanvasPanel> RootCanvas;
    UPROPERTY() TObjectPtr<UBorder> Backdrop;
    UPROPERTY() TObjectPtr<UCanvasPanel> MapCanvas;
    UPROPERTY() TObjectPtr<UTextBlock> PlayerMarker;
    UPROPERTY() TObjectPtr<UTextBlock> PositionText;
    UPROPERTY() TArray<TObjectPtr<UBorder>> RoadWidgets;
    UPROPERTY() TArray<TObjectPtr<UTextBlock>> ObjectiveWidgets;
    UPROPERTY() TArray<TObjectPtr<UTextBlock>> LandmarkWidgets;

    TArray<FTransform> RoadTransforms;
    TArray<FVector> LandmarkLocations;
    bool bRoadsBuilt = false;
};
