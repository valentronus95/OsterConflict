#include "OCWeaponVariants.h"

#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"

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

void HideStaticWeaponFallback(AOCWeaponBase* Owner)
{
    if (!Owner) return;

    TArray<UStaticMeshComponent*> StaticMeshComponents;
    Owner->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
    for (UStaticMeshComponent* Component : StaticMeshComponents)
    {
        if (!Component) continue;

        // Gameplay collision/debug geometry must never become production art. The imported weapon bridge is the
        // single runtime visual owner and will attach the exact asset after its non-blocking preload completes.
        Component->SetVisibility(false, false);
        Component->SetHiddenInGame(true, false);
        Component->SetCastShadow(false);
        Component->SetCanEverAffectNavigation(false);
    }
}

UStaticMeshComponent* ApplyStaticProductionWeapon(AOCWeaponBase* Owner, USceneComponent* Root,
    const TCHAR* AssetPath, const FName ComponentBaseName, float DesiredLengthCm);

UPrimitiveComponent* ApplySkeletalProductionWeapon(AOCWeaponBase* Owner, USceneComponent* Root,
    const TCHAR* AssetPath, const FName ComponentBaseName, float DesiredLengthCm)
{
    (void)Root;
    (void)AssetPath;
    (void)ComponentBaseName;
    (void)DesiredLengthCm;
    HideStaticWeaponFallback(Owner);
    return nullptr;
}

UStaticMeshComponent* ApplyStaticProductionWeapon(AOCWeaponBase* Owner, USceneComponent* Root,
    const TCHAR* AssetPath, const FName ComponentBaseName, float DesiredLengthCm)
{
    (void)Root;
    (void)AssetPath;
    (void)ComponentBaseName;
    (void)DesiredLengthCm;
    HideStaticWeaponFallback(Owner);
    return nullptr;
}
}

AOCWeapon_AssaultRifle::AOCWeapon_AssaultRifle()
{
    FOCWeaponTuning T = BasePreset(TEXT("OC_AR1"), TEXT("AK-47"), EOCWeaponClass::AssaultRifle,
        EOCInventorySlot::Primary, EOCAmmoType::Rifle);
    T.ActionType = EOCWeaponActionType::GasOperated;
    T.Damage = 34.0f; T.RangeCm = 13000.0f; T.RoundsPerMinute = 650.0f;
    T.HipSpreadDegrees = 1.25f; T.ADSSpreadDegrees = 0.20f; T.MagazineSize = 30;
    T.InitialReserveAmmo = 120; T.MaxReserveAmmo = 240; T.ReloadDuration = 2.15f;
    T.AudioLoudnessScale = 1.00f;
    ConfigureBuiltInTuning(T);
}

void AOCWeapon_AssaultRifle::BeginPlay()
{
    Super::BeginPlay();
    ApplySkeletalProductionWeapon(this, WeaponRoot,
        TEXT("/Game/AK-47/Mesh/SKM_AK-47.SKM_AK-47"),
        FName(TEXT("ProductionAK47")), 88.0f);
}

AOCWeapon_SMG::AOCWeapon_SMG()
{
    FOCWeaponTuning T = BasePreset(TEXT("OC_SMG1"), TEXT("MP5"), EOCWeaponClass::SMG,
        EOCInventorySlot::Primary, EOCAmmoType::Pistol);
    T.ActionType = EOCWeaponActionType::DelayedBlowback;
    T.Damage = 25.0f; T.RangeCm = 8500.0f; T.RoundsPerMinute = 850.0f;
    T.HipSpreadDegrees = 1.05f; T.ADSSpreadDegrees = 0.28f; T.MovingSpreadMultiplier = 1.35f;
    T.RecoilPitchMin = 0.35f; T.RecoilPitchMax = 0.62f; T.RecoilYawMax = 0.42f;
    T.MagazineSize = 30; T.InitialReserveAmmo = 150; T.MaxReserveAmmo = 300; T.ReloadDuration = 1.90f;
    T.AudioLoudnessScale = 0.92f;
    ConfigureBuiltInTuning(T);
}

