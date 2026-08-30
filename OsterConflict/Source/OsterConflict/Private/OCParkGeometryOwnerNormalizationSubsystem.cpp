#include "OCParkGeometryOwnerNormalizationSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInterface.h"

namespace
{
    constexpr int32 ExpectedLegacyGeometryInstances = 3;
    constexpr float NormalizationDelaySeconds = 0.45f;
    constexpr float SectorResolutionTimeoutSeconds = 5.0f;

    bool IsEngineCube(const UStaticMesh* Mesh)
    {
        return Mesh && Mesh->GetPathName().Contains(TEXT("/Engine/BasicShapes/Cube.Cube"), ESearchCase::IgnoreCase);
    }

    UInstancedStaticMeshComponent* FindISM(AOCWorldSectorOster* Sector, const FName Name)
    {
        if (!Sector) return nullptr;
        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Sector->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (Component && Component->GetFName() == Name) return Component;
        }
        return nullptr;
    }

    void DestroyTargets(TArray<UInstancedStaticMeshComponent*>& Targets)
    {
        for (UInstancedStaticMeshComponent* Target : Targets)
        {
            if (Target) Target->DestroyComponent();
        }
        Targets.Reset();
    }

    UInstancedStaticMeshComponent* CreateSemanticOwner(
        AOCWorldSectorOster* Sector,
        UInstancedStaticMeshComponent* Source,
        const FName Name,
        TArray<UInstancedStaticMeshComponent*>& CreatedTargets)
    {
        if (!Sector || !Source || FindISM(Sector, Name)) return nullptr;

        UInstancedStaticMeshComponent* Target = NewObject<UInstancedStaticMeshComponent>(Sector, Name, RF_Transient);
        if (!Target) return nullptr;

        Target->SetupAttachment(Sector->GetRootComponent());
        Target->SetMobility(Source->Mobility);
        Target->SetStaticMesh(Source->GetStaticMesh());
        Target->SetCollisionProfileName(Source->GetCollisionProfileName());
        Target->SetCastShadow(Source->CastShadow);
        for (int32 MaterialIndex = 0; MaterialIndex < Source->GetNumMaterials(); ++MaterialIndex)
        {
            Target->SetMaterial(MaterialIndex, Source->GetMaterial(MaterialIndex));
        }
        Target->RegisterComponent();
        CreatedTargets.Add(Target);
        return Target;
    }
}

bool UOCParkGeometryOwnerNormalizationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

TStatId UOCParkGeometryOwnerNormalizationSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCParkGeometryOwnerNormalizationSubsystem, STATGROUP_Tickables);
}

