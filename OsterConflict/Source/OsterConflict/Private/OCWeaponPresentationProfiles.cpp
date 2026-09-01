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

    FOCFirstPersonWeaponProfile MakeM700Profile()
    {
        FOCFirstPersonWeaponProfile Profile = MakeLegacyBaselineProfile(FName(TEXT("OC_SNP1")));
        // PASS45 item 16: M700 requires an authored bolt presentation. The repository contains the
        // production skeletal weapon but no accepted bolt-cycle sequence, so the old whole-weapon/arms
        // sine cue is disabled instead of impersonating moving-part animation.
        Profile.bManualActionCueDeclared = false;
        return Profile;
    }

    FOCFirstPersonWeaponProfile MakeRemington870Profile()
    {
        FOCFirstPersonWeaponProfile Profile = MakeLegacyBaselineProfile(FName(TEXT("OC_SG1")));
        // PASS45 item 16: Remington 870 requires an authored pump presentation. A tracked pump sound
        // exists, but no accepted fore-end animation is committed; do not move the whole weapon/arms
        // as a fake pump cycle while the authored content gap remains open.
        Profile.bManualActionCueDeclared = false;
        return Profile;
    }

    FOCFirstPersonWeaponProfile MakeLeverActionProfile()
    {
        FOCFirstPersonWeaponProfile Profile = MakeLegacyBaselineProfile(FName(TEXT("R13_LEVER4570")));
        // PASS45 item 16: the production skeletal LeverAction exists, but no verified lever-cycle
        // sequence is committed. Keep the presentation fail-visible and do not substitute a whole-
        // transform procedural cue for the missing articulated animation.
        Profile.bManualActionCueDeclared = false;
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
    if (WeaponId == FName(TEXT("OC_SNP1")))
    {
        return MakeM700Profile();
    }
    if (WeaponId == FName(TEXT("OC_SG1")))
    {
        return MakeRemington870Profile();
    }
    if (WeaponId == FName(TEXT("R13_LEVER4570")))
    {
        return MakeLeverActionProfile();
    }

    // Remaining weapons keep their previous baseline until each exact production mesh is
    // visually calibrated. The AK has a known verified axis correction from runtime history.
    return MakeLegacyBaselineProfile(WeaponId);
}
