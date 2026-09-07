#pragma once
#include "CoreMinimal.h"
#include "OCWeaponBase.h"
#include "OCAntiArmorLauncher.generated.h"

class FStreamableHandle;

UCLASS()
class OSTERCONFLICT_API AOCAntiArmorLauncher : public AOCWeaponBase
{
    GENERATED_BODY()
public:
    AOCAntiArmorLauncher();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual bool TryFireServer(AOCCharacter* Shooter, const FVector& TraceOrigin, const FVector& TraceDirection,
        bool bAiming, bool bMoving, FHitResult& OutHit, bool& bOutDamagedActor, bool& bOutFatalHit) override;
private:
    void CompleteProductionVisualPreload();

    TSharedPtr<FStreamableHandle> ProductionVisualPreloadHandle;
    double LastLauncherFireTime = -1000.0;
    int32 LauncherAudioEventCounter = 0;
};
