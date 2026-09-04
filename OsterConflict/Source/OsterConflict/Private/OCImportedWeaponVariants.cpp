#include "OCImportedWeaponVariants.h"

#include "Components/StaticMeshComponent.h"

namespace
{
    void HideSourceOnlyVisuals(AOCWeaponBase* Weapon)
    {
        if (!Weapon) return;
        TArray<UStaticMeshComponent*> Components;
        Weapon->GetComponents<UStaticMeshComponent>(Components);
        for (UStaticMeshComponent* Component : Components)
        {
            if (!Component) continue;
            Component->SetVisibility(false, true);
            Component->SetHiddenInGame(true, true);
            Component->SetCastShadow(false);
            Component->SetCanEverAffectNavigation(false);
        }
    }

    FOCWeaponTuning MakeImportedFirearm(
        const TCHAR* Id,
        const TCHAR* Name,
        EOCWeaponClass WeaponClass,
        EOCInventorySlot Slot,
        EOCAmmoType AmmoType,
        EOCWeaponActionType ActionType)
    {
        FOCWeaponTuning T;
        T.WeaponId = FName(Id);
        T.DisplayName = Name;
        T.WeaponClass = WeaponClass;
        T.PreferredSlot = Slot;
        T.AmmoType = AmmoType;
        T.ActionType = ActionType;
        return T;
    }

    FOCWeaponTuning MakeImportedLauncher(const TCHAR* Id, const TCHAR* Name)
    {
        FOCWeaponTuning T;
        T.WeaponId = FName(Id);
        T.DisplayName = Name;
        T.WeaponClass = EOCWeaponClass::Launcher;
        T.ActionType = EOCWeaponActionType::LauncherSingleShot;
        T.PreferredSlot = EOCInventorySlot::Primary;
        T.AmmoType = EOCAmmoType::Rocket;
        T.Damage = 620.0f;
        T.PelletsPerShot = 1;
        T.RangeCm = 18000.0f;
        T.RoundsPerMinute = 18.0f;
        T.HipSpreadDegrees = 1.7f;
        T.ADSSpreadDegrees = 0.35f;
        T.MovingSpreadMultiplier = 1.4f;
        T.RecoilPitchMin = 4.0f;
        T.RecoilPitchMax = 5.2f;
        T.RecoilYawMax = 1.1f;
        T.MagazineSize = 1;
        T.InitialReserveAmmo = 3;
        T.MaxReserveAmmo = 6;
        T.ReloadDuration = 3.8f;
        T.bSupportsSemiAutomatic = true;
        T.bSupportsAutomatic = false;
        T.bSupportsBurst3 = false;
        T.bSupersonicAmmo = false;
        T.AudioLoudnessScale = 1.20f;
        return T;
    }
}

AOCWeapon_AK74M::AOCWeapon_AK74M()
{
    FOCWeaponTuning T = MakeImportedFirearm(TEXT("IMP_AK74M"), TEXT("AK-74M"),
        EOCWeaponClass::AssaultRifle, EOCInventorySlot::Primary, EOCAmmoType::Rifle,
        EOCWeaponActionType::GasOperated);
    T.Damage = 32.0f; T.RangeCm = 14500.0f; T.RoundsPerMinute = 650.0f;
    T.HipSpreadDegrees = 1.20f; T.ADSSpreadDegrees = 0.18f;
    T.RecoilPitchMin = 0.50f; T.RecoilPitchMax = 0.84f; T.RecoilYawMax = 0.32f;
    T.MagazineSize = 30; T.InitialReserveAmmo = 120; T.MaxReserveAmmo = 240; T.ReloadDuration = 2.20f;
    T.bSupportsSemiAutomatic = true; T.bSupportsAutomatic = true;
    ConfigureBuiltInTuning(T);
}

void AOCWeapon_AK74M::BeginPlay()
{
    Super::BeginPlay();
    HideSourceOnlyVisuals(this);
}

