#include "OCProductionWeaponRuntimeValidationSubsystem.h"

#include "OCAntiArmorLauncher.h"
#include "OCGameMode.h"
#include "OCWeaponBase.h"
#include "OCWeaponVariants.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformMisc.h"
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

    bool UsesExpectedStaticMesh(AOCWeaponBase& Weapon, UStaticMesh* ExpectedMesh)
    {
        if (!ExpectedMesh) return false;
        TInlineComponentArray<UStaticMeshComponent*> Components;
        Weapon.GetComponents(Components);
        for (const UStaticMeshComponent* Component : Components)
        {
            if (!Component || !Component->ComponentHasTag(ProductionWeaponVisualTag)) continue;
            if (Component->GetStaticMesh() == ExpectedMesh && Component->IsVisible()) return true;
        }
        return false;
    }

    bool UsesExpectedSkeletalMesh(AOCWeaponBase& Weapon, USkeletalMesh* ExpectedMesh)
    {
        if (!ExpectedMesh) return false;
        TInlineComponentArray<USkeletalMeshComponent*> Components;
        Weapon.GetComponents(Components);
        for (const USkeletalMeshComponent* Component : Components)
        {
            if (!Component || !Component->ComponentHasTag(ProductionWeaponVisualTag)) continue;
            if (Component->GetSkeletalMeshAsset() == ExpectedMesh && Component->IsVisible()) return true;
        }
        return false;
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

    // Local UE 5.8 runtime inspection is authoritative. The restored Stein objects retain SKM_* names,
    // but the asset class is StaticMesh for these seven weapons. AK-47 remains a real SkeletalMesh.
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
    Report += FString::Printf(TEXT("Map=%s\n\n"), *World.GetMapName());

    bool bAllPass = true;
    int32 PassedWeapons = 0;
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
        FString ActualAssetName = TEXT("none");

        if (Weapon)
        {
            TemporaryWeapons.Add(Weapon);
            bIdMatches = Weapon->GetWeaponId() == Expected.WeaponId;
            bFallbackHidden = !HasVisibleFallbackStaticMesh(*Weapon);

            if (Expected.MeshKind == EExpectedWeaponMeshKind::Static)
            {
                UStaticMesh* ExpectedMesh = LoadObject<UStaticMesh>(nullptr, Expected.ObjectPath);
                bAssetLoads = ExpectedMesh != nullptr;
                bUsesExpectedProductionVisual = UsesExpectedStaticMesh(*Weapon, ExpectedMesh);
                if (ExpectedMesh) ActualAssetName = ExpectedMesh->GetPathName();
            }
            else if (Expected.MeshKind == EExpectedWeaponMeshKind::Skeletal)
            {
                USkeletalMesh* ExpectedMesh = LoadObject<USkeletalMesh>(nullptr, Expected.ObjectPath);
                bAssetLoads = ExpectedMesh != nullptr;
                bUsesExpectedProductionVisual = UsesExpectedSkeletalMesh(*Weapon, ExpectedMesh);
                if (ExpectedMesh) ActualAssetName = ExpectedMesh->GetPathName();
            }
        }

        const bool bCanonicalAssetDefined = Expected.ObjectPath != nullptr;
        const bool bPass = bSpawned && bIdMatches && bCanonicalAssetDefined && bAssetLoads &&
            bUsesExpectedProductionVisual && bFallbackHidden;

        bAllPass = bAllPass && bPass;
        if (bPass) ++PassedWeapons;

        Report += FString::Printf(
            TEXT("%s | id=%s | spawned=%s | idMatch=%s | canonical=%s | assetLoads=%s | productionVisual=%s | fallbackHidden=%s | asset=%s | RESULT=%s\n"),
            Expected.Label,
            *Expected.WeaponId.ToString(),
            bSpawned ? TEXT("PASS") : TEXT("FAIL"),
            bIdMatches ? TEXT("PASS") : TEXT("FAIL"),
            bCanonicalAssetDefined ? TEXT("PASS") : TEXT("MISSING"),
            bAssetLoads ? TEXT("PASS") : TEXT("FAIL"),
            bUsesExpectedProductionVisual ? TEXT("PASS") : TEXT("FAIL"),
            bFallbackHidden ? TEXT("PASS") : TEXT("FAIL"),
            *ActualAssetName,
            bPass ? TEXT("PASS") : TEXT("FAIL"));
    }

    for (const TWeakObjectPtr<AOCWeaponBase>& TemporaryWeapon : TemporaryWeapons)
    {
        if (AOCWeaponBase* Weapon = TemporaryWeapon.Get()) Weapon->Destroy();
    }

    Report += FString::Printf(TEXT("\nSUMMARY=%d/%d production weapon classes PASS\n"),
        PassedWeapons, UE_ARRAY_COUNT(Expectations));

    if (!FFileHelper::SaveStringToFile(Report, *ReportPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
    {
        UE_LOG(LogTemp, Warning, TEXT("R14 production weapon validation could not write report: %s"), *ReportPath);
        bAllPass = false;
    }

    if (bAllPass)
    {
        FFileHelper::SaveStringToFile(
            TEXT("R14_PRODUCTION_WEAPONS=PASS\n"),
            *SuccessSentinelPath,
            FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
        UE_LOG(LogTemp, Display,
            TEXT("R14 production weapon validation PASS: %d/%d classes. Report: %s"),
            PassedWeapons, UE_ARRAY_COUNT(Expectations), *ReportPath);
    }
    else
    {
        UE_LOG(LogTemp, Warning,
            TEXT("R14 production weapon validation FAILED: %d/%d classes. Inspect canonical asset, production component and fallback visibility results in: %s"),
            PassedWeapons, UE_ARRAY_COUNT(Expectations), *ReportPath);
    }

    if (bHeadlessGate)
    {
        // The CMD validator decides PASS from the explicit sentinel. Always terminate the headless
        // UE process after the report is flushed so a missing sentinel becomes a deterministic gate.
        FPlatformMisc::RequestExit(false);
    }
}