void UOCParkGeometryOwnerNormalizationSubsystem::Tick(float DeltaTime)
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
        if (ElapsedSeconds < SectorResolutionTimeoutSeconds) return;
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_PARK_GEOMETRY_OWNER_NORMALIZATION_FAIL reason=oster_sector_count_%d source_instances_preserved=1 primary_authoring=0 migration_bridge=1 gate_k_complete=0 runtime_acceptance=0"),
            SectorCount);
        return;
    }

    UInstancedStaticMeshComponent* LegacyGeometry = FindISM(Sector, TEXT("ParkGeometry"));
    if (!LegacyGeometry || LegacyGeometry->GetInstanceCount() != ExpectedLegacyGeometryInstances ||
        !IsEngineCube(LegacyGeometry->GetStaticMesh()))
    {
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_PARK_GEOMETRY_OWNER_NORMALIZATION_FAIL reason=legacy_contract park_geometry=%d expected=%d engine_cube=%d source_instances_preserved=1 primary_authoring=0 migration_bridge=1 gate_k_complete=0 runtime_acceptance=0"),
            LegacyGeometry ? LegacyGeometry->GetInstanceCount() : -1,
            ExpectedLegacyGeometryInstances,
            LegacyGeometry && IsEngineCube(LegacyGeometry->GetStaticMesh()) ? 1 : 0);
        return;
    }

    const FName DestinationNames[] = {
        TEXT("ParkCentralGround"),
        TEXT("ParkNorthCivicGround"),
        TEXT("CollegeRecreationGround")
    };
    for (const FName DestinationName : DestinationNames)
    {
        if (FindISM(Sector, DestinationName))
        {
            bFinished = true;
            UE_LOG(LogTemp, Error,
                TEXT("PASS45_PARK_GEOMETRY_OWNER_NORMALIZATION_FAIL reason=destination_preexists name=%s source_instances_preserved=1 primary_authoring=0 migration_bridge=1 gate_k_complete=0 runtime_acceptance=0"),
                *DestinationName.ToString());
            return;
        }
    }

    FTransform SourceTransforms[ExpectedLegacyGeometryInstances];
    for (int32 Index = 0; Index < ExpectedLegacyGeometryInstances; ++Index)
    {
        if (!LegacyGeometry->GetInstanceTransform(Index, SourceTransforms[Index], false))
        {
            bFinished = true;
            UE_LOG(LogTemp, Error,
                TEXT("PASS45_PARK_GEOMETRY_OWNER_NORMALIZATION_FAIL reason=source_transform_%d source_instances_preserved=1 primary_authoring=0 migration_bridge=1 gate_k_complete=0 runtime_acceptance=0"),
                Index);
            return;
        }
    }

    TArray<UInstancedStaticMeshComponent*> CreatedTargets;
    UInstancedStaticMeshComponent* ParkCentralGround = CreateSemanticOwner(
        Sector, LegacyGeometry, DestinationNames[0], CreatedTargets);
    UInstancedStaticMeshComponent* ParkNorthCivicGround = CreateSemanticOwner(
        Sector, LegacyGeometry, DestinationNames[1], CreatedTargets);
    UInstancedStaticMeshComponent* CollegeRecreationGround = CreateSemanticOwner(
        Sector, LegacyGeometry, DestinationNames[2], CreatedTargets);

    if (!ParkCentralGround || !ParkNorthCivicGround || !CollegeRecreationGround)
    {
        DestroyTargets(CreatedTargets);
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_PARK_GEOMETRY_OWNER_NORMALIZATION_FAIL reason=destination_creation source_instances_preserved=1 primary_authoring=0 migration_bridge=1 gate_k_complete=0 runtime_acceptance=0"));
        return;
    }

    ParkCentralGround->AddInstance(SourceTransforms[0], false);
    ParkNorthCivicGround->AddInstance(SourceTransforms[1], false);
    CollegeRecreationGround->AddInstance(SourceTransforms[2], false);

    if (ParkCentralGround->GetInstanceCount() != 1 ||
        ParkNorthCivicGround->GetInstanceCount() != 1 ||
        CollegeRecreationGround->GetInstanceCount() != 1)
    {
        DestroyTargets(CreatedTargets);
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_PARK_GEOMETRY_OWNER_NORMALIZATION_FAIL reason=destination_copy source_instances_preserved=1 primary_authoring=0 migration_bridge=1 gate_k_complete=0 runtime_acceptance=0"));
        return;
    }

    // Commit point: all three exact semantic owners exist with all three source transforms. Only now may the
    // mixed source bucket be emptied. The destination presentation remains the same visible Cube/material and is
    // therefore still truthfully subject to Gate K rather than hidden or cosmetically accepted.
    LegacyGeometry->ClearInstances();

    if (LegacyGeometry->GetInstanceCount() != 0 ||
        ParkCentralGround->GetInstanceCount() != 1 ||
        ParkNorthCivicGround->GetInstanceCount() != 1 ||
        CollegeRecreationGround->GetInstanceCount() != 1)
    {
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_PARK_GEOMETRY_OWNER_NORMALIZATION_FAIL reason=postcondition legacy=%d central=%d north=%d college=%d primary_authoring=0 migration_bridge=1 gate_k_complete=0 runtime_acceptance=0"),
            LegacyGeometry->GetInstanceCount(),
            ParkCentralGround->GetInstanceCount(),
            ParkNorthCivicGround->GetInstanceCount(),
            CollegeRecreationGround->GetInstanceCount());
        return;
    }

    bFinished = true;
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_PARK_GEOMETRY_OWNER_NORMALIZATION_READY legacy_park_geometry=0 park_central_ground=1 park_north_civic_ground=1 college_recreation_ground=1 source_instance_total=3 destination_instance_total=3 geometry_preserved=1 mesh_preserved=1 material_preserved=1 collision_preserved=1 authored_replacements=0 content_gap=1 tactical_map_bounds_preserved=1 primary_authoring=0 migration_bridge=1 gate_k_complete=0 runtime_acceptance=0"));
}
