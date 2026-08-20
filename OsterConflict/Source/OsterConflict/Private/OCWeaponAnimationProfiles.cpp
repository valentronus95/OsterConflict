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
        { FName(TEXT("OC_SNP1")), TEXT(""), TEXT(""), false },
        { FName(TEXT("OC_SG1")), TEXT(""), TEXT(""), true },
        { FName(TEXT("OC_LMG1")), TEXT(""), TEXT(""), true },
        { FName(TEXT("R13_M14")), TEXT(""), TEXT(""), false },
        { FName(TEXT("R13_MAC10")), TEXT(""), TEXT(""), false },
        { FName(TEXT("R13_TEC9")), TEXT(""), TEXT(""), false },
        { FName(TEXT("R13_LEVER4570")), TEXT(""), TEXT(""), false },
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
