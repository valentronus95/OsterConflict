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
    // Only one-sided sidewalk corridors remain unsplit here. Their ownership envelope is asymmetric and receives
    // a dedicated follow-up pass rather than reusing the symmetric two-sidewalk cut formula.
    static const TArray<FOCS01RoadCorridorSeed> Corridors = {
        { TEXT("S01_CROSS_WORLD_NW_01"), EOCS01RoadAnchor::Absolute,
            FVector(-48000, 51000, 8), FVector(52000, 720, 16), 63.0f, false,
            EOCS01RoadRelation::Crossing, Provisional,
            TEXT("North-west generic corridor intersects S01; one-sided sidewalk requires asymmetric ownership split") },
        { TEXT("S01_CROSS_PARK_NORTH_LINK"), EOCS01RoadAnchor::CentralPark,
            FVector(-9000, 13500, 8), FVector(37000, 700, 16), 79.0f, false,
            EOCS01RoadRelation::Crossing, Provisional,
            TEXT("Park north approach crosses north/west workflow bounds; one-sided sidewalk requires asymmetric split") },
    };
    return Corridors;
}

const TArray<FOCS01RoadCorridorSeed>& FOCLocationSectorS01RoadData::KrushelnytskaSpineSegments()
{
    static const TArray<FOCS01RoadCorridorSeed> Segments = {
        { TEXT("S01_KR_SPINE_SOUTH_SHARED"), EOCS01RoadAnchor::Absolute,
            FVector(-32459.619, -14730.542, 8), FVector(32511.678, 920, 16), 91.5f, true,
            EOCS01RoadRelation::Crossing, Provisional,
            TEXT("South shared remainder of the original Krushelnytska spine after S01 ownership split") },
        { TEXT("S01_KR_SPINE_INSIDE"), EOCS01RoadAnchor::Absolute,
            FVector(-33570.873, 27706.534, 8), FVector(52391.568, 920, 16), 91.5f, true,
            EOCS01RoadRelation::Inside, Provisional,
            TEXT("Middle Krushelnytska spine segment fully inside S01 including both generated sidewalk envelopes") },
        { TEXT("S01_KR_SPINE_NORTH_SHARED"), EOCS01RoadAnchor::Absolute,
            FVector(-34611.254, 67437.076, 8), FVector(27096.754, 920, 16), 91.5f, true,
            EOCS01RoadRelation::Crossing, Provisional,
            TEXT("North shared remainder of the original Krushelnytska spine after S01 ownership split") },
    };
    return Segments;
}

const TArray<FOCS01RoadCorridorSeed>& FOCLocationSectorS01RoadData::EastWest02Segments()
{
    static const TArray<FOCS01RoadCorridorSeed> Segments = {
        { TEXT("S01_EW02_INSIDE"), EOCS01RoadAnchor::Absolute,
            FVector(-33364.762, 17000, 8), FVector(30270.476, 820, 16), 0.0f, true,
            EOCS01RoadRelation::Inside, Provisional,
            TEXT("West piece of the original east-west corridor, ending microscopically inside the current S01 east workflow boundary") },
        { TEXT("S01_EW02_EAST_SHARED"), EOCS01RoadAnchor::Absolute,
            FVector(-2864.762, 17000, 8), FVector(30729.524, 820, 16), 0.0f, true,
            EOCS01RoadRelation::Crossing, Provisional,
            TEXT("East shared remainder of the original east-west corridor after the S01 ownership split") },
    };
    return Segments;
}

const TArray<FOCS01RoadCorridorSeed>& FOCLocationSectorS01RoadData::WorldDiag01Segments()
{
    // Former FVector(-23500,40500) / 51000x760 / yaw 18 corridor. The cut is 0.001 cm inside the calculated
    // ownership limit so decimal storage cannot round the S01 piece back across the workflow boundary.
    static const TArray<FOCS01RoadCorridorSeed> Segments = {
        { TEXT("S01_DIAG01_INSIDE"), EOCS01RoadAnchor::Absolute,
            FVector(-33109.704296, 37377.617799, 8), FVector(30791.517892, 760, 16), 18.0f, true,
            EOCS01RoadRelation::Inside, Provisional,
            TEXT("Inside portion of the former yaw-18 shared diagonal with both sidewalk envelopes retained") },
        { TEXT("S01_DIAG01_EAST_SHARED"), EOCS01RoadAnchor::Absolute,
            FVector(-8857.763131, 45257.551156, 8), FVector(20208.482108, 760, 16), 18.0f, true,
            EOCS01RoadRelation::Crossing, Provisional,
            TEXT("East shared remainder of the former yaw-18 diagonal") },
    };
    return Segments;
}

const TArray<FOCS01RoadCorridorSeed>& FOCLocationSectorS01RoadData::WorldDiag02Segments()
{
    // Former FVector(-5000,33500) / 49000x760 / yaw -34 corridor. Only the western end reaches S01.
    static const TArray<FOCS01RoadCorridorSeed> Segments = {
        { TEXT("S01_DIAG02_INSIDE"), EOCS01RoadAnchor::Absolute,
            FVector(-21985.761642, 44957.040892, 8), FVector(8022.938878, 760, 16), -34.0f, true,
            EOCS01RoadRelation::Inside, Provisional,
            TEXT("Small inside portion of the former yaw--34 diagonal, preserving both sidewalks") },
        { TEXT("S01_DIAG02_EAST_SHARED"), EOCS01RoadAnchor::Absolute,
            FVector(-1674.341114, 31256.814757, 8), FVector(40977.061122, 760, 16), -34.0f, true,
            EOCS01RoadRelation::Crossing, Provisional,
            TEXT("Large east shared remainder of the former yaw--34 diagonal") },
    };
    return Segments;
}

const TArray<FOCS01RoadCorridorSeed>& FOCLocationSectorS01RoadData::ParkSouthSegments()
{
    // Former CentralPark + FVector(0,-8500) / 43000x720 / yaw 2 corridor. Western remainder stays shared;
    // eastern/longer piece fits S01 with the complete two-sidewalk envelope.
    static const TArray<FOCS01RoadCorridorSeed> Segments = {
        { TEXT("S01_PARK_SOUTH_WEST_SHARED"), EOCS01RoadAnchor::Absolute,
            FVector(-72865.674939, 28831.354830, 8), FVector(9518.877046, 720, 16), 2.0f, true,
            EOCS01RoadRelation::Crossing, Provisional,
            TEXT("West shared remainder of the former Central Park south approach") },
        { TEXT("S01_PARK_SOUTH_INSIDE"), EOCS01RoadAnchor::Absolute,
            FVector(-51378.772158, 29581.694009, 8), FVector(33481.122954, 720, 16), 2.0f, true,
            EOCS01RoadRelation::Inside, Provisional,
            TEXT("Inside portion of the Central Park south approach with both sidewalk envelopes retained") },
    };
    return Segments;
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
