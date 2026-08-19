#include "OCLocationSectorPlan.h"

#include "OCGeoReference.h"

FOCLocationSectorBounds FOCLocationSectorPlan::KrushelnytskaCollegePark()
{
    const FOCGeoReferencePoint CollegeRef = FOCGeoReference::College();
    const FOCGeoReferencePoint ParkRef = FOCGeoReference::CentralPark();
    const FVector College = FOCGeoReference::ToLocalCm(CollegeRef.Latitude, CollegeRef.Longitude);
    const FVector Park = FOCGeoReference::ToLocalCm(ParkRef.Latitude, ParkRef.Longitude);

    // Workflow margin around the verified college -> Krushelnytska -> central-park corridor.
    // These margins reserve enough room for frontage, yards and approach streets without pretending
    // that the rectangle is a real administrative/cadastral boundary.
    FOCLocationSectorBounds Result;
    Result.Id = TEXT("S01_Krushelnytska_College_Park");
    Result.Min = FVector2D(FMath::Min(College.X, Park.X) - 12000.0f,
                           FMath::Min(College.Y, Park.Y) - 9000.0f);
    Result.Max = FVector2D(FMath::Max(College.X, Park.X) + 15000.0f,
                           FMath::Max(College.Y, Park.Y) + 16000.0f);
    return Result;
}

bool FOCLocationSectorPlan::IsInsideKrushelnytskaCollegePark(const FVector& Location)
{
    return KrushelnytskaCollegePark().Contains2D(Location);
}
