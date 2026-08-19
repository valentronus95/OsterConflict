#pragma once

#include "CoreMinimal.h"
#include "OCGeoReference.h"

/**
 * Individually addressable residential plot seed for S01.
 * Confidence C means the placement is retained only as a gameplay/blockout approximation until replaced by
 * stronger reference evidence. Registry membership must never be interpreted as factual cadastral data.
 */
struct FOCS01ResidentialPlotSeed
{
    FName Id = NAME_None;
    FVector HouseCenter = FVector::ZeroVector;
    FVector HouseSizeCm = FVector::ZeroVector;
    float HouseYaw = 0.0f;
    int32 VisualVariant = 0;
    bool bHasPrimaryHouse = true;

    FVector OutbuildingCenter = FVector::ZeroVector;
    FVector OutbuildingSizeCm = FVector::ZeroVector;
    float OutbuildingYaw = 0.0f;
    bool bHasOutbuilding = false;
    bool bOutbuildingHasRoof = false;

    EOCReferenceConfidence Confidence = EOCReferenceConfidence::C;
    const TCHAR* Note = TEXT("");
};

/**
 * One individually addressable pair of residential frontages inherited from the old S01 blockout.
 * All current values are confidence C until reference-backed plot/fence boundaries replace them.
 */
struct FOCS01FrontageSeed
{
    FName Id = NAME_None;
    FVector WestFenceCenter = FVector::ZeroVector;
    FVector EastFenceCenter = FVector::ZeroVector;
    FVector FenceSizeCm = FVector::ZeroVector;
    float FenceYaw = 0.0f;
    FVector WestWalkCenter = FVector::ZeroVector;
    FVector EastWalkCenter = FVector::ZeroVector;
    FVector WalkSizeCm = FVector::ZeroVector;
    float WalkYaw = 0.0f;
    EOCReferenceConfidence Confidence = EOCReferenceConfidence::C;
    const TCHAR* Note = TEXT("");
};

/** Explicit provisional road segment owned by S01 rather than a city-wide procedural rule. */
struct FOCS01RoadSeed
{
    FName Id = NAME_None;
    FVector Center = FVector::ZeroVector;
    FVector SizeCm = FVector::ZeroVector;
    float Yaw = 0.0f;
    EOCReferenceConfidence Confidence = EOCReferenceConfidence::C;
    const TCHAR* Note = TEXT("");
};

/**
 * Explicit S01 authored-data registry. Current entries intentionally reproduce the old blockout one-for-one,
 * but remove arithmetic placement coupling so every plot/frontage/road can be corrected independently.
 */
class OSTERCONFLICT_API FOCLocationSectorS01Data
{
public:
    static const TArray<FOCS01ResidentialPlotSeed>& ProvisionalResidentialPlots();
    static const TArray<FOCS01FrontageSeed>& ProvisionalFrontages();
    static const TArray<FOCS01RoadSeed>& ProvisionalServiceRoads();
};
