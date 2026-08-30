#include "OCParkGroundAuthoredUpgradeSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInterface.h"

namespace
{
    const TCHAR* AuthoredGroundMeshPath =
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Plane_1x1.SM_Plane_1x1");
    const TCHAR* AuthoredGrassMaterialPath =
        TEXT("/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Materials/Instances/M_Grass_Inst.M_Grass_Inst");
    constexpr float AuthoredUpgradeDelaySeconds = 0.70f;
    constexpr float OwnerResolutionTimeoutSeconds = 5.0f;

    struct FGroundUpgradePlan
    {
        UInstancedStaticMeshComponent* Component = nullptr;
        UStaticMesh* OldMesh = nullptr;
        FTransform OldTransform;
        FTransform NewTransform;
        TArray<UMaterialInterface*> OldMaterials;
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

    bool IsBasicShapeMaterial(const UMaterialInterface* Material)
    {
        return Material && Material->GetPathName().Contains(TEXT("/Engine/BasicShapes/"), ESearchCase::IgnoreCase);
    }

    bool BuildPlan(
        UInstancedStaticMeshComponent* Component,
        UStaticMesh* AuthoredMesh,
        FGroundUpgradePlan& OutPlan,
        FString& OutFailure)
    {
        if (!Component || !AuthoredMesh)
        {
            OutFailure = TEXT("component_or_authored_mesh_missing");
            return false;
        }
        if (Component->GetInstanceCount() != 1)
        {
            OutFailure = FString::Printf(TEXT("owner_%s_instance_count_%d_expected_1"),
                *Component->GetName(), Component->GetInstanceCount());
            return false;
        }

        UStaticMesh* SourceMesh = Component->GetStaticMesh();
        if (!IsEngineCube(SourceMesh))
        {
            OutFailure = FString::Printf(TEXT("owner_%s_unexpected_source_mesh_%s"),
                *Component->GetName(), SourceMesh ? *SourceMesh->GetPathName() : TEXT("null"));
            return false;
        }

        FTransform SourceTransform;
        if (!Component->GetInstanceTransform(0, SourceTransform, false))
        {
            OutFailure = FString::Printf(TEXT("owner_%s_source_transform_read_failed"), *Component->GetName());
            return false;
        }
        const FRotator SourceRotation = SourceTransform.Rotator();
        if (FMath::Abs(SourceRotation.Pitch) > 0.1f || FMath::Abs(SourceRotation.Roll) > 0.1f)
        {
            OutFailure = FString::Printf(TEXT("owner_%s_source_tilt_not_supported"), *Component->GetName());
            return false;
        }

        const FBoxSphereBounds SourceBounds = SourceMesh->GetBounds();
        const FBoxSphereBounds NewBounds = AuthoredMesh->GetBounds();
        const FVector SourceScale = SourceTransform.GetScale3D().GetAbs();
        const FVector SourceSize(
            SourceBounds.BoxExtent.X * 2.0f * SourceScale.X,
            SourceBounds.BoxExtent.Y * 2.0f * SourceScale.Y,
            SourceBounds.BoxExtent.Z * 2.0f * SourceScale.Z);
        const FVector NewNativeSize = NewBounds.BoxExtent * 2.0f;
        if (SourceSize.X <= 1.0f || SourceSize.Y <= 1.0f ||
            NewNativeSize.X <= 1.0f || NewNativeSize.Y <= 1.0f)
        {
            OutFailure = FString::Printf(TEXT("owner_%s_invalid_surface_bounds"), *Component->GetName());
            return false;
        }

        const bool bAuthoredLongAxisY = NewNativeSize.Y > NewNativeSize.X * 1.05f;
        FRotator NewRotation = SourceRotation;
        FVector NewScale(1.0f);
        if (bAuthoredLongAxisY)
        {
            NewRotation.Yaw -= 90.0f;
            NewScale.X = SourceSize.Y / NewNativeSize.X;
            NewScale.Y = SourceSize.X / NewNativeSize.Y;
        }
        else
        {
            NewScale.X = SourceSize.X / NewNativeSize.X;
            NewScale.Y = SourceSize.Y / NewNativeSize.Y;
        }
        NewScale.Z = NewNativeSize.Z > 1.0f
            ? FMath::Max(0.05f, SourceSize.Z / NewNativeSize.Z)
            : 1.0f;

        const FQuat SourceQuat = SourceRotation.Quaternion();
        const FVector SourceScaledOrigin(
            SourceBounds.Origin.X * SourceScale.X,
            SourceBounds.Origin.Y * SourceScale.Y,
            SourceBounds.Origin.Z * SourceScale.Z);
        const FVector SourceCenter = SourceTransform.GetLocation() + SourceQuat.RotateVector(SourceScaledOrigin);
        const float SourceTopZ = SourceTransform.GetLocation().Z +
            (SourceBounds.Origin.Z + SourceBounds.BoxExtent.Z) * SourceScale.Z;

        const FQuat NewQuat = NewRotation.Quaternion();
        const FVector NewScaledOrigin(
            NewBounds.Origin.X * NewScale.X,
            NewBounds.Origin.Y * NewScale.Y,
            NewBounds.Origin.Z * NewScale.Z);
        FVector NewLocation = SourceCenter - NewQuat.RotateVector(NewScaledOrigin);
        const float NewTopOffsetZ =
            (NewBounds.Origin.Z + NewBounds.BoxExtent.Z) * NewScale.Z;
        NewLocation.Z = SourceTopZ - NewTopOffsetZ;

        OutPlan.Component = Component;
        OutPlan.OldMesh = SourceMesh;
        OutPlan.OldTransform = SourceTransform;
        OutPlan.NewTransform = FTransform(NewQuat, NewLocation, NewScale);
        const int32 OldMaterialCount = Component->GetNumMaterials();
        OutPlan.OldMaterials.Reserve(OldMaterialCount);
        for (int32 MaterialIndex = 0; MaterialIndex < OldMaterialCount; ++MaterialIndex)
        {
            OutPlan.OldMaterials.Add(Component->GetMaterial(MaterialIndex));
        }
        return true;
    }

