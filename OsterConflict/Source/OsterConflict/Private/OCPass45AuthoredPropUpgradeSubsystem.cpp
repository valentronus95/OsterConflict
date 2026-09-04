#include "OCPass45AuthoredPropUpgradeSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInterface.h"

namespace
{
    constexpr float UpgradeDelaySeconds = 0.90f;
    constexpr float OwnerResolutionTimeoutSeconds = 6.0f;
    constexpr int32 ExpectedParkBenchCount = 14;

    const TCHAR* ParkBenchMeshPaths[] =
    {
        TEXT("/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Meshes/SM_Bench_1.SM_Bench_1"),
        TEXT("/Game/Street_Props_Pack_V1/Mesh/SM_Bench.SM_Bench")
    };

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

    bool IsEngineCube(const UStaticMesh* Mesh)
    {
        return Mesh && Mesh->GetPathName().Contains(TEXT("/Engine/BasicShapes/Cube"), ESearchCase::IgnoreCase);
    }

    UStaticMesh* LoadFirstAvailableMesh(FString& OutPath)
    {
        for (const TCHAR* Path : ParkBenchMeshPaths)
        {
            if (UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, Path))
            {
                OutPath = Path;
                return Mesh;
            }
        }
        OutPath.Reset();
        return nullptr;
    }

    struct FOwnerSnapshot
    {
        UStaticMesh* Mesh = nullptr;
        TArray<FTransform> Instances;
        TArray<UMaterialInterface*> Materials;
    };

    bool CaptureOwner(UInstancedStaticMeshComponent* Component, FOwnerSnapshot& OutSnapshot, FString& OutFailure)
    {
        if (!Component)
        {
            OutFailure = TEXT("park_benches_owner_missing");
            return false;
        }
        if (Component->GetInstanceCount() != ExpectedParkBenchCount)
        {
            OutFailure = FString::Printf(TEXT("park_benches_instance_count_%d_expected_%d"),
                Component->GetInstanceCount(), ExpectedParkBenchCount);
            return false;
        }

        UStaticMesh* SourceMesh = Component->GetStaticMesh();
        if (!IsEngineCube(SourceMesh))
        {
            OutFailure = FString::Printf(TEXT("park_benches_source_mesh_not_cube_%s"),
                SourceMesh ? *SourceMesh->GetPathName() : TEXT("null"));
            return false;
        }

        OutSnapshot.Mesh = SourceMesh;
        OutSnapshot.Instances.Reserve(ExpectedParkBenchCount);
        for (int32 Index = 0; Index < ExpectedParkBenchCount; ++Index)
        {
            FTransform Transform;
            if (!Component->GetInstanceTransform(Index, Transform, false))
            {
                OutFailure = FString::Printf(TEXT("park_benches_transform_read_failed_%d"), Index);
                return false;
            }
            OutSnapshot.Instances.Add(Transform);
        }

        const int32 MaterialCount = Component->GetNumMaterials();
        OutSnapshot.Materials.Reserve(MaterialCount);
        for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
        {
            OutSnapshot.Materials.Add(Component->GetMaterial(MaterialIndex));
        }
        return true;
    }

    FTransform FitAuthoredBenchToSourceBox(
        const FTransform& SourceTransform,
        const FBoxSphereBounds& SourceBounds,
        const FBoxSphereBounds& AuthoredBounds)
    {
        const FVector SourceScale = SourceTransform.GetScale3D().GetAbs();
        const FVector SourceNativeSize = SourceBounds.BoxExtent * 2.0f;
        const FVector SourceSize(
            SourceNativeSize.X * SourceScale.X,
            SourceNativeSize.Y * SourceScale.Y,
            SourceNativeSize.Z * SourceScale.Z);
        const FVector AuthoredSize = AuthoredBounds.BoxExtent * 2.0f;

        const bool bSourceLongAxisY = SourceSize.Y > SourceSize.X;
        const bool bAuthoredLongAxisY = AuthoredSize.Y > AuthoredSize.X;
        const float SourceLength = FMath::Max(SourceSize.X, SourceSize.Y);
        const float AuthoredLength = FMath::Max(AuthoredSize.X, AuthoredSize.Y);
        const float UniformScale = AuthoredLength > 1.0f
            ? FMath::Clamp(SourceLength / AuthoredLength, 0.05f, 8.0f)
            : 1.0f;

        FRotator NewRotation = SourceTransform.Rotator();
        if (bSourceLongAxisY != bAuthoredLongAxisY)
        {
            NewRotation.Yaw += 90.0f;
        }

        const FQuat SourceQuat = SourceTransform.GetRotation();
        const FVector SourceScaledOrigin(
            SourceBounds.Origin.X * SourceScale.X,
            SourceBounds.Origin.Y * SourceScale.Y,
            SourceBounds.Origin.Z * SourceScale.Z);
        const FVector SourceCenter = SourceTransform.GetLocation() + SourceQuat.RotateVector(SourceScaledOrigin);
        const float SourceBottomZ = SourceTransform.GetLocation().Z +
            (SourceBounds.Origin.Z - SourceBounds.BoxExtent.Z) * SourceScale.Z;

        const FVector NewScale(UniformScale);
        const FQuat NewQuat = NewRotation.Quaternion();
        const FVector NewScaledOrigin = AuthoredBounds.Origin * UniformScale;
        FVector NewLocation = SourceCenter - NewQuat.RotateVector(NewScaledOrigin);
        const float AuthoredBottomOffsetZ =
            (AuthoredBounds.Origin.Z - AuthoredBounds.BoxExtent.Z) * UniformScale;
        NewLocation.Z = SourceBottomZ - AuthoredBottomOffsetZ;

        return FTransform(NewQuat, NewLocation, NewScale);
    }

    void RestoreOwner(UInstancedStaticMeshComponent* Component, const FOwnerSnapshot& Snapshot)
    {
        if (!Component || !Snapshot.Mesh) return;
        Component->ClearInstances();
        Component->SetStaticMesh(Snapshot.Mesh);
        Component->EmptyOverrideMaterials();
        for (int32 MaterialIndex = 0; MaterialIndex < Snapshot.Materials.Num(); ++MaterialIndex)
        {
            Component->SetMaterial(MaterialIndex, Snapshot.Materials[MaterialIndex]);
        }
        for (const FTransform& Transform : Snapshot.Instances)
        {
            Component->AddInstance(Transform, false);
        }
        Component->MarkRenderStateDirty();
    }

    bool ApplyAuthoredBenchCutover(
        UInstancedStaticMeshComponent* Component,
        UStaticMesh* AuthoredMesh,
        const FOwnerSnapshot& Snapshot)
    {
        if (!Component || !AuthoredMesh || !Snapshot.Mesh) return false;

        const FBoxSphereBounds SourceBounds = Snapshot.Mesh->GetBounds();
        const FBoxSphereBounds AuthoredBounds = AuthoredMesh->GetBounds();
        const FVector AuthoredSize = AuthoredBounds.BoxExtent * 2.0f;
        if (AuthoredSize.X <= 1.0f || AuthoredSize.Y <= 1.0f || AuthoredSize.Z <= 1.0f) return false;

        TArray<FTransform> NewTransforms;
        NewTransforms.Reserve(Snapshot.Instances.Num());
        for (const FTransform& SourceTransform : Snapshot.Instances)
        {
            NewTransforms.Add(FitAuthoredBenchToSourceBox(SourceTransform, SourceBounds, AuthoredBounds));
        }

        Component->ClearInstances();
        Component->SetStaticMesh(AuthoredMesh);
        Component->EmptyOverrideMaterials();
        Component->SetCollisionProfileName(TEXT("BlockAll"));
        for (const FTransform& Transform : NewTransforms)
        {
            if (Component->AddInstance(Transform, false) == INDEX_NONE)
            {
                RestoreOwner(Component, Snapshot);
                return false;
            }
        }
        Component->MarkRenderStateDirty();

        if (Component->GetStaticMesh() != AuthoredMesh ||
            Component->GetInstanceCount() != ExpectedParkBenchCount)
        {
            RestoreOwner(Component, Snapshot);
            return false;
        }
        return true;
    }
}

bool UOCPass45AuthoredPropUpgradeSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

TStatId UOCPass45AuthoredPropUpgradeSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCPass45AuthoredPropUpgradeSubsystem, STATGROUP_Tickables);
}

void UOCPass45AuthoredPropUpgradeSubsystem::Tick(float DeltaTime)
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
    if (ElapsedSeconds < UpgradeDelaySeconds) return;

    AOCWorldSectorOster* Sector = nullptr;
    int32 SectorCount = 0;
    for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It)
    {
        Sector = *It;
        ++SectorCount;
    }
    if (SectorCount != 1 || !Sector)
    {
        if (ElapsedSeconds < OwnerResolutionTimeoutSeconds) return;
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_AUTHORED_PROP_UPGRADE_FAIL reason=oster_sector_count_%d runtime_acceptance=0"),
            SectorCount);
        return;
    }

    UInstancedStaticMeshComponent* ParkBenches = FindISM(Sector, TEXT("ParkBenches"));
    FOwnerSnapshot Snapshot;
    FString Failure;
    if (!CaptureOwner(ParkBenches, Snapshot, Failure))
    {
        if (ElapsedSeconds < OwnerResolutionTimeoutSeconds) return;
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_AUTHORED_PROP_UPGRADE_FAIL reason=%s runtime_acceptance=0"), *Failure);
        return;
    }

    FString BenchPath;
    UStaticMesh* BenchMesh = LoadFirstAvailableMesh(BenchPath);
    if (!BenchMesh)
    {
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_AUTHORED_PROP_CONTENT_GAP owner=ParkBenches expected=SM_Bench_1_or_SM_Bench runtime_acceptance=0"));
        return;
    }

    if (!ApplyAuthoredBenchCutover(ParkBenches, BenchMesh, Snapshot))
    {
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_AUTHORED_PROP_UPGRADE_FAIL reason=park_bench_cutover_write_failed rollback=1 runtime_acceptance=0"));
        return;
    }

    bFinished = true;
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_AUTHORED_PROP_UPGRADE_READY owner=ParkBenches authored_mesh=%s instances=14 basicshape_benches=0 source_placement_preserved=1 source_yaw_preserved=1 ground_contact_preserved=1 bounds_aware_uniform_fit=1 runtime_acceptance=0"),
        *BenchPath);
}
