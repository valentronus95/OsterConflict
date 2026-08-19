#pragma once

#include "CoreMinimal.h"
#include "OCGeoReference.h"

/**
 * Public-map/address evidence used to correct S01 without pretending an address marker is a surveyed road centerline.
 * These records are reference evidence only; runtime road geometry must be derived separately and explicitly reviewed.
 */
struct FOCS01StreetAddressReference
{
    FName Id = NAME_None;
    const TCHAR* AddressLabel = TEXT("");
    double Latitude = 0.0;
    double Longitude = 0.0;
    EOCReferenceConfidence Confidence = EOCReferenceConfidence::B;
    const TCHAR* SourceNote = TEXT("");
};

/**
 * Public map's street-object extent/label center. The center is metadata for the whole mapped street object,
 * not a carriageway center point and never a runtime waypoint.
 */
struct FOCS01StreetExtentReference
{
    FName Id = NAME_None;
    double CenterLatitude = 0.0;
    double CenterLongitude = 0.0;
    double MinLatitude = 0.0;
    double MinLongitude = 0.0;
    double MaxLatitude = 0.0;
    double MaxLongitude = 0.0;
    EOCReferenceConfidence Confidence = EOCReferenceConfidence::B;
    const TCHAR* SourceNote = TEXT("");
};

class OSTERCONFLICT_API FOCLocationSectorS01ReferenceData
{
public:
    /** Ordered south-to-north address evidence for vul. Solomii Krushelnytskoi in Oster. */
    static const TArray<FOCS01StreetAddressReference>& KrushelnytskaAddressReferences();

    /** Whole-street public-map extent used as a macro sanity bound, never as a road centerline. */
    static const FOCS01StreetExtentReference& KrushelnytskaStreetExtentReference();
};