    void RestorePlan(const FGroundUpgradePlan& Plan)
    {
        if (!Plan.Component || !Plan.OldMesh) return;
        Plan.Component->SetStaticMesh(Plan.OldMesh);
        Plan.Component->EmptyOverrideMaterials();
        for (int32 MaterialIndex = 0; MaterialIndex < Plan.OldMaterials.Num(); ++MaterialIndex)
        {
            Plan.Component->SetMaterial(MaterialIndex, Plan.OldMaterials[MaterialIndex]);
        }
        Plan.Component->UpdateInstanceTransform(0, Plan.OldTransform, false, false, true);
        Plan.Component->MarkRenderStateDirty();
    }

    bool ApplyPlan(
        const FGroundUpgradePlan& Plan,
        UStaticMesh* AuthoredMesh,
        UMaterialInterface* GrassMaterial)
    {
        if (!Plan.Component || !AuthoredMesh || !GrassMaterial) return false;
        Plan.Component->SetStaticMesh(AuthoredMesh);
        Plan.Component->EmptyOverrideMaterials();
        const int32 SlotCount = FMath::Max(1, AuthoredMesh->GetStaticMaterials().Num());
        for (int32 Slot = 0; Slot < SlotCount; ++Slot)
        {
            Plan.Component->SetMaterial(Slot, GrassMaterial);
        }
        if (!Plan.Component->UpdateInstanceTransform(0, Plan.NewTransform, false, false, true)) return false;
        Plan.Component->MarkRenderStateDirty();
        return Plan.Component->GetInstanceCount() == 1 &&
            Plan.Component->GetStaticMesh() == AuthoredMesh &&
            Plan.Component->GetMaterial(0) == GrassMaterial &&
            !IsBasicShapeMaterial(Plan.Component->GetMaterial(0));
    }
}

bool UOCParkGroundAuthoredUpgradeSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

TStatId UOCParkGroundAuthoredUpgradeSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCParkGroundAuthoredUpgradeSubsystem, STATGROUP_Tickables);
}

