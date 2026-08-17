#include "OCWeaponVariants.h"

namespace
{
FOCWeaponTuning BasePreset(const TCHAR* Id, const TCHAR* Name, EOCWeaponClass WeaponClass,
    EOCInventorySlot Slot, EOCAmmoType AmmoType)
{
    FOCWeaponTuning T;
    T.WeaponId = FName(Id);
    T.DisplayName = Name;
    T.WeaponClass = WeaponClass;
    T.PreferredSlot = Slot;
    T.AmmoType = AmmoType;
    return T;
}
}

AOCWeapon_AssaultRifle::AOCWeapon_AssaultRifle()
{
    FOCWeaponTuning T = BasePreset(TEXT("OC_AR1"), TEXT("OC-AR1"), EOCWeaponClass::AssaultRifle,
        EOCInventorySlot::Primary, EOCAmmoType::Rifle);
    T.Damage = 34.0f; T.RangeCm = 13000.0f; T.RoundsPerMinute = 650.0f;
    T.HipSpreadDegrees = 1.25f; T.ADSSpreadDegrees = 0.20f; T.MagazineSize = 30;
    // R13 gameplay QA: the legacy local recoil path applies vertical controller pitch with the wrong visual sign
    // on the current mouse/camera convention. Disable only AR vertical kick for this playable pass instead of
    // letting automatic fire drag the view toward the player's feet; retain restrained horizontal feedback.
    T.RecoilPitchMin = 0.0f; T.RecoilPitchMax = 0.0f; T.RecoilYawMax = 0.28f;
    T.InitialReserveAmmo = 120; T.MaxReserveAmmo = 240; T.ReloadDuration = 2.15f;
    T.AudioLoudnessScale = 1.00f;
    ConfigureBuiltInTuning(T);
}

AOCWeapon_SMG::AOCWeapon_SMG()
{
    FOCWeaponTuning T = BasePreset(TEXT("OC_SMG1"), TEXT("OC-SMG1"), EOCWeaponClass::SMG,
        EOCInventorySlot::Primary, EOCAmmoType::Pistol);
    T.Damage = 25.0f; T.RangeCm = 8500.0f; T.RoundsPerMinute = 850.0f;
    T.HipSpreadDegrees = 1.05f; T.ADSSpreadDegrees = 0.28f; T.MovingSpreadMultiplier = 1.35f;
    T.RecoilPitchMin = 0.35f; T.RecoilPitchMax = 0.62f; T.RecoilYawMax = 0.42f;
    T.MagazineSize = 30; T.InitialReserveAmmo = 150; T.MaxReserveAmmo = 300; T.ReloadDuration = 1.90f;
    T.AudioLoudnessScale = 0.92f;
    ConfigureBuiltInTuning(T);
}

AOCWeapon_Pistol::AOCWeapon_Pistol()
{
    FOCWeaponTuning T = BasePreset(TEXT("OC_PST1"), TEXT("OC-PST1"), EOCWeaponClass::Pistol,
        EOCInventorySlot::Secondary, EOCAmmoType::Pistol);
    T.Damage = 29.0f; T.RangeCm = 7000.0f; T.RoundsPerMinute = 420.0f;
    T.HipSpreadDegrees = 1.10f; T.ADSSpreadDegrees = 0.32f;
    T.RecoilPitchMin = 0.45f; T.RecoilPitchMax = 0.75f; T.RecoilYawMax = 0.28f;
    T.MagazineSize = 15; T.InitialReserveAmmo = 60; T.MaxReserveAmmo = 120; T.ReloadDuration = 1.55f;
    T.bSupportsAutomatic = false; T.bSupportsSemiAutomatic = true;
    T.AudioLoudnessScale = 0.88f;
    ConfigureBuiltInTuning(T);
}

AOCWeapon_Sniper::AOCWeapon_Sniper()
{
    FOCWeaponTuning T = BasePreset(TEXT("OC_SNP1"), TEXT("OC-SNP1"), EOCWeaponClass::SniperRifle,
        EOCInventorySlot::Primary, EOCAmmoType::Precision);
    T.Damage = 90.0f; T.RangeCm = 35000.0f; T.RoundsPerMinute = 55.0f;
    T.HipSpreadDegrees = 3.25f; T.ADSSpreadDegrees = 0.035f; T.MovingSpreadMultiplier = 2.40f;
    T.RecoilPitchMin = 1.90f; T.RecoilPitchMax = 2.60f; T.RecoilYawMax = 0.55f;
    T.MagazineSize = 5; T.InitialReserveAmmo = 25; T.MaxReserveAmmo = 50; T.ReloadDuration = 2.85f;
    T.bSupportsAutomatic = false; T.bSupportsSemiAutomatic = true;
    T.AudioLoudnessScale = 1.18f;
    ConfigureBuiltInTuning(T);
}

AOCWeapon_Shotgun::AOCWeapon_Shotgun()
{
    FOCWeaponTuning T = BasePreset(TEXT("OC_SG1"), TEXT("OC-SG1"), EOCWeaponClass::Shotgun,
        EOCInventorySlot::Primary, EOCAmmoType::Shell);
    T.Damage = 11.5f; T.PelletsPerShot = 8; T.RangeCm = 4200.0f; T.RoundsPerMinute = 85.0f;
    T.HipSpreadDegrees = 2.60f; T.ADSSpreadDegrees = 1.10f; T.MovingSpreadMultiplier = 1.35f;
    T.RecoilPitchMin = 1.65f; T.RecoilPitchMax = 2.20f; T.RecoilYawMax = 0.65f;
    T.MagazineSize = 6; T.InitialReserveAmmo = 30; T.MaxReserveAmmo = 60; T.ReloadDuration = 2.70f;
    T.bSupportsAutomatic = false; T.bSupportsSemiAutomatic = true;
    T.bSupersonicAmmo = false; T.AudioLoudnessScale = 1.15f;
    ConfigureBuiltInTuning(T);
}

AOCWeapon_LMG::AOCWeapon_LMG()
{
    FOCWeaponTuning T = BasePreset(TEXT("OC_LMG1"), TEXT("OC-LMG1"), EOCWeaponClass::LMG,
        EOCInventorySlot::Primary, EOCAmmoType::Rifle);
    T.Damage = 31.0f; T.RangeCm = 15000.0f; T.RoundsPerMinute = 720.0f;
    T.HipSpreadDegrees = 1.85f; T.ADSSpreadDegrees = 0.30f; T.MovingSpreadMultiplier = 1.95f;
    T.RecoilPitchMin = 0.65f; T.RecoilPitchMax = 1.05f; T.RecoilYawMax = 0.48f;
    T.MagazineSize = 75; T.InitialReserveAmmo = 225; T.MaxReserveAmmo = 450; T.ReloadDuration = 4.20f;
    T.AudioLoudnessScale = 1.08f;
    ConfigureBuiltInTuning(T);
}
