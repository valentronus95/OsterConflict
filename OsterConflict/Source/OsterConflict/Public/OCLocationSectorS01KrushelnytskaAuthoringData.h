#pragma once

#include "CoreMinimal.h"
#include "OCGeoReference.h"

/**
 * Review-only uncertainty gate for a future Krushelnytska carriageway centerline.
 * The gate is derived from reference relationships; it is not runtime road geometry.
 */
struct FOCS01CenterlineAuthoringGate
{
    FName Id = NAME_None;
    FVector2D LocalCenterCm = FVector2D::ZeroVector;
    FVector2D HalfSizeCm = FVector2D::ZeroVector;
    EOCReferenceConfidence Confidence = EOCReferenceConfidence::C;
    const TCHAR* DerivationNote = TEXT("");
};

class OSTERCONFLICT_API FOCLocationSectorS01KrushelnytskaAuthoringData
{
public:
    /**
     * Derived review gates for the S01 slice only. A future centerline candidate must cross these windows,
     * but OCWorldSectorOster must never render the gates themselves.
     */
    static const TArray<FOCS01CenterlineAuthoringGate>& ReviewOnlyCenterlineGates();
};