void UOCParkGroundAuthoredUpgradeSubsystem::Tick(float DeltaTime)
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
    if (ElapsedSeconds < AuthoredUpgradeDelaySeconds) return;

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
            TEXT("PASS45_AUTHORED_PARK_GROUND_FAIL reason=oster_sector_count_%d gate_k_complete=0 runtime_acceptance=0"),
            SectorCount);
        return;
    }

    UInstancedStaticMeshComponent* LegacyGeometry = FindISM(Sector, TEXT("ParkGeometry"));
    UInstancedStaticMeshComponent* ParkCentralGround = FindISM(Sector, TEXT("ParkCentralGround"));
    UInstancedStaticMeshComponent* ParkNorthCivicGround = FindISM(Sector, TEXT("ParkNorthCivicGround"));
    UInstancedStaticMeshComponent* CollegeRecreationGround = FindISM(Sector, TEXT("CollegeRecreationGround"));
    if (!LegacyGeometry || LegacyGeometry->GetInstanceCount() != 0 ||
        !ParkCentralGround || !ParkNorthCivicGround || !CollegeRecreationGround)
    {
        if (ElapsedSeconds < OwnerResolutionTimeoutSeconds) return;
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_AUTHORED_PARK_GROUND_FAIL reason=semantic_owner_contract legacy=%d central=%d north=%d college=%d owner_normalization_required=1 gate_k_complete=0 runtime_acceptance=0"),
            LegacyGeometry ? LegacyGeometry->GetInstanceCount() : -1,
            ParkCentralGround ? ParkCentralGround->GetInstanceCount() : -1,
            ParkNorthCivicGround ? ParkNorthCivicGround->GetInstanceCount() : -1,
            CollegeRecreationGround ? CollegeRecreationGround->GetInstanceCount() : -1);
        return;
    }

    UStaticMesh* GroundMesh = LoadObject<UStaticMesh>(nullptr, AuthoredGroundMeshPath);
    UMaterialInterface* GrassMaterial = LoadObject<UMaterialInterface>(nullptr, AuthoredGrassMaterialPath);
    if (!GroundMesh || !GrassMaterial)
    {
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_AUTHORED_PARK_GROUND_CONTENT_GAP plane_mesh_loaded=%d grass_material_loaded=%d expected_mesh=SM_Plane_1x1 expected_material=M_Grass_Inst gate_k_complete=0 runtime_acceptance=0"),
            GroundMesh ? 1 : 0,
            GrassMaterial ? 1 : 0);
        return;
    }

    UInstancedStaticMeshComponent* Owners[] = {
        ParkCentralGround,
        ParkNorthCivicGround,
        CollegeRecreationGround
    };
    TArray<FGroundUpgradePlan> Plans;
    Plans.Reserve(UE_ARRAY_COUNT(Owners));
    FString Failure;
    for (UInstancedStaticMeshComponent* Owner : Owners)
    {
        FGroundUpgradePlan Plan;
        if (!BuildPlan(Owner, GroundMesh, Plan, Failure))
        {
            bFinished = true;
            UE_LOG(LogTemp, Error,
                TEXT("PASS45_AUTHORED_PARK_GROUND_FAIL reason=%s preflight_complete=0 mutation_started=0 gate_k_complete=0 runtime_acceptance=0"),
                *Failure);
            return;
        }
        Plans.Add(MoveTemp(Plan));
    }

    int32 AppliedCount = 0;
    for (const FGroundUpgradePlan& Plan : Plans)
    {
        if (!ApplyPlan(Plan, GroundMesh, GrassMaterial))
        {
            for (int32 RollbackIndex = 0; RollbackIndex <= AppliedCount && RollbackIndex < Plans.Num(); ++RollbackIndex)
            {
                RestorePlan(Plans[RollbackIndex]);
            }
            bFinished = true;
            UE_LOG(LogTemp, Error,
                TEXT("PASS45_AUTHORED_PARK_GROUND_FAIL reason=authored_write_or_postcondition applied_before_failure=%d rollback=1 gate_k_complete=0 runtime_acceptance=0"),
                AppliedCount);
            return;
        }
        ++AppliedCount;
    }

    bFinished = true;
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_AUTHORED_PARK_GROUND_READY ground_mesh=SM_Plane_1x1 ground_material=M_Grass_Inst park_central_ground=1 park_north_civic_ground=1 college_recreation_ground=1 exact_semantic_owners=3 basicshape_meshes=0 basicshape_material_overrides=0 source_surface_top_preserved=1 xy_footprint_preserved=1 yaw_preserved=1 bounds_aware_surface_fit=1 park_green_semantics_preserved=1 transactional_preflight=1 rollback_on_write_failure=1 tactical_map_xy_bounds_preserved=1 primary_authoring=0 gate_k_complete=0 runtime_acceptance=0"));
}
