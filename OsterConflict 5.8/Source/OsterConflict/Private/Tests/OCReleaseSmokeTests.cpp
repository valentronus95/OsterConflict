#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "OCBuildVersion.h"
#include "OCCharacterVisualTypes.h"
#include "OCGeoReference.h"
#include "OCGameInstance.h"
#include "OCPlayerUserSettings.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOCReleaseBuildFingerprintTest,
    "OsterConflict.Release.BuildFingerprint",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FOCReleaseBuildFingerprintTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("Milestone"), FString(OCBuildVersion::Milestone), FString(TEXT("S18B")));
    TestEqual(TEXT("Project version"), FString(OCBuildVersion::ProjectVersion), FString(TEXT("0.0.18B-S18B")));
    TestEqual(TEXT("Network protocol baseline"), OCBuildVersion::NetworkProtocol, 18);
    TestTrue(TEXT("Release map must be a /Game path"), FString(OCBuildVersion::ReleaseMap).StartsWith(TEXT("/Game/")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOCReleaseGeoReferenceTest,
    "OsterConflict.Release.GeoReferenceOrigin",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FOCReleaseGeoReferenceTest::RunTest(const FString& Parameters)
{
    const FVector Origin = FOCGeoReference::ToLocalCm(FOCGeoReference::OriginLatitude, FOCGeoReference::OriginLongitude);
    TestTrue(TEXT("Museum origin X near zero"), FMath::Abs(Origin.X) < 0.01);
    TestTrue(TEXT("Museum origin Y near zero"), FMath::Abs(Origin.Y) < 0.01);

    const FOCGeoReferencePoint Museum = FOCGeoReference::Museum();
    const FOCGeoReferencePoint College = FOCGeoReference::College();
    TestEqual(TEXT("Museum confidence"), static_cast<uint8>(Museum.Confidence), static_cast<uint8>(EOCReferenceConfidence::A));
    TestTrue(TEXT("College must not collapse onto museum"),
        FOCGeoReference::ToLocalCm(College.Latitude, College.Longitude).Size2D() > 1000.0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOCReleaseFactionNamesTest,
    "OsterConflict.Release.FactionNames",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FOCReleaseFactionNamesTest::RunTest(const FString& Parameters)
{
    const EOCFactionArchetype Factions[] =
    {
        EOCFactionArchetype::UASpecialUnit,
        EOCFactionArchetype::MaskedFighters,
        EOCFactionArchetype::USRangers,
        EOCFactionArchetype::Insurgents
    };
    for (const EOCFactionArchetype Faction : Factions)
    {
        TestNotEqual(TEXT("Every production faction has a display name"), OCFactionToString(Faction), FString(TEXT("UNKNOWN")));
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOCReleaseHardeningContractTest,
    "OsterConflict.Release.HardeningContract",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FOCReleaseHardeningContractTest::RunTest(const FString& Parameters)
{
    TestNotNull(TEXT("Custom game instance class exists"), UOCGameInstance::StaticClass());
    TestEqual(TEXT("Settings schema baseline"), UOCPlayerUserSettings::CurrentSettingsSchemaVersion, 1);
    TestEqual(TEXT("Protocol remains corrected S18B runtime baseline"), OCBuildVersion::NetworkProtocol, 18);
    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
