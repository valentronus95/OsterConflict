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

const TArray<FOCS01TreeSeed>& FOCLocationSectorS01Data::ProvisionalVegetationTrees()
{
    static const TArray<FOCS01TreeSeed> Trees = {
        { TEXT("S01_PARK_TREE_01"), EOCS01VegetationAnchor::CentralPark, FVector(-8300, -6060, 0), 0.90f, EOCS01TreeFamily::Broadleaf, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_02"), EOCS01VegetationAnchor::CentralPark, FVector(-5910, -5900, 0), 0.95f, EOCS01TreeFamily::Broadleaf, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_03"), EOCS01VegetationAnchor::CentralPark, FVector(-4420, -5740, 0), 1.00f, EOCS01TreeFamily::Poplar, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_04"), EOCS01VegetationAnchor::CentralPark, FVector(-2930, -5580, 0), 0.85f, EOCS01TreeFamily::Broadleaf, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_05"), EOCS01VegetationAnchor::CentralPark, FVector(-540, -5420, 0), 0.90f, EOCS01TreeFamily::Birch, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_06"), EOCS01VegetationAnchor::CentralPark, FVector(950, -6060, 0), 0.95f, EOCS01TreeFamily::Broadleaf, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_07"), EOCS01VegetationAnchor::CentralPark, FVector(3340, -5100, 0), 1.00f, EOCS01TreeFamily::Pine, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_08"), EOCS01VegetationAnchor::CentralPark, FVector(4830, -4940, 0), 0.85f, EOCS01TreeFamily::Poplar, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_09"), EOCS01VegetationAnchor::CentralPark, FVector(6320, -4780, 0), 0.90f, EOCS01TreeFamily::Broadleaf, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_10"), EOCS01VegetationAnchor::CentralPark, FVector(-7940, -4360, 0), 0.95f, EOCS01TreeFamily::Poplar, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_11"), EOCS01VegetationAnchor::CentralPark, FVector(-6450, -4200, 0), 1.00f, EOCS01TreeFamily::Broadleaf, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_12"), EOCS01VegetationAnchor::CentralPark, FVector(-4060, -4040, 0), 0.85f, EOCS01TreeFamily::Birch, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_13"), EOCS01VegetationAnchor::CentralPark, FVector(-2570, -3880, 0), 0.90f, EOCS01TreeFamily::Broadleaf, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_14"), EOCS01VegetationAnchor::CentralPark, FVector(-1080, -3720, 0), 0.95f, EOCS01TreeFamily::Broadleaf, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_15"), EOCS01VegetationAnchor::CentralPark, FVector(1310, -3560, 0), 1.00f, EOCS01TreeFamily::Poplar, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_16"), EOCS01VegetationAnchor::CentralPark, FVector(2800, -3400, 0), 0.85f, EOCS01TreeFamily::Broadleaf, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_17"), EOCS01VegetationAnchor::CentralPark, FVector(5190, -3240, 0), 0.90f, EOCS01TreeFamily::Birch, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_18"), EOCS01VegetationAnchor::CentralPark, FVector(6680, -3080, 0), 0.95f, EOCS01TreeFamily::Poplar, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_19"), EOCS01VegetationAnchor::CentralPark, FVector(-8480, -2660, 0), 1.00f, EOCS01TreeFamily::Pine, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_20"), EOCS01VegetationAnchor::CentralPark, FVector(-6090, -2500, 0), 0.85f, EOCS01TreeFamily::Poplar, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_21"), EOCS01VegetationAnchor::CentralPark, FVector(-4600, -2340, 0), 0.90f, EOCS01TreeFamily::Broadleaf, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_22"), EOCS01VegetationAnchor::CentralPark, FVector(3160, -1700, 0), 0.90f, EOCS01TreeFamily::Poplar, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_23"), EOCS01VegetationAnchor::CentralPark, FVector(5550, -1540, 0), 0.95f, EOCS01TreeFamily::Broadleaf, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_24"), EOCS01VegetationAnchor::CentralPark, FVector(7040, -1380, 0), 1.00f, EOCS01TreeFamily::Broadleaf, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_25"), EOCS01VegetationAnchor::CentralPark, FVector(-8120, -960, 0), 0.85f, EOCS01TreeFamily::Broadleaf, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_26"), EOCS01VegetationAnchor::CentralPark, FVector(-6630, -800, 0), 0.90f, EOCS01TreeFamily::Birch, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_27"), EOCS01VegetationAnchor::CentralPark, FVector(-4240, -640, 0), 0.95f, EOCS01TreeFamily::Broadleaf, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_28"), EOCS01VegetationAnchor::CentralPark, FVector(3520, 0, 0), 0.95f, EOCS01TreeFamily::Broadleaf, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_29"), EOCS01VegetationAnchor::CentralPark, FVector(5910, 160, 0), 1.00f, EOCS01TreeFamily::Birch, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_30"), EOCS01VegetationAnchor::CentralPark, FVector(7400, 320, 0), 0.85f, EOCS01TreeFamily::Broadleaf, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_31"), EOCS01VegetationAnchor::CentralPark, FVector(-7760, 740, 0), 0.90f, EOCS01TreeFamily::Broadleaf, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_32"), EOCS01VegetationAnchor::CentralPark, FVector(-6270, 900, 0), 0.95f, EOCS01TreeFamily::Broadleaf, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_33"), EOCS01VegetationAnchor::CentralPark, FVector(-3880, 1060, 0), 1.00f, EOCS01TreeFamily::Poplar, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_34"), EOCS01VegetationAnchor::CentralPark, FVector(3880, 1700, 0), 1.00f, EOCS01TreeFamily::Broadleaf, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_35"), EOCS01VegetationAnchor::CentralPark, FVector(5370, 1860, 0), 0.85f, EOCS01TreeFamily::Poplar, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_36"), EOCS01VegetationAnchor::CentralPark, FVector(7760, 2020, 0), 0.90f, EOCS01TreeFamily::Pine, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_37"), EOCS01VegetationAnchor::CentralPark, FVector(-7400, 2440, 0), 0.95f, EOCS01TreeFamily::Poplar, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_38"), EOCS01VegetationAnchor::CentralPark, FVector(-5910, 2600, 0), 1.00f, EOCS01TreeFamily::Birch, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_39"), EOCS01VegetationAnchor::CentralPark, FVector(-3520, 2760, 0), 0.85f, EOCS01TreeFamily::Broadleaf, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_40"), EOCS01VegetationAnchor::CentralPark, FVector(-2030, 2920, 0), 0.90f, EOCS01TreeFamily::Poplar, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_41"), EOCS01VegetationAnchor::CentralPark, FVector(360, 3080, 0), 0.95f, EOCS01TreeFamily::Broadleaf, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_42"), EOCS01VegetationAnchor::CentralPark, FVector(1850, 3240, 0), 1.00f, EOCS01TreeFamily::Broadleaf, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_43"), EOCS01VegetationAnchor::CentralPark, FVector(3340, 3400, 0), 0.85f, EOCS01TreeFamily::Birch, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_44"), EOCS01VegetationAnchor::CentralPark, FVector(5730, 3560, 0), 0.90f, EOCS01TreeFamily::Broadleaf, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_45"), EOCS01VegetationAnchor::CentralPark, FVector(7220, 3720, 0), 0.95f, EOCS01TreeFamily::Poplar, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_46"), EOCS01VegetationAnchor::CentralPark, FVector(-7040, 4140, 0), 1.00f, EOCS01TreeFamily::Broadleaf, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_47"), EOCS01VegetationAnchor::CentralPark, FVector(-5550, 4300, 0), 0.85f, EOCS01TreeFamily::Poplar, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_48"), EOCS01VegetationAnchor::CentralPark, FVector(-4060, 4460, 0), 0.90f, EOCS01TreeFamily::Pine, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_49"), EOCS01VegetationAnchor::CentralPark, FVector(-1670, 5420, 0), 0.95f, EOCS01TreeFamily::Broadleaf, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_50"), EOCS01VegetationAnchor::CentralPark, FVector(-180, 4780, 0), 1.00f, EOCS01TreeFamily::Birch, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_51"), EOCS01VegetationAnchor::CentralPark, FVector(2210, 4940, 0), 0.85f, EOCS01TreeFamily::Broadleaf, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_52"), EOCS01VegetationAnchor::CentralPark, FVector(3700, 5100, 0), 0.90f, EOCS01TreeFamily::Poplar, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_53"), EOCS01VegetationAnchor::CentralPark, FVector(5190, 5260, 0), 0.95f, EOCS01TreeFamily::Broadleaf, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_PARK_TREE_54"), EOCS01VegetationAnchor::CentralPark, FVector(7580, 5420, 0), 1.00f, EOCS01TreeFamily::Broadleaf, Provisional, TEXT("Migrated from legacy central-park canopy loop; provisional placement") },
        { TEXT("S01_COLLEGE_TREE_01"), EOCS01VegetationAnchor::College, FVector(-3800, -1100, 0), 1.20f, EOCS01TreeFamily::Pine, Provisional, TEXT("Migrated from legacy college vegetation placement; provisional until reference-backed") },
        { TEXT("S01_COLLEGE_TREE_02"), EOCS01VegetationAnchor::College, FVector(3900, -950, 0), 1.15f, EOCS01TreeFamily::Pine, Provisional, TEXT("Migrated from legacy college vegetation placement; provisional until reference-backed") },
        { TEXT("S01_COLLEGE_TREE_03"), EOCS01VegetationAnchor::College, FVector(-4600, 1500, 0), 1.00f, EOCS01TreeFamily::Pine, Provisional, TEXT("Migrated from legacy college vegetation placement; provisional until reference-backed") },
        { TEXT("S01_COLLEGE_TREE_04"), EOCS01VegetationAnchor::College, FVector(4700, 2100, 0), 0.90f, EOCS01TreeFamily::Birch, Provisional, TEXT("Migrated from legacy college vegetation placement; provisional until reference-backed") },
    };
    return Trees;
}

const TArray<FOCS01GrassPatchSeed>& FOCLocationSectorS01Data::ProvisionalGrassPatches()
{
    static const TArray<FOCS01GrassPatchSeed> Patches = {
        { TEXT("S01_PARK_GRASS_01"), EOCS01VegetationAnchor::CentralPark, FVector(0, 0, 0), FVector(19000, 14500, 4), 6.0f, Provisional,
            TEXT("Migrated mown-grass park footprint; explicit but still provisional") },
        { TEXT("S01_COLLEGE_GRASS_01"), EOCS01VegetationAnchor::College, FVector(0, 5200, 0), FVector(12500, 7600, 4), 2.0f, Provisional,
            TEXT("Migrated college mown-grass footprint; explicit but still provisional") },
    };
    return Patches;
}
