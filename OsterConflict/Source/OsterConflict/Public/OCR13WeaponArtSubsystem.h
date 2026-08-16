#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13WeaponArtSubsystem.generated.h"

class AOCWeaponBase;

/**
 * R13 content bridge. Once /Game/R13/Weapons has been imported, this swaps the old source-only
 * cube/cylinder weapon silhouettes for actual static meshes without touching authoritative weapon logic.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13WeaponArtSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

private:
    float ScanAccumulator = 0.0f;
    TSet<TWeakObjectPtr<AOCWeaponBase>> ProcessedWeapons;
    void ApplyArt(AOCWeaponBase* Weapon);
};
