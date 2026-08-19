#include "OCLocationSectorS01ReferenceData.h"

const TArray<FOCS01StreetAddressReference>& FOCLocationSectorS01ReferenceData::KrushelnytskaAddressReferences()
{
    // Address markers establish the street's real macro progression, but they are not carriageway-center samples.
    // Keep them B-confidence as alignment evidence even where the address identity itself is independently strong.
    static const TArray<FOCS01StreetAddressReference> References =
    {
        { TEXT("S01_KR_REF_08"), TEXT("8"), 50.94759774583321, 30.876405160556917, EOCReferenceConfidence::B,
            TEXT("Visicom Oster address marker; south-end street evidence, not a road-center coordinate") },
        { TEXT("S01_KR_REF_14"), TEXT("14"), 50.94843423662540, 30.878767729754150, EOCReferenceConfidence::B,
            TEXT("Visicom Oster address marker; street evidence only") },
        { TEXT("S01_KR_REF_7A_COLLEGE"), TEXT("7A"), 50.949214117728445, 30.879129750813650, EOCReferenceConfidence::B,
            TEXT("Visicom Oster address marker cross-checked against the official Oster College address; marker is not road centerline") },
        { TEXT("S01_KR_REF_28"), TEXT("28"), 50.94932787287711, 30.881081789926040, EOCReferenceConfidence::B,
            TEXT("Visicom Oster address marker; demonstrates eastward street progression") },
        { TEXT("S01_KR_REF_40"), TEXT("40"), 50.95038900824071, 30.882703249013880, EOCReferenceConfidence::B,
            TEXT("Visicom Oster address marker; alignment evidence only") },
        { TEXT("S01_KR_REF_42"), TEXT("42"), 50.95059613903775, 30.882914353105647, EOCReferenceConfidence::B,
            TEXT("Visicom Oster address marker; alignment evidence only") },
        { TEXT("S01_KR_REF_74"), TEXT("74"), 50.953855214938635, 30.885876996912668, EOCReferenceConfidence::B,
            TEXT("Visicom Oster address marker; north-east progression evidence") },
        { TEXT("S01_KR_REF_78"), TEXT("78"), 50.95445505467416, 30.885954252027110, EOCReferenceConfidence::B,
            TEXT("Visicom Oster address marker; north-east progression evidence") },
        { TEXT("S01_KR_REF_98"), TEXT("98"), 50.957596730285466, 30.886583072725990, EOCReferenceConfidence::B,
            TEXT("Visicom Oster address marker; northern street extent evidence") },
    };
    return References;
}
