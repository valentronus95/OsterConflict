#include "OCParkSemanticOwnerNormalizationSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInterface.h"

namespace
{
    constexpr int32 ExpectedLegacyMemorialInstances = 2;
    constexpr int32 ExpectedLegacySkateInstances = 3;
    constexpr float NormalizationDelaySeconds = 0.35f;

    UInstancedStaticMeshComponent* FindISM(AActor* Actor, const FName Name)
    {
        if (!Actor) return nullptr;
        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (Component && Component->GetFName() == Name) return Component;
        }
        return nullptr;
    }

    bool IsEngineCube(const UStaticMesh* Mesh)
    {
        return Mesh && Mesh->GetPathName().Contains(TEXT("/Engine/BasicShapes/Cube"), ESearchCase::IgnoreCase);
    }

    UInstancedStaticMeshComponent* CreateSemanticISM(
        AOCWorldSectorOster* Sector,
        UInstancedStaticMeshComponent* Source,
        const FName Name,
        FString& OutFailure)
    {
        if (!Sector || !Source)
        {
            OutFailure = TEXT("semantic_owner_source_missing");
            return nullptr;
        }
        if (FindISM(Sector, Name))
        {
            OutFailure = FString::Printf(TEXT("semantic_owner_already_exists_%s"), *Name.ToString());
            return nullptr;
        }

        UInstancedStaticMeshComponent* Destination = NewObject<UInstancedStaticMeshComponent>(Sector, Name, RF_Transient);
        if (!Destination)
        {
            OutFailure = FString::Printf(TEXT("semantic_owner_create_failed_%s"), *Name.ToString());
            return nullptr;
        }

        Sector->AddInstanceComponent(Destination);
        Destination->SetupAttachment(Sector->GetRootComponent());
        Destination->SetCollisionProfileName(Source->GetCollisionProfileName());
        Destination->SetStaticMesh(Source->GetStaticMesh());
        for (int32 MaterialIndex = 0; MaterialIndex < Source->GetNumMaterials(); ++MaterialIndex)
        {
            Destination->SetMaterial(MaterialIndex, Source->GetMaterial(MaterialIndex));
        }
        Destination->RegisterComponent();
        return Destination;
    }

    void DestroyIfCreated(UInstancedStaticMeshComponent*& Component)
    {
        if (!Component) return;
        Component->DestroyComponent();
        Component = nullptr;
    }

    bool AddExactInstance(
        UInstancedStaticMeshComponent* Destination,
        const FTransform& Transform,
        const TCHAR* Label,
        FString& OutFailure)
    {
        if (!Destination)
        {
            OutFailure = FString::Printf(TEXT("destination_missing_%s"), Label);
            return false;
        }
        if (Destination->AddInstance(Transform, false) < 0)
        {
            OutFailure = FString::Printf(TEXT("destination_add_failed_%s"), Label);
            return false;
        }
        return true;
    }
}

bool UOCParkSemanticOwnerNormalizationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

TStatId UOCParkSemanticOwnerNormalizationSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCParkSemanticOwnerNormalizationSubsystem, STATGROUP_Tickables);
}

