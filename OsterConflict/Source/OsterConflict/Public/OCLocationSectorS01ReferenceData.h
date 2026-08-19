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

class OSTERCONFLICT_API FOCLocationSectorS01ReferenceData
{
public:
    /** Ordered south-to-north address evidence for vul. Solomii Krushelnytskoi in Oster. */
    static const TArray<FOCS01StreetAddressReference>& KrushelnytskaAddressReferences();
};
