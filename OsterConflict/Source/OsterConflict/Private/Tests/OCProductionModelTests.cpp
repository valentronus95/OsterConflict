#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    struct FOCProductionStaticMeshExpectation
    {
        const TCHAR* Label;
        const TCHAR* ObjectPath;
        float MinLargestDimensionCm;
    };

    struct FOCProductionSkeletalMeshExpectation
    {
        const TCHAR* Label;
        const TCHAR* ObjectPath;
        float MinLargestDimensionCm;
    };

    bool ValidateProductionStaticMesh(FAutomationTestBase& Test, const FOCProductionStaticMeshExpectation& Expected)
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

    bool ValidateProductionSkeletalMesh(FAutomationTestBase& Test, const FOCProductionSkeletalMeshExpectation& Expected)
    {
        USkeletalMesh* Mesh = LoadObject<USkeletalMesh>(nullptr, Expected.ObjectPath);
        Test.TestNotNull(FString::Printf(TEXT("%s skeletal mesh loads"), Expected.Label), Mesh);
        if (!Mesh) return false;

        const FVector Size = Mesh->GetBounds().BoxExtent * 2.0f;
        const float LargestDimension = FMath::Max3(Size.X, Size.Y, Size.Z);
        const bool bBoundsUsable = Size.X > 1.0f && Size.Y > 1.0f && Size.Z > 1.0f &&
            LargestDimension >= Expected.MinLargestDimensionCm;
        Test.TestTrue(FString::Printf(TEXT("%s has usable skeletal bounds"), Expected.Label), bBoundsUsable);

        const bool bHasSkeleton = Mesh->GetSkeleton() != nullptr;
        Test.TestTrue(FString::Printf(TEXT("%s has a skeleton"), Expected.Label), bHasSkeleton);

        const bool bHasMaterials = Mesh->GetMaterials().Num() > 0;
        Test.TestTrue(FString::Printf(TEXT("%s has material slots"), Expected.Label), bHasMaterials);

        const bool bHasRenderLODs = Mesh->GetLODNum() > 0;
        Test.TestTrue(FString::Printf(TEXT("%s has at least one render LOD"), Expected.Label), bHasRenderLODs);

        return bBoundsUsable && bHasSkeleton && bHasMaterials && bHasRenderLODs;
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

    const FOCProductionStaticMeshExpectation StaticExpectations[] =
    {
        { TEXT("Ukrainian HMMWV"), TEXT("/Game/Production/Vehicles/HMMWV/SM_HMMWV_UA.SM_HMMWV_UA"), 100.0f },
        { TEXT("M2 Browning"), TEXT("/Game/Production/Weapons/M2/SM_M2_Browning.SM_M2_Browning"), 40.0f },
        { TEXT("BTR-4 Bucephalus"), TEXT("/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus.SM_BTR4_Bucephalus"), 150.0f },
        { TEXT("Armed pickup visual"), TEXT("/Game/VehicleVarietyPack/Meshes/SM_Pickup.SM_Pickup"), 100.0f },
        { TEXT("Remington 870"), TEXT("/Game/Production/Weapons/Remington870/SM_Remington870.SM_Remington870"), 40.0f },
        { TEXT("M249"), TEXT("/Game/Production/Weapons/M249/SM_M249.SM_M249"), 40.0f },
    };

    const FOCProductionSkeletalMeshExpectation SkeletalExpectations[] =
    {
        { TEXT("AK-47"), TEXT("/Game/AK-47/Mesh/SKM_AK-47.SKM_AK-47"), 40.0f },
        { TEXT("MP5"), TEXT("/Game/R13/Weapons/Stein/MP5/SKM_MP5.SKM_MP5"), 25.0f },
        { TEXT("M1911"), TEXT("/Game/R13/Weapons/Stein/1911/SKM_1911.SKM_1911"), 10.0f },
        { TEXT("M700"), TEXT("/Game/R13/Weapons/Stein/M700/SKM_M700.SKM_M700"), 50.0f },
        { TEXT("M14"), TEXT("/Game/R13/Weapons/Stein/M14/SKM_M14.SKM_M14"), 50.0f },
        { TEXT("MAC-10"), TEXT("/Game/R13/Weapons/Stein/Mac10/SKM_Mac10.SKM_Mac10"), 10.0f },
        { TEXT("TEC-9"), TEXT("/Game/R13/Weapons/Stein/Tec9/SKM_Tec9.SKM_Tec9"), 10.0f },
        { TEXT("Lever Action"), TEXT("/Game/R13/Weapons/Stein/LeverAction/SKM_LeverAction.SKM_LeverAction"), 40.0f },
        { TEXT("QuantumCharacter body"), TEXT("/Game/QuantumCharacter/Mesh/SKM_QuantumCharacter.SKM_QuantumCharacter"), 100.0f },
        { TEXT("QuantumCharacter first-person arms"), TEXT("/Game/QuantumCharacter/Mesh/Modules/SKM_Arms.SKM_Arms"), 20.0f },
    };

    bool bAllPass = true;
    FString SentinelBody;

    for (const FOCProductionStaticMeshExpectation& Expected : StaticExpectations)
    {
        const bool bPass = ValidateProductionStaticMesh(*this, Expected);
        bAllPass = bAllPass && bPass;
        SentinelBody += FString::Printf(TEXT("%s=%s\n"), Expected.Label, bPass ? TEXT("PASS") : TEXT("FAIL"));
    }

    for (const FOCProductionSkeletalMeshExpectation& Expected : SkeletalExpectations)
    {
        const bool bPass = ValidateProductionSkeletalMesh(*this, Expected);
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