void AOCWeapon_SMG::BeginPlay()
{
    Super::BeginPlay();
    ApplySkeletalProductionWeapon(this, WeaponRoot,
        TEXT("/Game/R13/Weapons/Stein/MP5/SKM_MP5.SKM_MP5"),
        FName(TEXT("ProductionMP5")), 68.0f);
}

AOCWeapon_Pistol::AOCWeapon_Pistol()
{
    FOCWeaponTuning T = BasePreset(TEXT("OC_PST1"), TEXT("M1911"), EOCWeaponClass::Pistol,
        EOCInventorySlot::Secondary, EOCAmmoType::Pistol);
    T.ActionType = EOCWeaponActionType::ShortRecoil;
    T.Damage = 29.0f; T.RangeCm = 7000.0f; T.RoundsPerMinute = 420.0f;
    T.HipSpreadDegrees = 1.10f; T.ADSSpreadDegrees = 0.32f;
    T.RecoilPitchMin = 0.45f; T.RecoilPitchMax = 0.75f; T.RecoilYawMax = 0.28f;
    T.MagazineSize = 15; T.InitialReserveAmmo = 60; T.MaxReserveAmmo = 120; T.ReloadDuration = 1.55f;
    T.bSupportsAutomatic = false; T.bSupportsSemiAutomatic = true;
    T.AudioLoudnessScale = 0.88f;
    ConfigureBuiltInTuning(T);
}

void AOCWeapon_Pistol::BeginPlay()
{
    Super::BeginPlay();
    ApplySkeletalProductionWeapon(this, WeaponRoot,
        TEXT("/Game/R13/Weapons/Stein/1911/SKM_1911.SKM_1911"),
        FName(TEXT("Production1911")), 23.0f);
}

AOCWeapon_Sniper::AOCWeapon_Sniper()
{
    FOCWeaponTuning T = BasePreset(TEXT("OC_SNP1"), TEXT("M700"), EOCWeaponClass::SniperRifle,
        EOCInventorySlot::Primary, EOCAmmoType::Precision);
    T.ActionType = EOCWeaponActionType::BoltAction;
    T.ManualActionCycleSeconds = 1.10f;
    T.Damage = 90.0f; T.RangeCm = 35000.0f; T.RoundsPerMinute = 55.0f;
    T.HipSpreadDegrees = 3.25f; T.ADSSpreadDegrees = 0.035f; T.MovingSpreadMultiplier = 2.40f;
    T.RecoilPitchMin = 1.90f; T.RecoilPitchMax = 2.60f; T.RecoilYawMax = 0.55f;
    T.MagazineSize = 5; T.InitialReserveAmmo = 25; T.MaxReserveAmmo = 50; T.ReloadDuration = 2.85f;
    T.bSupportsAutomatic = false; T.bSupportsSemiAutomatic = true;
    T.AudioLoudnessScale = 1.18f;
    ConfigureBuiltInTuning(T);
}

void AOCWeapon_Sniper::BeginPlay()
{
    Super::BeginPlay();
    ApplySkeletalProductionWeapon(this, WeaponRoot,
        TEXT("/Game/R13/Weapons/Stein/M700/SKM_M700.SKM_M700"),
        FName(TEXT("ProductionM700")), 112.0f);
}

AOCWeapon_Shotgun::AOCWeapon_Shotgun()
{
    FOCWeaponTuning T = BasePreset(TEXT("OC_SG1"), TEXT("Remington 870"), EOCWeaponClass::Shotgun,
        EOCInventorySlot::Primary, EOCAmmoType::Shell);
    T.ActionType = EOCWeaponActionType::PumpAction;
    T.ManualActionCycleSeconds = 0.72f;
    T.Damage = 11.5f; T.PelletsPerShot = 8; T.RangeCm = 4200.0f; T.RoundsPerMinute = 85.0f;
    T.HipSpreadDegrees = 2.60f; T.ADSSpreadDegrees = 1.10f; T.MovingSpreadMultiplier = 1.35f;
    T.RecoilPitchMin = 1.65f; T.RecoilPitchMax = 2.20f; T.RecoilYawMax = 0.65f;
    T.MagazineSize = 6; T.InitialReserveAmmo = 30; T.MaxReserveAmmo = 60; T.ReloadDuration = 2.70f;
    T.bSupportsAutomatic = false; T.bSupportsSemiAutomatic = true;
    T.bSupersonicAmmo = false; T.AudioLoudnessScale = 1.15f;
    ConfigureBuiltInTuning(T);
}

