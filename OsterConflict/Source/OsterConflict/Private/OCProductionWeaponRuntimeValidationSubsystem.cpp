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
    const FName RealFallbackWeaponVisualTag(TEXT("OC_RealFallbackWeaponVisual"));

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

    bool IsPlaceholderTexture(const UTexture* Texture)
    {
        if (!Texture) return true;
        const FString Path = Texture->GetPathName();
        const FString Name = Texture->GetName();
        return Path.Contains(TEXT("DefaultTexture"), ESearchCase::IgnoreCase) ||
            Path.Contains(TEXT("WhiteSquareTexture"), ESearchCase::IgnoreCase) ||
            Name.Equals(TEXT("DefaultTexture"), ESearchCase::IgnoreCase) ||
            Name.Equals(TEXT("WhiteSquareTexture"), ESearchCase::IgnoreCase);
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

    bool AppendMaterialDependencyLine(
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

        bool bTextureDependenciesReady = AuthoredMaterial != nullptr && !UsedTextures.IsEmpty();
        for (const UTexture* Texture : UsedTextures)
        {
            if (IsPlaceholderTexture(Texture))
            {
                bTextureDependenciesReady = false;
                break;
            }
        }

        OutReport += FString::Printf(
            TEXT("  slot=%d | authoredMaterial=%s | runtimeMaterial=%s | placeholder=%d | textureCount=%d | textureDependency=%s | textures=%s\n"),
            Slot,
            AuthoredMaterial ? *AuthoredMaterial->GetPathName() : TEXT("<missing>"),
            RuntimeMaterial ? *RuntimeMaterial->GetPathName() : TEXT("<missing>"),
            IsPlaceholderMaterial(AuthoredMaterial) ? 1 : 0,
            UsedTextures.Num(),
            bTextureDependenciesReady ? TEXT("PASS") : TEXT("GAP"),
            *JoinTexturePaths(UsedTextures));
        return bTextureDependenciesReady;
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

    UStaticMeshComponent* FindRealFallbackStaticMeshComponent(AOCWeaponBase& Weapon)
    {
        TInlineComponentArray<UStaticMeshComponent*> Components;
        Weapon.GetComponents(Components);
        for (UStaticMeshComponent* Component : Components)
        {
            if (!Component || !Component->ComponentHasTag(RealFallbackWeaponVisualTag)) continue;
            if (Component->GetStaticMesh() && Component->IsVisible()) return Component;
        }
        return nullptr;
    }

    bool ValidateStaticMeshMaterials(
        UStaticMesh& Mesh,
        UStaticMeshComponent& Component,
        int32& OutMaterialSlots,
        int32& OutPlaceholderOrMissingSlots,
        int32& OutUnexpectedOverrides,
        int32& OutTextureDependencyGaps,
        FString& OutDependencyReport)
    {
        OutMaterialSlots = Mesh.GetStaticMaterials().Num();
        OutPlaceholderOrMissingSlots = 0;
        OutUnexpectedOverrides = 0;
        OutTextureDependencyGaps = 0;

        if (OutMaterialSlots <= 0)
        {
            OutPlaceholderOrMissingSlots = 1;
            OutTextureDependencyGaps = 1;
            OutDependencyReport += TEXT("  slot=<none> | authoredMaterial=<missing> | runtimeMaterial=<missing> | placeholder=1 | textureCount=0 | textureDependency=GAP | textures=none\n");
            return false;
        }

        for (int32 Slot = 0; Slot < OutMaterialSlots; ++Slot)
        {
            UMaterialInterface* AuthoredMaterial = Mesh.GetMaterial(Slot);
            UMaterialInterface* RuntimeMaterial = Component.GetMaterial(Slot);
            if (IsPlaceholderMaterial(AuthoredMaterial)) ++OutPlaceholderOrMissingSlots;
            if (RuntimeMaterial != AuthoredMaterial) ++OutUnexpectedOverrides;
            if (!AppendMaterialDependencyLine(OutDependencyReport, Slot, AuthoredMaterial, RuntimeMaterial))
            {
                ++OutTextureDependencyGaps;
            }
        }

        return OutPlaceholderOrMissingSlots == 0 && OutUnexpectedOverrides == 0 && OutTextureDependencyGaps == 0;
    }

    bool ValidateSkeletalMeshMaterials(
        USkeletalMesh& Mesh,
        USkeletalMeshComponent& Component,
        int32& OutMaterialSlots,
        int32& OutPlaceholderOrMissingSlots,
        int32& OutUnexpectedOverrides,
        int32& OutTextureDependencyGaps,
        FString& OutDependencyReport)
    {
        const TArray<FSkeletalMaterial>& Materials = Mesh.GetMaterials();
        OutMaterialSlots = Materials.Num();
        OutPlaceholderOrMissingSlots = 0;
        OutUnexpectedOverrides = 0;
        OutTextureDependencyGaps = 0;

        if (OutMaterialSlots <= 0)
        {
            OutPlaceholderOrMissingSlots = 1;
            OutTextureDependencyGaps = 1;
            OutDependencyReport += TEXT("  slot=<none> | authoredMaterial=<missing> | runtimeMaterial=<missing> | placeholder=1 | textureCount=0 | textureDependency=GAP | textures=none\n");
            return false;
        }

        for (int32 Slot = 0; Slot < OutMaterialSlots; ++Slot)
        {
            UMaterialInterface* AuthoredMaterial = Materials[Slot].MaterialInterface;
            UMaterialInterface* RuntimeMaterial = Component.GetMaterial(Slot);
            if (IsPlaceholderMaterial(AuthoredMaterial)) ++OutPlaceholderOrMissingSlots;
            if (RuntimeMaterial != AuthoredMaterial) ++OutUnexpectedOverrides;
            if (!AppendMaterialDependencyLine(OutDependencyReport, Slot, AuthoredMaterial, RuntimeMaterial))
            {
                ++OutTextureDependencyGaps;
            }
        }

        return OutPlaceholderOrMissingSlots == 0 && OutUnexpectedOverrides == 0 && OutTextureDependencyGaps == 0;
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

    // Pass45 Gate F validates every required weapon visual that is actually available. Exact production payload
    // gaps remain explicit CONTENT GAP and may use a real authored fallback; they are never relabelled production-ready.
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
    Report += TEXT("OSTER CONFLICT PASS45 REQUIRED AVAILABLE WEAPON RUNTIME VALIDATION\n");
    Report += TEXT("PASS45 dependency contract: weapon class -> exact mesh OR explicit real fallback -> material slot -> material asset -> used texture dependencies -> runtime material\n");
    Report += FString::Printf(TEXT("Map=%s\n\n"), *World.GetMapName());

    bool bAllRequiredAvailablePass = true;
    int32 PassedRequiredAvailableVisuals = 0;
    int32 ExactProductionPass = 0;
    int32 RealFallbackPass = 0;
    int32 ExactContentGaps = 0;
    int32 TotalMaterialSlots = 0;
    int32 TotalMaterialGaps = 0;
    int32 TotalUnexpectedOverrides = 0;
    int32 TotalTextureDependencyGaps = 0;
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
        bool bExactAssetLoads = false;
        bool bUsesExpectedProductionVisual = false;
        bool bUsesRealFallbackVisual = false;
        bool bFallbackHidden = false;
        bool bAuthoredMaterialsReady = false;
        bool bExactContentGap = false;
        int32 MaterialSlots = 0;
        int32 PlaceholderOrMissingSlots = 0;
        int32 UnexpectedOverrides = 0;
        int32 TextureDependencyGaps = 0;
        FString ActualAssetName = TEXT("none");
        FString MaterialDependencyReport;
        FString VisualMode = TEXT("UNRESOLVED");

        if (Weapon)
        {
            TemporaryWeapons.Add(Weapon);
            bIdMatches = Weapon->GetWeaponId() == Expected.WeaponId;

            if (Expected.MeshKind == EExpectedWeaponMeshKind::Static)
            {
                UStaticMesh* ExpectedMesh = LoadObject<UStaticMesh>(nullptr, Expected.ObjectPath);
                bExactAssetLoads = ExpectedMesh != nullptr;
                if (ExpectedMesh)
                {
                    UStaticMeshComponent* ProductionComponent = FindExpectedStaticMeshComponent(*Weapon, ExpectedMesh);
                    bUsesExpectedProductionVisual = ProductionComponent != nullptr;
                    bFallbackHidden = !HasVisibleFallbackStaticMesh(*Weapon);
                    ActualAssetName = ExpectedMesh->GetPathName();
                    VisualMode = TEXT("EXACT_PRODUCTION");
                    if (ProductionComponent)
                    {
                        bAuthoredMaterialsReady = ValidateStaticMeshMaterials(
                            *ExpectedMesh,
                            *ProductionComponent,
                            MaterialSlots,
                            PlaceholderOrMissingSlots,
                            UnexpectedOverrides,
                            TextureDependencyGaps,
                            MaterialDependencyReport);
                    }
                }
            }
            else if (Expected.MeshKind == EExpectedWeaponMeshKind::Skeletal)
            {
                USkeletalMesh* ExpectedMesh = LoadObject<USkeletalMesh>(nullptr, Expected.ObjectPath);
                bExactAssetLoads = ExpectedMesh != nullptr;
                if (ExpectedMesh)
                {
                    USkeletalMeshComponent* ProductionComponent = FindExpectedSkeletalMeshComponent(*Weapon, ExpectedMesh);
                    bUsesExpectedProductionVisual = ProductionComponent != nullptr;
                    bFallbackHidden = !HasVisibleFallbackStaticMesh(*Weapon);
                    ActualAssetName = ExpectedMesh->GetPathName();
                    VisualMode = TEXT("EXACT_PRODUCTION");
                    if (ProductionComponent)
                    {
                        bAuthoredMaterialsReady = ValidateSkeletalMeshMaterials(
                            *ExpectedMesh,
                            *ProductionComponent,
                            MaterialSlots,
                            PlaceholderOrMissingSlots,
                            UnexpectedOverrides,
                            TextureDependencyGaps,
                            MaterialDependencyReport);
                    }
                }
            }

            if (!bExactAssetLoads)
            {
                bExactContentGap = true;
                ++ExactContentGaps;
                VisualMode = TEXT("REAL_FALLBACK_CONTENT_GAP");

                if (UStaticMeshComponent* FallbackComponent = FindRealFallbackStaticMeshComponent(*Weapon))
                {
                    if (UStaticMesh* FallbackMesh = FallbackComponent->GetStaticMesh())
                    {
                        bUsesRealFallbackVisual = true;
                        ActualAssetName = FallbackMesh->GetPathName();
                        bAuthoredMaterialsReady = ValidateStaticMeshMaterials(
                            *FallbackMesh,
                            *FallbackComponent,
                            MaterialSlots,
                            PlaceholderOrMissingSlots,
                            UnexpectedOverrides,
                            TextureDependencyGaps,
                            MaterialDependencyReport);
                    }
                }

                UE_LOG(LogTemp, Warning,
                    TEXT("PASS45_EXACT_WEAPON_CONTENT_GAP label=%s expected=%s fallbackVisual=%d fallbackMaterialsReady=%d exactProductionReadyNotClaimed=1"),
                    Expected.Label,
                    Expected.ObjectPath,
                    bUsesRealFallbackVisual ? 1 : 0,
                    bAuthoredMaterialsReady ? 1 : 0);
            }
        }

        if (MaterialDependencyReport.IsEmpty())
        {
            MaterialDependencyReport = TEXT("  material dependency chain unavailable because neither exact production nor explicit real fallback visual resolved\n");
        }

        TotalMaterialSlots += MaterialSlots;
        TotalMaterialGaps += PlaceholderOrMissingSlots;
        TotalUnexpectedOverrides += UnexpectedOverrides;
        TotalTextureDependencyGaps += TextureDependencyGaps;

        const bool bExactPass = bSpawned && bIdMatches && bExactAssetLoads &&
            bUsesExpectedProductionVisual && bFallbackHidden && bAuthoredMaterialsReady;
        const bool bFallbackGapPass = bSpawned && bIdMatches && bExactContentGap &&
            bUsesRealFallbackVisual && bAuthoredMaterialsReady;
        const bool bPass = bExactPass || bFallbackGapPass;

        bAllRequiredAvailablePass = bAllRequiredAvailablePass && bPass;
        if (bPass)
        {
            ++PassedRequiredAvailableVisuals;
            if (bExactPass) ++ExactProductionPass;
            if (bFallbackGapPass) ++RealFallbackPass;
        }

        const TCHAR* ResultText = bExactPass
            ? TEXT("PASS")
            : (bFallbackGapPass ? TEXT("CONTENT_GAP_FALLBACK_PASS") : TEXT("FAIL"));

        Report += FString::Printf(
            TEXT("%s | id=%s | spawned=%s | idMatch=%s | exactAsset=%s | visualMode=%s | productionVisual=%s | realFallbackVisual=%s | fallbackHidden=%s | authoredMaterials=%s | materialSlots=%d | materialGaps=%d | textureGaps=%d | unexpectedOverrides=%d | asset=%s | RESULT=%s\n"),
            Expected.Label,
            *Expected.WeaponId.ToString(),
            bSpawned ? TEXT("PASS") : TEXT("FAIL"),
            bIdMatches ? TEXT("PASS") : TEXT("FAIL"),
            bExactAssetLoads ? TEXT("PASS") : TEXT("CONTENT_GAP"),
            *VisualMode,
            bUsesExpectedProductionVisual ? TEXT("PASS") : TEXT("NO"),
            bUsesRealFallbackVisual ? TEXT("PASS") : TEXT("NO"),
            bFallbackHidden ? TEXT("PASS") : TEXT("N/A"),
            bAuthoredMaterialsReady ? TEXT("PASS") : TEXT("FAIL"),
            MaterialSlots,
            PlaceholderOrMissingSlots,
            TextureDependencyGaps,
            UnexpectedOverrides,
            *ActualAssetName,
            ResultText);
        Report += MaterialDependencyReport;
        Report += TEXT("\n");
    }

    for (const TWeakObjectPtr<AOCWeaponBase>& TemporaryWeapon : TemporaryWeapons)
    {
        if (AOCWeaponBase* Weapon = TemporaryWeapon.Get()) Weapon->Destroy();
    }

    Report += FString::Printf(
        TEXT("SUMMARY=%d/%d required available weapon visuals PASS | exactProduction=%d | realFallback=%d | exactContentGaps=%d | materialSlots=%d | materialGaps=%d | textureGaps=%d | unexpectedOverrides=%d\n"),
        PassedRequiredAvailableVisuals,
        UE_ARRAY_COUNT(Expectations),
        ExactProductionPass,
        RealFallbackPass,
        ExactContentGaps,
        TotalMaterialSlots,
        TotalMaterialGaps,
        TotalTextureDependencyGaps,
        TotalUnexpectedOverrides);

    if (!FFileHelper::SaveStringToFile(Report, *ReportPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
    {
        UE_LOG(LogTemp, Warning, TEXT("Pass45 required-available weapon validation could not write report: %s"), *ReportPath);
        bAllRequiredAvailablePass = false;
    }

    if (bAllRequiredAvailablePass)
    {
        const FString Sentinel = FString::Printf(
            TEXT("PASS45_REQUIRED_AVAILABLE_WEAPONS=PASS\nPASS45_AUTHORED_WEAPON_MATERIALS=PASS\nPASS45_WEAPON_DEPENDENCY_REPORT=PASS\nPASS45_EXACT_PRODUCTION_CONTENT_GAPS=%d\n"),
            ExactContentGaps);
        FFileHelper::SaveStringToFile(
            Sentinel,
            *SuccessSentinelPath,
            FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_REQUIRED_AVAILABLE_WEAPON_VISUALS_VALIDATED_READY classes=%d/%d exactProduction=%d realFallback=%d exactContentGaps=%d material_slots=%d material_gaps=%d texture_gaps=%d authored_materials=1 dependency_report=1 unexpected_overrides=0 validation_only=1 mutation=0"),
            PassedRequiredAvailableVisuals,
            UE_ARRAY_COUNT(Expectations),
            ExactProductionPass,
            RealFallbackPass,
            ExactContentGaps,
            TotalMaterialSlots,
            TotalMaterialGaps,
            TotalTextureDependencyGaps);
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_REQUIRED_AVAILABLE_WEAPON_RUNTIME_FAIL classes=%d/%d exactProduction=%d realFallback=%d exactContentGaps=%d material_slots=%d material_gaps=%d texture_gaps=%d unexpected_overrides=%d dependency_report=1 validation_only=1 mutation=0 report=%s"),
            PassedRequiredAvailableVisuals,
            UE_ARRAY_COUNT(Expectations),
            ExactProductionPass,
            RealFallbackPass,
            ExactContentGaps,
            TotalMaterialSlots,
            TotalMaterialGaps,
            TotalTextureDependencyGaps,
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
