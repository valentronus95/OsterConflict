#include "OCGeoReference.h"

FVector FOCGeoReference::ToLocalCm(double Latitude, double Longitude, double ZCm)
{
    // Small-area tangent-plane approximation. Over the S16A Oster play area the error is tiny compared with
    // gameplay/photogrammetry uncertainty, and it keeps source-only anchors deterministic without GIS plugins.
    constexpr double MetersPerDegreeLatitude = 111320.0;
    const double OriginLatitudeRadians = FMath::DegreesToRadians(OriginLatitude);
    const double MetersPerDegreeLongitude = 111320.0 * FMath::Cos(OriginLatitudeRadians);

    const double EastMeters = (Longitude - OriginLongitude) * MetersPerDegreeLongitude;
    const double NorthMeters = (Latitude - OriginLatitude) * MetersPerDegreeLatitude;
    return FVector(EastMeters * 100.0, NorthMeters * 100.0, ZCm);
}

FOCGeoReferencePoint FOCGeoReference::Museum()
{
    return { TEXT("MuseumSolonyna"), 50.948239, 30.883865, EOCReferenceConfidence::A,
        TEXT("Public heritage/museum coordinates; Tatarivska 30; facade photo references available") };
}

FOCGeoReferencePoint FOCGeoReference::College()
{
    return { TEXT("OsterCollege"), 50.949182, 30.879127, EOCReferenceConfidence::A,
        TEXT("Public coordinates; Solomii Krushelnytskoi 7A; official facade references available") };
}

FOCGeoReferencePoint FOCGeoReference::CentralPark()
{
    return { TEXT("CentralCityPark"), 50.951645, 30.875861, EOCReferenceConfidence::A,
        TEXT("Published cultural-heritage coordinate explicitly identified as Central city park") };
}

FOCGeoReferencePoint FOCGeoReference::CultureParkNorth()
{
    return { TEXT("CultureHouseParkNorth"), 50.954943, 30.875112, EOCReferenceConfidence::B,
        TEXT("Published monument coordinate: city park near culture house; used as north park/civic reference") };
}

FOCGeoReferencePoint FOCGeoReference::FormerCityAdministration()
{
    return { TEXT("FormerCityAdministration"), 50.949419, 30.877258, EOCReferenceConfidence::B,
        TEXT("Published heritage coordinate; useful central-street topology anchor") };
}

FOCGeoReferencePoint FOCGeoReference::HistoricCourtBuilding()
{
    return { TEXT("HistoricCourt"), 50.952622, 30.877788, EOCReferenceConfidence::B,
        TEXT("Published heritage coordinate; used only for macro street/site placement") };
}

FOCGeoReferencePoint FOCGeoReference::ResurrectionChurch()
{
    return { TEXT("ResurrectionChurch"), 50.954472, 30.873668, EOCReferenceConfidence::B,
        TEXT("Published heritage coordinate; used as north-west urban reference") };
}
