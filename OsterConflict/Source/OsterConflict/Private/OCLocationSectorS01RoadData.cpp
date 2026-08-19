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
    static const TArray<FOCS01RoadCorridorSeed> Corridors;
    return Corridors;
}

const TArray<FOCS01RoadCorridorSeed>& FOCLocationSectorS01RoadData::KrushelnytskaSpineSegments()
{
    static const TArray<FOCS01RoadCorridorSeed> Segments = {
        { TEXT("S01_KR_SPINE_SOUTH_SHARED"), EOCS01RoadAnchor::Absolute,
            FVector(-32459.619, -14730.542, 8), FVector(32511.678, 920, 16), 91.5f, true,
            EOCS01RoadRelation::Crossing, Provisional, TEXT("South shared remainder of the original Krushelnytska spine") },
        { TEXT("S01_KR_SPINE_INSIDE"), EOCS01RoadAnchor::Absolute,
            FVector(-33570.873, 27706.534, 8), FVector(52391.568, 920, 16), 91.5f, true,
            EOCS01RoadRelation::Inside, Provisional, TEXT("Middle S01-owned Krushelnytska spine segment") },
        { TEXT("S01_KR_SPINE_NORTH_SHARED"), EOCS01RoadAnchor::Absolute,
            FVector(-34611.254, 67437.076, 8), FVector(27096.754, 920, 16), 91.5f, true,
            EOCS01RoadRelation::Crossing, Provisional, TEXT("North shared remainder of the original Krushelnytska spine") },
    };
    return Segments;
}

const TArray<FOCS01RoadCorridorSeed>& FOCLocationSectorS01RoadData::EastWest02Segments()
{
    static const TArray<FOCS01RoadCorridorSeed> Segments = {
        { TEXT("S01_EW02_INSIDE"), EOCS01RoadAnchor::Absolute,
            FVector(-33364.762, 17000, 8), FVector(30270.476, 820, 16), 0.0f, true,
            EOCS01RoadRelation::Inside, Provisional, TEXT("S01-owned west piece of the former east-west corridor") },
        { TEXT("S01_EW02_EAST_SHARED"), EOCS01RoadAnchor::Absolute,
            FVector(-2864.762, 17000, 8), FVector(30729.524, 820, 16), 0.0f, true,
            EOCS01RoadRelation::Crossing, Provisional, TEXT("East shared remainder of the former east-west corridor") },
    };
    return Segments;
}

const TArray<FOCS01RoadCorridorSeed>& FOCLocationSectorS01RoadData::WorldDiag01Segments()
{
    static const TArray<FOCS01RoadCorridorSeed> Segments = {
        { TEXT("S01_DIAG01_INSIDE"), EOCS01RoadAnchor::Absolute,
            FVector(-33109.704296, 37377.617799, 8), FVector(30791.517892, 760, 16), 18.0f, true,
            EOCS01RoadRelation::Inside, Provisional, TEXT("S01-owned portion of the former yaw-18 diagonal") },
        { TEXT("S01_DIAG01_EAST_SHARED"), EOCS01RoadAnchor::Absolute,
            FVector(-8857.763131, 45257.551156, 8), FVector(20208.482108, 760, 16), 18.0f, true,
            EOCS01RoadRelation::Crossing, Provisional, TEXT("East shared remainder of the former yaw-18 diagonal") },
    };
    return Segments;
}

const TArray<FOCS01RoadCorridorSeed>& FOCLocationSectorS01RoadData::WorldDiag02Segments()
{
    static const TArray<FOCS01RoadCorridorSeed> Segments = {
        { TEXT("S01_DIAG02_INSIDE"), EOCS01RoadAnchor::Absolute,
            FVector(-21985.761642, 44957.040892, 8), FVector(8022.938878, 760, 16), -34.0f, true,
            EOCS01RoadRelation::Inside, Provisional, TEXT("S01-owned western tip of the former yaw--34 diagonal") },
        { TEXT("S01_DIAG02_EAST_SHARED"), EOCS01RoadAnchor::Absolute,
            FVector(-1674.341114, 31256.814757, 8), FVector(40977.061122, 760, 16), -34.0f, true,
            EOCS01RoadRelation::Crossing, Provisional, TEXT("East shared remainder of the former yaw--34 diagonal") },
    };
    return Segments;
}

const TArray<FOCS01RoadCorridorSeed>& FOCLocationSectorS01RoadData::ParkSouthSegments()
{
    static const TArray<FOCS01RoadCorridorSeed> Segments = {
        { TEXT("S01_PARK_SOUTH_WEST_SHARED"), EOCS01RoadAnchor::Absolute,
            FVector(-72865.674939, 28831.354830, 8), FVector(9518.877046, 720, 16), 2.0f, true,
            EOCS01RoadRelation::Crossing, Provisional, TEXT("West shared remainder of the former Central Park south approach") },
        { TEXT("S01_PARK_SOUTH_INSIDE"), EOCS01RoadAnchor::Absolute,
            FVector(-51378.772158, 29581.694009, 8), FVector(33481.122954, 720, 16), 2.0f, true,
            EOCS01RoadRelation::Inside, Provisional, TEXT("S01-owned portion of the Central Park south approach") },
    };
    return Segments;
}

