#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCLandmarkShellOwnershipGuardSubsystem.generated.h"

class AActor;
class UWorld;

/**
 * Pass 45 runtime ownership guard for Museum / Silpo / Culture House.
 *
 * One site has exactly one current visible shell owner:
 * - Museum shell: R13.8 segmented architecture.
 * - Silpo shell: R14.0 photo model.
 * - Culture House shell: R14.6 authoritative model.
 *
 * R13.7 Museum remains a reference/detail/interactivity parent after R13.8 suppresses its solid prototype;
 * it is deliberately not counted as a second Museum shell. Historical detail actors may decorate a site,
 * but they may never satisfy or duplicate the shell-owner contract.
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