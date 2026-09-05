#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCBlock0EvidenceCaptureSubsystem.generated.h"

class ACameraActor;
class APlayerController;

/**
 * PASS45 Block0 acceptance-only evidence camera.
 *
 * This subsystem is completely inert in normal gameplay. It activates only when the process is launched with
 * -Pass45Block0Evidence and captures the five direct ground/grass views required by PASS45_BLOCK_EXECUTION_PLAN.md.
 * It never promotes runtime acceptance; the screenshots still require factual visual review.
 */
UCLASS()
class OSTERCONFLICT_API UOCBlock0EvidenceCaptureSubsystem final : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    struct FEvidenceView
    {
        FString FileStem;
        FVector CameraLocation = FVector::ZeroVector;
        FVector LookAtLocation = FVector::ZeroVector;
    };

    void BeginCaptureSequence();
    void PositionCurrentView();
    void CaptureCurrentView();
    void FinishCaptureSequence();

    bool bEvidenceMode = false;
    int32 CurrentViewIndex = 0;
    TArray<FEvidenceView> Views;

    UPROPERTY()
    TObjectPtr<ACameraActor> EvidenceCamera;

    UPROPERTY()
    TObjectPtr<APlayerController> EvidencePlayerController;

    FTimerHandle StartupDelayHandle;
    FTimerHandle SettleDelayHandle;
    FTimerHandle FinishDelayHandle;
};
