#include "OCAuthoredWorldSurfaceUpgradeSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInterface.h"

namespace
{
    const TCHAR* AuthoredGroundMeshPath =
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Plane_1x1.SM_Plane_1x1");
    const TCHAR* AuthoredGroundMaterialPath =
        TEXT("/Game/KiteDemo/Environments/GroundTiles/Grass/M_Ground_Grass2.M_Ground_Grass2");
    const TCHAR* AuthoredRoadPath =
        TEXT("/Game/Scene_RoadsideConstruction/Assets/Custom/Urb_Roa_Asphalt_01/SM_Urb_Roa_Asphalt_01.SM_Urb_Roa_Asphalt_01");
    const TCHAR* AuthoredSidewalkPath =
        TEXT("/Game/Scene_RoadsideConstruction/Assets/Custom/Urb_Roa_Sidewalk_01/SM_Urb_Roa_Sidewalk_01.SM_Urb_Roa_Sidewalk_01");
    const TCHAR* AuthoredParkPathPath =
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Stonepath_Var01.SM_Stonepath_Var01");
    const TCHAR* AuthoredFencePath =
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Fence_Var01.SM_Fence_Var01");

    struct FParkPathProxySpec
    {
        FVector Center = FVector::ZeroVector;
        FVector SizeCm = FVector::ZeroVector;
        float YawDegrees = 0.0f;
    };

    UStaticMeshComponent* FindStaticMeshComponent(AActor* Actor, const FName Name)
    {
        if (!Actor) return nullptr;
        TInlineComponentArray<UStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (UStaticMeshComponent* Component : Components)
        {
            if (Component && Component->GetFName() == Name) return Component;
        }
        return nullptr;
    }

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

    bool UpgradeGroundSurface(
        UStaticMeshComponent* Component,
        UStaticMesh* AuthoredMesh,
        UMaterialInterface* AuthoredMaterial,
        FString& OutFailure)
    {
        if (!Component || !AuthoredMesh || !AuthoredMaterial)
        {
            OutFailure = TEXT("ground_component_mesh_or_material_missing");
            return false;
        }

        UStaticMesh* CurrentMesh = Component->GetStaticMesh();
        if (CurrentMesh == AuthoredMesh)
        {
            if (Component->GetMaterial(0) != AuthoredMaterial)
            {
                OutFailure = TEXT("ground_authored_material_contract_drift");
                return false;
            }
            return true;
        }

        if (!IsEngineCube(CurrentMesh))
        {
            OutFailure = FString::Printf(TEXT("ground_unexpected_source_mesh_%s"),
                CurrentMesh ? *CurrentMesh->GetPathName() : TEXT("null"));
            return false;
        }

        const FTransform Old = Component->GetRelativeTransform();
        const FRotator OldRotation = Old.Rotator();
        if (FMath::Abs(OldRotation.Pitch) > 0.1f || FMath::Abs(OldRotation.Roll) > 0.1f)
        {
            OutFailure = TEXT("ground_source_tilt_not_supported");
            return false;
        }

        const FBoxSphereBounds OldBounds = CurrentMesh->GetBounds();
        const FBoxSphereBounds NewBounds = AuthoredMesh->GetBounds();
        const FVector OldNativeSize = OldBounds.BoxExtent * 2.0f;
        const FVector NewNativeSize = NewBounds.BoxExtent * 2.0f;
        if (OldNativeSize.X <= 1.0f || OldNativeSize.Y <= 1.0f ||
            NewNativeSize.X <= 1.0f || NewNativeSize.Y <= 1.0f)
        {
            OutFailure = TEXT("ground_xy_bounds_invalid");
            return false;
        }

        const FVector OldScale = Old.GetScale3D().GetAbs();
        const FVector DesiredSizeCm(
            OldNativeSize.X * OldScale.X,
            OldNativeSize.Y * OldScale.Y,
            OldNativeSize.Z * OldScale.Z);
        const FVector NewScale(
            DesiredSizeCm.X / NewNativeSize.X,
            DesiredSizeCm.Y / NewNativeSize.Y,
            1.0f);

        const FQuat RotationQuat = OldRotation.Quaternion();
        const FVector OldScaledOrigin(
            OldBounds.Origin.X * OldScale.X,
            OldBounds.Origin.Y * OldScale.Y,
            OldBounds.Origin.Z * OldScale.Z);
        const FVector SurfaceCenter = Old.GetLocation() + RotationQuat.RotateVector(OldScaledOrigin);
        const FVector NewScaledOrigin(
            NewBounds.Origin.X * NewScale.X,
            NewBounds.Origin.Y * NewScale.Y,
            NewBounds.Origin.Z * NewScale.Z);
        FVector NewLocation = SurfaceCenter - RotationQuat.RotateVector(NewScaledOrigin);

        const float OldTopZ = Old.GetLocation().Z +
            (OldBounds.Origin.Z + OldBounds.BoxExtent.Z) * OldScale.Z;
        const float NewTopOffsetZ = RotationQuat.RotateVector(FVector(
            0.0f,
            0.0f,
            (NewBounds.Origin.Z + NewBounds.BoxExtent.Z) * NewScale.Z)).Z;
        NewLocation.Z = OldTopZ - NewTopOffsetZ;

        Component->SetStaticMesh(AuthoredMesh);
        Component->EmptyOverrideMaterials();
        Component->SetMaterial(0, AuthoredMaterial);
        Component->SetRelativeTransform(FTransform(RotationQuat, NewLocation, NewScale));
        Component->MarkRenderStateDirty();

        UMaterialInterface* RuntimeMaterial = Component->GetMaterial(0);
        if (Component->GetStaticMesh() != AuthoredMesh || RuntimeMaterial != AuthoredMaterial ||
            IsBasicShapeMaterial(RuntimeMaterial))
        {
            OutFailure = TEXT("ground_authored_surface_postcondition_failed");
            return false;
        }
        return true;
    }