AOCWeapon_AR15::AOCWeapon_AR15()
{
    FOCWeaponTuning T = MakeImportedFirearm(TEXT("IMP_AR15"), TEXT("AR-15"),
        EOCWeaponClass::AssaultRifle, EOCInventorySlot::Primary, EOCAmmoType::Rifle,
        EOCWeaponActionType::GasOperated);
    T.Damage = 35.0f; T.RangeCm = 16000.0f; T.RoundsPerMinute = 500.0f;
    T.HipSpreadDegrees = 1.18f; T.ADSSpreadDegrees = 0.16f;
    T.RecoilPitchMin = 0.48f; T.RecoilPitchMax = 0.80f; T.RecoilYawMax = 0.30f;
    T.MagazineSize = 30; T.InitialReserveAmmo = 120; T.MaxReserveAmmo = 240; T.ReloadDuration = 2.15f;
    T.bSupportsSemiAutomatic = true; T.bSupportsAutomatic = false;
    ConfigureBuiltInTuning(T);
}

void AOCWeapon_AR15::BeginPlay()
{
    Super::BeginPlay();
    HideSourceOnlyVisuals(this);
}

AOCWeapon_M4A1::AOCWeapon_M4A1()
{
    FOCWeaponTuning T = MakeImportedFirearm(TEXT("IMP_M4A1"), TEXT("M4A1"),
        EOCWeaponClass::AssaultRifle, EOCInventorySlot::Primary, EOCAmmoType::Rifle,
        EOCWeaponActionType::GasOperated);
    T.Damage = 32.0f; T.RangeCm = 14500.0f; T.RoundsPerMinute = 800.0f;
    T.HipSpreadDegrees = 1.08f; T.ADSSpreadDegrees = 0.17f;
    T.RecoilPitchMin = 0.44f; T.RecoilPitchMax = 0.76f; T.RecoilYawMax = 0.30f;
    T.MagazineSize = 30; T.InitialReserveAmmo = 120; T.MaxReserveAmmo = 240; T.ReloadDuration = 2.05f;
    T.bSupportsSemiAutomatic = true; T.bSupportsAutomatic = true;
    ConfigureBuiltInTuning(T);
}

void AOCWeapon_M4A1::BeginPlay()
{
    Super::BeginPlay();
    HideSourceOnlyVisuals(this);
}

AOCWeapon_FnBallista::AOCWeapon_FnBallista()
{
    FOCWeaponTuning T = MakeImportedFirearm(TEXT("IMP_BALLISTA"), TEXT("FN Ballista"),
        EOCWeaponClass::SniperRifle, EOCInventorySlot::Primary, EOCAmmoType::Precision,
        EOCWeaponActionType::BoltAction);
    T.ManualActionCycleSeconds = 1.10f;
    T.Damage = 96.0f; T.RangeCm = 38000.0f; T.RoundsPerMinute = 50.0f;
    T.HipSpreadDegrees = 3.40f; T.ADSSpreadDegrees = 0.030f; T.MovingSpreadMultiplier = 2.50f;
    T.RecoilPitchMin = 2.00f; T.RecoilPitchMax = 2.80f; T.RecoilYawMax = 0.52f;
    T.MagazineSize = 5; T.InitialReserveAmmo = 25; T.MaxReserveAmmo = 50; T.ReloadDuration = 3.00f;
    T.bSupportsSemiAutomatic = true; T.bSupportsAutomatic = false;
    ConfigureBuiltInTuning(T);
}

void AOCWeapon_FnBallista::BeginPlay()
{
    Super::BeginPlay();
    HideSourceOnlyVisuals(this);
}

AOCWeapon_Kar98k::AOCWeapon_Kar98k()
{
    FOCWeaponTuning T = MakeImportedFirearm(TEXT("IMP_KAR98K"), TEXT("Kar98k"),
        EOCWeaponClass::SniperRifle, EOCInventorySlot::Primary, EOCAmmoType::Precision,
        EOCWeaponActionType::BoltAction);
    T.ManualActionCycleSeconds = 1.15f;
    T.Damage = 88.0f; T.RangeCm = 30000.0f; T.RoundsPerMinute = 48.0f;
    T.HipSpreadDegrees = 3.10f; T.ADSSpreadDegrees = 0.045f; T.MovingSpreadMultiplier = 2.35f;
    T.RecoilPitchMin = 1.85f; T.RecoilPitchMax = 2.55f; T.RecoilYawMax = 0.50f;
    T.MagazineSize = 5; T.InitialReserveAmmo = 30; T.MaxReserveAmmo = 60; T.ReloadDuration = 3.10f;
    T.bSupportsSemiAutomatic = true; T.bSupportsAutomatic = false;
    ConfigureBuiltInTuning(T);
}

