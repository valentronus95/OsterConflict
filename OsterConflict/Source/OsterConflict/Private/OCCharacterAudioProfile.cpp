#include "OCCharacterAudioProfile.h"
const TArray<TObjectPtr<USoundBase>>& UOCCharacterAudioProfile::GetFootstepSet(EOCImpactSurface Surface) const
{switch(Surface){case EOCImpactSurface::Dirt:return FootstepDirt;case EOCImpactSurface::Wood:return FootstepWood;case EOCImpactSurface::Metal:return FootstepMetal;default:return FootstepMasonry;}}
