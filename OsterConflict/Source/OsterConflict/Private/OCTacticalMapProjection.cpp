#include "OCTacticalMapProjection.h"

bool FOCTacticalMapProjection::IsValid() const
{
    return WorldMax.X > WorldMin.X + KINDA_SMALL_NUMBER &&
        WorldMax.Y > WorldMin.Y + KINDA_SMALL_NUMBER;
}

FVector2D FOCTacticalMapProjection::WorldToUV(const FVector& WorldLocation, const bool bClampToBounds) const
{
    if (!IsValid())
    {
        return FVector2D(0.5f, 0.5f);
    }

    float U = (WorldLocation.X - WorldMin.X) / (WorldMax.X - WorldMin.X);
    float V = (WorldLocation.Y - WorldMin.Y) / (WorldMax.Y - WorldMin.Y);

    if (bInvertX) U = 1.0f - U;
    if (bInvertY) V = 1.0f - V;

    if (bClampToBounds)
    {
        U = FMath::Clamp(U, 0.0f, 1.0f);
        V = FMath::Clamp(V, 0.0f, 1.0f);
    }

    return FVector2D(U, V);
}

FVector FOCTacticalMapProjection::UVToWorld(const FVector2D& UV, const float WorldZ, const bool bClampToBounds) const
{
    if (!IsValid())
    {
        return FVector(0.0f, 0.0f, WorldZ);
    }

    float U = UV.X;
    float V = UV.Y;
    if (bClampToBounds)
    {
        U = FMath::Clamp(U, 0.0f, 1.0f);
        V = FMath::Clamp(V, 0.0f, 1.0f);
    }

    if (bInvertX) U = 1.0f - U;
    if (bInvertY) V = 1.0f - V;

    return FVector(
        FMath::Lerp(WorldMin.X, WorldMax.X, U),
        FMath::Lerp(WorldMin.Y, WorldMax.Y, V),
        WorldZ);
}

float FOCTacticalMapProjection::WorldYawToMapDegrees(const float WorldYawDegrees) const
{
    float Result = 90.0f - WorldYawDegrees;
    if (bInvertX) Result = -Result;
    if (!bInvertY) Result += 180.0f;
    return FMath::UnwindDegrees(Result);
}
