#include "OCPass45ImportedGrenadeVisualSubsystem.h"

#include "OCGameMode.h"
#include "OCGrenadeProjectile.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Modules/ModuleManager.h"

namespace
{
    const FName ImportedGrenadeTag(TEXT("OC_PASS45_IMPORTED_GRENADE_VISUAL"));
    constexpr int32 FastRefreshPasses = 12;
    constexpr float PersistentRefreshIntervalSeconds = 0.25f;
    constexpr float DesiredGrenadeLengthCm = 14.0f;

    enum class EImportedGrenadeVisualKind : uint8
    {
        None,
        Fragmentation,
        Smoke,
        Flash
    };

    FString CandidateText(const FAssetData& Asset)
    {
        return (Asset.PackageName.ToString() + TEXT("/") + Asset.AssetName.ToString()).ToLower();
    }

    bool IsRejectedGrenadeCandidate(const FString& Candidate)
    {
        // Fab now also contains actual launchers. A path containing "grenade" must never be enough to make an
        // RPG/grenade-launcher mesh become the hand-grenade body. Keep identity fail-closed instead of substituting.
        return Candidate.Contains(TEXT("grenade_launcher")) ||
            Candidate.Contains(TEXT("grenade-launcher")) ||
            Candidate.Contains(TEXT("grenadelauncher")) ||
            Candidate.Contains(TEXT("rocket_launcher")) ||
            Candidate.Contains(TEXT("rocket-launcher")) ||
            Candidate.Contains(TEXT("rocketlauncher")) ||
            Candidate.Contains(TEXT("rpg-")) ||
            Candidate.Contains(TEXT("rpg_")) ||
            Candidate.Contains(TEXT("/rpg"));
    }

    EImportedGrenadeVisualKind ClassifyCandidate(const FAssetData& Asset)
    {
        const FString Candidate = CandidateText(Asset);
        if (IsRejectedGrenadeCandidate(Candidate)) return EImportedGrenadeVisualKind::None;
        if (Candidate.Contains(TEXT("flash"))) return EImportedGrenadeVisualKind::Flash;
        if (Candidate.Contains(TEXT("smoke")) || Candidate.Contains(TEXT("m18"))) return EImportedGrenadeVisualKind::Smoke;
        if (Candidate.Contains(TEXT("frag")) || Candidate.Contains(TEXT("fragment")) ||
            Candidate.Contains(TEXT("grenade")) || Candidate.Contains(TEXT("granade")))
        {
            return EImportedGrenadeVisualKind::Fragmentation;
        }
        return EImportedGrenadeVisualKind::None;
    }

    FString FabPackKey(const FAssetData& Asset)
    {
        TArray<FString> Parts;
        Asset.PackageName.ToString().ParseIntoArray(Parts, TEXT("/"), true);
        if (Parts.Num() >= 3 && Parts[0].Equals(TEXT("Game"), ESearchCase::IgnoreCase) &&
            Parts[1].Equals(TEXT("Fab"), ESearchCase::IgnoreCase))
        {
            return Parts[2].ToLower();
        }
        return Asset.PackagePath.ToString().ToLower();
    }

    int32 CandidateScore(const FAssetData& Asset, EImportedGrenadeVisualKind Kind)
    {
        const FString Candidate = CandidateText(Asset);
        if (IsRejectedGrenadeCandidate(Candidate)) return MIN_int32 / 2;

        int32 Score = 0;
        if (Candidate.Contains(TEXT("grenade")) || Candidate.Contains(TEXT("granade"))) Score += 40;
        if (Kind == EImportedGrenadeVisualKind::Fragmentation &&
            (Candidate.Contains(TEXT("frag")) || Candidate.Contains(TEXT("fragment")))) Score += 45;
        if (Kind == EImportedGrenadeVisualKind::Smoke &&
            (Candidate.Contains(TEXT("smoke")) || Candidate.Contains(TEXT("m18")))) Score += 45;
        if (Kind == EImportedGrenadeVisualKind::Flash && Candidate.Contains(TEXT("flash"))) Score += 45;
        if (Candidate.Contains(TEXT("mesh")) || Candidate.Contains(TEXT("body"))) Score += 8;
        if (Candidate.Contains(TEXT("collision")) || Candidate.Contains(TEXT("proxy")) ||
            Candidate.Contains(TEXT("preview")) || Candidate.Contains(TEXT("lod"))) Score -= 30;
        if (Candidate.Contains(TEXT("pin")) || Candidate.Contains(TEXT("ring")) ||
            Candidate.Contains(TEXT("spoon")) || Candidate.Contains(TEXT("lever")) ||
            Candidate.Contains(TEXT("cap"))) Score -= 24;
        return Score;
    }

