#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "OCTacticalMapProjection.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOCTacticalMapProjectionTest,
    "OsterConflict.UI.TacticalMap.Projection",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FOCTacticalMapProjectionTest::RunTest(const FString& Parameters)
{
    FOCTacticalMapProjection Projection;
    Projection.WorldMin = FVector2D(-1000.0f, -2000.0f);
    Projection.WorldMax = FVector2D(3000.0f, 6000.0f);
    Projection.bInvertX = false;
    Projection.bInvertY = true;

    TestTrue(TEXT("Projection bounds are valid"), Projection.IsValid());

    const FVector2D SouthWest = Projection.WorldToUV(FVector(-1000.0f, -2000.0f, 0.0f));
    TestTrue(TEXT("West edge maps to U=0"), FMath::IsNearlyEqual(SouthWest.X, 0.0f));
    TestTrue(TEXT("South edge maps to bottom V=1"), FMath::IsNearlyEqual(SouthWest.Y, 1.0f));

    const FVector2D NorthEast = Projection.WorldToUV(FVector(3000.0f, 6000.0f, 0.0f));
    TestTrue(TEXT("East edge maps to U=1"), FMath::IsNearlyEqual(NorthEast.X, 1.0f));
    TestTrue(TEXT("North edge maps to top V=0"), FMath::IsNearlyEqual(NorthEast.Y, 0.0f));

    const FVector2D Center = Projection.WorldToUV(FVector(1000.0f, 2000.0f, 0.0f));
    TestTrue(TEXT("World center maps to U=0.5"), FMath::IsNearlyEqual(Center.X, 0.5f));
    TestTrue(TEXT("World center maps to V=0.5"), FMath::IsNearlyEqual(Center.Y, 0.5f));

    const FVector RoundTrip = Projection.UVToWorld(Center, 125.0f);
    TestTrue(TEXT("Round-trip X is stable"), FMath::IsNearlyEqual(RoundTrip.X, 1000.0f));
    TestTrue(TEXT("Round-trip Y is stable"), FMath::IsNearlyEqual(RoundTrip.Y, 2000.0f));
    TestTrue(TEXT("Round-trip keeps requested Z"), FMath::IsNearlyEqual(RoundTrip.Z, 125.0f));

    const FVector2D Clamped = Projection.WorldToUV(FVector(999999.0f, -999999.0f, 0.0f));
    TestTrue(TEXT("Out-of-bounds X clamps to 1"), FMath::IsNearlyEqual(Clamped.X, 1.0f));
    TestTrue(TEXT("Out-of-bounds Y clamps to 1"), FMath::IsNearlyEqual(Clamped.Y, 1.0f));

    TestTrue(TEXT("North yaw keeps marker pointing up"),
        FMath::IsNearlyEqual(Projection.WorldYawToMapDegrees(90.0f), 0.0f));
    TestTrue(TEXT("East yaw points marker right"),
        FMath::IsNearlyEqual(Projection.WorldYawToMapDegrees(0.0f), 90.0f));

    FOCTacticalMapProjection Invalid;
    Invalid.WorldMin = FVector2D(1.0f, 1.0f);
    Invalid.WorldMax = FVector2D(1.0f, 1.0f);
    TestFalse(TEXT("Zero-size bounds are rejected"), Invalid.IsValid());
    const FVector2D SafeFallback = Invalid.WorldToUV(FVector::ZeroVector);
    TestTrue(TEXT("Invalid projection fails safely at center"), SafeFallback.Equals(FVector2D(0.5f, 0.5f)));

    return true;
}

#endif
