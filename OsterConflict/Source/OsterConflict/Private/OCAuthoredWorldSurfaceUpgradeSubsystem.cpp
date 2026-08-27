#include "OCAuthoredWorldSurfaceUpgradeSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInterface.h"

namespace
{
    const TCHAR* AuthoredRoadPath =
        TEXT("/Game/Scene_RoadsideConstruction/Assets/Custom/Urb_Roa_Asphalt_01/SM_Urb_Roa_Asphalt_01.SM_Urb_Roa_Asphalt_01");
    const TCHAR* AuthoredSidewalkPath =
        TEXT("/Game/Scene_RoadsideConstruction/Assets/Custom/Urb_Roa_Sidewalk_01/SM_Urb_Roa_Sidewalk_01.SM_Urb_Roa_Sidewalk_01");

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

    bool UpgradeCubeFamily(
        UInstancedStaticMeshComponent* Component,
        UStaticMesh* AuthoredMesh,
        int32& OutInstanceCount,
        FString& OutFailure)
    {
        OutInstanceCount = 0;
        if (!Component || !AuthoredMesh)
        {
            OutFailure = TEXT("component_or_authored_mesh_missing");
            return false;
        }

        UStaticMesh* CurrentMesh = Component->GetStaticMesh();
        if (CurrentMesh == AuthoredMesh)
        {
            OutInstanceCount = Component->GetInstanceCount();
            UMaterialInterface* RuntimeMaterial = Component->GetMaterial(0);
            if (!RuntimeMaterial || IsBasicShapeMaterial(RuntimeMaterial))
            {
                OutFailure = TEXT("authored_family_material_not_ready");
                return false;
            }
            return true;
        }

        if (!IsEngineCube(CurrentMesh))
        {
            OutFailure = FString::Printf(TEXT("unexpected_source_mesh_%s"),
                CurrentMesh ? *CurrentMesh->GetPathName() : TEXT("null"));
            return false;
        }

        const FBoxSphereBounds NewBounds = AuthoredMesh->GetBounds();
        const FVector NativeSize = NewBounds.BoxExtent * 2.0f;
        if (NativeSize.X <= 1.0f || NativeSize.Y <= 1.0f)
        {
            OutFailure = TEXT("authored_mesh_xy_bounds_invalid");
            return false;
        }

        TArray<FTransform> OldTransforms;
        OldTransforms.Reserve(Component->GetInstanceCount());
        for (int32 Index = 0; Index < Component->GetInstanceCount(); ++Index)
        {
            FTransform Transform;
            if (!Component->GetInstanceTransform(Index, Transform, false))
            {
                OutFailure = FString::Printf(TEXT("source_transform_read_failed_%d"), Index);
                return false;
            }
            OldTransforms.Add(Transform);
        }

        Component->SetStaticMesh(AuthoredMesh);
        Component->EmptyOverrideMaterials();

        const bool bNativeLongAxisY = NativeSize.Y > NativeSize.X * 1.05f;
        for (int32 Index = 0; Index < OldTransforms.Num(); ++Index)
        {
            const FTransform& Old = OldTransforms[Index];
            const FVector OldScale = Old.GetScale3D().GetAbs();

            // The retired Engine Cube is exactly 100 cm per axis, so its old scale is factual desired size in metres.
            const FVector DesiredSizeCm = OldScale * 100.0f;
            FVector NewScale;
            FRotator NewRotation = Old.Rotator();
            if (bNativeLongAxisY)
            {
                NewScale.X = DesiredSizeCm.Y / NativeSize.X;
                NewScale.Y = DesiredSizeCm.X / NativeSize.Y;
                NewRotation.Yaw -= 90.0f;
            }
            else
            {
                NewScale.X = DesiredSizeCm.X / NativeSize.X;
                NewScale.Y = DesiredSizeCm.Y / NativeSize.Y;
            }
            NewScale.Z = NativeSize.Z > 1.0f
                ? FMath::Max(0.05f, DesiredSizeCm.Z / NativeSize.Z)
                : 1.0f;

            const FQuat RotationQuat = NewRotation.Quaternion();
            const FVector ScaledBoundsOrigin(
                NewBounds.Origin.X * NewScale.X,
                NewBounds.Origin.Y * NewScale.Y,
                NewBounds.Origin.Z * NewScale.Z);
            const FVector NewLocation = Old.GetLocation() - RotationQuat.RotateVector(ScaledBoundsOrigin);
            const FTransform NewTransform(RotationQuat, NewLocation, NewScale);

            if (!Component->UpdateInstanceTransform(Index, NewTransform, false, false, true))
            {
                OutFailure = FString::Printf(TEXT("authored_transform_write_failed_%d"), Index);
                return false;
            }
        }

        Component->MarkRenderStateDirty();
        OutInstanceCount = Component->GetInstanceCount();

        UMaterialInterface* RuntimeMaterial = Component->GetMaterial(0);
        if (!RuntimeMaterial || IsBasicShapeMaterial(RuntimeMaterial))
        {
            OutFailure = TEXT("authored_packaged_material_missing_or_basicshape");
            return false;
        }
        return true;
    }
}

