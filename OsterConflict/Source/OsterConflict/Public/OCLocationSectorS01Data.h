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
    bool bHasPrimaryHouse = true;

    FVector OutbuildingCenter = FVector::ZeroVector;
    FVector OutbuildingSizeCm = FVector::ZeroVector;
    float OutbuildingYaw = 0.0f;
    bool bHasOutbuilding = false;

    EOCReferenceConfidence Confidence = EOCReferenceConfidence::C;
    const TCHAR* Note = TEXT("");
};

/**
 * Explicit S01 authored-data registry. Current residential entries intentionally reproduce the old blockout
 * one-for-one, but remove arithmetic placement coupling so every plot can be corrected independently.
 */
class OSTERCONFLICT_API FOCLocationSectorS01Data
{
public:
    static const TArray<FOCS01ResidentialPlotSeed>& ProvisionalResidentialPlots();
};
