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
    /** Corridors completely inside S01 and safe to move out of the generic city road builder one-for-one. */
    static const TArray<FOCS01RoadCorridorSeed>& OwnedInsideCorridors();

    /** Current unsplit BuildRoadNetwork corridors that intersect S01 and still remain audit-only. */
    static const TArray<FOCS01RoadCorridorSeed>& SharedCrossingCorridors();

    /**
     * Three contiguous pieces replacing the former single 112000 cm Krushelnytska spine corridor.
     * South/North remain Crossing/shared; the middle piece is fully Inside including both sidewalk envelopes.
     */
    static const TArray<FOCS01RoadCorridorSeed>& KrushelnytskaSpineSegments();

    /**
     * Two contiguous pieces replacing the former 61000 cm east-west corridor at Y=17000.
     * West piece is fully Inside; east remainder stays shared/Crossing.
     */
    static const TArray<FOCS01RoadCorridorSeed>& EastWest02Segments();

    /** Four current Central Park internal sidewalk strips, all fully inside S01. */
    static const TArray<FOCS01PathSeed>& OwnedCentralParkPaths();

    /** Current College campus sidewalk surface, fully inside S01. */
    static const TArray<FOCS01PathSeed>& OwnedCollegePaths();
};
