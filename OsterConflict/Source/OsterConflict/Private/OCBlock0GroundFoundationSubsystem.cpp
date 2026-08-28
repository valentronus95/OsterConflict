#include "OCBlock0GroundFoundationSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

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
        TEXT("/Game/AdvancedVillagePack/Materials/M_Inst_Landscape.M_Inst_Landscape");

    UStaticMeshComponent* FindGroundComponent(AActor* Actor)
    {
        if (!Actor) return nullptr;
        TInlineComponentArray<UStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        const FName GroundName(TEXT("Ground"));
        for (UStaticMeshComponent* Component : Components)
        {
            if (Component && Component->GetFName() == GroundName) return Component;
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

    bool ApplyAuthoredGroundBeforeFirstTick(
        UStaticMeshComponent* Ground,
        UStaticMesh* AuthoredMesh,
        UMaterialInterface* AuthoredMaterial,
        FString& OutFailure)
    {
        if (!Ground || !AuthoredMesh || !AuthoredMaterial)
        {
            OutFailure = TEXT("ground_component_mesh_or_material_missing");
            return false;
        }

        UStaticMesh* CurrentMesh = Ground->GetStaticMesh();
        if (CurrentMesh == AuthoredMesh)
        {
            if (Ground->GetMaterial(0) != AuthoredMaterial || IsBasicShapeMaterial(Ground->GetMaterial(0)))
            {
                OutFailure = TEXT("existing_authored_ground_material_drift");
                return false;
            }
            if (Ground->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
            {
                OutFailure = TEXT("existing_authored_ground_collision_disabled");
                return false;
            }
            return true;
        }

        if (!IsEngineCube(CurrentMesh))
        {
            OutFailure = FString::Printf(TEXT("unexpected_source_ground_mesh_%s"),
                CurrentMesh ? *CurrentMesh->GetPathName() : TEXT("null"));
            return false;
        }

        const FTransform Old = Ground->GetRelativeTransform();
        const FRotator OldRotation = Old.Rotator();
        if (FMath::Abs(OldRotation.Pitch) > 0.1f || FMath::Abs(OldRotation.Roll) > 0.1f)
        {
            OutFailure = TEXT("source_ground_tilt_not_supported");
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

        Ground->SetStaticMesh(AuthoredMesh);
        Ground->EmptyOverrideMaterials();
        Ground->SetMaterial(0, AuthoredMaterial);
        Ground->SetRelativeTransform(FTransform(RotationQuat, NewLocation, NewScale));
        Ground->MarkRenderStateDirty();

        UMaterialInterface* RuntimeMaterial = Ground->GetMaterial(0);
        if (Ground->GetStaticMesh() != AuthoredMesh || RuntimeMaterial != AuthoredMaterial ||
            IsBasicShapeMaterial(RuntimeMaterial))
        {
            OutFailure = TEXT("pretick_authored_ground_postcondition_failed");
            return false;
        }

        const FTransform Applied = Ground->GetRelativeTransform();
        const FVector AppliedScale = Applied.GetScale3D().GetAbs();
        const FVector AppliedSizeCm(
            NewNativeSize.X * AppliedScale.X,
            NewNativeSize.Y * AppliedScale.Y,
            NewNativeSize.Z * AppliedScale.Z);
        const float AppliedTopOffsetZ = Applied.GetRotation().RotateVector(FVector(
            0.0f,
            0.0f,
            (NewBounds.Origin.Z + NewBounds.BoxExtent.Z) * AppliedScale.Z)).Z;
        const float AppliedTopZ = Applied.GetLocation().Z + AppliedTopOffsetZ;

        if (!FMath::IsNearlyEqual(AppliedSizeCm.X, DesiredSizeCm.X, 1.0f) ||
            !FMath::IsNearlyEqual(AppliedSizeCm.Y, DesiredSizeCm.Y, 1.0f) ||
            !FMath::IsNearlyEqual(AppliedTopZ, OldTopZ, 0.5f))
        {
            OutFailure = FString::Printf(
                TEXT("pretick_ground_geometry_postcondition_failed size_x=%.2f/%.2f size_y=%.2f/%.2f top_z=%.2f/%.2f"),
                AppliedSizeCm.X,
                DesiredSizeCm.X,
                AppliedSizeCm.Y,
                DesiredSizeCm.Y,
                AppliedTopZ,
                OldTopZ);
            return false;
        }

        if (Ground->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
        {
            OutFailure = TEXT("pretick_ground_collision_disabled");
            return false;
        }
        return true;
    }
}

bool UOCBlock0GroundFoundationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCBlock0GroundFoundationSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    if (!InWorld.IsGameWorld()) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    AOCWorldSectorOster* Sector = nullptr;
    int32 SectorCount = 0;
    for (TActorIterator<AOCWorldSectorOster> It(&InWorld); It; ++It)
    {
        Sector = *It;
        ++SectorCount;
    }
    if (SectorCount != 1 || !Sector)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_BLOCK0_PRETICK_GROUND_FAIL reason=oster_sector_count_%d authored_before_first_tick=0 runtime_acceptance=0"),
            SectorCount);
        return;
    }

    UStaticMesh* AuthoredMesh = LoadObject<UStaticMesh>(nullptr, AuthoredGroundMeshPath);
    UMaterialInterface* AuthoredMaterial = LoadObject<UMaterialInterface>(nullptr, AuthoredGroundMaterialPath);
    if (!AuthoredMesh || !AuthoredMaterial)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_BLOCK0_PRETICK_GROUND_CONTENT_GAP ground_mesh_loaded=%d ground_material_loaded=%d authored_before_first_tick=0 runtime_acceptance=0"),
            AuthoredMesh ? 1 : 0,
            AuthoredMaterial ? 1 : 0);
        return;
    }

    FString Failure;
    UStaticMeshComponent* Ground = FindGroundComponent(Sector);
    if (!ApplyAuthoredGroundBeforeFirstTick(Ground, AuthoredMesh, AuthoredMaterial, Failure))
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_BLOCK0_PRETICK_GROUND_FAIL reason=%s authored_before_first_tick=0 runtime_acceptance=0"),
            *Failure);
        return;
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_BLOCK0_PRETICK_GROUND_READY ground_mesh=SM_Plane_1x1 ground_material=M_Inst_Landscape basicshape_material=0 authored_before_first_tick=1 footprint_preserved=1 top_z_preserved=1 geometry_postcondition=1 collision_enabled=1 delayed_ground_mutation_required=0 runtime_acceptance=0"));
}
