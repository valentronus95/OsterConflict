#include "OCProductionWeaponRuntimeValidationSubsystem.h"

#include "OCAntiArmorLauncher.h"
#include "OCGameMode.h"
#include "OCWeaponBase.h"
#include "OCWeaponVariants.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformMisc.h"
#include "Materials/MaterialInterface.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    constexpr float ValidationDelaySeconds = 2.0f;
    constexpr float ValidationDepthCm = -1000000.0f;
    const FName ProductionWeaponVisualTag(TEXT("OC_ProductionWeaponVisual"));

    enum class EExpectedWeaponMeshKind : uint8
    {
        Static,
        Skeletal
    };

    struct FExpectedWeaponVisual
    {
        const TCHAR* Label;
        FName WeaponId;
        UClass* WeaponClass;
        const TCHAR* ObjectPath;
        EExpectedWeaponMeshKind MeshKind;
    };

    bool IsPlaceholderMaterial(const UMaterialInterface* Material)
    {
        if (!Material) return true;
        const FString Path = Material->GetPathName();
        return Path.Contains(TEXT("BasicShapeMaterial"), ESearchCase::IgnoreCase) ||
            Path.Contains(TEXT("DefaultMaterial"), ESearchCase::IgnoreCase) ||
            Path.Contains(TEXT("WorldGridMaterial"), ESearchCase::IgnoreCase) ||
            Path.Contains(TEXT("_defaultMat"), ESearchCase::IgnoreCase);
    }

    FString JoinTexturePaths(const TArray<UTexture*>& Textures)
    {
        if (Textures.IsEmpty()) return TEXT("none");

        FString Result;
        for (int32 Index = 0; Index < Textures.Num(); ++Index)
        {
            if (Index > 0) Result += TEXT(",");
            const UTexture* Texture = Textures[Index];
            Result += Texture ? Texture->GetPathName() : TEXT("<null>");
        }
        return Result;
    }

    void AppendMaterialDependencyLine(
        FString& OutReport,
        int32 Slot,
        UMaterialInterface* AuthoredMaterial,
        UMaterialInterface* RuntimeMaterial)
    {
        TArray<UTexture*> UsedTextures;
        if (AuthoredMaterial)
        {
            // UE 5.8 UMaterialInterface::GetUsedTextures reports textures used to render the material.
            AuthoredMaterial->GetUsedTextures(
                UsedTextures,
                EMaterialQualityLevel::High,
                true,
                ERHIFeatureLevel::SM5,
                true);
        }

        OutReport += FString::Printf(
            TEXT("  slot=%d | authoredMaterial=%s | runtimeMaterial=%s | placeholder=%d | textureCount=%d | textures=%s\n"),
            Slot,
            AuthoredMaterial ? *AuthoredMaterial->GetPathName() : TEXT("<missing>"),
            RuntimeMaterial ? *RuntimeMaterial->GetPathName() : TEXT("<missing>"),
            IsPlaceholderMaterial(AuthoredMaterial) ? 1 : 0,
            UsedTextures.Num(),
            *JoinTexturePaths(UsedTextures));
    }

    bool HasVisibleFallbackStaticMesh(AOCWeaponBase& Weapon)
    {
        TInlineComponentArray<UStaticMeshComponent*> Components;
        Weapon.GetComponents(Components);
        for (const UStaticMeshComponent* Component : Components)
        {
            if (!Component || Component->ComponentHasTag(ProductionWeaponVisualTag)) continue;
            if (Component->GetStaticMesh() && Component->IsVisible()) return true;
        }
        return false;
    }

    UStaticMeshComponent* FindExpectedStaticMeshComponent(AOCWeaponBase& Weapon, UStaticMesh* ExpectedMesh)
    {
        if (!ExpectedMesh) return nullptr;
        TInlineComponentArray<UStaticMeshComponent*> Components;
        Weapon.GetComponents(Components);
        for (UStaticMeshComponent* Component : Components)
        {
            if (!Component || !Component->ComponentHasTag(ProductionWeaponVisualTag)) continue;
            if (Component->GetStaticMesh() == ExpectedMesh && Component->IsVisible()) return Component;
        }
        return nullptr;
    }

    USkeletalMeshComponent* FindExpectedSkeletalMeshComponent(AOCWeaponBase& Weapon, USkeletalMesh* ExpectedMesh)
    {
        if (!ExpectedMesh) return nullptr;
        TInlineComponentArray<USkeletalMeshComponent*> Components;
        Weapon.GetComponents(Components);
        for (USkeletalMeshComponent* Component : Components)
        {
            if (!Component || !Component->ComponentHasTag(ProductionWeaponVisualTag)) continue;
            if (Component->GetSkeletalMeshAsset() == ExpectedMesh && Component->IsVisible()) return Component;
        }
        return nullptr;
    }

    bool ValidateStaticMeshMaterials(
        UStaticMesh& Mesh,
        UStaticMeshComponent& Component,
        int32& OutMaterialSlots,
        int32& OutPlaceholderOrMissingSlots,
        int32& OutUnexpectedOverrides,
        FString& OutDependencyReport)
    {
        OutMaterialSlots = Mesh.GetStaticMaterials().Num();
        OutPlaceholderOrMissingSlots = 0;
        OutUnexpectedOverrides = 0;

        if (OutMaterialSlots <= 0)
        {
            OutPlaceholderOrMissingSlots = 1;
            OutDependencyReport += TEXT("  slot=<none> | authoredMaterial=<missing> | runtimeMaterial=<missing> | placeholder=1 | textureCount=0 | textures=none\n");
            return false;
        }

        for (int32 Slot = 0; Slot < OutMaterialSlots; ++Slot)
        {
            UMaterialInterface* AuthoredMaterial = Mesh.GetMaterial(Slot);
            UMaterialInterface* RuntimeMaterial = Component.GetMaterial(Slot);
            if (IsPlaceholderMaterial(AuthoredMaterial)) ++OutPlaceholderOrMissingSlots;
            if (RuntimeMaterial != AuthoredMaterial) ++OutUnexpectedOverrides;
            AppendMaterialDependencyLine(OutDependencyReport, Slot, AuthoredMaterial, RuntimeMaterial);
        }

        return OutPlaceholderOrMissingSlots == 0 && OutUnexpectedOverrides == 0;
    }

    bool ValidateSkeletalMeshMaterials(
        USkeletalMesh& Mesh,
        USkeletalMeshComponent& Component,
        int32& OutMaterialSlots,
        int32& OutPlaceholderOrMissingSlots,
        int32& OutUnexpectedOverrides,
        FString& OutDependencyReport)
    {
        const TArray<FSkeletalMaterial>& Materials = Mesh.GetMaterials();
        OutMaterialSlots = Materials.Num();
        OutPlaceholderOrMissingSlots = 0;
        OutUnexpectedOverrides = 0;

        if (OutMaterialSlots <= 0)
        {
            OutPlaceholderOrMissingSlots = 1;
            OutDependencyReport += TEXT("  slot=<none> | authoredMaterial=<missing> | runtimeMaterial=<missing> | placeholder=1 | textureCount=0 | textures=none\n");
            return false;
        }

        for (int32 Slot = 0; Slot < OutMaterialSlots; ++Slot)
        {
            UMaterialInterface* AuthoredMaterial = Materials[Slot].MaterialInterface;
            UMaterialInterface* RuntimeMaterial = Component.GetMaterial(Slot);
            if (IsPlaceholderMaterial(AuthoredMaterial)) ++OutPlaceholderOrMissingSlots;
            if (RuntimeMaterial != AuthoredMaterial) ++OutUnexpectedOverrides;
            AppendMaterialDependencyLine(OutDependencyReport, Slot, AuthoredMaterial, RuntimeMaterial);
        }

        return OutPlaceholderOrMissingSlots == 0 && OutUnexpectedOverrides == 0;
    }
}

bool UOCProductionWeaponRuntimeValidationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCProductionWeaponRuntimeValidationSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    if (InWorld.GetNetMode() == NM_DedicatedServer) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (!FParse::Param(FCommandLine::Get(), TEXT("ValidateProductionWeapons"))) return;

    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(
        Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) ValidateProductionWeapons(*World);
        }),
        ValidationDelaySeconds,
        false);
}

void UOCProductionWeaponRuntimeValidationSubsystem::ValidateProductionWeapons(UWorld& World)
{
    const bool bHeadlessGate = FParse::Param(FCommandLine::Get(), TEXT("ValidateProductionWeaponsHeadless"));

    // Local UE 5.8 runtime inspection is authoritative. Mesh presence alone is not material readiness:
    // every expected production visual must retain authored, non-placeholder material slots at runtime,
    // and the report must expose the exact slot -> material -> used-texture dependency chain.
    const FExpectedWeaponVisual Expectations[] =
    {
        { TEXT("AK-47"), FName(TEXT("OC_AR1")), AOCWeapon_AssaultRifle::StaticClass(),
            TEXT("/Game/AK-47/Mesh/SKM_AK-47.SKM_AK-47"), EExpectedWeaponMeshKind::Skeletal },
        { TEXT("MP5"), FName(TEXT("OC_SMG1")), AOCWeapon_SMG::StaticClass(),
            TEXT("/Game/R13/Weapons/Stein/MP5/SKM_MP5.SKM_MP5"), EExpectedWeaponMeshKind::Static },
        { TEXT("M1911"), FName(TEXT("OC_PST1")), AOCWeapon_Pistol::StaticClass(),
            TEXT("/Game/R13/Weapons/Stein/1911/SKM_1911.SKM_1911"), EExpectedWeaponMeshKind::Static },
        { TEXT("M700"), FName(TEXT("OC_SNP1")), AOCWeapon_Sniper::StaticClass(),
            TEXT("/Game/R13/Weapons/Stein/M700/SKM_M700.SKM_M700"), EExpectedWeaponMeshKind::Static },
        { TEXT("Remington 870"), FName(TEXT("OC_SG1")), AOCWeapon_Shotgun::StaticClass(),
            TEXT("/Game/Production/Weapons/Remington870/SM_Remington870.SM_Remington870"), EExpectedWeaponMeshKind::Static },
        { TEXT("M249"), FName(TEXT("OC_LMG1")), AOCWeapon_LMG::StaticClass(),
            TEXT("/Game/Production/Weapons/M249/SM_M249.SM_M249"), EExpectedWeaponMeshKind::Static },
        { TEXT("M14"), FName(TEXT("R13_M14")), AOCWeapon_M14::StaticClass(),
            TEXT("/Game/R13/Weapons/Stein/M14/SKM_M14.SKM_M14"), EExpectedWeaponMeshKind::Static },
        { TEXT("MAC-10"), FName(TEXT("R13_MAC10")), AOCWeapon_Mac10::StaticClass(),
            TEXT("/Game/R13/Weapons/Stein/Mac10/SKM_Mac10.SKM_Mac10"), EExpectedWeaponMeshKind::Static },
        { TEXT("TEC-9"), FName(TEXT("R13_TEC9")), AOCWeapon_Tec9::StaticClass(),
            TEXT("/Game/R13/Weapons/Stein/Tec9/SKM_Tec9.SKM_Tec9"), EExpectedWeaponMeshKind::Static },
        { TEXT("Lever Action .45-70"), FName(TEXT("R13_LEVER4570")), AOCWeapon_LeverAction::StaticClass(),
            TEXT("/Game/R13/Weapons/Stein/LeverAction/SKM_LeverAction.SKM_LeverAction"), EExpectedWeaponMeshKind::Static },
        { TEXT("Anti-Armor Launcher"), FName(TEXT("OC_RPG1")), AOCAntiArmorLauncher::StaticClass(),
            TEXT("/Game/R13/Weapons/rocketlauncherModern.rocketlauncherModern"), EExpectedWeaponMeshKind::Static },
    };

    const FString ReportDir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("AutomationReports"), TEXT("ProductionModels"));
    const FString ReportPath = FPaths::Combine(ReportDir, TEXT("weapon_runtime_validation.txt"));
    const FString SuccessSentinelPath = FPaths::Combine(ReportDir, TEXT("production_weapon_runtime_success.txt"));
    IFileManager::Get().MakeDirectory(*ReportDir, true);
    IFileManager::Get().Delete(*SuccessSentinelPath, false, true, true);

    FString Report;
    Report += TEXT("OSTER CONFLICT R14 PRODUCTION WEAPON RUNTIME VALIDATION\n");
    Report += TEXT("PASS45 dependency contract: weapon class -> exact mesh -> material slot -> material asset -> used texture dependencies -> runtime material\n");
    Report += FString::Printf(TEXT("Map=%s\n\n"), *World.GetMapName());

    bool bAllPass = true;
    int32 PassedWeapons = 0;
    int32 TotalMaterialSlots = 0;
    int32 TotalMaterialGaps = 0;
    int32 TotalUnexpectedOverrides = 0;
    TArray<TWeakObjectPtr<AOCWeaponBase>> TemporaryWeapons;

    for (int32 Index = 0; Index < UE_ARRAY_COUNT(Expectations); ++Index)
    {
        const FExpectedWeaponVisual& Expected = Expectations[Index];

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        SpawnParams.ObjectFlags |= RF_Transient;

        const FVector SpawnLocation(
            static_cast<float>(Index) * 250.0f,
            0.0f,
            ValidationDepthCm);

        AOCWeaponBase* Weapon = World.SpawnActor<AOCWeaponBase>(
            Expected.WeaponClass,
            SpawnLocation,
            FRotator::ZeroRotator,
            SpawnParams);

        const bool bSpawned = Weapon != nullptr;
        bool bIdMatches = false;
        bool bAssetLoads = false;
        bool bUsesExpectedProductionVisual = false;
        bool bFallbackHidden = false;
        bool bAuthoredMaterialsReady = false;
        int32 MaterialSlots = 0;
        int32 PlaceholderOrMissingSlots = 0;
        int32 UnexpectedOverrides = 0;
        FString ActualAssetName = TEXT("none");
        FString MaterialDependencyReport;

        if (Weapon)
        {
            TemporaryWeapons.Add(Weapon);
            bIdMatches = Weapon->GetWeaponId() == Expected.WeaponId;
            bFallbackHidden = !HasVisibleFallbackStaticMesh(*Weapon);

            if (Expected.MeshKind == EExpectedWeaponMeshKind::Static)
            {
                UStaticMesh* ExpectedMesh = LoadObject<UStaticMesh>(nullptr, Expected.ObjectPath);
                bAssetLoads = ExpectedMesh != nullptr;
                UStaticMeshComponent* ProductionComponent = FindExpectedStaticMeshComponent(*Weapon, ExpectedMesh);
                bUsesExpectedProductionVisual = ProductionComponent != nullptr;
                if (ExpectedMesh)
                {
                    ActualAssetName = ExpectedMesh->GetPathName();
                    if (ProductionComponent)
                    {
                        bAuthoredMaterialsReady = ValidateStaticMeshMaterials(
                            *ExpectedMesh,
                            *ProductionComponent,
                            MaterialSlots,
                            PlaceholderOrMissingSlots,
                            UnexpectedOverrides,
                            MaterialDependencyReport);
                    }
                }
            }
            else if (Expected.MeshKind == EExpectedWeaponMeshKind::Skeletal)
            {
                USkeletalMesh* ExpectedMesh = LoadObject<USkeletalMesh>(nullptr, Expected.ObjectPath);
                bAssetLoads = ExpectedMesh != nullptr;
                USkeletalMeshComponent* ProductionComponent = FindExpectedSkeletalMeshComponent(*Weapon, ExpectedMesh);
                bUsesExpectedProductionVisual = ProductionComponent != nullptr;
                if (ExpectedMesh)
                {
                    ActualAssetName = ExpectedMesh->GetPathName();
                    if (ProductionComponent)
                    {
                        bAuthoredMaterialsReady = ValidateSkeletalMeshMaterials(
                            *ExpectedMesh,
                            *ProductionComponent,
                            MaterialSlots,
                            PlaceholderOrMissingSlots,
                            UnexpectedOverrides,
                            MaterialDependencyReport);
                    }
                }
            }
        }

        if (MaterialDependencyReport.IsEmpty())
        {
            MaterialDependencyReport = TEXT("  material dependency chain unavailable because expected runtime production visual was not resolved\n");
        }

        TotalMaterialSlots += MaterialSlots;
        TotalMaterialGaps += PlaceholderOrMissingSlots;
        TotalUnexpectedOverrides += UnexpectedOverrides;

        const bool bCanonicalAssetDefined = Expected.ObjectPath != nullptr;
        const bool bPass = bSpawned && bIdMatches && bCanonicalAssetDefined && bAssetLoads &&
            bUsesExpectedProductionVisual && bFallbackHidden && bAuthoredMaterialsReady;

        bAllPass = bAllPass && bPass;
        if (bPass) ++PassedWeapons;

        Report += FString::Printf(
            TEXT("%s | id=%s | spawned=%s | idMatch=%s | canonical=%s | assetLoads=%s | productionVisual=%s | fallbackHidden=%s | authoredMaterials=%s | materialSlots=%d | materialGaps=%d | unexpectedOverrides=%d | asset=%s | RESULT=%s\n"),
            Expected.Label,
            *Expected.WeaponId.ToString(),
            bSpawned ? TEXT("PASS") : TEXT("FAIL"),
            bIdMatches ? TEXT("PASS") : TEXT("FAIL"),
            bCanonicalAssetDefined ? TEXT("PASS") : TEXT("MISSING"),
            bAssetLoads ? TEXT("PASS") : TEXT("FAIL"),
            bUsesExpectedProductionVisual ? TEXT("PASS") : TEXT("FAIL"),
            bFallbackHidden ? TEXT("PASS") : TEXT("FAIL"),
            bAuthoredMaterialsReady ? TEXT("PASS") : TEXT("FAIL"),
            MaterialSlots,
            PlaceholderOrMissingSlots,
            UnexpectedOverrides,
            *ActualAssetName,
            bPass ? TEXT("PASS") : TEXT("FAIL"));
        Report += MaterialDependencyReport;
        Report += TEXT("\n");
    }

    for (const TWeakObjectPtr<AOCWeaponBase>& TemporaryWeapon : TemporaryWeapons)
    {
        if (AOCWeaponBase* Weapon = TemporaryWeapon.Get()) Weapon->Destroy();
    }

    Report += FString::Printf(
        TEXT("SUMMARY=%d/%d production weapon classes PASS | materialSlots=%d | materialGaps=%d | unexpectedOverrides=%d\n"),
        PassedWeapons,
        UE_ARRAY_COUNT(Expectations),
        TotalMaterialSlots,
        TotalMaterialGaps,
        TotalUnexpectedOverrides);

    if (!FFileHelper::SaveStringToFile(Report, *ReportPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
    {
        UE_LOG(LogTemp, Warning, TEXT("R14 production weapon validation could not write report: %s"), *ReportPath);
        bAllPass = false;
    }

    if (bAllPass)
    {
        FFileHelper::SaveStringToFile(
            TEXT("R14_PRODUCTION_WEAPONS=PASS\nPASS45_AUTHORED_WEAPON_MATERIALS=PASS\nPASS45_WEAPON_DEPENDENCY_REPORT=PASS\n"),
            *SuccessSentinelPath,
            FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_PRODUCTION_WEAPON_VISUALS_VALIDATED_READY classes=%d/%d material_slots=%d authored_materials=1 dependency_report=1 unexpected_overrides=0 validation_only=1 mutation=0"),
            PassedWeapons,
            UE_ARRAY_COUNT(Expectations),
            TotalMaterialSlots);
    }
    else
    {
        UE_LOG(LogTemp, Warning,
            TEXT("PASS45_PRODUCTION_WEAPON_CONTENT_GAP classes=%d/%d material_slots=%d material_gaps=%d unexpected_overrides=%d exact_material_ready=0 dependency_report=1 validation_only=1 mutation=0 report=%s"),
            PassedWeapons,
            UE_ARRAY_COUNT(Expectations),
            TotalMaterialSlots,
            TotalMaterialGaps,
            TotalUnexpectedOverrides,
            *ReportPath);
    }

    if (bHeadlessGate)
    {
        // The CMD validator decides PASS from the explicit sentinel. Always terminate the headless
        // UE process after the report is flushed so a missing sentinel becomes a deterministic gate.
        FPlatformMisc::RequestExit(false);
    }
}
