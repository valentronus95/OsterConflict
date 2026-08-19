#pragma once

#include "CoreMinimal.h"
#include "OCGeoReference.h"

enum class EOCS01RoadAnchor : uint8
{
    Absolute,
    CentralPark,
    College
};

enum class EOCS01RoadRelation : uint8
{
    Inside,
    Crossing
};

struct FOCS01RoadCorridorSeed
{
    FName Id = NAME_None;
    EOCS01RoadAnchor Anchor = EOCS01RoadAnchor::Absolute;
    FVector LocalOffset = FVector::ZeroVector;
    FVector SizeCm = FVector::ZeroVector;
    float Yaw = 0.0f;
    bool bTwoWalks = true;
    EOCS01RoadRelation Relation = EOCS01RoadRelation::Crossing;
    EOCReferenceConfidence Confidence = EOCReferenceConfidence::C;
    const TCHAR* Note = TEXT("");
};

struct FOCS01PathSeed
{
    FName Id = NAME_None;
    EOCS01RoadAnchor Anchor = EOCS01RoadAnchor::CentralPark;
    FVector LocalOffset = FVector::ZeroVector;
    FVector SizeCm = FVector::ZeroVector;
    float Yaw = 0.0f;
    EOCS01RoadRelation Relation = EOCS01RoadRelation::Inside;
    EOCReferenceConfidence Confidence = EOCReferenceConfidence::C;
    const TCHAR* Note = TEXT("");
};

class OSTERCONFLICT_API FOCLocationSectorS01RoadData
{
public:
    static const TArray<FOCS01RoadCorridorSeed>& OwnedInsideCorridors();

    /** Empty after all audited BuildRoadNetwork crossings receive explicit split manifests. */
    static const TArray<FOCS01RoadCorridorSeed>& SharedCrossingCorridors();

    static const TArray<FOCS01RoadCorridorSeed>& KrushelnytskaSpineSegments();
    static const TArray<FOCS01RoadCorridorSeed>& EastWest02Segments();
    static const TArray<FOCS01RoadCorridorSeed>& WorldDiag01Segments();
    static const TArray<FOCS01RoadCorridorSeed>& WorldDiag02Segments();
    static const TArray<FOCS01RoadCorridorSeed>& ParkSouthSegments();

    /** One-sided sidewalk split of the former 52000 cm / yaw 63 north-west corridor. */
    static const TArray<FOCS01RoadCorridorSeed>& WorldNW01Segments();

    /** Three-piece one-sided sidewalk split of the Central Park north approach. */
    static const TArray<FOCS01RoadCorridorSeed>& ParkNorthLinkSegments();

    static const TArray<FOCS01PathSeed>& OwnedCentralParkPaths();
    static const TArray<FOCS01PathSeed>& OwnedCollegePaths();
};
