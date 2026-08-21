#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCDeploymentLoadingSubsystem.generated.h"

class AOCPlayerController;
class UBorder;
class UProgressBar;
class UTextBlock;

/** Full-screen deployment transition shown after the staged deployment START button. */
UCLASS()
class OSTERCONFLICT_API UOCDeploymentLoadingWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetLoadingProgress(float NormalizedProgress);

protected:
    virtual void NativeConstruct() override;

private:
    UPROPERTY() TObjectPtr<UProgressBar> ProgressBar;
    UPROPERTY() TObjectPtr<UTextBlock> PercentText;
};

/**
 * Keeps the deployment menu stable for one rendered frame, then sends the authoritative ready request while
 * a blocking 0-100% loading overlay remains visible until the player pawn has actually been possessed.
 */
UCLASS()
class OSTERCONFLICT_API UOCDeploymentLoadingSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override { return true; }

    void BeginDeployment(AOCPlayerController* Controller);

private:
    TWeakObjectPtr<AOCPlayerController> PendingController;
    TWeakObjectPtr<UOCDeploymentLoadingWidget> LoadingWidget;
    double StartTimeSeconds = 0.0;
    double CompletionStartSeconds = -1.0;
    bool bActive = false;
    bool bReadySent = false;

    void FinishDeploymentTransition();
};