void AOCWeapon_Shotgun::BeginPlay()
{
    Super::BeginPlay();
    ApplySkeletalProductionWeapon(this, WeaponRoot,
        TEXT("/Game/Production/Weapons/Remington870/SKM_Remington870.SKM_Remington870"),
        FName(TEXT("ProductionRemington870")), 100.0f);
}

AOCWeapon_LMG::AOCWeapon_LMG()
{
    FOCWeaponTuning T = BasePreset(TEXT("OC_LMG1"), TEXT("M249"), EOCWeaponClass::LMG,
        EOCInventorySlot::Primary, EOCAmmoType::Rifle);
    T.ActionType = EOCWeaponActionType::BeltFed;
    T.Damage = 31.0f; T.RangeCm = 15000.0f; T.RoundsPerMinute = 720.0f;
    T.HipSpreadDegrees = 1.85f; T.ADSSpreadDegrees = 0.30f; T.MovingSpreadMultiplier = 1.95f;
    T.RecoilPitchMin = 0.65f; T.RecoilPitchMax = 1.05f; T.RecoilYawMax = 0.48f;
    T.MagazineSize = 75; T.InitialReserveAmmo = 225; T.MaxReserveAmmo = 450; T.ReloadDuration = 4.20f;
    T.AudioLoudnessScale = 1.08f;
    ConfigureBuiltInTuning(T);
}

void AOCWeapon_LMG::BeginPlay()
{
    Super::BeginPlay();
    ApplyStaticProductionWeapon(this, WeaponRoot,
        TEXT("/Game/Production/Weapons/M249/SM_M249.SM_M249"),
        FName(TEXT("ProductionM249")), 104.0f);
}

AOCWeapon_M14::AOCWeapon_M14()
{
    FOCWeaponTuning T = BasePreset(TEXT("R13_M14"), TEXT("M14"), EOCWeaponClass::AssaultRifle,
        EOCInventorySlot::Primary, EOCAmmoType::Rifle);
    T.ActionType = EOCWeaponActionType::GasOperated;
    T.Damage = 44.0f; T.RangeCm = 19000.0f; T.RoundsPerMinute = 700.0f;
    T.HipSpreadDegrees = 1.55f; T.ADSSpreadDegrees = 0.14f; T.MovingSpreadMultiplier = 1.65f;
    T.RecoilPitchMin = 0.82f; T.RecoilPitchMax = 1.18f; T.RecoilYawMax = 0.45f;
    T.MagazineSize = 20; T.InitialReserveAmmo = 100; T.MaxReserveAmmo = 200; T.ReloadDuration = 2.45f;
    T.bSupportsAutomatic = true; T.bSupportsSemiAutomatic = true;
    T.AudioLoudnessScale = 1.08f;
    ConfigureBuiltInTuning(T);
}

void AOCWeapon_M14::BeginPlay()
{
    Super::BeginPlay();
    ApplySkeletalProductionWeapon(this, WeaponRoot,
        TEXT("/Game/R13/Weapons/Stein/M14/SKM_M14.SKM_M14"),
        FName(TEXT("ProductionM14")), 112.0f);
}

