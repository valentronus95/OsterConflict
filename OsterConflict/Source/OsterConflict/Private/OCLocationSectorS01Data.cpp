#include "OCLocationSectorS01Data.h"

namespace
{
    constexpr EOCReferenceConfidence Provisional = EOCReferenceConfidence::C;
}

const TArray<FOCS01ResidentialPlotSeed>& FOCLocationSectorS01Data::ProvisionalResidentialPlots()
{
    // These entries deliberately preserve the pre-location-first blockout positions one-for-one.
    // They are not claims about real property boundaries or exact real-world house placement.
    static const TArray<FOCS01ResidentialPlotSeed> Plots = {
        { TEXT("S01_KR_W_01"), FVector(-39200, 21200, 260), FVector(1700, 1120, 520), 85.0f, 1, true,
            FVector(-40900, 22450, 150), FVector(700, 1100, 300), 85.0f, true, true, Provisional,
            TEXT("Migrated from legacy arithmetic Krushelnytska blockout; replace individually when referenced") },
        { TEXT("S01_KR_E_01"), FVector(-27800, 20500, 270), FVector(1780, 1180, 540), -88.0f, 0, true,
            FVector(-26200, 22250, 140), FVector(650, 1000, 280), -88.0f, true, false, Provisional,
            TEXT("Migrated from legacy arithmetic Krushelnytska blockout; replace individually when referenced") },

        { TEXT("S01_KR_W_02"), FVector(-39200, 26000, 260), FVector(1700, 1280, 520), 87.0f, 2, true,
            FVector(-40900, 27250, 150), FVector(700, 1100, 300), 87.0f, true, true, Provisional,
            TEXT("Migrated from legacy arithmetic Krushelnytska blockout; replace individually when referenced") },
        { TEXT("S01_KR_E_02"), FVector(-27800, 25300, 270), FVector(1950, 1180, 540), -85.0f, 1, true,
            FVector(-26200, 27050, 140), FVector(650, 1000, 280), -85.0f, true, false, Provisional,
            TEXT("Migrated from legacy arithmetic Krushelnytska blockout; replace individually when referenced") },

        { TEXT("S01_KR_W_03"), FVector(-39200, 30800, 260), FVector(1700, 1120, 520), 89.0f, 3, true,
            FVector(-40900, 32050, 150), FVector(700, 1100, 300), 89.0f, true, true, Provisional,
            TEXT("Migrated from legacy arithmetic Krushelnytska blockout; east-side primary house intentionally absent in legacy slot 03") },
        { TEXT("S01_KR_E_03"), FVector(-27800, 30100, 270), FVector(2120, 1180, 540), -88.0f, 2, false,
            FVector(-26200, 31850, 140), FVector(650, 1000, 280), -88.0f, true, false, Provisional,
            TEXT("Legacy slot retained as an addressable plot; primary house was intentionally absent") },

        { TEXT("S01_KR_W_04"), FVector(-39200, 35600, 260), FVector(1700, 1280, 520), 85.0f, 4, true,
            FVector(-40900, 36850, 150), FVector(700, 1100, 300), 85.0f, true, true, Provisional,
            TEXT("Migrated from legacy arithmetic Krushelnytska blockout; replace individually when referenced") },
        { TEXT("S01_KR_E_04"), FVector(-27800, 34900, 270), FVector(1780, 1180, 540), -85.0f, 3, true,
            FVector(-26200, 36650, 140), FVector(650, 1000, 280), -85.0f, true, false, Provisional,
            TEXT("Migrated from legacy arithmetic Krushelnytska blockout; replace individually when referenced") },

        { TEXT("S01_KR_W_05"), FVector(-39200, 40400, 260), FVector(1700, 1120, 520), 87.0f, 5, true,
            FVector(-40900, 41650, 150), FVector(700, 1100, 300), 87.0f, true, true, Provisional,
            TEXT("Migrated from legacy arithmetic Krushelnytska blockout; replace individually when referenced") },
        { TEXT("S01_KR_E_05"), FVector(-27800, 39700, 270), FVector(1950, 1180, 540), -88.0f, 4, true,
            FVector(-26200, 41450, 140), FVector(650, 1000, 280), -88.0f, true, false, Provisional,
            TEXT("Migrated from legacy arithmetic Krushelnytska blockout; replace individually when referenced") },

        { TEXT("S01_KR_W_06"), FVector(-39200, 45200, 260), FVector(1700, 1280, 520), 89.0f, 6, true,
            FVector(-40900, 46450, 150), FVector(700, 1100, 300), 89.0f, true, true, Provisional,
            TEXT("Migrated from legacy arithmetic Krushelnytska blockout; replace individually when referenced") },
        { TEXT("S01_KR_E_06"), FVector(-27800, 44500, 270), FVector(2120, 1180, 540), -85.0f, 5, true,
            FVector(-26200, 46250, 140), FVector(650, 1000, 280), -85.0f, true, false, Provisional,
            TEXT("Migrated from legacy arithmetic Krushelnytska blockout; replace individually when referenced") },

        { TEXT("S01_KR_W_07"), FVector(-39200, 50000, 260), FVector(1700, 1120, 520), 85.0f, 7, true,
            FVector(-40900, 51250, 150), FVector(700, 1100, 300), 85.0f, true, true, Provisional,
            TEXT("Migrated from legacy arithmetic Krushelnytska blockout; replace individually when referenced") },
        { TEXT("S01_KR_E_07"), FVector(-27800, 49300, 270), FVector(1780, 1180, 540), -88.0f, 6, true,
            FVector(-26200, 51050, 140), FVector(650, 1000, 280), -88.0f, true, false, Provisional,
            TEXT("Migrated from legacy arithmetic Krushelnytska blockout; replace individually when referenced") },

        { TEXT("S01_KR_W_08"), FVector(-39200, 54800, 260), FVector(1700, 1280, 520), 87.0f, 8, true,
            FVector(-40900, 56050, 150), FVector(700, 1100, 300), 87.0f, true, true, Provisional,
            TEXT("Migrated from legacy arithmetic Krushelnytska blockout; replace individually when referenced") },
        { TEXT("S01_KR_E_08"), FVector(-27800, 54100, 270), FVector(1950, 1180, 540), -85.0f, 7, true,
            FVector(-26200, 55850, 140), FVector(650, 1000, 280), -85.0f, true, false, Provisional,
            TEXT("Migrated from legacy arithmetic Krushelnytska blockout; replace individually when referenced") },
    };

    return Plots;
}