    TArray<FParkPathProxySpec> BuildExpectedParkPathProxySpecs()
    {
        const FVector Park = AOCWorldSectorOster::ParkAnchor();
        const FVector NorthCivic = AOCWorldSectorOster::CultureParkNorthAnchor();
        const FVector Mid = (Park + NorthCivic) * 0.5f;
        const FVector Delta = NorthCivic - Park;
        const float LinkYaw = FMath::RadiansToDegrees(FMath::Atan2(Delta.Y, Delta.X));

        TArray<FParkPathProxySpec> Specs;
        Specs.Reserve(5);
        Specs.Add({ Park + FVector(0, 0, 14), FVector(17800, 360, 18), 0.0f });
        Specs.Add({ Park + FVector(0, -300, 14), FVector(360, 13200, 18), 0.0f });
        Specs.Add({ Park + FVector(1800, 900, 14), FVector(11800, 260, 18), 31.0f });
        Specs.Add({ Park + FVector(-2300, 1300, 14), FVector(9300, 240, 18), -28.0f });
        Specs.Add({ Mid + FVector(0, 0, 15), FVector(Delta.Size2D(), 260, 18), LinkYaw });
        return Specs;
    }

    bool MatchesParkPathProxy(const FTransform& Transform, const FParkPathProxySpec& Spec)
    {
        const FVector ExpectedScale = Spec.SizeCm / 100.0f;
        return Transform.GetLocation().Equals(Spec.Center, 0.5f) &&
            Transform.GetScale3D().GetAbs().Equals(ExpectedScale.GetAbs(), 0.01f) &&
            FMath::Abs(FMath::FindDeltaAngleDegrees(Transform.Rotator().Yaw, Spec.YawDegrees)) <= 0.1f;
    }

    int32 CountExpectedParkPathProxies(UInstancedStaticMeshComponent* Component, const TArray<FParkPathProxySpec>& Specs)
    {
        if (!Component) return 0;
        int32 MatchCount = 0;
        for (int32 Index = 0; Index < Component->GetInstanceCount(); ++Index)
        {
            FTransform Transform;
            if (!Component->GetInstanceTransform(Index, Transform, false)) continue;
            for (const FParkPathProxySpec& Spec : Specs)
            {
                if (MatchesParkPathProxy(Transform, Spec))
                {
                    ++MatchCount;
                    break;
                }
            }
        }
        return MatchCount;
    }

