#include "OCWeaponPresentationProfiles.h"

namespace
{
    const FName DeclaredWeaponIds[] =
    {
        FName(TEXT("OC_AR1")),
        FName(TEXT("OC_SMG1")),
        FName(TEXT("OC_PST1")),
        FName(TEXT("OC_SNP1")),
        FName(TEXT("OC_SG1")),
        FName(TEXT("OC_LMG1")),
        FName(TEXT("R13_M14")),
        FName(TEXT("R13_MAC10")),
        FName(TEXT("R13_TEC9")),
        FName(TEXT("R13_LEVER4570")),
        FName(TEXT("OC_RPG1")),
    };

    FOCFirstPersonWeaponProfile MakeLegacyBaselineProfile(const FName WeaponId)
    {
        FOCFirstPersonWeaponProfile Profile;
        Profile.WeaponId = WeaponId;

        // Preserve the old camera-space presentation until the exact mesh is visually
        // inspected. R14 deliberately does not invent per-weapon grip coordinates.
        Profile.CameraLocation = FVector(38.0f, 12.0f, -14.0f);
        Profile.CameraRotation = FRotator::ZeroRotator;
        Profile.bGripCalibrated = false;
        return Profile;
    }
}

bool OCHasDeclaredFirstPersonWeaponProfile(const FName WeaponId)
{
    for (const FName DeclaredId : DeclaredWeaponIds)
    {
        if (DeclaredId == WeaponId) return true;
    }
    return false;
}

FOCFirstPersonWeaponProfile OCResolveFirstPersonWeaponProfile(const FName WeaponId)
{
    // Every known weapon currently starts from the legacy baseline. The important R14
    // change is that each id now owns an explicit profile slot and calibration state,
    // so visual tuning can be done weapon-by-weapon without hidden shared constants.
    if (OCHasDeclaredFirstPersonWeaponProfile(WeaponId))
    {
        return MakeLegacyBaselineProfile(WeaponId);
    }

    return MakeLegacyBaselineProfile(WeaponId);
}
