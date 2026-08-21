#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCTacticalMapProjection.h"
#include "OCMinimapSubsystem.generated.h"

class AOCPlayerController;
class UBorder;
class UCanvasPanel;
class UImage;
class UTextBlock;
class UTextureRenderTarget2D;

/** Compact HUD map driven by the same world projection used by Tactical Map 2.0. */
UCLASS()
class OSTERCONFLICT_API UOCMinimapWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void Configure(UTextureRenderTarget2D* InMapTexture, const FOCTacticalMapProjection& InProjection);
    void UpdatePlayerMarker(const FVector& WorldLocation, float WorldYawDegrees);

protected:
    virtual void NativeConstruct() override;

private:
    UPROPERTY() TObjectPtr<UImage> MapImage;
    UPROPERTY() TObjectPtr<UTextBlock> PlayerMarker;
    FOCTacticalMapProjection Projection;
    bool bConfigured = false;
};

/** Creates and updates the always-on HUD minimap after deployment. */
UCLASS()
class OSTERCONFLICT_API UOCMinimapSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override { return true; }
    virtual void Deinitialize() override;

private:
    TWeakObjectPtr<UOCMinimapWidget> MinimapWidget;
    double NextBuildAttemptSeconds = 0.0;

    void EnsureMinimap(AOCPlayerController& PlayerController);
};