AOCWeapon_Mac10::AOCWeapon_Mac10()
{
    FOCWeaponTuning T = BasePreset(TEXT("R13_MAC10"), TEXT("MAC-10"), EOCWeaponClass::SMG,
        EOCInventorySlot::Primary, EOCAmmoType::Pistol);
    T.ActionType = EOCWeaponActionType::Blowback;
    T.Damage = 22.0f; T.RangeCm = 6500.0f; T.RoundsPerMinute = 1050.0f;
    T.HipSpreadDegrees = 1.45f; T.ADSSpreadDegrees = 0.42f; T.MovingSpreadMultiplier = 1.45f;
    T.RecoilPitchMin = 0.42f; T.RecoilPitchMax = 0.72f; T.RecoilYawMax = 0.58f;
    T.MagazineSize = 32; T.InitialReserveAmmo = 160; T.MaxReserveAmmo = 320; T.ReloadDuration = 1.85f;
    T.bSupportsAutomatic = true; T.bSupportsSemiAutomatic = true;
    T.AudioLoudnessScale = 0.92f;
    ConfigureBuiltInTuning(T);
}

void AOCWeapon_Mac10::BeginPlay()
{
    Super::BeginPlay();
    ApplySkeletalProductionWeapon(this, WeaponRoot,
        TEXT("/Game/R13/Weapons/Stein/Mac10/SKM_Mac10.SKM_Mac10"),
        FName(TEXT("ProductionMac10")), 30.0f);
}

AOCWeapon_Tec9::AOCWeapon_Tec9()
{
    FOCWeaponTuning T = BasePreset(TEXT("R13_TEC9"), TEXT("TEC-9"), EOCWeaponClass::SMG,
        EOCInventorySlot::Secondary, EOCAmmoType::Pistol);
    T.ActionType = EOCWeaponActionType::Blowback;
    T.Damage = 24.0f; T.RangeCm = 7200.0f; T.RoundsPerMinute = 520.0f;
    T.HipSpreadDegrees = 1.25f; T.ADSSpreadDegrees = 0.35f; T.MovingSpreadMultiplier = 1.35f;
    T.RecoilPitchMin = 0.38f; T.RecoilPitchMax = 0.66f; T.RecoilYawMax = 0.42f;
    T.MagazineSize = 20; T.InitialReserveAmmo = 100; T.MaxReserveAmmo = 200; T.ReloadDuration = 1.75f;
    T.bSupportsAutomatic = false; T.bSupportsSemiAutomatic = true;
    T.AudioLoudnessScale = 0.88f;
    ConfigureBuiltInTuning(T);
}

void AOCWeapon_Tec9::BeginPlay()
{
    Super::BeginPlay();
    ApplySkeletalProductionWeapon(this, WeaponRoot,
        TEXT("/Game/R13/Weapons/Stein/Tec9/SKM_Tec9.SKM_Tec9"),
        FName(TEXT("ProductionTec9")), 33.0f);
}

AOCWeapon_LeverAction::AOCWeapon_LeverAction()
{
    FOCWeaponTuning T = BasePreset(TEXT("R13_LEVER4570"), TEXT("Lever Action .45-70"), EOCWeaponClass::SniperRifle,
        EOCInventorySlot::Primary, EOCAmmoType::Precision);
    T.ActionType = EOCWeaponActionType::LeverAction;
    T.ManualActionCycleSeconds = 0.85f;
    T.Damage = 78.0f; T.RangeCm = 22000.0f; T.RoundsPerMinute = 72.0f;
    T.HipSpreadDegrees = 2.05f; T.ADSSpreadDegrees = 0.11f; T.MovingSpreadMultiplier = 1.85f;
    T.RecoilPitchMin = 1.45f; T.RecoilPitchMax = 2.05f; T.RecoilYawMax = 0.42f;
    T.MagazineSize = 6; T.InitialReserveAmmo = 36; T.MaxReserveAmmo = 72; T.ReloadDuration = 3.15f;
    T.bSupportsAutomatic = false; T.bSupportsSemiAutomatic = true;
    T.AudioLoudnessScale = 1.12f;
    ConfigureBuiltInTuning(T);
}

void AOCWeapon_LeverAction::BeginPlay()
{
    Super::BeginPlay();
    ApplySkeletalProductionWeapon(this, WeaponRoot,
        TEXT("/Game/R13/Weapons/Stein/LeverAction/SKM_LeverAction.SKM_LeverAction"),
        FName(TEXT("ProductionLeverAction")), 101.0f);
}
