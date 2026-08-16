#include "OCWeaponAudioProfile.h"

const TArray<TObjectPtr<USoundBase>>& UOCWeaponAudioProfile::GetImpactSet(EOCImpactSurface Surface) const
{
    switch (Surface)
    {
        case EOCImpactSurface::Flesh: return ImpactFlesh;
        case EOCImpactSurface::Glass: return ImpactGlass;
        case EOCImpactSurface::Wood: return ImpactWood;
        case EOCImpactSurface::Metal: return ImpactMetal;
        case EOCImpactSurface::Dirt: return ImpactDirt;
        case EOCImpactSurface::Masonry:
        default: return ImpactMasonry;
    }
}
