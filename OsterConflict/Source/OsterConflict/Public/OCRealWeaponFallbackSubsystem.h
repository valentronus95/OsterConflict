#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCRealWeaponFallbackSubsystem.generated.h"

class UStaticMesh;

/**
 * Replaces visible primitive weapon fallbacks with already-imported real R13 meshes when an exact
 * production asset is missing or fails to load. This does NOT mark the weapon production-verified:
 * exact asset validation remains authoritative and must still fail for a generic fallback.
 */
UCLASS()
class OSTERCONFLICT_API UOCRealWeaponFallbackSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    FTimerHandle RefreshTimer;

    // These meshes are loaded once and then used later from a timer. They MUST be
    // reflected UObject references so UE garbage collection cannot reclaim them
    // between OnWorldBeginPlay() and RefreshWeaponFallbacks().
    UPROPERTY(Transient)
    TObjectPtr<UStaticMesh> GenericMachineGun;

    UPROPERTY(Transient)
    TObjectPtr<UStaticMesh> GenericPistol;

    UPROPERTY(Transient)
    TObjectPtr<UStaticMesh> GenericSMG;

    UPROPERTY(Transient)
    TObjectPtr<UStaticMesh> GenericShotgun;

    void RefreshWeaponFallbacks();
    bool ApplyRealFallback(class AOCWeaponBase& Weapon, UStaticMesh* Mesh, float DesiredLengthCm, const TCHAR* FallbackLabel);
};