    bool SeparateParkPathFamily(
        AOCWorldSectorOster* Sector,
        UInstancedStaticMeshComponent* Sidewalks,
        UInstancedStaticMeshComponent*& OutParkPaths,
        int32& OutParkPathCount,
        FString& OutFailure)
    {
        OutParkPaths = nullptr;
        OutParkPathCount = 0;
        if (!Sector || !Sidewalks)
        {
            OutFailure = TEXT("sector_or_sidewalks_missing");
            return false;
        }

        const TArray<FParkPathProxySpec> Specs = BuildExpectedParkPathProxySpecs();
        if (Specs.Num() != 5)
        {
            OutFailure = FString::Printf(TEXT("park_path_spec_count_%d"), Specs.Num());
            return false;
        }

        if (UInstancedStaticMeshComponent* ExistingParkPaths = FindISM(Sector, TEXT("ParkPaths")))
        {
            const int32 RemainingInSidewalks = CountExpectedParkPathProxies(Sidewalks, Specs);
            if (ExistingParkPaths->GetInstanceCount() != 5 || RemainingInSidewalks != 0)
            {
                OutFailure = FString::Printf(TEXT("park_path_existing_contract_instances_%d_sidewalk_matches_%d"),
                    ExistingParkPaths->GetInstanceCount(), RemainingInSidewalks);
                return false;
            }
            OutParkPaths = ExistingParkPaths;
            OutParkPathCount = 5;
            return true;
        }

        if (!IsEngineCube(Sidewalks->GetStaticMesh()))
        {
            OutFailure = TEXT("park_path_split_requires_source_cube_sidewalks");
            return false;
        }

        TArray<int32> SourceIndices;
        TArray<FTransform> SourceTransforms;
        SourceIndices.Reserve(5);
        SourceTransforms.Reserve(5);

        for (int32 SpecIndex = 0; SpecIndex < Specs.Num(); ++SpecIndex)
        {
            int32 FoundIndex = INDEX_NONE;
            FTransform FoundTransform;
            int32 FoundCount = 0;
            for (int32 Index = 0; Index < Sidewalks->GetInstanceCount(); ++Index)
            {
                FTransform Transform;
                if (!Sidewalks->GetInstanceTransform(Index, Transform, false))
                {
                    OutFailure = FString::Printf(TEXT("park_path_source_transform_read_failed_%d"), Index);
                    return false;
                }
                if (!MatchesParkPathProxy(Transform, Specs[SpecIndex])) continue;
                ++FoundCount;
                FoundIndex = Index;
                FoundTransform = Transform;
            }

            if (FoundCount != 1 || FoundIndex == INDEX_NONE || SourceIndices.Contains(FoundIndex))
            {
                OutFailure = FString::Printf(TEXT("park_path_spec_%d_matches_%d"), SpecIndex, FoundCount);
                return false;
            }
            SourceIndices.Add(FoundIndex);
            SourceTransforms.Add(FoundTransform);
        }

        if (SourceIndices.Num() != 5 || SourceTransforms.Num() != 5)
        {
            OutFailure = TEXT("park_path_preflight_not_exactly_five");
            return false;
        }

        UInstancedStaticMeshComponent* ParkPaths = NewObject<UInstancedStaticMeshComponent>(Sector, TEXT("ParkPaths"));
        if (!ParkPaths)
        {
            OutFailure = TEXT("park_paths_component_create_failed");
            return false;
        }
        ParkPaths->SetupAttachment(Sector->GetRootComponent());
        ParkPaths->SetCollisionProfileName(Sidewalks->GetCollisionProfileName());
        ParkPaths->SetMobility(Sidewalks->Mobility);
        ParkPaths->SetStaticMesh(Sidewalks->GetStaticMesh());
        Sector->AddInstanceComponent(ParkPaths);
        ParkPaths->RegisterComponent();

        for (const FTransform& Transform : SourceTransforms)
        {
            ParkPaths->AddInstance(Transform);
        }
        if (ParkPaths->GetInstanceCount() != 5)
        {
            ParkPaths->DestroyComponent();
            OutFailure = FString::Printf(TEXT("park_paths_copy_count_%d"), ParkPaths->GetInstanceCount());
            return false;
        }

        SourceIndices.Sort([](const int32 A, const int32 B) { return A > B; });
        for (const int32 Index : SourceIndices)
        {
            if (!Sidewalks->RemoveInstance(Index))
            {
                ParkPaths->DestroyComponent();
                OutFailure = FString::Printf(TEXT("park_path_source_remove_failed_%d"), Index);
                return false;
            }
        }
        Sidewalks->MarkRenderStateDirty();
        ParkPaths->MarkRenderStateDirty();

        const int32 RemainingInSidewalks = CountExpectedParkPathProxies(Sidewalks, Specs);
        if (RemainingInSidewalks != 0 || ParkPaths->GetInstanceCount() != 5)
        {
            OutFailure = FString::Printf(TEXT("park_path_postsplit_sidewalk_matches_%d_park_instances_%d"),
                RemainingInSidewalks, ParkPaths->GetInstanceCount());
            return false;
        }

        OutParkPaths = ParkPaths;
        OutParkPathCount = 5;
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_PARK_PATH_OWNERSHIP_READY component=ParkPaths park_path_instances=5 sidewalk_park_path_matches=0 source_proxy_count=5"));
        return true;
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
            const bool bDesiredLongAxisY = DesiredSizeCm.Y > DesiredSizeCm.X * 1.05f;
            FVector NewScale;
            FRotator NewRotation = Old.Rotator();

            // Preserve the source footprint even when an authored mesh's native long axis differs from the Cube family.
            // This matters for Fences and ParkPaths, whose canonical topology contains mixed source orientations.
            if (bNativeLongAxisY != bDesiredLongAxisY)
            {
                NewScale.X = DesiredSizeCm.Y / NativeSize.X;
                NewScale.Y = DesiredSizeCm.X / NativeSize.Y;
                NewRotation.Yaw += bNativeLongAxisY ? -90.0f : 90.0f;
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

    UStaticMesh* GroundMesh = LoadObject<UStaticMesh>(nullptr, AuthoredGroundMeshPath);
    UMaterialInterface* GroundMaterial = LoadObject<UMaterialInterface>(nullptr, AuthoredGroundMaterialPath);
    UStaticMesh* RoadMesh = LoadObject<UStaticMesh>(nullptr, AuthoredRoadPath);
    UStaticMesh* SidewalkMesh = LoadObject<UStaticMesh>(nullptr, AuthoredSidewalkPath);
    UStaticMesh* ParkPathMesh = LoadObject<UStaticMesh>(nullptr, AuthoredParkPathPath);
    UStaticMesh* FenceMesh = LoadObject<UStaticMesh>(nullptr, AuthoredFencePath);
    if (!GroundMesh || !GroundMaterial || !RoadMesh || !SidewalkMesh || !ParkPathMesh || !FenceMesh)
    {
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_AUTHORED_WORLD_SURFACE_CONTENT_GAP ground_mesh_loaded=%d ground_material_loaded=%d road_loaded=%d sidewalk_loaded=%d park_path_loaded=%d fence_loaded=%d tracked_ground_pack=KiteDemo tracked_road_pack=Scene_RoadsideConstruction tracked_park_pack=AdvancedVillagePack tracked_fence_pack=AdvancedVillagePack gate_k_complete=0"),
            GroundMesh ? 1 : 0,
            GroundMaterial ? 1 : 0,
            RoadMesh ? 1 : 0,
            SidewalkMesh ? 1 : 0,
            ParkPathMesh ? 1 : 0,
            FenceMesh ? 1 : 0);
        return;
    }

    UStaticMeshComponent* Ground = FindStaticMeshComponent(Sector, TEXT("Ground"));
    UInstancedStaticMeshComponent* Roads = FindISM(Sector, TEXT("Roads"));
    UInstancedStaticMeshComponent* Sidewalks = FindISM(Sector, TEXT("Sidewalks"));
    UInstancedStaticMeshComponent* Fences = FindISM(Sector, TEXT("Fences"));
    UInstancedStaticMeshComponent* ParkPaths = nullptr;

    int32 SeparatedParkPathInstances = 0;
    FString ParkPathOwnershipFailure;
    const bool bParkPathOwnershipReady = SeparateParkPathFamily(
        Sector, Sidewalks, ParkPaths, SeparatedParkPathInstances, ParkPathOwnershipFailure);
    if (!bParkPathOwnershipReady)
    {
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_AUTHORED_WORLD_SURFACE_FAIL park_path_ownership_ready=0 park_path_reason=%s gate_k_complete=0"),
            *ParkPathOwnershipFailure);
        return;
    }

