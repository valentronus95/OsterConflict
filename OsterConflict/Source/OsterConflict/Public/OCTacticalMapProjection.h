#pragma once

#include "CoreMinimal.h"

/**
 * Deterministic world <-> tactical-map transform.
 *
 * Oster's source world uses +X = east and +Y = north. The tactical map is north-up,
 * therefore east increases to the right while north decreases screen-space Y.
 * The projection deliberately owns no UI state so it can be tested independently.
 */
struct OSTERCONFLICT_API FOCTacticalMapProjection
{
    FVector2D WorldMin = FVector2D(-120000.0f, -120000.0f);
    FVector2D WorldMax = FVector2D(120000.0f, 120000.0f);
    bool bInvertX = false;
    bool bInvertY = true;

    bool IsValid() const;

    /** Convert world-space centimeters to normalized map UV. */
    FVector2D WorldToUV(const FVector& WorldLocation, bool bClampToBounds = true) const;

    /** Convert normalized map UV back to world-space centimeters. */
    FVector UVToWorld(const FVector2D& UV, float WorldZ = 0.0f, bool bClampToBounds = true) const;

    /** Rotation for an up-pointing UMG marker on a north-up map. UE yaw 0=east, 90=north. */
    float WorldYawToMapDegrees(float WorldYawDegrees) const;
};
