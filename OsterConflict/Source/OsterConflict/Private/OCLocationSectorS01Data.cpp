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
