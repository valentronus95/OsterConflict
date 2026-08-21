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
        Profile.CameraLocation = FVector(38.0f, 12.0f, -14.0f);
        Profile.CameraRotation = FRotator::ZeroRotator;
        Profile.bGripCalibrated = false;
        return Profile;
    }

    FOCFirstPersonWeaponProfile MakeAK47Profile()
    {
        FOCFirstPersonWeaponProfile Profile = MakeLegacyBaselineProfile(FName(TEXT("OC_AR1")));

        // Historical runtime QA established that the Fab AK-47 mesh is authored with its
        // long axis on Y while the game's first-person attach convention is X-forward.
        // The -90 degree yaw is therefore an asset-axis correction, not an aesthetic guess.
        Profile.CameraRotation = FRotator(0.0f, -90.0f, 0.0f);
        Profile.bGripCalibrated = true;
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
    if (WeaponId == FName(TEXT("OC_AR1")))
    {
        return MakeAK47Profile();
    }

    // Remaining weapons keep their previous baseline until each exact production mesh is
    // visually calibrated. The AK has a known verified axis correction from runtime history.
    return MakeLegacyBaselineProfile(WeaponId);
}
