#pragma once

#include "CoreMinimal.h"
#include "OCGeoReference.h"

/** Anchor used to keep an audited road/path corridor attached to canonical geography where appropriate. */
enum class EOCS01RoadAnchor : uint8
{
    Absolute,
    CentralPark,
    College
};

/** Relationship between the current unsplit corridor/path and the S01 workflow bounds. */
enum class EOCS01RoadRelation : uint8
{
    Inside,
    Crossing
};

/**
 * S01 road-corridor record. Relation controls ownership; a split manifest may contain Crossing pieces that remain
 * shared even though all pieces are rendered from one explicit continuity-preserving record set.
 */
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

/** Explicit sidewalk/path rectangle fully owned by an S01 anchor site. */
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

    /** Only one-sided sidewalk corridors remain unsplit here after symmetric-corridor ownership migration. */
    static const TArray<FOCS01RoadCorridorSeed>& SharedCrossingCorridors();

    /** Three contiguous pieces replacing the former 112000 cm Krushelnytska spine. */
    static const TArray<FOCS01RoadCorridorSeed>& KrushelnytskaSpineSegments();

    /** Two contiguous pieces replacing the former 61000 cm east-west corridor at Y=17000. */
    static const TArray<FOCS01RoadCorridorSeed>& EastWest02Segments();

    /** Two contiguous pieces replacing the former 51000 cm / yaw 18 diagonal crossing. */
    static const TArray<FOCS01RoadCorridorSeed>& WorldDiag01Segments();

    /** Two contiguous pieces replacing the former 49000 cm / yaw -34 diagonal crossing. */
    static const TArray<FOCS01RoadCorridorSeed>& WorldDiag02Segments();

    /** Two contiguous pieces replacing the former CentralPark south approach crossing. */
    static const TArray<FOCS01RoadCorridorSeed>& ParkSouthSegments();

    static const TArray<FOCS01PathSeed>& OwnedCentralParkPaths();
    static const TArray<FOCS01PathSeed>& OwnedCollegePaths();
};
