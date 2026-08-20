#pragma once

#include "CoreMinimal.h"

/**
 * Source-of-truth animation coverage for one implemented weapon id.
 *
 * Empty paths are intentional: R14 must not invent animation assets that are not
 * actually present in Content. A profile can therefore be declared while its
 * authored Fire/Reload coverage is still pending.
 */
struct FOCWeaponAnimationProfile
{
    FName WeaponId = NAME_None;
    FString FireAnimationObjectPath;
    FString ReloadAnimationObjectPath;

    /** True when convincing reload/fire mechanics require moving weapon parts. */
    bool bRequiresArticulatedWeapon = false;

    bool HasFireAnimation() const { return !FireAnimationObjectPath.IsEmpty(); }
    bool HasReloadAnimation() const { return !ReloadAnimationObjectPath.IsEmpty(); }
    bool HasCompleteAuthoredCoverage() const { return HasFireAnimation() && HasReloadAnimation(); }
};

/** True for every weapon id that currently exists in the R14 gameplay set. */
OSTERCONFLICT_API bool OCHasDeclaredWeaponAnimationProfile(FName WeaponId);

/**
 * Returns the explicit animation profile for WeaponId.
 * Unknown ids return a profile carrying that id with empty paths so callers fail safe.
 */
OSTERCONFLICT_API FOCWeaponAnimationProfile OCResolveWeaponAnimationProfile(FName WeaponId);
