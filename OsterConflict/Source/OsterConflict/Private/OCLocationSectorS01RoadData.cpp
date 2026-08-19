#include "OCLocationSectorS01RoadData.h"

namespace
{
    constexpr EOCReferenceConfidence Provisional = EOCReferenceConfidence::C;
}

const TArray<FOCS01RoadCorridorSeed>& FOCLocationSectorS01RoadData::OwnedInsideCorridors()
{
    static const TArray<FOCS01RoadCorridorSeed> Corridors = {
        { TEXT("S01_ROAD_COLLEGE_APPROACH"), EOCS01RoadAnchor::College,
            FVector(-13500, 0, 8), FVector(30000, 660, 14), 0.0f, true,
            EOCS01RoadRelation::Inside, Provisional,
            TEXT("Former direct BuildRoadNetwork college approach; whole oriented corridor fits inside S01 workflow bounds") },
    };
    return Corridors;
}

const TArray<FOCS01RoadCorridorSeed>& FOCLocationSectorS01RoadData::SharedCrossingCorridors()
{
    // These are intentionally NOT consumed by S01 runtime yet. Each intersects S01 but extends beyond its
    // workflow bounds. Moving the whole segment into S01 ownership would silently claim/change neighboring map space.
    static const TArray<FOCS01RoadCorridorSeed> Corridors = {
        { TEXT("S01_CROSS_WORLD_EW_02"), EOCS01RoadAnchor::Absolute,
            FVector(-18000, 17000, 8), FVector(61000, 820, 16), 0.0f, true,
            EOCS01RoadRelation::Crossing, Provisional,
            TEXT("Generic east-west corridor crossing the S01 east boundary; split before ownership migration") },
        { TEXT("S01_CROSS_KRUSHELNYTSKA_SPINE"), EOCS01RoadAnchor::Absolute,
            FVector(-33500, 25000, 8), FVector(112000, 920, 16), 91.5f, true,
            EOCS01RoadRelation::Crossing, Provisional,
            TEXT("Long Krushelnytska-oriented spine crosses both S01 north/south workflow bounds; split before migration") },
        { TEXT("S01_CROSS_WORLD_DIAG_01"), EOCS01RoadAnchor::Absolute,
            FVector(-23500, 40500, 8), FVector(51000, 760, 16), 18.0f, true,
            EOCS01RoadRelation::Crossing, Provisional,
            TEXT("Diagonal generic corridor intersects S01 and continues outside the east side") },
        { TEXT("S01_CROSS_WORLD_NW_01"), EOCS01RoadAnchor::Absolute,
            FVector(-48000, 51000, 8), FVector(52000, 720, 16), 63.0f, false,
            EOCS01RoadRelation::Crossing, Provisional,
            TEXT("North-west generic corridor intersects S01 and continues outside the workflow bounds") },
        { TEXT("S01_CROSS_WORLD_DIAG_02"), EOCS01RoadAnchor::Absolute,
            FVector(-5000, 33500, 8), FVector(49000, 760, 16), -34.0f, true,
            EOCS01RoadRelation::Crossing, Provisional,
            TEXT("East-side diagonal touches S01 but is mostly city-wide; keep shared until split") },
        { TEXT("S01_CROSS_PARK_SOUTH"), EOCS01RoadAnchor::CentralPark,
            FVector(0, -8500, 8), FVector(43000, 720, 16), 2.0f, true,
            EOCS01RoadRelation::Crossing, Provisional,
            TEXT("Park south approach crosses the S01 west workflow boundary; anchor stays tied to CentralPark") },
        { TEXT("S01_CROSS_PARK_NORTH_LINK"), EOCS01RoadAnchor::CentralPark,
            FVector(-9000, 13500, 8), FVector(37000, 700, 16), 79.0f, false,
            EOCS01RoadRelation::Crossing, Provisional,
            TEXT("Park north approach crosses the S01 north/west workflow bounds; split before ownership migration") },
    };
    return Corridors;
}

const TArray<FOCS01PathSeed>& FOCLocationSectorS01RoadData::OwnedCentralParkPaths()
{
    static const TArray<FOCS01PathSeed> Paths = {
        { TEXT("S01_PATH_PARK_EW"), EOCS01RoadAnchor::CentralPark,
            FVector(0, 0, 14), FVector(17800, 360, 18), 0.0f,
            EOCS01RoadRelation::Inside, Provisional,
            TEXT("Migrated direct Central Park east-west sidewalk strip; C-confidence retained blockout") },
        { TEXT("S01_PATH_PARK_NS"), EOCS01RoadAnchor::CentralPark,
            FVector(0, -300, 14), FVector(360, 13200, 18), 0.0f,
            EOCS01RoadRelation::Inside, Provisional,
            TEXT("Migrated direct Central Park north-south sidewalk strip; C-confidence retained blockout") },
        { TEXT("S01_PATH_PARK_DIAG_E"), EOCS01RoadAnchor::CentralPark,
            FVector(1800, 900, 14), FVector(11800, 260, 18), 31.0f,
            EOCS01RoadRelation::Inside, Provisional,
            TEXT("Migrated direct Central Park diagonal sidewalk strip; C-confidence retained blockout") },
        { TEXT("S01_PATH_PARK_DIAG_W"), EOCS01RoadAnchor::CentralPark,
            FVector(-2300, 1300, 14), FVector(9300, 240, 18), -28.0f,
            EOCS01RoadRelation::Inside, Provisional,
            TEXT("Migrated direct Central Park diagonal sidewalk strip; C-confidence retained blockout") },
    };
    return Paths;
}

const TArray<FOCS01PathSeed>& FOCLocationSectorS01RoadData::OwnedCollegePaths()
{
    static const TArray<FOCS01PathSeed> Paths = {
        { TEXT("S01_PATH_COLLEGE_CAMPUS"), EOCS01RoadAnchor::College,
            FVector(900, 5200, 12), FVector(8000, 5900, 18), 1.0f,
            EOCS01RoadRelation::Inside, Provisional,
            TEXT("Migrated College campus sidewalk surface; whole oriented rectangle fits inside S01") },
    };
    return Paths;
}
