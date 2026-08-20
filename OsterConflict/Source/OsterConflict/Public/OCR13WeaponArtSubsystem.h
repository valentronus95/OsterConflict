#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13WeaponArtSubsystem.generated.h"

class AOCWeaponBase;

/**
 * R13 content bridge. Swaps source-only weapon silhouettes for imported art and continuously
 * keeps equipped/world-pickup presentation sane while the authoritative inventory logic changes state.
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
    void RepairPresentation(AOCWeaponBase* Weapon);
};