#include "OCAntiArmorLauncher.h"
#include "OCAntiArmorProjectile.h"
#include "OCCharacter.h"
#include "Engine/World.h"

AOCAntiArmorLauncher::AOCAntiArmorLauncher()
{
    FOCWeaponTuning T;
    T.WeaponId=FName(TEXT("OC_RPG1")); T.DisplayName=TEXT("OC Anti-Armor Launcher");
    T.WeaponClass=EOCWeaponClass::Launcher; T.PreferredSlot=EOCInventorySlot::Primary; T.AmmoType=EOCAmmoType::Rocket;
    T.Damage=620.0f; T.PelletsPerShot=1; T.RangeCm=18000.0f; T.RoundsPerMinute=18.0f;
    T.HipSpreadDegrees=1.7f; T.ADSSpreadDegrees=0.35f; T.MovingSpreadMultiplier=1.4f;
    T.RecoilPitchMin=4.0f; T.RecoilPitchMax=5.2f; T.RecoilYawMax=1.1f;
    T.MagazineSize=1; T.InitialReserveAmmo=4; T.MaxReserveAmmo=6; T.ReloadDuration=3.8f;
    T.bSupportsSemiAutomatic=true; T.bSupportsAutomatic=false;
    ConfigureBuiltInTuning(T);
}

bool AOCAntiArmorLauncher::TryFireServer(AOCCharacter* Shooter, const FVector& TraceOrigin, const FVector& TraceDirection,
    bool, bool, FHitResult& OutHit, bool& bOutDamagedActor, bool& bOutFatalHit)
{
    OutHit=FHitResult(); bOutDamagedActor=false; bOutFatalHit=false;
    if(!HasAuthority()||!Shooter||IsWorldPickup()||AmmoInMagazine<=0||bIsReloading) return false;
    const double Now=GetWorld()->GetTimeSeconds();
    if((Now-LastLauncherFireTime)<GetFireInterval()) return false;
    LastLauncherFireTime=Now; --AmmoInMagazine;
    FActorSpawnParameters Params; Params.Owner=Shooter; Params.Instigator=Shooter; Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    const FVector Dir=TraceDirection.GetSafeNormal();
    GetWorld()->SpawnActor<AOCAntiArmorProjectile>(AOCAntiArmorProjectile::StaticClass(), TraceOrigin+Dir*90.0f, Dir.Rotation(), Params);
    MulticastFireTraceFX(TraceOrigin, TraceOrigin+Dir*220.0f, false);
    return true;
}
