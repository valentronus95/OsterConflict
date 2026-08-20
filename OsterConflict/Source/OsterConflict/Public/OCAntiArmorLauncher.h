#pragma once
#include "CoreMinimal.h"
#include "OCWeaponBase.h"
#include "OCAntiArmorLauncher.generated.h"
UCLASS()
class OSTERCONFLICT_API AOCAntiArmorLauncher : public AOCWeaponBase
{
    GENERATED_BODY()
public:
    AOCAntiArmorLauncher();
    virtual void BeginPlay() override;
    virtual bool TryFireServer(AOCCharacter* Shooter, const FVector& TraceOrigin, const FVector& TraceDirection,
        bool bAiming, bool bMoving, FHitResult& OutHit, bool& bOutDamagedActor, bool& bOutFatalHit) override;
private:
    double LastLauncherFireTime = -1000.0;
};
