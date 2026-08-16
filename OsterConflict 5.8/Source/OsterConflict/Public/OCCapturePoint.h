#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OCTeamTypes.h"
#include "OCCapturePoint.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class OSTERCONFLICT_API AOCCapturePoint : public AActor
{
    GENERATED_BODY()

public:
    AOCCapturePoint();

    virtual void Tick(float DeltaSeconds) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintPure, Category="Objective")
    FName GetPointId() const { return PointId; }

    UFUNCTION(BlueprintPure, Category="Objective")
    EOCTeam GetOwnerTeam() const { return OwnerTeam; }

    UFUNCTION(BlueprintPure, Category="Objective")
    float GetCaptureProgress() const { return CaptureProgress; }

    UFUNCTION(BlueprintPure, Category="Objective")
    bool IsContested() const { return bContested; }

    /** Server-side setup helper used by the source-only prototype arena. */
    void ConfigureServer(FName InPointId, float InCaptureRadius, float InCaptureSeconds);

    /** Authority-only round reset. */
    void ResetPointServer();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<USphereComponent> CaptureSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<UStaticMeshComponent> MarkerMesh;

    UPROPERTY(Replicated, EditInstanceOnly, BlueprintReadOnly, Category="Objective")
    FName PointId = NAME_None;

    /** -1 = fully Team Two, 0 = neutral, +1 = fully Team One. */
    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Objective")
    float CaptureProgress = 0.0f;

    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Objective")
    EOCTeam OwnerTeam = EOCTeam::None;

    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Objective")
    bool bContested = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Objective", meta=(ClampMin="100.0"))
    float CaptureRadius = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Objective", meta=(ClampMin="1.0"))
    float CaptureSeconds = 12.0f;

private:
    void UpdateCaptureServer(float DeltaSeconds);
    void SetOwnerServer(EOCTeam NewOwner);
};
