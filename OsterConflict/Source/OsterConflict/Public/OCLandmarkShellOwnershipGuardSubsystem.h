#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCLandmarkShellOwnershipGuardSubsystem.generated.h"

class AActor;
class UWorld;

/**
 * Runtime ownership guard for the current Museum / Silpo / Culture House presentation layers.
 *
 * Several historical landmark stages still keep their old delayed startup timer in addition to the
 * current startup coordinator. If one of those delayed callbacks survives, a second current shell can
 * be spawned on the same canonical site. This guard makes the final runtime presentation fail-closed:
 * it removes duplicate current owners with a site-specific keep policy and then emits explicit runtime
 * evidence once the historical startup window has elapsed.
 */
UCLASS()
class OSTERCONFLICT_API UOCLandmarkShellOwnershipGuardSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    TWeakObjectPtr<UWorld> GuardWorld;
    FDelegateHandle ActorSpawnedHandle;
    FTimerHandle FinalValidationTimer;
    int32 DuplicateRepairs = 0;

    void HandleActorSpawned(AActor* Actor);
    void EvaluateSpawnedActor(TWeakObjectPtr<AActor> WeakActor);
    int32 RepairTaggedOwners(UWorld& World, FName OwnerTag, const TCHAR* SiteLabel, bool bKeepNewest);
    void RunFinalValidation();

    static int32 CountTaggedActors(UWorld& World, FName OwnerTag);
    static AActor* FindTaggedActor(UWorld& World, FName OwnerTag);
    static bool HasInstanceGeometryNear(AActor* Actor, const FVector& SiteCenter, float RadiusCm);
};
