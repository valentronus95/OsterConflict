#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCPass45WeaponRackStabilitySubsystem.generated.h"

class AOCWeaponBase;

/**
 * Sandbox-only recovery owner for the admin/test weapon arsenal presentation.
 *
 * The legacy rack paths create real world pickups through DropToWorldServer(), which intentionally
 * enables rigid-body physics for normal player drops. That is correct gameplay behavior but wrong for
 * a showcase arsenal: the models immediately roll, rotate and overlap. This subsystem detects only the
 * compact seven-identity admin rack, freezes that cluster on the next frame, lays it out deterministically,
 * and keeps the invisible pickup collision separate from the rendered production visual.
 *
 * It is deliberately fail-closed: exact production visuals win when present, temporary/local fallbacks
 * remain visible only until an exact visual exists, and Engine BasicShapes are never accepted as rack art.
 */
UCLASS()
class OSTERCONFLICT_API UOCPass45WeaponRackStabilitySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    void HandleActorSpawned(AActor* Actor);
    void ScheduleRefresh(float FirstDelaySeconds);
    void RefreshRack();
    bool FindAdminRack(TArray<AOCWeaponBase*>& OutRackWeapons, FVector& OutRackCenter, float& OutRackYaw) const;
    int32 StabilizeRackWeapon(AOCWeaponBase& Weapon, const FVector& Location, const FRotator& Rotation,
        int32& OutHiddenBasicShapes, int32& OutRetiredCompetingVisuals, bool& bOutExactVisual) const;

    FDelegateHandle ActorSpawnedHandle;
    FTimerHandle RefreshTimer;
    int32 RefreshPass = 0;
    bool bSandboxActive = false;
};
