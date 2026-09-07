#include "OCParkSemanticAuthoredUpgradeSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInterface.h"
#include "UObject/SoftObjectPath.h"

namespace
{
    const TCHAR* AuthoredBenchPath =
        TEXT("/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Meshes/SM_Bench_1.SM_Bench_1");
    constexpr int32 ExpectedBenchInstances = 14;
    constexpr float DesiredBenchLengthCm = 180.0f;

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

    bool IsBasicShapeMaterial(const UMaterialInterface* Material)
    {
        return Material && Material->GetPathName().Contains(TEXT("/Engine/BasicShapes/"), ESearchCase::IgnoreCase);
    }

    bool UpgradeBenchFamily(
        UInstancedStaticMeshComponent* Component,
        UStaticMesh* AuthoredMesh,
        int32& OutInstanceCount,
        FString& OutFailure)
    {
        OutInstanceCount = 0;
        if (!Component || !AuthoredMesh)
        {
            OutFailure = TEXT("park_bench_component_or_mesh_missing");
            return false;
        }

        if (Component->GetInstanceCount() != ExpectedBenchInstances)
        {
            OutFailure = FString::Printf(TEXT("park_bench_instance_count_%d_expected_%d"),
                Component->GetInstanceCount(), ExpectedBenchInstances);
            return false;
        }

        UStaticMesh* CurrentMesh = Component->GetStaticMesh();
        if (CurrentMesh == AuthoredMesh)
        {
            UMaterialInterface* RuntimeMaterial = Component->GetMaterial(0);
            if (!RuntimeMaterial || IsBasicShapeMaterial(RuntimeMaterial))
            {
                OutFailure = TEXT("park_bench_authored_material_not_ready");
                return false;
            }
            OutInstanceCount = Component->GetInstanceCount();
            return true;
        }

        if (!IsEngineCube(CurrentMesh))
        {
            OutFailure = FString::Printf(TEXT("park_bench_unexpected_source_mesh_%s"),
                CurrentMesh ? *CurrentMesh->GetPathName() : TEXT("null"));
            return false;
        }

        const FBoxSphereBounds OldBounds = CurrentMesh->GetBounds();
        const FBoxSphereBounds NewBounds = AuthoredMesh->GetBounds();
        const FVector NewNativeSize = NewBounds.BoxExtent * 2.0f;
        if (NewNativeSize.X <= 1.0f || NewNativeSize.Y <= 1.0f || NewNativeSize.Z <= 1.0f)
        {
            OutFailure = TEXT("park_bench_authored_bounds_invalid");
            return false;
        }

        const bool bNativeLongAxisY = NewNativeSize.Y > NewNativeSize.X;
        const float NativeLength = bNativeLongAxisY ? NewNativeSize.Y : NewNativeSize.X;
        if (NativeLength <= 1.0f)
        {
            OutFailure = TEXT("park_bench_authored_length_invalid");
            return false;
        }

        // Preserve the authored bench native proportions with one uniform scale factor.
        const float UniformScale = FMath::Clamp(DesiredBenchLengthCm / NativeLength, 0.20f, 5.0f);

        TArray<FTransform> OldTransforms;
        OldTransforms.Reserve(ExpectedBenchInstances);
        for (int32 Index = 0; Index < Component->GetInstanceCount(); ++Index)
        {
            FTransform Transform;
            if (!Component->GetInstanceTransform(Index, Transform, false))
            {
                OutFailure = FString::Printf(TEXT("park_bench_source_transform_read_failed_%d"), Index);
                return false;
            }
            const FRotator Rotation = Transform.Rotator();
            if (FMath::Abs(Rotation.Pitch) > 0.1f || FMath::Abs(Rotation.Roll) > 0.1f)
            {
                OutFailure = FString::Printf(TEXT("park_bench_source_tilt_not_supported_%d"), Index);
                return false;
            }
            OldTransforms.Add(Transform);
        }

        Component->SetStaticMesh(AuthoredMesh);
        Component->EmptyOverrideMaterials();

        for (int32 Index = 0; Index < OldTransforms.Num(); ++Index)
        {
            const FTransform& Old = OldTransforms[Index];
            const FVector OldScale = Old.GetScale3D().GetAbs();
            FRotator Rotation = Old.Rotator();
            if (bNativeLongAxisY) Rotation.Yaw -= 90.0f;
            const FQuat RotationQuat = Rotation.Quaternion();

            const FVector OldScaledOrigin(
                OldBounds.Origin.X * OldScale.X,
                OldBounds.Origin.Y * OldScale.Y,
                OldBounds.Origin.Z * OldScale.Z);
            const FVector SourceCenter = Old.GetLocation() + Old.Rotator().Quaternion().RotateVector(OldScaledOrigin);
            const float SourceBottomZ = Old.GetLocation().Z +
                (OldBounds.Origin.Z - OldBounds.BoxExtent.Z) * OldScale.Z;

            const FVector NewScale(UniformScale);
            const FVector NewScaledOrigin(
                NewBounds.Origin.X * UniformScale,
                NewBounds.Origin.Y * UniformScale,
                NewBounds.Origin.Z * UniformScale);
            FVector NewLocation = SourceCenter - RotationQuat.RotateVector(NewScaledOrigin);
            const float NewBottomOffsetZ =
                (NewBounds.Origin.Z - NewBounds.BoxExtent.Z) * UniformScale;
            NewLocation.Z = SourceBottomZ - NewBottomOffsetZ;

            if (!Component->UpdateInstanceTransform(
                Index,
                FTransform(RotationQuat, NewLocation, NewScale),
                false,
                false,
                true))
            {
                OutFailure = FString::Printf(TEXT("park_bench_authored_transform_write_failed_%d"), Index);
                return false;
            }
        }

        Component->MarkRenderStateDirty();
        UMaterialInterface* RuntimeMaterial = Component->GetMaterial(0);
        if (!RuntimeMaterial || IsBasicShapeMaterial(RuntimeMaterial) || Component->GetStaticMesh() != AuthoredMesh)
        {
            OutFailure = TEXT("park_bench_authored_postcondition_failed");
            return false;
        }

        OutInstanceCount = Component->GetInstanceCount();
        return OutInstanceCount == ExpectedBenchInstances;
    }
}

bool UOCParkSemanticAuthoredUpgradeSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCParkSemanticAuthoredUpgradeSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    BenchPreloadHandle.Reset();
    bPreloadRequested = false;
    bFinished = false;
    bSucceeded = false;

    if (!InWorld.IsGameWorld()) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    BeginBenchPreload();
}

void UOCParkSemanticAuthoredUpgradeSubsystem::BeginBenchPreload()
{
    if (bPreloadRequested || bFinished) return;
    bPreloadRequested = true;

    TArray<FSoftObjectPath> Paths;
    Paths.Emplace(AuthoredBenchPath);
    BenchPreloadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
        Paths,
        FStreamableDelegate::CreateUObject(this, &UOCParkSemanticAuthoredUpgradeSubsystem::HandleBenchPreloadComplete),
        FStreamableManager::AsyncLoadHighPriority,
        false,
        false,
        TEXT("GameRecoveryParkBenchPreload"));

    if (!BenchPreloadHandle.IsValid())
    {
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_AUTHORED_PARK_SEMANTIC_CONTENT_GAP bench_mesh_loaded=0 expected=SM_Bench_1 family=ParkBenches pre_spawn=1 async_preload=0 sync_load=0 gate_k_complete=0 runtime_acceptance=0"));
        return;
    }

    UE_LOG(LogTemp, Display,
        TEXT("GAME_RECOVERY_PARK_BENCH_PRELOAD_BEGIN assets=1 async=1 pre_spawn=1 sync_load=0"));
}

void UOCParkSemanticAuthoredUpgradeSubsystem::HandleBenchPreloadComplete()
{
    if (bFinished) return;

    UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld())
    {
        bFinished = true;
        return;
    }

    AOCWorldSectorOster* Sector = nullptr;
    int32 SectorCount = 0;
    for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It)
    {
        Sector = *It;
        ++SectorCount;
    }
    if (SectorCount != 1 || !Sector)
    {
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_AUTHORED_PARK_SEMANTIC_FAIL reason=oster_sector_count_%d pre_spawn=1 sync_load=0 gate_k_complete=0 runtime_acceptance=0"),
            SectorCount);
        return;
    }

    UStaticMesh* BenchMesh = Cast<UStaticMesh>(FSoftObjectPath(AuthoredBenchPath).ResolveObject());
    if (!BenchMesh)
    {
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_AUTHORED_PARK_SEMANTIC_CONTENT_GAP bench_mesh_loaded=0 expected=SM_Bench_1 family=ParkBenches pre_spawn=1 async_preload=1 sync_load=0 resident_only=1 gate_k_complete=0 runtime_acceptance=0"));
        return;
    }

    UInstancedStaticMeshComponent* ParkBenches = FindISM(Sector, TEXT("ParkBenches"));
    int32 BenchInstances = 0;
    FString Failure;
    const bool bBenchReady = UpgradeBenchFamily(ParkBenches, BenchMesh, BenchInstances, Failure);
    bFinished = true;
    bSucceeded = bBenchReady && BenchInstances == ExpectedBenchInstances;

    if (!bSucceeded)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_AUTHORED_PARK_SEMANTIC_FAIL family=ParkBenches ready=%d instances=%d expected=%d reason=%s pre_spawn=1 sync_load=0 gate_k_complete=0 runtime_acceptance=0"),
            bBenchReady ? 1 : 0,
            BenchInstances,
            ExpectedBenchInstances,
            *Failure);
        return;
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_AUTHORED_PARK_BENCHES_READY bench_mesh=SM_Bench_1 bench_pack=Mega_Street_Props_Pack bench_instances=14 semantic_owner=ParkBenches basicshape_meshes=0 basicshape_material_overrides=0 uniform_scale=1 native_proportions_preserved=1 ground_bottom_preserved=1 bounds_aware_upgrade=1 async_preloaded=1 prerequisite_resident=1 pre_spawn=1 sync_load=0 gate_k_complete=0 runtime_acceptance=0"));
}

void UOCParkSemanticAuthoredUpgradeSubsystem::Deinitialize()
{
    if (BenchPreloadHandle.IsValid()) BenchPreloadHandle->CancelHandle();
    BenchPreloadHandle.Reset();
    bPreloadRequested = false;
    bFinished = true;
    bSucceeded = false;
    Super::Deinitialize();
}
