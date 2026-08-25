#include "OCWorldProductionVisualsSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"

namespace
{
    const FName ProductionVisualOwnerTag(TEXT("OC_Pass45_ProductionVisualOwner"));

    constexpr const TCHAR* HouseVar01Path = TEXT("/Game/AdvancedVillagePack/Meshes/SM_House_Var01.SM_House_Var01");
    constexpr const TCHAR* HouseVar02Path = TEXT("/Game/AdvancedVillagePack/Meshes/SM_House_Var02.SM_House_Var02");
    constexpr const TCHAR* FenceVar01Path = TEXT("/Game/AdvancedVillagePack/Meshes/SM_Fence_Var01.SM_Fence_Var01");
    constexpr const TCHAR* FenceVar02Path = TEXT("/Game/AdvancedVillagePack/Meshes/SM_Fence_Var02.SM_Fence_Var02");
    constexpr const TCHAR* FenceVar03Path = TEXT("/Game/AdvancedVillagePack/Meshes/SM_Fence_Var03.SM_Fence_Var03");
    constexpr const TCHAR* FenceVar04Path = TEXT("/Game/AdvancedVillagePack/Meshes/SM_Fence_Var04.SM_Fence_Var04");
    constexpr const TCHAR* GroundMaterialPath = TEXT("/Game/AdvancedVillagePack/Materials/M_Inst_Landscape.M_Inst_Landscape");
    constexpr const TCHAR* RoadMaterialPath = TEXT("/Game/Scene_RoadsideConstruction/Materials/MaterialInstances/MI_Urb_Roa_Asphalt_01.MI_Urb_Roa_Asphalt_01");
    constexpr const TCHAR* SidewalkMaterialPath = TEXT("/Game/Scene_RoadsideConstruction/Materials/MaterialInstances/MI_Urb_Roa_Sidewalk_01.MI_Urb_Roa_Sidewalk_01");

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

    void HideVisualKeepCollision(UPrimitiveComponent* Component)
    {
        if (!Component) return;
        Component->SetVisibility(false, true);
        Component->SetHiddenInGame(true, true);
        Component->SetCastShadow(false);
    }