bool UOCAuthoredWorldSurfaceUpgradeSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

TStatId UOCAuthoredWorldSurfaceUpgradeSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCAuthoredWorldSurfaceUpgradeSubsystem, STATGROUP_Tickables);
}

void UOCAuthoredWorldSurfaceUpgradeSubsystem::Tick(float DeltaTime)
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
    if (ElapsedSeconds < 0.75f) return;

    AOCWorldSectorOster* Sector = nullptr;
    int32 SectorCount = 0;
    for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It)
    {
        Sector = *It;
        ++SectorCount;
    }
    if (SectorCount != 1 || !Sector)
    {
        if (ElapsedSeconds < 5.0f) return;
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_AUTHORED_WORLD_SURFACE_FAIL reason=oster_sector_count_%d"), SectorCount);
        return;
    }

    UStaticMesh* RoadMesh = LoadObject<UStaticMesh>(nullptr, AuthoredRoadPath);
    UStaticMesh* SidewalkMesh = LoadObject<UStaticMesh>(nullptr, AuthoredSidewalkPath);
    if (!RoadMesh || !SidewalkMesh)
    {
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_AUTHORED_WORLD_SURFACE_CONTENT_GAP road_loaded=%d sidewalk_loaded=%d tracked_pack=Scene_RoadsideConstruction gate_k_complete=0"),
            RoadMesh ? 1 : 0,
            SidewalkMesh ? 1 : 0);
        return;
    }

    UInstancedStaticMeshComponent* Roads = FindISM(Sector, TEXT("Roads"));
    UInstancedStaticMeshComponent* Sidewalks = FindISM(Sector, TEXT("Sidewalks"));

    int32 RoadInstances = 0;
    int32 SidewalkInstances = 0;
    FString RoadFailure;
    FString SidewalkFailure;
    const bool bRoadsReady = UpgradeCubeFamily(Roads, RoadMesh, RoadInstances, RoadFailure);
    const bool bSidewalksReady = UpgradeCubeFamily(Sidewalks, SidewalkMesh, SidewalkInstances, SidewalkFailure);

    bFinished = true;
    if (!bRoadsReady || !bSidewalksReady)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_AUTHORED_WORLD_SURFACE_FAIL roads_ready=%d sidewalks_ready=%d road_reason=%s sidewalk_reason=%s gate_k_complete=0"),
            bRoadsReady ? 1 : 0,
            bSidewalksReady ? 1 : 0,
            *RoadFailure,
            *SidewalkFailure);
        return;
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_AUTHORED_ROAD_SURFACE_READY roads_mesh=SM_Urb_Roa_Asphalt_01 sidewalks_mesh=SM_Urb_Roa_Sidewalk_01 road_instances=%d sidewalk_instances=%d basicshape_meshes=0 basicshape_material_overrides=0 topology_preserved=1 pass12_baseline_deadline_s=12"),
        RoadInstances,
        SidewalkInstances);
}