    void GatherFabGrenadeMeshes(
        TArray<FAssetData>& OutFrag,
        TArray<FAssetData>& OutSmoke,
        TArray<FAssetData>& OutFlash)
    {
        FARFilter Filter;
        Filter.ClassPaths.Add(UStaticMesh::StaticClass()->GetClassPathName());
        Filter.PackagePaths.Add(FName(TEXT("/Game/Fab")));
        Filter.bRecursivePaths = true;
        Filter.bRecursiveClasses = true;

        FAssetRegistryModule& RegistryModule =
            FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
        TArray<FAssetData> Assets;
        RegistryModule.Get().GetAssets(Filter, Assets);

        TMap<FString, FAssetData> BestFragByPack;
        TMap<FString, FAssetData> BestSmokeByPack;
        TMap<FString, FAssetData> BestFlashByPack;

        for (const FAssetData& Asset : Assets)
        {
            const EImportedGrenadeVisualKind Kind = ClassifyCandidate(Asset);
            if (Kind == EImportedGrenadeVisualKind::None) continue;

            TMap<FString, FAssetData>* Target = nullptr;
            switch (Kind)
            {
                case EImportedGrenadeVisualKind::Fragmentation: Target = &BestFragByPack; break;
                case EImportedGrenadeVisualKind::Smoke: Target = &BestSmokeByPack; break;
                case EImportedGrenadeVisualKind::Flash: Target = &BestFlashByPack; break;
                default: break;
            }
            if (!Target) continue;

            const FString PackKey = FabPackKey(Asset);
            FAssetData* Existing = Target->Find(PackKey);
            if (!Existing || CandidateScore(Asset, Kind) > CandidateScore(*Existing, Kind) ||
                (CandidateScore(Asset, Kind) == CandidateScore(*Existing, Kind) &&
                 Asset.GetObjectPathString() < Existing->GetObjectPathString()))
            {
                Target->Add(PackKey, Asset);
            }
        }

        BestFragByPack.GenerateValueArray(OutFrag);
        BestSmokeByPack.GenerateValueArray(OutSmoke);
        BestFlashByPack.GenerateValueArray(OutFlash);

        auto SortKind = [](TArray<FAssetData>& Items, EImportedGrenadeVisualKind Kind)
        {
            Items.Sort([Kind](const FAssetData& A, const FAssetData& B)
            {
                const int32 ScoreA = CandidateScore(A, Kind);
                const int32 ScoreB = CandidateScore(B, Kind);
                if (ScoreA != ScoreB) return ScoreA > ScoreB;
                return A.GetObjectPathString() < B.GetObjectPathString();
            });
        };
        SortKind(OutFrag, EImportedGrenadeVisualKind::Fragmentation);
        SortKind(OutSmoke, EImportedGrenadeVisualKind::Smoke);
        SortKind(OutFlash, EImportedGrenadeVisualKind::Flash);
    }

    struct FGrenadeCatalogCache
    {
        bool bLoaded = false;
        TArray<FAssetData> Frag;
        TArray<FAssetData> Smoke;
        TArray<FAssetData> Flash;
    };

    FGrenadeCatalogCache& GetGrenadeCatalogCache()
    {
        static FGrenadeCatalogCache Cache;
        if (!Cache.bLoaded)
        {
            GatherFabGrenadeMeshes(Cache.Frag, Cache.Smoke, Cache.Flash);
            Cache.bLoaded = true;
        }
        return Cache;
    }

    UStaticMesh* ResolveVisualFor(AOCGrenadeProjectile& Grenade,
        const TArray<FAssetData>& Frag,
        const TArray<FAssetData>& Smoke,
        const TArray<FAssetData>& Flash,
        int32& OutVariantIndex,
        int32& OutVariantCount)
    {
        const TArray<FAssetData>* Candidates = nullptr;
        switch (Grenade.GetGrenadeType())
        {
            case EOCGrenadeType::Fragmentation: Candidates = &Frag; break;
            case EOCGrenadeType::Smoke: Candidates = &Smoke; break;
            case EOCGrenadeType::Flash: Candidates = &Flash; break;
            default: break;
        }
        if (!Candidates || Candidates->IsEmpty()) return nullptr;

        OutVariantCount = Candidates->Num();
        if (Grenade.GetGrenadeType() == EOCGrenadeType::Fragmentation && Candidates->Num() > 1)
        {
            const FVector Location = Grenade.GetActorLocation();
            const int32 X = FMath::RoundToInt(Location.X / 10.0f);
            const int32 Y = FMath::RoundToInt(Location.Y / 10.0f);
            const int32 Z = FMath::RoundToInt(Location.Z / 10.0f);
            const uint32 Hash = HashCombine(GetTypeHash(X), HashCombine(GetTypeHash(Y), GetTypeHash(Z)));
            OutVariantIndex = static_cast<int32>(Hash % static_cast<uint32>(Candidates->Num()));
        }
        else
        {
            OutVariantIndex = 0;
        }
        return Cast<UStaticMesh>((*Candidates)[OutVariantIndex].GetAsset());
    }

