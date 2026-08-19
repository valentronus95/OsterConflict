#pragma once

#include "CoreMinimal.h"

struct FOCLocationSectorBounds
{
    FName Id = NAME_None;
    FVector2D Min = FVector2D::ZeroVector;
    FVector2D Max = FVector2D::ZeroVector;

    bool Contains2D(const FVector& Location) const
    {
        return Location.X >= Min.X && Location.X <= Max.X &&
            Location.Y >= Min.Y && Location.Y <= Max.Y;
    }
};

/**
 * Location-first reconstruction sectors. These are workflow/ownership bounds, not claims about cadastral borders.
 * Permanent geography inside a locked sector must come from explicit authored/reference-backed placement rather
 * than generic city-wide infill.
 */
class OSTERCONFLICT_API FOCLocationSectorPlan
{
public:
    static FOCLocationSectorBounds KrushelnytskaCollegePark();
    static bool IsInsideKrushelnytskaCollegePark(const FVector& Location);

    static FOCLocationSectorBounds MuseumSolonynaParkStadium();
    static bool IsInsideMuseumSolonynaParkStadium(const FVector& Location);
};
