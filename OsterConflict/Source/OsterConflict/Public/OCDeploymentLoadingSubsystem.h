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
    void SetLoadingStatus(const FText& Status);

protected:
    virtual void NativeConstruct() override;

private:
    UPROPERTY() TObjectPtr<UProgressBar> ProgressBar;
    UPROPERTY() TObjectPtr<UTextBlock> PercentText;
    UPROPERTY() TObjectPtr<UTextBlock> StatusText;
};

/**
 * Holds an opaque loading overlay until the critical world startup coordinator reports ready,
 * then sends the authoritative deployment request. The subsystem continues ticking while the
 * world is paused so the UI cannot deadlock behind its own pre-game pause.
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
    virtual bool IsTickableWhenPaused() const override { return true; }

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
