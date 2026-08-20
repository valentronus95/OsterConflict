#include "OCLocationSectorS01RoadData.h"

const TArray<FOCS01PathSeed>& FOCLocationSectorS01RoadData::ParkNorthCivicPathSegments()
{
    // Exact no-layout-change split of the former CentralPark -> CultureParkNorth path.
    // Original derived geometry: length ~= 37087.241614 cm, width 260 cm, yaw ~= 98.142765 deg.
    // The ownership cut is stored 0.001 cm inside the safe S01 interval to keep finite decimal data strictly Inside.
    static const TArray<FOCS01PathSeed> Paths = {
        { TEXT("S01_PATH_PARK_NORTH_CIVIC_INSIDE"), EOCS01RoadAnchor::Absolute,
            FVector(-57278.655313, 45906.384896, 15), FVector(16144.349662, 260, 18), 98.142765f,
            EOCS01RoadRelation::Inside, EOCReferenceConfidence::C,
            TEXT("S01-owned beginning of the former Central Park to north-civic path") },
        { TEXT("S01_PATH_PARK_NORTH_CIVIC_SHARED"), EOCS01RoadAnchor::Absolute,
            FVector(-59905.176315, 64263.052896, 15), FVector(20942.891952, 260, 18), 98.142765f,
            EOCS01RoadRelation::Crossing, EOCReferenceConfidence::C,
            TEXT("Shared north remainder of the former Central Park to north-civic path") },
    };
    return Paths;
}
