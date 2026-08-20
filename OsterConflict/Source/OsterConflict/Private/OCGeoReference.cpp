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

FOCGeoReferencePoint FOCGeoReference::Stadium()
{
    // R13 playtest topology correction: with the player standing south of the museum and looking north at its front
    // entrance, left is local west (-X). Keep the stadium on the museum northing and move only west far enough that
    // its authored ~119 m pitch/stand footprint no longer overlaps the museum site. The relationship is intentional;
    // the exact distance/centroid is still not survey-grade and therefore remains confidence C.
    return { TEXT("StadionOster"), 50.948239, 30.882150, EOCReferenceConfidence::C,
        TEXT("R13 playtest topology anchor: stadium is west/left of the museum facade on the same provisional northing; approximately 120 m center separation, exact centroid still requires site verification") };
}

FOCGeoReferencePoint FOCGeoReference::SolonynaEstatePark()
{
    return { TEXT("SolonynaEstatePark"), 50.948455, 30.885832, EOCReferenceConfidence::B,
        TEXT("Inferred site center from public museum/stadium adjacency; topology anchor, not cadastral centroid") };
}

FOCGeoReferencePoint FOCGeoReference::College()
{
    return { TEXT("OsterCollege"), 50.949214117728445, 30.87912975081365, EOCReferenceConfidence::A,
        TEXT("Verified address coordinate: Solomii Krushelnytskoi (8 Bereznia) 7A; official college facade photos available") };
}

FOCGeoReferencePoint FOCGeoReference::Silpo()
{
    return { TEXT("OsterSilpo"), 50.948833799986254, 30.87572244094098, EOCReferenceConfidence::A,
        TEXT("Verified address coordinate: Bohdana Khmelnytskoho 54; official Silpo listing and public map agree") };
}

FOCGeoReferencePoint FOCGeoReference::BusStation()
{
    return { TEXT("OsterBusStation"), 50.946585220941095, 30.881431234571565, EOCReferenceConfidence::A,
        TEXT("Verified public-map bus-station coordinate; former Haharina 12; historic exterior photo exists on Wikimedia Commons") };
}

FOCGeoReferencePoint FOCGeoReference::CityCouncil()
{
    return { TEXT("OsterCityCouncil"), 50.95180891094453, 30.877121117838357, EOCReferenceConfidence::A,
        TEXT("Verified address coordinate: Nezalezhnosti 21; current council/CNAP address and two Wikimedia Commons facade references agree on the landmark") };
}

FOCGeoReferencePoint FOCGeoReference::CultureHouse()
{
    return { TEXT("OsterCultureHouse"), 50.948694, 30.881435, EOCReferenceConfidence::B,
        TEXT("Public travel/map coordinate paired with the Oster Soviet-era culture house; current official address is Hranovskoho 3 and modern facade photos are available") };
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