    UInstancedStaticMeshComponent* MakeVisualISM(AActor& Owner, USceneComponent& Root, const FName Name, UStaticMesh* Mesh)
    {
        if (!Mesh) return nullptr;
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(&Owner, Name);
        if (!Component) return nullptr;
        Component->SetupAttachment(&Root);
        Component->SetStaticMesh(Mesh);
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        Component->SetCastShadow(true);
        Owner.AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    FVector SourceBoxWorldSize(const FTransform& Transform)
    {
        return Transform.GetScale3D().GetAbs() * 100.0f;
    }

    bool AddPreservedShapeInstance(
        UInstancedStaticMeshComponent* Target,
        UStaticMesh* Mesh,
        const FTransform& SourceCubeTransform,
        const FVector& DesiredBoxCm)
    {
        if (!Target || !Mesh) return false;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector MeshSize = Bounds.BoxExtent * 2.0f;
        if (MeshSize.X < 1.0f || MeshSize.Y < 1.0f || MeshSize.Z < 1.0f) return false;

        // Preserve authored house proportions. The hidden source cube remains collision authority.
        const float UniformScale = FMath::Clamp(
            FMath::Min(DesiredBoxCm.X / MeshSize.X, DesiredBoxCm.Y / MeshSize.Y),
            0.15f,
            12.0f);

        const FQuat Rotation = SourceCubeTransform.GetRotation();
        FVector Location = SourceCubeTransform.GetLocation();
        const FVector ScaledOrigin = Bounds.Origin * UniformScale;
        Location -= Rotation.RotateVector(FVector(ScaledOrigin.X, ScaledOrigin.Y, 0.0f));

        const float TargetBottom = SourceCubeTransform.GetLocation().Z - DesiredBoxCm.Z * 0.5f;
        const float MeshBottomLocal = (Bounds.Origin.Z - Bounds.BoxExtent.Z) * UniformScale;
        Location.Z = TargetBottom - MeshBottomLocal;

        FTransform VisualTransform;
        VisualTransform.SetLocation(Location);
        VisualTransform.SetRotation(Rotation);
        VisualTransform.SetScale3D(FVector(UniformScale));
        Target->AddInstance(VisualTransform, true);
        return true;
    }

    int32 CopyResidentialHouses(
        UInstancedStaticMeshComponent* Source,
        UInstancedStaticMeshComponent* TargetA,
        UStaticMesh* MeshA,
        UInstancedStaticMeshComponent* TargetB,
        UStaticMesh* MeshB)
    {
        if (!Source || !TargetA || !MeshA || !TargetB || !MeshB) return 0;
        int32 Added = 0;
        for (int32 Index = 0; Index < Source->GetInstanceCount(); ++Index)
        {
            FTransform SourceTransform;
            if (!Source->GetInstanceTransform(Index, SourceTransform, true)) continue;
            const bool bUseA = (Index % 2) == 0;
            if (AddPreservedShapeInstance(
                bUseA ? TargetA : TargetB,
                bUseA ? MeshA : MeshB,
                SourceTransform,
                SourceBoxWorldSize(SourceTransform)))
            {
                ++Added;
            }
        }
        return Added;
    }

    int32 TileFenceInstances(
        UInstancedStaticMeshComponent* Source,
        UInstancedStaticMeshComponent* Target,
        UStaticMesh* FenceMesh)
    {
        if (!Source || !Target || !FenceMesh) return 0;
        const FBoxSphereBounds Bounds = FenceMesh->GetBounds();
        const FVector MeshSize = Bounds.BoxExtent * 2.0f;
        if (MeshSize.X < 1.0f || MeshSize.Y < 1.0f || MeshSize.Z < 1.0f) return 0;

        const bool bMeshLengthX = MeshSize.X >= MeshSize.Y;
        const float MeshLength = bMeshLengthX ? MeshSize.X : MeshSize.Y;
        int32 Added = 0;

        for (int32 Index = 0; Index < Source->GetInstanceCount(); ++Index)
        {
            FTransform SourceTransform;
            if (!Source->GetInstanceTransform(Index, SourceTransform, true)) continue;

            const FVector Desired = SourceBoxWorldSize(SourceTransform);
            const float TargetLength = FMath::Max(Desired.X, Desired.Y);
            const float HeightScale = FMath::Clamp(Desired.Z / MeshSize.Z, 0.2f, 5.0f);
            const float NominalLength = FMath::Max(100.0f, MeshLength * HeightScale);
            const int32 Count = FMath::Clamp(FMath::CeilToInt(TargetLength / NominalLength), 1, 64);
            const float SegmentLength = TargetLength / static_cast<float>(Count);
            const float LengthScale = SegmentLength / MeshLength;

            FRotator VisualRotation = SourceTransform.Rotator();
            if (!bMeshLengthX) VisualRotation.Yaw += 90.0f;
            const FVector Direction = VisualRotation.RotateVector(FVector::ForwardVector);
            const float FirstOffset = -TargetLength * 0.5f + SegmentLength * 0.5f;

            for (int32 Segment = 0; Segment < Count; ++Segment)
            {
                FVector Location = SourceTransform.GetLocation() + Direction * (FirstOffset + Segment * SegmentLength);
                const float TargetBottom = SourceTransform.GetLocation().Z - Desired.Z * 0.5f;
                const float MeshBottomLocal = (Bounds.Origin.Z - Bounds.BoxExtent.Z) * HeightScale;
                Location.Z = TargetBottom - MeshBottomLocal;

                FVector Scale(HeightScale);
                if (bMeshLengthX) Scale.X = LengthScale;
                else Scale.Y = LengthScale;

                FTransform VisualTransform;
                VisualTransform.SetLocation(Location);
                VisualTransform.SetRotation(FQuat(VisualRotation));
                VisualTransform.SetScale3D(Scale);
                Target->AddInstance(VisualTransform, true);
                ++Added;
            }
        }
        return Added;
    }
}

bool UOCWorldProductionVisualsSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCWorldProductionVisualsSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    RuntimeWorld = &InWorld;
    TryBuildProductionVisuals();
}