void UOCParkSemanticOwnerNormalizationSubsystem::Tick(float DeltaTime)
{
    if (bFinished) return;

    UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld()) return;
    if (!World->GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    if (const AOCGameMode* GameMode = World->GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    ElapsedSeconds += FMath::Max(0.0f, DeltaTime);
    if (ElapsedSeconds < NormalizationDelaySeconds) return;

    AOCWorldSectorOster* Sector = nullptr;
    int32 SectorCount = 0;
    for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It)
    {
        Sector = *It;
        ++SectorCount;
    }
    if (SectorCount != 1 || !Sector)
    {
        if (ElapsedSeconds < 2.0f) return;
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_PARK_SEMANTIC_OWNER_NORMALIZATION_FAIL reason=oster_sector_count_%d primary_authoring=0 migration_bridge=1 gate_k_complete=0 runtime_acceptance=0"),
            SectorCount);
        return;
    }

    UInstancedStaticMeshComponent* LegacyMemorial = FindISM(Sector, TEXT("ParkMemorialPlaza"));
    UInstancedStaticMeshComponent* LegacySkate = FindISM(Sector, TEXT("ParkSkateFitness"));
    if (!LegacyMemorial || !LegacySkate)
    {
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_PARK_SEMANTIC_OWNER_NORMALIZATION_FAIL reason=legacy_component_missing memorial=%d skate=%d primary_authoring=0 migration_bridge=1 gate_k_complete=0 runtime_acceptance=0"),
            LegacyMemorial ? 1 : 0,
            LegacySkate ? 1 : 0);
        return;
    }

    if (LegacyMemorial->GetInstanceCount() != ExpectedLegacyMemorialInstances ||
        LegacySkate->GetInstanceCount() != ExpectedLegacySkateInstances)
    {
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_PARK_SEMANTIC_OWNER_NORMALIZATION_FAIL reason=legacy_instance_contract memorial=%d expected_memorial=2 skate=%d expected_skate=3 primary_authoring=0 migration_bridge=1 gate_k_complete=0 runtime_acceptance=0"),
            LegacyMemorial->GetInstanceCount(),
            LegacySkate->GetInstanceCount());
        return;
    }

    if (!IsEngineCube(LegacyMemorial->GetStaticMesh()) || !IsEngineCube(LegacySkate->GetStaticMesh()))
    {
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_PARK_SEMANTIC_OWNER_NORMALIZATION_FAIL reason=unexpected_legacy_mesh memorial=%s skate=%s primary_authoring=0 migration_bridge=1 gate_k_complete=0 runtime_acceptance=0"),
            LegacyMemorial->GetStaticMesh() ? *LegacyMemorial->GetStaticMesh()->GetPathName() : TEXT("null"),
            LegacySkate->GetStaticMesh() ? *LegacySkate->GetStaticMesh()->GetPathName() : TEXT("null"));
        return;
    }

    FTransform MemorialTransforms[ExpectedLegacyMemorialInstances];
    FTransform SkateTransforms[ExpectedLegacySkateInstances];
    for (int32 Index = 0; Index < ExpectedLegacyMemorialInstances; ++Index)
    {
        if (!LegacyMemorial->GetInstanceTransform(Index, MemorialTransforms[Index], false))
        {
            bFinished = true;
            UE_LOG(LogTemp, Error,
                TEXT("PASS45_PARK_SEMANTIC_OWNER_NORMALIZATION_FAIL reason=memorial_transform_read_%d primary_authoring=0 migration_bridge=1 gate_k_complete=0 runtime_acceptance=0"),
                Index);
            return;
        }
    }
    for (int32 Index = 0; Index < ExpectedLegacySkateInstances; ++Index)
    {
        if (!LegacySkate->GetInstanceTransform(Index, SkateTransforms[Index], false))
        {
            bFinished = true;
            UE_LOG(LogTemp, Error,
                TEXT("PASS45_PARK_SEMANTIC_OWNER_NORMALIZATION_FAIL reason=skate_transform_read_%d primary_authoring=0 migration_bridge=1 gate_k_complete=0 runtime_acceptance=0"),
                Index);
            return;
        }
    }

    FString Failure;
    UInstancedStaticMeshComponent* MemorialSurface = CreateSemanticISM(Sector, LegacyMemorial, TEXT("ParkMemorialSurface"), Failure);
    UInstancedStaticMeshComponent* MemorialMonument = MemorialSurface
        ? CreateSemanticISM(Sector, LegacyMemorial, TEXT("ParkMemorialMonument"), Failure)
        : nullptr;
    UInstancedStaticMeshComponent* SkateSurface = MemorialMonument
        ? CreateSemanticISM(Sector, LegacySkate, TEXT("ParkSkateSurface"), Failure)
        : nullptr;
    UInstancedStaticMeshComponent* SkateRamps = SkateSurface
        ? CreateSemanticISM(Sector, LegacySkate, TEXT("ParkSkateRamps"), Failure)
        : nullptr;

    const bool bOwnersCreated = MemorialSurface && MemorialMonument && SkateSurface && SkateRamps;
    const bool bInstancesCopied = bOwnersCreated &&
        AddExactInstance(MemorialSurface, MemorialTransforms[0], TEXT("memorial_surface"), Failure) &&
        AddExactInstance(MemorialMonument, MemorialTransforms[1], TEXT("memorial_monument"), Failure) &&
        AddExactInstance(SkateSurface, SkateTransforms[0], TEXT("skate_surface"), Failure) &&
        AddExactInstance(SkateRamps, SkateTransforms[1], TEXT("skate_ramp_0"), Failure) &&
        AddExactInstance(SkateRamps, SkateTransforms[2], TEXT("skate_ramp_1"), Failure);

    if (!bInstancesCopied)
    {
        DestroyIfCreated(SkateRamps);
        DestroyIfCreated(SkateSurface);
        DestroyIfCreated(MemorialMonument);
        DestroyIfCreated(MemorialSurface);
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_PARK_SEMANTIC_OWNER_NORMALIZATION_FAIL reason=%s source_instances_preserved=1 primary_authoring=0 migration_bridge=1 gate_k_complete=0 runtime_acceptance=0"),
            Failure.IsEmpty() ? TEXT("owner_or_copy_failure") : *Failure);
        return;
    }

    // Transaction boundary: only retire the two mixed source buckets after all exact destination owners exist and
    // contain the full 2+3 instance set. ParkDetails, ParkMemorialApproach and ParkBenches are intentionally untouched.
    LegacyMemorial->ClearInstances();
    LegacySkate->ClearInstances();
    MemorialSurface->MarkRenderStateDirty();
    MemorialMonument->MarkRenderStateDirty();
    SkateSurface->MarkRenderStateDirty();
    SkateRamps->MarkRenderStateDirty();

    const bool bPostcondition =
        LegacyMemorial->GetInstanceCount() == 0 &&
        LegacySkate->GetInstanceCount() == 0 &&
        MemorialSurface->GetInstanceCount() == 1 &&
        MemorialMonument->GetInstanceCount() == 1 &&
        SkateSurface->GetInstanceCount() == 1 &&
        SkateRamps->GetInstanceCount() == 2;

    bFinished = true;
    if (!bPostcondition)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_PARK_SEMANTIC_OWNER_NORMALIZATION_FAIL reason=postcondition legacy_memorial=%d legacy_skate=%d memorial_surface=%d memorial_monument=%d skate_surface=%d skate_ramps=%d primary_authoring=0 migration_bridge=1 gate_k_complete=0 runtime_acceptance=0"),
            LegacyMemorial->GetInstanceCount(),
            LegacySkate->GetInstanceCount(),
            MemorialSurface->GetInstanceCount(),
            MemorialMonument->GetInstanceCount(),
            SkateSurface->GetInstanceCount(),
            SkateRamps->GetInstanceCount());
        return;
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_PARK_SEMANTIC_OWNER_NORMALIZATION_READY legacy_memorial_plaza=0 legacy_skate_fitness=0 memorial_surface=1 memorial_monument=1 skate_surface=1 skate_ramps=2 source_instance_total=5 destination_instance_total=5 geometry_preserved=1 park_details_mutation=0 memorial_approach_mutation=0 benches_mutation=0 primary_authoring=0 migration_bridge=1 gate_k_complete=0 runtime_acceptance=0"));
}