    bool ApplyImportedVisual(AOCGrenadeProjectile& Grenade, UStaticMesh* Mesh, int32 VariantIndex, int32 VariantCount)
    {
        UStaticMeshComponent* Component = Grenade.GetGrenadeMeshComponent();
        if (!Component || !Mesh || Component->ComponentHasTag(ImportedGrenadeTag)) return false;

        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        const float NativeLength = FMath::Max3(NativeSize.X, NativeSize.Y, NativeSize.Z);
        if (NativeLength <= 1.0f) return false;

        const float Scale = DesiredGrenadeLengthCm / NativeLength;
        Component->SetStaticMesh(Mesh);
        Component->EmptyOverrideMaterials();
        Component->SetRelativeLocation(-Bounds.Origin * Scale);
        Component->SetRelativeRotation(FRotator::ZeroRotator);
        Component->SetRelativeScale3D(FVector(Scale));
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetHiddenInGame(false, true);
        Component->SetVisibility(true, true);
        Component->ComponentTags.AddUnique(ImportedGrenadeTag);

        UE_LOG(LogTemp, Display,
            TEXT("PASS45_IMPORTED_GRENADE_VISUAL_READY type=%d asset=%s variant=%d variant_count=%d shared_generic_body=0 exact_imported_body=1 launcher_false_match=0 runtime_acceptance=0"),
            static_cast<int32>(Grenade.GetGrenadeType()), *Mesh->GetPathName(), VariantIndex, VariantCount);
        return true;
    }
}

bool UOCPass45ImportedGrenadeVisualSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCPass45ImportedGrenadeVisualSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    InWorld.GetTimerManager().SetTimer(
        RefreshTimer,
        this,
        &UOCPass45ImportedGrenadeVisualSubsystem::RefreshGrenadeVisuals,
        0.20f,
        true,
        0.05f);
}

void UOCPass45ImportedGrenadeVisualSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(RefreshTimer);
    RefreshPass = 0;
    Super::Deinitialize();
}

void UOCPass45ImportedGrenadeVisualSubsystem::RefreshGrenadeVisuals()
{
    UWorld* World = GetWorld();
    if (!World) return;
    ++RefreshPass;

    const FGrenadeCatalogCache& Catalog = GetGrenadeCatalogCache();

    int32 Applied = 0;
    for (TActorIterator<AOCGrenadeProjectile> It(World); It; ++It)
    {
        AOCGrenadeProjectile* Grenade = *It;
        if (!Grenade || Grenade->IsActorBeingDestroyed()) continue;
        UStaticMeshComponent* Component = Grenade->GetGrenadeMeshComponent();
        if (!Component || Component->ComponentHasTag(ImportedGrenadeTag)) continue;

        int32 VariantIndex = 0;
        int32 VariantCount = 0;
        if (UStaticMesh* Mesh = ResolveVisualFor(
            *Grenade, Catalog.Frag, Catalog.Smoke, Catalog.Flash, VariantIndex, VariantCount))
        {
            if (ApplyImportedVisual(*Grenade, Mesh, VariantIndex, VariantCount)) ++Applied;
        }
    }

    if (RefreshPass == 1)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_IMPORTED_GRENADE_CATALOG frag_variants=%d smoke_variants=%d flash_variants=%d two_frag_supported=%d launcher_candidates_rejected=1 asset_registry_scans=1 local_uncommitted_assets_visible_to_ue=1 runtime_acceptance=0"),
            Catalog.Frag.Num(), Catalog.Smoke.Num(), Catalog.Flash.Num(), Catalog.Frag.Num() >= 2 ? 1 : 0);
    }
    if (Applied > 0)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_IMPORTED_GRENADE_VISUAL_PASS pass=%d applied=%d"), RefreshPass, Applied);
    }

    if (RefreshPass == FastRefreshPasses)
    {
        // Grenades are transient actors created throughout the match. Stopping here would mean every grenade
        // thrown after the first few seconds silently reverts to the old shared body. Keep a cheap actor-only
        // watch alive; the Fab AssetRegistry catalog above is cached once and is not rescanned every tick.
        World->GetTimerManager().SetTimer(
            RefreshTimer,
            this,
            &UOCPass45ImportedGrenadeVisualSubsystem::RefreshGrenadeVisuals,
            PersistentRefreshIntervalSeconds,
            true,
            PersistentRefreshIntervalSeconds);
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_IMPORTED_GRENADE_LATE_SPAWN_WATCH_READY fast_passes=%d interval_s=%.2f asset_registry_scans=1 late_throw_support=1 shared_generic_body=0"),
            FastRefreshPasses, PersistentRefreshIntervalSeconds);
    }
}