const TArray<FOCS01FrontageSeed>& FOCLocationSectorS01Data::ProvisionalFrontages()
{
    static const FVector FenceSize(3200.0f, 35.0f, 170.0f);
    static const FVector WalkSize(2100.0f, 160.0f, 18.0f);
    static const TArray<FOCS01FrontageSeed> Frontages = {
        { TEXT("S01_KR_FRONT_01"), FVector(-37100, 19300, 85), FVector(-29900, 19300, 85), FenceSize, 90.0f,
            FVector(-36500, 20950, 18), FVector(-30500, 20150, 18), WalkSize, 0.0f, Provisional,
            TEXT("Migrated frontage from legacy 4.8 m arithmetic slot; replace with reference-backed plot boundary") },
        { TEXT("S01_KR_FRONT_02"), FVector(-37100, 24100, 85), FVector(-29900, 24100, 85), FenceSize, 90.0f,
            FVector(-36500, 25750, 18), FVector(-30500, 24950, 18), WalkSize, 0.0f, Provisional,
            TEXT("Migrated frontage from legacy 4.8 m arithmetic slot; replace with reference-backed plot boundary") },
        { TEXT("S01_KR_FRONT_03"), FVector(-37100, 28900, 85), FVector(-29900, 28900, 85), FenceSize, 90.0f,
            FVector(-36500, 30550, 18), FVector(-30500, 29750, 18), WalkSize, 0.0f, Provisional,
            TEXT("Migrated frontage from legacy 4.8 m arithmetic slot; replace with reference-backed plot boundary") },
        { TEXT("S01_KR_FRONT_04"), FVector(-37100, 33700, 85), FVector(-29900, 33700, 85), FenceSize, 90.0f,
            FVector(-36500, 35350, 18), FVector(-30500, 34550, 18), WalkSize, 0.0f, Provisional,
            TEXT("Migrated frontage from legacy 4.8 m arithmetic slot; replace with reference-backed plot boundary") },
        { TEXT("S01_KR_FRONT_05"), FVector(-37100, 38500, 85), FVector(-29900, 38500, 85), FenceSize, 90.0f,
            FVector(-36500, 40150, 18), FVector(-30500, 39350, 18), WalkSize, 0.0f, Provisional,
            TEXT("Migrated frontage from legacy 4.8 m arithmetic slot; replace with reference-backed plot boundary") },
        { TEXT("S01_KR_FRONT_06"), FVector(-37100, 43300, 85), FVector(-29900, 43300, 85), FenceSize, 90.0f,
            FVector(-36500, 44950, 18), FVector(-30500, 44150, 18), WalkSize, 0.0f, Provisional,
            TEXT("Migrated frontage from legacy 4.8 m arithmetic slot; replace with reference-backed plot boundary") },
        { TEXT("S01_KR_FRONT_07"), FVector(-37100, 48100, 85), FVector(-29900, 48100, 85), FenceSize, 90.0f,
            FVector(-36500, 49750, 18), FVector(-30500, 48950, 18), WalkSize, 0.0f, Provisional,
            TEXT("Migrated frontage from legacy 4.8 m arithmetic slot; replace with reference-backed plot boundary") },
        { TEXT("S01_KR_FRONT_08"), FVector(-37100, 52900, 85), FVector(-29900, 52900, 85), FenceSize, 90.0f,
            FVector(-36500, 54550, 18), FVector(-30500, 53750, 18), WalkSize, 0.0f, Provisional,
            TEXT("Migrated frontage from legacy 4.8 m arithmetic slot; replace with reference-backed plot boundary") },
    };
    return Frontages;
}

const TArray<FOCS01RoadSeed>& FOCLocationSectorS01Data::ProvisionalServiceRoads()
{
    static const TArray<FOCS01RoadSeed> Roads = {
        { TEXT("S01_KR_SERVICE_W"), FVector(-43000, 36000, 8), FVector(560, 42000, 14), 0.0f, Provisional,
            TEXT("Legacy service-road blockout retained as explicit C-confidence S01 segment") },
        { TEXT("S01_KR_SERVICE_E"), FVector(-24200, 37000, 8), FVector(560, 39000, 14), 0.0f, Provisional,
            TEXT("Legacy service-road blockout retained as explicit C-confidence S01 segment") },
    };
    return Roads;
}