void AOCWeapon_Kar98k::BeginPlay()
{
    Super::BeginPlay();
    HideSourceOnlyVisuals(this);
}

AOCWeapon_Makarov::AOCWeapon_Makarov()
{
    FOCWeaponTuning T = MakeImportedFirearm(TEXT("IMP_MAKAROV"), TEXT("Makarov PM"),
        EOCWeaponClass::Pistol, EOCInventorySlot::Secondary, EOCAmmoType::Pistol,
        EOCWeaponActionType::Blowback);
    T.Damage = 25.0f; T.RangeCm = 6000.0f; T.RoundsPerMinute = 380.0f;
    T.HipSpreadDegrees = 1.15f; T.ADSSpreadDegrees = 0.34f;
    T.RecoilPitchMin = 0.40f; T.RecoilPitchMax = 0.68f; T.RecoilYawMax = 0.24f;
    T.MagazineSize = 8; T.InitialReserveAmmo = 48; T.MaxReserveAmmo = 96; T.ReloadDuration = 1.55f;
    T.bSupportsSemiAutomatic = true; T.bSupportsAutomatic = false;
    ConfigureBuiltInTuning(T);
}

void AOCWeapon_Makarov::BeginPlay()
{
    Super::BeginPlay();
    HideSourceOnlyVisuals(this);
}

AOCWeapon_TommyGun::AOCWeapon_TommyGun()
{
    FOCWeaponTuning T = MakeImportedFirearm(TEXT("IMP_TOMMY"), TEXT("Thompson M1A1"),
        EOCWeaponClass::SMG, EOCInventorySlot::Primary, EOCAmmoType::Pistol,
        EOCWeaponActionType::Blowback);
    T.Damage = 28.0f; T.RangeCm = 8000.0f; T.RoundsPerMinute = 700.0f;
    T.HipSpreadDegrees = 1.35f; T.ADSSpreadDegrees = 0.30f; T.MovingSpreadMultiplier = 1.45f;
    T.RecoilPitchMin = 0.55f; T.RecoilPitchMax = 0.92f; T.RecoilYawMax = 0.45f;
    T.MagazineSize = 30; T.InitialReserveAmmo = 150; T.MaxReserveAmmo = 300; T.ReloadDuration = 2.35f;
    T.bSupportsSemiAutomatic = true; T.bSupportsAutomatic = true;
    ConfigureBuiltInTuning(T);
}

void AOCWeapon_TommyGun::BeginPlay()
{
    Super::BeginPlay();
    HideSourceOnlyVisuals(this);
}

AOCWeapon_M72LAW::AOCWeapon_M72LAW()
{
    FOCWeaponTuning T = MakeImportedLauncher(TEXT("IMP_M72"), TEXT("M72 LAW"));
    T.InitialReserveAmmo = 2;
    T.MaxReserveAmmo = 4;
    T.ReloadDuration = 4.10f;
    ConfigureBuiltInTuning(T);
}

void AOCWeapon_M72LAW::BeginPlay()
{
    // Skip AOCAntiArmorLauncher::BeginPlay because that owner is tied to the old tracked generic launcher visual.
    // Gameplay firing stays inherited from AOCAntiArmorLauncher while the imported-weapon bridge owns this exact visual.
    AOCWeaponBase::BeginPlay();
    HideSourceOnlyVisuals(this);
}

AOCWeapon_RPG26::AOCWeapon_RPG26()
{
    FOCWeaponTuning T = MakeImportedLauncher(TEXT("IMP_RPG26"), TEXT("RPG-26"));
    T.InitialReserveAmmo = 3;
    T.MaxReserveAmmo = 5;
    T.ReloadDuration = 3.70f;
    ConfigureBuiltInTuning(T);
}

void AOCWeapon_RPG26::BeginPlay()
{
    AOCWeaponBase::BeginPlay();
    HideSourceOnlyVisuals(this);
}
