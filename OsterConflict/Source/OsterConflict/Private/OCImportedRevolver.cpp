#include "OCImportedWeaponVariants.h"

#include "Components/StaticMeshComponent.h"

AOCWeapon_Revolver::AOCWeapon_Revolver()
{
    FOCWeaponTuning T;
    T.WeaponId = FName(TEXT("IMP_REVOLVER"));
    T.DisplayName = TEXT("Revolver");
    T.WeaponClass = EOCWeaponClass::Pistol;
    T.ActionType = EOCWeaponActionType::Revolver;
    T.PreferredSlot = EOCInventorySlot::Secondary;
    T.AmmoType = EOCAmmoType::Pistol;
    T.Damage = 44.0f;
    T.PelletsPerShot = 1;
    T.RangeCm = 9000.0f;
    T.RoundsPerMinute = 180.0f;
    T.HipSpreadDegrees = 1.35f;
    T.ADSSpreadDegrees = 0.28f;
    T.MovingSpreadMultiplier = 1.55f;
    T.RecoilPitchMin = 0.95f;
    T.RecoilPitchMax = 1.45f;
    T.RecoilYawMax = 0.42f;
    T.MagazineSize = 6;
    T.InitialReserveAmmo = 36;
    T.MaxReserveAmmo = 72;
    T.ReloadDuration = 2.85f;
    T.bSupportsSemiAutomatic = true;
    T.bSupportsBurst3 = false;
    T.bSupportsAutomatic = false;
    T.bSupersonicAmmo = true;
    T.AudioLoudnessScale = 1.05f;
    ConfigureBuiltInTuning(T);
}

void AOCWeapon_Revolver::BeginPlay()
{
    Super::BeginPlay();

    // The imported-weapon bridge owns the exact Fab mesh. Hide only constructor/source placeholders here.
    TArray<UStaticMeshComponent*> Components;
    GetComponents<UStaticMeshComponent>(Components);
    for (UStaticMeshComponent* Component : Components)
    {
        if (!Component) continue;
        Component->SetVisibility(false, true);
        Component->SetHiddenInGame(true, true);
        Component->SetCastShadow(false);
        Component->SetCanEverAffectNavigation(false);
    }
}
