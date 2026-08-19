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
 * S01 road-corridor record. Crossing records are audit-only until the shared corridor can be split without
 * changing visible geometry outside the sector. Inside records may be consumed directly by S01 runtime code.
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
    /** Corridors completely inside S01 and safe to move out of the generic city road builder one-for-one. */
    static const TArray<FOCS01RoadCorridorSeed>& OwnedInsideCorridors();

    /** Current BuildRoadNetwork corridors that intersect S01 but extend outside it. Audit-only until split. */
    static const TArray<FOCS01RoadCorridorSeed>& SharedCrossingCorridors();

    /** Four current Central Park internal sidewalk strips, all fully inside S01. */
    static const TArray<FOCS01PathSeed>& OwnedCentralParkPaths();

    /** Current College campus sidewalk surface, fully inside S01. */
    static const TArray<FOCS01PathSeed>& OwnedCollegePaths();
};
