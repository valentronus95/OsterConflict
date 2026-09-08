#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCRealWeaponFallbackSubsystem.generated.h"

struct FStreamableHandle;
class UStaticMesh;

/**
 * Runtime weapon visual safety/audit subsystem.
 *
 * Generic look-alike weapon substitution is retired: an AK, shotgun, SMG, optic, barrel, shell or other nearby
 * asset may not stand in for a missing exact weapon identity. Exact visual ownership belongs to the imported/local
 * weapon bridge. This subsystem hides rejected Engine/BasicShapes primitive visuals and audits authored materials
 * without turning a content gap into a false production-ready result.
 *
 * Legacy fallback members/functions remain temporarily for ABI/source compatibility, but ApplyRealFallback is a
 * deliberate no-op and no fallback assets are preloaded.
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
    TSharedPtr<FStreamableHandle> FallbackPreloadHandle;

    UPROPERTY(Transient)
    TObjectPtr<UStaticMesh> GenericMachineGun;

    UPROPERTY(Transient)
    TObjectPtr<UStaticMesh> GenericPistol;

    UPROPERTY(Transient)
    TObjectPtr<UStaticMesh> GenericSMG;

    UPROPERTY(Transient)
    TObjectPtr<UStaticMesh> GenericShotgun;

    UPROPERTY(Transient)
    TObjectPtr<UStaticMesh> AuthoredAKFallback;

    bool bRackMaterialAuditReadyLogged = false;
    int32 RefreshPassCount = 0;

    void CompleteFallbackPreload();
    void RefreshWeaponFallbacks();
    int32 AuditAndRepairWeaponMaterials(class AOCWeaponBase& Weapon);
    bool ApplyRealFallback(class AOCWeaponBase& Weapon, UStaticMesh* Mesh, float DesiredLengthCm, const TCHAR* FallbackLabel);
};
