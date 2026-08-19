#include "OCLocationSectorS01KrushelnytskaAuthoringData.h"

const TArray<FOCS01CenterlineAuthoringGate>&
FOCLocationSectorS01KrushelnytskaAuthoringData::ReviewOnlyCenterlineGates()
{
    // Centers are deterministic intersections of broad address-progression chords with the current S01 workflow
    // rectangle. Large uncertainty windows deliberately prevent these derived values from masquerading as surveyed
    // road-center coordinates. The permanent verifier recomputes both centers from the source evidence.
    static const TArray<FOCS01CenterlineAuthoringGate> Gates =
    {
        {
            TEXT("S01_KR_GATE_SOUTH_ENTRY"),
            FVector2D(-36951.940661f, 1497.476000f),
            FVector2D(7500.0f, 1000.0f),
            EOCReferenceConfidence::C,
            TEXT("Review-only: S01 south-boundary intersection of address-evidence progression 8 -> 14; +/-75m lateral uncertainty")
        },
        {
            TEXT("S01_KR_GATE_EAST_EXIT"),
            FVector2D(-18229.523391f, 13461.586269f),
            FVector2D(1000.0f, 8000.0f),
            EOCReferenceConfidence::C,
            TEXT("Review-only: S01 east-boundary intersection of address-evidence progression 28 -> 40; +/-80m longitudinal uncertainty")
        },
    };
    return Gates;
}

const TArray<FOCS01ReferenceConflictRecord>&
FOCLocationSectorS01KrushelnytskaAuthoringData::ReferenceConflictedRuntimeSegments()
{
    // These are intentionally still rendered so the current playtest remains traversable, but public Oster-specific
    // evidence now contradicts treating their near-vertical alignment as verified street geography. Replacement must
    // be atomic: keep the safe C blockout until a reviewed centerline can replace the complete S01/shared continuity.
    static const TArray<FOCS01ReferenceConflictRecord> Conflicts =
    {
        {
            TEXT("S01_KR_SPINE_SOUTH_SHARED"),
            EOCReferenceConfidence::C,
            TEXT("Retained migration blockout; south approach is only bounded by review gate, not a verified road centerline")
        },
        {
            TEXT("S01_KR_SPINE_INSIDE"),
            EOCReferenceConfidence::C,
            TEXT("Retained migration blockout; address evidence shows the College-side street progresses east rather than remaining near-vertical")
        },
        {
            TEXT("S01_KR_SPINE_NORTH_SHARED"),
            EOCReferenceConfidence::C,
            TEXT("Retained migration blockout conflicts with evidence that Krushelnytska exits S01 to the east; do not promote or lock")
        },
    };
    return Conflicts;
}
