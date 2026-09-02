#include "OCWeaponAnimationProfiles.h"

namespace
{
    const FOCWeaponAnimationProfile Profiles[] =
    {
        {
            FName(TEXT("OC_AR1")),
            TEXT("/Game/AK-47/Animations/AK-47_Fire_W.AK-47_Fire_W"),
            TEXT("/Game/AK-47/Animations/AK-47_Reload_W.AK-47_Reload_W"),
            false,
        },
        { FName(TEXT("OC_SMG1")), TEXT(""), TEXT(""), false },
        { FName(TEXT("OC_PST1")), TEXT(""), TEXT(""), false },

        // M700 is authoritative BoltAction. No committed authored bolt sequence exists yet, so the empty
        // manual-action path is intentional and must remain fail-visible rather than borrowing AK/reload motion.
        { FName(TEXT("OC_SNP1")), TEXT(""), TEXT(""), true, TEXT(""), true },

        // Remington 870 is authoritative PumpAction. The exact registered donor is reused through the audited
        // PASS45_PumpForeEnd derivative. Production import/fresh-load must pass before gameplay can reach this path;
        // local UE 5.8 visual/audio runtime acceptance is still required before item 16 can close.
        { FName(TEXT("OC_SG1")), TEXT(""), TEXT(""), true,
            TEXT("/Game/Production/Weapons/Remington870/AN_Remington870_PumpCycle.AN_Remington870_PumpCycle"), true },

        { FName(TEXT("OC_LMG1")), TEXT(""), TEXT(""), true },
        { FName(TEXT("R13_M14")), TEXT(""), TEXT(""), false },
        { FName(TEXT("R13_MAC10")), TEXT(""), TEXT(""), false },
        { FName(TEXT("R13_TEC9")), TEXT(""), TEXT(""), false },

        // LeverAction is authoritative LeverAction. The skeletal mesh is committed, but a verified lever-cycle
        // sequence is not, therefore the authored slot stays empty and explicitly required.
        { FName(TEXT("R13_LEVER4570")), TEXT(""), TEXT(""), true, TEXT(""), true },

        { FName(TEXT("OC_RPG1")), TEXT(""), TEXT(""), false },
    };
}

bool OCHasDeclaredWeaponAnimationProfile(const FName WeaponId)
{
    for (const FOCWeaponAnimationProfile& Profile : Profiles)
    {
        if (Profile.WeaponId == WeaponId)
        {
            return true;
        }
    }
    return false;
}

FOCWeaponAnimationProfile OCResolveWeaponAnimationProfile(const FName WeaponId)
{
    for (const FOCWeaponAnimationProfile& Profile : Profiles)
    {
        if (Profile.WeaponId == WeaponId)
        {
            return Profile;
        }
    }

    FOCWeaponAnimationProfile Missing;
    Missing.WeaponId = WeaponId;
    return Missing;
}