    int32 RoadInstances = 0;
    int32 SidewalkInstances = 0;
    int32 ParkPathInstances = 0;
    int32 FenceInstances = 0;
    FString GroundFailure;
    FString RoadFailure;
    FString SidewalkFailure;
    FString ParkPathFailure;
    FString FenceFailure;
    const bool bGroundReady = UpgradeGroundSurface(Ground, GroundMesh, GroundMaterial, GroundFailure);
    const bool bRoadsReady = UpgradeCubeFamily(Roads, RoadMesh, RoadInstances, RoadFailure);
    const bool bSidewalksReady = UpgradeCubeFamily(Sidewalks, SidewalkMesh, SidewalkInstances, SidewalkFailure);
    const bool bParkPathsReady = UpgradeCubeFamily(ParkPaths, ParkPathMesh, ParkPathInstances, ParkPathFailure);
    const bool bFencesReady = UpgradeCubeFamily(Fences, FenceMesh, FenceInstances, FenceFailure);

    bFinished = true;
    if (!bGroundReady || !bRoadsReady || !bSidewalksReady || !bParkPathsReady || !bFencesReady ||
        SeparatedParkPathInstances != 5 || ParkPathInstances != 5)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_AUTHORED_WORLD_SURFACE_FAIL ground_ready=%d roads_ready=%d sidewalks_ready=%d park_paths_ready=%d fences_ready=%d park_path_source_instances=%d park_path_runtime_instances=%d ground_reason=%s road_reason=%s sidewalk_reason=%s park_path_reason=%s fence_reason=%s gate_k_complete=0"),
            bGroundReady ? 1 : 0,
            bRoadsReady ? 1 : 0,
            bSidewalksReady ? 1 : 0,
            bParkPathsReady ? 1 : 0,
            bFencesReady ? 1 : 0,
            SeparatedParkPathInstances,
            ParkPathInstances,
            *GroundFailure,
            *RoadFailure,
            *SidewalkFailure,
            *ParkPathFailure,
            *FenceFailure);
        return;
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_AUTHORED_GROUND_SURFACE_READY ground_mesh=SM_Plane_1x1 ground_material=M_Ground_Grass2 ground_pack=KiteDemo content_intake_ground_selected=1 basicshape_meshes=0 basicshape_material_overrides=0 playable_footprint_preserved=1 ground_top_z_preserved=1 bounds_aware_upgrade=1 pass12_baseline_deadline_s=12"));
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_AUTHORED_ROAD_SURFACE_READY roads_mesh=SM_Urb_Roa_Asphalt_01 sidewalks_mesh=SM_Urb_Roa_Sidewalk_01 road_instances=%d sidewalk_instances=%d basicshape_meshes=0 basicshape_material_overrides=0 topology_preserved=1 pass12_baseline_deadline_s=12"),
        RoadInstances,
        SidewalkInstances);
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_AUTHORED_PARK_PATH_SURFACE_READY park_paths_mesh=SM_Stonepath_Var01 park_path_instances=%d sidewalk_park_path_matches=0 basicshape_meshes=0 basicshape_material_overrides=0 topology_preserved=1 bounds_aware_upgrade=1 gate_k_complete=0"),
        ParkPathInstances);
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_AUTHORED_WORLD_FENCE_READY fence_mesh=SM_Fence_Var01 fence_instances=%d basicshape_meshes=0 basicshape_material_overrides=0 topology_preserved=1 gate_k_complete=0"),
        FenceInstances);
}
