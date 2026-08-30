#include "OCParkHardscapeAuthoredUpgradeSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInterface.h"

namespace
{
    const TCHAR* AuthoredSurfaceMeshPath =
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Plane_1x1.SM_Plane_1x1");
    const TCHAR* AuthoredConcreteMaterialPath =
        TEXT("/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Materials/Instances/M_Concrete_1_Inst.M_Concrete_1_Inst");
    constexpr float HardscapeUpgradeDelaySeconds = 0.85f;
    constexpr float OwnerResolutionTimeoutSeconds = 5.0f;

    struct FSurfaceUpgradePlan
    {
        UInstancedStaticMeshComponent* Component = nullptr;
        UStaticMesh* OldMesh = nullptr;
        FTransform OldTransform;
        FTransform NewTransform;
        TArray<UMaterialInterface*> OldMaterials;
    };

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

    bool BuildPlan(
        UInstancedStaticMeshComponent* Component,
        UStaticMesh* AuthoredMesh,
        FSurfaceUpgradePlan& OutPlan,
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

    void RestorePlan(const FSurfaceUpgradePlan& Plan)
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
        const FSurfaceUpgradePlan& Plan,
        UStaticMesh* AuthoredMesh,
        UMaterialInterface* ConcreteMaterial)
    {
        if (!Plan.Component || !AuthoredMesh || !ConcreteMaterial) return false;
        Plan.Component->SetStaticMesh(AuthoredMesh);
        Plan.Component->EmptyOverrideMaterials();
        const int32 SlotCount = FMath::Max(1, AuthoredMesh->GetStaticMaterials().Num());
        for (int32 Slot = 0; Slot < SlotCount; ++Slot)
        {
            Plan.Component->SetMaterial(Slot, ConcreteMaterial);
        }
        if (!Plan.Component->UpdateInstanceTransform(0, Plan.NewTransform, false, false, true)) return false;
        Plan.Component->MarkRenderStateDirty();
        return Plan.Component->GetInstanceCount() == 1 &&
            Plan.Component->GetStaticMesh() == AuthoredMesh &&
            Plan.Component->GetMaterial(0) == ConcreteMaterial &&
            !IsBasicShapeMaterial(Plan.Component->GetMaterial(0));
    }
}

bool UOCParkHardscapeAuthoredUpgradeSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

TStatId UOCParkHardscapeAuthoredUpgradeSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCParkHardscapeAuthoredUpgradeSubsystem, STATGROUP_Tickables);
}

