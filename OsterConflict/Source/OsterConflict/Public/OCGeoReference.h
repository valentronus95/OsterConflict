#pragma once

#include "CoreMinimal.h"

/**
 * S16A lightweight WGS84 -> local Unreal mapping for the Oster gameplay reconstruction.
 * Museum/Solonyna estate is the local origin. Values are reference anchors, not cadastral survey data.
 */
enum class EOCReferenceConfidence : uint8
{
    A, // verified public coordinates + strong photo/document support
    B, // reliable public placement, incomplete architectural/site detail
    C  // gameplay-authored / characteristic archetype only
};

struct FOCGeoReferencePoint
{
    FName Id = NAME_None;
    double Latitude = 0.0;
    double Longitude = 0.0;
    EOCReferenceConfidence Confidence = EOCReferenceConfidence::C;
    const TCHAR* Note = TEXT("");
};

class OSTERCONFLICT_API FOCGeoReference
{
public:
    // Heritage/public-map coordinate used as the S16A local origin.
    static constexpr double OriginLatitude = 50.948239;
    static constexpr double OriginLongitude = 30.883865;

    static FVector ToLocalCm(double Latitude, double Longitude, double ZCm = 0.0);

    static FOCGeoReferencePoint Museum();
    static FOCGeoReferencePoint Stadium();
    static FOCGeoReferencePoint SolonynaEstatePark();
    static FOCGeoReferencePoint College();
    static FOCGeoReferencePoint CentralPark();
    static FOCGeoReferencePoint CultureParkNorth();
    static FOCGeoReferencePoint FormerCityAdministration();
    static FOCGeoReferencePoint HistoricCourtBuilding();
    static FOCGeoReferencePoint ResurrectionChurch();
};
