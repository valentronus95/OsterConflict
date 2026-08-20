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

/**
 * Broad review-only corridor around the College transition slice. It is a constraint envelope for later centerline
 * authoring, not a road footprint and not an excuse to infer centimeter-accurate geometry from address markers.
 */
struct FOCS01CenterlineAuthoringRegion
{
    FName Id = NAME_None;
    FVector2D LocalCenterCm = FVector2D::ZeroVector;
    FVector2D HalfSizeCm = FVector2D::ZeroVector;
    float EvidencePaddingCm = 0.0f;
    EOCReferenceConfidence Confidence = EOCReferenceConfidence::C;
    const TCHAR* DerivationNote = TEXT("");
};

/**
 * Explicit record that a retained runtime blockout segment conflicts with newer public reference evidence.
 * This prevents tidy migrated geometry from being mistaken for verified geography before replacement is ready.
 */
struct FOCS01ReferenceConflictRecord
{
    FName RuntimeGeometryId = NAME_None;
    EOCReferenceConfidence MaximumAllowedConfidence = EOCReferenceConfidence::C;
    const TCHAR* ConflictReason = TEXT("");
};

class OSTERCONFLICT_API FOCLocationSectorS01KrushelnytskaAuthoringData
{
public:
    /**
     * Derived review gates for the S01 slice only. A future centerline candidate must cross these windows,
     * but OCWorldSectorOster must never render the gates themselves.
     */
    static const TArray<FOCS01CenterlineAuthoringGate>& ReviewOnlyCenterlineGates();

    /**
     * Padded/clipped envelope of the 14 -> 7A -> 28 evidence around the College-side transition.
     * It contains both entry/exit gates but remains review-only until direct road-shape evidence exists.
     */
    static const FOCS01CenterlineAuthoringRegion& CollegeTransitionRegion();

    /** Retained C-confidence Krushelnytska blockout pieces that must not be promoted before replacement. */
    static const TArray<FOCS01ReferenceConflictRecord>& ReferenceConflictedRuntimeSegments();
};