void UOCWorldProductionVisualsSubsystem::Deinitialize()
{
    if (UWorld* World = RuntimeWorld.Get()) World->GetTimerManager().ClearTimer(RetryHandle);
    RuntimeWorld.Reset();
    Super::Deinitialize();
}

void UOCWorldProductionVisualsSubsystem::TryBuildProductionVisuals()
{
    if (bBuilt) return;
    UWorld* World = RuntimeWorld.Get();
    if (!World) return;

    AOCWorldSectorOster* Sector = nullptr;
    for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It)
    {
        Sector = *It;
        break;
    }

    if (!Sector)
    {
        ++Attempts;
        if (Attempts >= 20)
        {
            World->GetTimerManager().ClearTimer(RetryHandle);
            UE_LOG(LogTemp, Error, TEXT("PASS45_B2_PRODUCTION_VISUALS_FAIL reason=sector_missing attempts=%d"), Attempts);
            return;
        }
        if (!World->GetTimerManager().IsTimerActive(RetryHandle))
        {
            World->GetTimerManager().SetTimer(
                RetryHandle,
                this,
                &UOCWorldProductionVisualsSubsystem::TryBuildProductionVisuals,
                0.10f,
                true);
        }
        return;
    }

    UStaticMesh* HouseA = LoadObject<UStaticMesh>(nullptr, HouseVar01Path);
    UStaticMesh* HouseB = LoadObject<UStaticMesh>(nullptr, HouseVar02Path);
    UStaticMesh* FenceA = LoadObject<UStaticMesh>(nullptr, FenceVar01Path);
    UStaticMesh* FenceB = LoadObject<UStaticMesh>(nullptr, FenceVar02Path);
    UStaticMesh* FenceC = LoadObject<UStaticMesh>(nullptr, FenceVar03Path);
    UStaticMesh* FenceD = LoadObject<UStaticMesh>(nullptr, FenceVar04Path);
    UMaterialInterface* GroundMaterial = LoadObject<UMaterialInterface>(nullptr, GroundMaterialPath);
    UMaterialInterface* RoadMaterial = LoadObject<UMaterialInterface>(nullptr, RoadMaterialPath);
    UMaterialInterface* SidewalkMaterial = LoadObject<UMaterialInterface>(nullptr, SidewalkMaterialPath);

    if (!HouseA || !HouseB || !FenceA || !FenceB || !FenceC || !FenceD || !GroundMaterial || !RoadMaterial || !SidewalkMaterial)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_B2_PRODUCTION_VISUALS_FAIL reason=required_imported_asset_missing houseA=%d houseB=%d fenceA=%d fenceB=%d fenceC=%d fenceD=%d groundMat=%d asphaltMat=%d sidewalkMat=%d"),
            HouseA ? 1 : 0, HouseB ? 1 : 0, FenceA ? 1 : 0, FenceB ? 1 : 0,
            FenceC ? 1 : 0, FenceD ? 1 : 0, GroundMaterial ? 1 : 0,
            RoadMaterial ? 1 : 0, SidewalkMaterial ? 1 : 0);
        return;
    }

    AActor* Owner = World->SpawnActor<AActor>();
    if (!Owner)
    {
        UE_LOG(LogTemp, Error, TEXT("PASS45_B2_PRODUCTION_VISUALS_FAIL reason=owner_spawn_failed"));
        return;
    }
    Owner->Tags.AddUnique(ProductionVisualOwnerTag);

    USceneComponent* Root = NewObject<USceneComponent>(Owner, TEXT("ProductionVisualRoot"));
    Owner->AddInstanceComponent(Root);
    Owner->SetRootComponent(Root);
    Root->RegisterComponent();

    UInstancedStaticMeshComponent* HousesA = MakeVisualISM(*Owner, *Root, TEXT("ProductionHousesA"), HouseA);
    UInstancedStaticMeshComponent* HousesB = MakeVisualISM(*Owner, *Root, TEXT("ProductionHousesB"), HouseB);
    UInstancedStaticMeshComponent* FencesA = MakeVisualISM(*Owner, *Root, TEXT("ProductionFencesA"), FenceA);
    UInstancedStaticMeshComponent* FencesB = MakeVisualISM(*Owner, *Root, TEXT("ProductionFencesB"), FenceB);
    UInstancedStaticMeshComponent* FencesC = MakeVisualISM(*Owner, *Root, TEXT("ProductionFencesC"), FenceC);
    UInstancedStaticMeshComponent* FencesD = MakeVisualISM(*Owner, *Root, TEXT("ProductionFencesD"), FenceD);
    if (!HousesA || !HousesB || !FencesA || !FencesB || !FencesC || !FencesD)
    {
        Owner->Destroy();
        UE_LOG(LogTemp, Error, TEXT("PASS45_B2_PRODUCTION_VISUALS_FAIL reason=visual_component_creation_failed"));
        return;
    }

    UInstancedStaticMeshComponent* Buildings = FindISM(Sector, TEXT("Buildings"));
    UInstancedStaticMeshComponent* Roofs = FindISM(Sector, TEXT("ResidentialRoofs"));
    UInstancedStaticMeshComponent* Details = FindISM(Sector, TEXT("ResidentialDetails"));
    UInstancedStaticMeshComponent* PublicFences = FindISM(Sector, TEXT("Fences"));
    UInstancedStaticMeshComponent* WoodFences = FindISM(Sector, TEXT("WoodFences"));
    UInstancedStaticMeshComponent* MetalFences = FindISM(Sector, TEXT("MetalFences"));
    UInstancedStaticMeshComponent* SheetFences = FindISM(Sector, TEXT("LightSheetFences"));
    UInstancedStaticMeshComponent* Roads = FindISM(Sector, TEXT("Roads"));
    UInstancedStaticMeshComponent* Sidewalks = FindISM(Sector, TEXT("Sidewalks"));
    UStaticMeshComponent* Ground = FindStaticMeshComponent(Sector, TEXT("Ground"));

    const int32 HouseInstances = CopyResidentialHouses(Buildings, HousesA, HouseA, HousesB, HouseB);
    const int32 SourceHouseCount = Buildings ? Buildings->GetInstanceCount() : 0;
    const int32 FenceInstances =
        TileFenceInstances(PublicFences, FencesD, FenceD) +
        TileFenceInstances(WoodFences, FencesA, FenceA) +
        TileFenceInstances(MetalFences, FencesB, FenceB) +
        TileFenceInstances(SheetFences, FencesC, FenceC);

    if (SourceHouseCount > 0 && HouseInstances == SourceHouseCount)
    {
        HideVisualKeepCollision(Buildings);
        HideVisualKeepCollision(Roofs);
        HideVisualKeepCollision(Details);
    }
    else
    {
        UE_LOG(LogTemp, Warning,
            TEXT("PASS45_B2_RESIDENTIAL_VISUAL_GAP source=%d production=%d primitive_hidden=0"),
            SourceHouseCount, HouseInstances);
    }

    if (FenceInstances > 0)
    {
        HideVisualKeepCollision(PublicFences);
        HideVisualKeepCollision(WoodFences);
        HideVisualKeepCollision(MetalFences);
        HideVisualKeepCollision(SheetFences);
    }

    if (Ground) Ground->SetMaterial(0, GroundMaterial);
    if (Roads) Roads->SetMaterial(0, RoadMaterial);
    if (Sidewalks) Sidewalks->SetMaterial(0, SidewalkMaterial);

    bBuilt = true;
    World->GetTimerManager().ClearTimer(RetryHandle);

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_B2_PRODUCTION_VISUALS_READY houses=%d fence_segments=%d real_house_meshes=2 real_fence_meshes=4 ground_material=landscape asphalt_material=1 sidewalk_material=1 polling_after_ready=0 collision_backstop_retained=1"),
        HouseInstances, FenceInstances);
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_B2_REMAINING_CONTENT_GAPS generic_landmark_blocks=1 park_detail_proxies=1 water_bridge_instances=0 dedicated_site_owners_override_museum_silpo_culture=1"));
}
