#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/StaticMesh.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    struct FOCProductionMeshExpectation
    {
        const TCHAR* Label;
        const TCHAR* ObjectPath;
        float MinLargestDimensionCm;
    };

    bool ValidateProductionStaticMesh(FAutomationTestBase& Test, const FOCProductionMeshExpectation& Expected)
    {
        UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, Expected.ObjectPath);
        Test.TestNotNull(FString::Printf(TEXT("%s production mesh loads"), Expected.Label), Mesh);
        if (!Mesh) return false;

        const FVector Size = Mesh->GetBounds().BoxExtent * 2.0f;
        const float LargestDimension = FMath::Max3(Size.X, Size.Y, Size.Z);
        const bool bBoundsUsable = Size.X > 1.0f && Size.Y > 1.0f && Size.Z > 1.0f &&
            LargestDimension >= Expected.MinLargestDimensionCm;
        Test.TestTrue(FString::Printf(TEXT("%s has usable imported bounds"), Expected.Label), bBoundsUsable);

        const bool bHasMaterials = Mesh->GetStaticMaterials().Num() > 0;
        Test.TestTrue(FString::Printf(TEXT("%s has authored/imported material slots"), Expected.Label), bHasMaterials);

        const bool bHasRenderLODs = Mesh->GetNumLODs() > 0;
        Test.TestTrue(FString::Printf(TEXT("%s has at least one render LOD"), Expected.Label), bHasRenderLODs);

        return bBoundsUsable && bHasMaterials && bHasRenderLODs;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOCProductionCanonicalAssetsTest,
    "OsterConflict.ProductionModels.CanonicalAssets",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FOCProductionCanonicalAssetsTest::RunTest(const FString& Parameters)
{
    const FString SentinelPath = FPaths::Combine(
        FPaths::ProjectSavedDir(), TEXT("ProductionAssetImportCache"), TEXT("production_automation_success.txt"));
    IFileManager::Get().Delete(*SentinelPath, false, true, true);

    const FOCProductionMeshExpectation Expectations[] =
    {
        { TEXT("Ukrainian HMMWV"), TEXT("/Game/Production/Vehicles/HMMWV/SM_HMMWV_UA.SM_HMMWV_UA"), 100.0f },
        { TEXT("M2 Browning"), TEXT("/Game/Production/Weapons/M2/SM_M2_Browning.SM_M2_Browning"), 40.0f },
        { TEXT("BTR-4 Bucephalus"), TEXT("/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus.SM_BTR4_Bucephalus"), 150.0f },
    };

    bool bAllPass = true;
    FString SentinelBody;
    for (const FOCProductionMeshExpectation& Expected : Expectations)
    {
        const bool bPass = ValidateProductionStaticMesh(*this, Expected);
        bAllPass = bAllPass && bPass;
        SentinelBody += FString::Printf(TEXT("%s=%s\n"), Expected.Label, bPass ? TEXT("PASS") : TEXT("FAIL"));
    }

    if (bAllPass)
    {
        const FString ParentDir = FPaths::GetPath(SentinelPath);
        IFileManager::Get().MakeDirectory(*ParentDir, true);
        if (!FFileHelper::SaveStringToFile(SentinelBody, *SentinelPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
        {
            AddError(FString::Printf(TEXT("Could not write production automation success sentinel: %s"), *SentinelPath));
            return false;
        }
        AddInfo(FString::Printf(TEXT("Production model automation sentinel written: %s"), *SentinelPath));
    }

    return bAllPass;
}

#endif // WITH_DEV_AUTOMATION_TESTS