void UOCParkHardscapeAuthoredUpgradeSubsystem::Tick(float DeltaTime)
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
    if (ElapsedSeconds < HardscapeUpgradeDelaySeconds) return;

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
            TEXT("PASS45_AUTHORED_PARK_HARDSCAPE_FAIL reason=oster_sector_count_%d gate_k_complete=0 runtime_acceptance=0"),
            SectorCount);
        return;
    }

    UInstancedStaticMeshComponent* LegacyMemorial = FindISM(Sector, TEXT("ParkMemorialPlaza"));
    UInstancedStaticMeshComponent* LegacySkate = FindISM(Sector, TEXT("ParkSkateFitness"));
    UInstancedStaticMeshComponent* ParkMemorialSurface = FindISM(Sector, TEXT("ParkMemorialSurface"));
    UInstancedStaticMeshComponent* ParkMemorialMonument = FindISM(Sector, TEXT("ParkMemorialMonument"));
    UInstancedStaticMeshComponent* ParkSkateSurface = FindISM(Sector, TEXT("ParkSkateSurface"));
    UInstancedStaticMeshComponent* ParkSkateRamps = FindISM(Sector, TEXT("ParkSkateRamps"));

    const bool bOwnerContractReady =
        LegacyMemorial && LegacyMemorial->GetInstanceCount() == 0 &&
        LegacySkate && LegacySkate->GetInstanceCount() == 0 &&
        ParkMemorialSurface && ParkMemorialSurface->GetInstanceCount() == 1 &&
        ParkMemorialMonument && ParkMemorialMonument->GetInstanceCount() == 1 &&
        ParkSkateSurface && ParkSkateSurface->GetInstanceCount() == 1 &&
        ParkSkateRamps && ParkSkateRamps->GetInstanceCount() == 2;
    if (!bOwnerContractReady)
    {
        if (ElapsedSeconds < OwnerResolutionTimeoutSeconds) return;
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_AUTHORED_PARK_HARDSCAPE_FAIL reason=semantic_owner_contract legacy_memorial=%d legacy_skate=%d memorial_surface=%d memorial_monument=%d skate_surface=%d skate_ramps=%d normalization_required=1 gate_k_complete=0 runtime_acceptance=0"),
            LegacyMemorial ? LegacyMemorial->GetInstanceCount() : -1,
            LegacySkate ? LegacySkate->GetInstanceCount() : -1,
            ParkMemorialSurface ? ParkMemorialSurface->GetInstanceCount() : -1,
            ParkMemorialMonument ? ParkMemorialMonument->GetInstanceCount() : -1,
            ParkSkateSurface ? ParkSkateSurface->GetInstanceCount() : -1,
            ParkSkateRamps ? ParkSkateRamps->GetInstanceCount() : -1);
        return;
    }

    UStaticMesh* SurfaceMesh = LoadObject<UStaticMesh>(nullptr, AuthoredSurfaceMeshPath);
    UMaterialInterface* ConcreteMaterial = LoadObject<UMaterialInterface>(nullptr, AuthoredConcreteMaterialPath);
    if (!SurfaceMesh || !ConcreteMaterial)
    {
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_AUTHORED_PARK_HARDSCAPE_CONTENT_GAP plane_mesh_loaded=%d concrete_material_loaded=%d expected_mesh=SM_Plane_1x1 expected_material=M_Concrete_1_Inst memorial_surface=1 skate_surface=1 gate_k_complete=0 runtime_acceptance=0"),
            SurfaceMesh ? 1 : 0,
            ConcreteMaterial ? 1 : 0);
        return;
    }

    UStaticMesh* MemorialMonumentMeshBefore = ParkMemorialMonument->GetStaticMesh();
    UStaticMesh* SkateRampsMeshBefore = ParkSkateRamps->GetStaticMesh();
    const int32 MemorialMonumentCountBefore = ParkMemorialMonument->GetInstanceCount();
    const int32 SkateRampsCountBefore = ParkSkateRamps->GetInstanceCount();

    FSurfaceUpgradePlan MemorialPlan;
    FSurfaceUpgradePlan SkatePlan;
    FString Failure;
    if (!BuildPlan(ParkMemorialSurface, SurfaceMesh, MemorialPlan, Failure) ||
        !BuildPlan(ParkSkateSurface, SurfaceMesh, SkatePlan, Failure))
    {
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_AUTHORED_PARK_HARDSCAPE_FAIL reason=%s preflight=0 mutation=0 gate_k_complete=0 runtime_acceptance=0"),
            Failure.IsEmpty() ? TEXT("surface_preflight_failed") : *Failure);
        return;
    }

    const bool bMemorialApplied = ApplyPlan(MemorialPlan, SurfaceMesh, ConcreteMaterial);
    const bool bSkateApplied = bMemorialApplied && ApplyPlan(SkatePlan, SurfaceMesh, ConcreteMaterial);
    if (!bMemorialApplied || !bSkateApplied)
    {
        RestorePlan(SkatePlan);
        RestorePlan(MemorialPlan);
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_AUTHORED_PARK_HARDSCAPE_FAIL reason=transaction_write_failed memorial_applied=%d skate_applied=%d rollback=1 gate_k_complete=0 runtime_acceptance=0"),
            bMemorialApplied ? 1 : 0,
            bSkateApplied ? 1 : 0);
        return;
    }

    const bool bUntouchedContentGapsPreserved =
        ParkMemorialMonument->GetInstanceCount() == MemorialMonumentCountBefore &&
        ParkSkateRamps->GetInstanceCount() == SkateRampsCountBefore &&
        ParkMemorialMonument->GetStaticMesh() == MemorialMonumentMeshBefore &&
        ParkSkateRamps->GetStaticMesh() == SkateRampsMeshBefore;
    const bool bPostcondition =
        ParkMemorialSurface->GetStaticMesh() == SurfaceMesh &&
        ParkSkateSurface->GetStaticMesh() == SurfaceMesh &&
        ParkMemorialSurface->GetMaterial(0) == ConcreteMaterial &&
        ParkSkateSurface->GetMaterial(0) == ConcreteMaterial &&
        ParkMemorialSurface->GetInstanceCount() == 1 &&
        ParkSkateSurface->GetInstanceCount() == 1 &&
        bUntouchedContentGapsPreserved;

    if (!bPostcondition)
    {
        RestorePlan(SkatePlan);
        RestorePlan(MemorialPlan);
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_AUTHORED_PARK_HARDSCAPE_FAIL reason=postcondition rollback=1 untouched_content_gaps=%d gate_k_complete=0 runtime_acceptance=0"),
            bUntouchedContentGapsPreserved ? 1 : 0);
        return;
    }

    bFinished = true;
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_AUTHORED_PARK_HARDSCAPE_READY memorial_surface=1 skate_surface=1 memorial_monument_untouched=1 skate_ramps_untouched=2 mesh=SM_Plane_1x1 material=M_Concrete_1_Inst xy_footprint_preserved=1 source_top_preserved=1 family_scope_exact=1 primary_authoring=0 migration_bridge_required=1 remaining_content_gap_instances=3 gate_k_complete=0 runtime_acceptance=0"));
}
