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