const TArray<FOCS01RoadCorridorSeed>& FOCLocationSectorS01RoadData::WorldNW01Segments()
{
    // Former FVector(-48000,51000) / 52000x720 / yaw 63 with one positive-side sidewalk.
    static const TArray<FOCS01RoadCorridorSeed> Segments = {
        { TEXT("S01_NW01_INSIDE"), EOCS01RoadAnchor::Absolute,
            FVector(-53245.837454, 40704.464303, 8), FVector(28890.100313, 720, 16), 63.0f, false,
            EOCS01RoadRelation::Inside, Provisional, TEXT("S01-owned beginning of the former one-sided north-west corridor") },
        { TEXT("S01_NW01_NORTH_SHARED"), EOCS01RoadAnchor::Absolute,
            FVector(-41442.084461, 63870.633932, 8), FVector(23109.899687, 720, 16), 63.0f, false,
            EOCS01RoadRelation::Crossing, Provisional, TEXT("North shared remainder of the former one-sided north-west corridor") },
    };
    return Segments;
}

const TArray<FOCS01RoadCorridorSeed>& FOCLocationSectorS01RoadData::ParkNorthLinkSegments()
{
    // Former CentralPark + FVector(-9000,13500) / 37000x700 / yaw 79 with one positive-side sidewalk.
    static const TArray<FOCS01RoadCorridorSeed> Segments = {
        { TEXT("S01_PARK_NORTH_SOUTH_SHARED"), EOCS01RoadAnchor::Absolute,
            FVector(-68037.092413, 36487.222625, 8), FVector(6584.441779, 700, 16), 79.0f, false,
            EOCS01RoadRelation::Crossing, Provisional, TEXT("South/west shared beginning of the former one-sided park north approach") },
        { TEXT("S01_PARK_NORTH_INSIDE"), EOCS01RoadAnchor::Absolute,
            FVector(-66042.857034, 46746.674253, 8), FVector(14318.507529, 700, 16), 79.0f, false,
            EOCS01RoadRelation::Inside, Provisional, TEXT("S01-owned middle of the former one-sided park north approach") },
        { TEXT("S01_PARK_NORTH_NORTH_SHARED"), EOCS01RoadAnchor::Absolute,
            FVector(-63141.075980, 61675.043628, 8), FVector(16097.050691, 700, 16), 79.0f, false,
            EOCS01RoadRelation::Crossing, Provisional, TEXT("North shared remainder of the former one-sided park north approach") },
    };
    return Segments;
}

const TArray<FOCS01PathSeed>& FOCLocationSectorS01RoadData::OwnedCentralParkPaths()
{
    static const TArray<FOCS01PathSeed> Paths = {
        { TEXT("S01_PATH_PARK_EW"), EOCS01RoadAnchor::CentralPark, FVector(0, 0, 14), FVector(17800, 360, 18), 0.0f,
            EOCS01RoadRelation::Inside, Provisional, TEXT("Central Park east-west sidewalk strip") },
        { TEXT("S01_PATH_PARK_NS"), EOCS01RoadAnchor::CentralPark, FVector(0, -300, 14), FVector(360, 13200, 18), 0.0f,
            EOCS01RoadRelation::Inside, Provisional, TEXT("Central Park north-south sidewalk strip") },
        { TEXT("S01_PATH_PARK_DIAG_E"), EOCS01RoadAnchor::CentralPark, FVector(1800, 900, 14), FVector(11800, 260, 18), 31.0f,
            EOCS01RoadRelation::Inside, Provisional, TEXT("Central Park east diagonal sidewalk strip") },
        { TEXT("S01_PATH_PARK_DIAG_W"), EOCS01RoadAnchor::CentralPark, FVector(-2300, 1300, 14), FVector(9300, 240, 18), -28.0f,
            EOCS01RoadRelation::Inside, Provisional, TEXT("Central Park west diagonal sidewalk strip") },
    };
    return Paths;
}

const TArray<FOCS01PathSeed>& FOCLocationSectorS01RoadData::OwnedCollegePaths()
{
    static const TArray<FOCS01PathSeed> Paths = {
        { TEXT("S01_PATH_COLLEGE_CAMPUS"), EOCS01RoadAnchor::College, FVector(900, 5200, 12), FVector(8000, 5900, 18), 1.0f,
            EOCS01RoadRelation::Inside, Provisional, TEXT("College campus sidewalk surface") },
    };
    return Paths;
}
