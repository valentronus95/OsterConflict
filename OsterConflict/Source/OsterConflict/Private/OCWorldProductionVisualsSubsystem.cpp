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

    UInstancedStaticMeshComponent* MakeVisualISM(AActor& Owner, USceneComponent& Root, const FName Name,
        UStaticMesh* Mesh, const int32 StartCullCm, const int32 EndCullCm, const bool bCastShadow)
    {
        if (!Mesh) return nullptr;
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(&Owner, Name);
        if (!Component) return nullptr;
        Component->SetupAttachment(&Root);
        Component->SetStaticMesh(Mesh);
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        Component->SetCastShadow(bCastShadow);
        Component->SetCullDistances(StartCullCm, EndCullCm);
        Owner.AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    FVector SourceBoxWorldSize(const FTransform& Transform)
    {
        return Transform.GetScale3D().GetAbs() * 100.0f;
    }

    bool AddPreservedShapeInstance(UInstancedStaticMeshComponent* Target, UStaticMesh* Mesh,
        const FTransform& SourceCubeTransform, const FVector& DesiredBoxCm)
    {
        if (!Target || !Mesh) return false;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector MeshSize = Bounds.BoxExtent * 2.0f;
        if (MeshSize.X < 1.0f || MeshSize.Y < 1.0f || MeshSize.Z < 1.0f) return false;

        const float UniformScale = FMath::Clamp(
            FMath::Min(DesiredBoxCm.X / MeshSize.X, DesiredBoxCm.Y / MeshSize.Y), 0.15f, 12.0f);
        const FQuat Rotation = SourceCubeTransform.GetRotation();
        FVector Location = SourceCubeTransform.GetLocation();
        const FVector ScaledOrigin = Bounds.Origin * UniformScale;
        Location -= Rotation.RotateVector(FVector(ScaledOrigin.X, ScaledOrigin.Y, 0.0f));
        const float TargetBottom = SourceCubeTransform.GetLocation().Z - DesiredBoxCm.Z * 0.5f;
        Location.Z = TargetBottom - (Bounds.Origin.Z - Bounds.BoxExtent.Z) * UniformScale;

        FTransform VisualTransform;
        VisualTransform.SetLocation(Location);
        VisualTransform.SetRotation(Rotation);
        VisualTransform.SetScale3D(FVector(UniformScale));
        Target->AddInstance(VisualTransform, true);
        return true;
    }

    int32 CopyResidentialHouses(UInstancedStaticMeshComponent* Source,
        UInstancedStaticMeshComponent* TargetA, UStaticMesh* MeshA,
        UInstancedStaticMeshComponent* TargetB, UStaticMesh* MeshB)
    {
        if (!Source || !TargetA || !MeshA || !TargetB || !MeshB) return 0;
        int32 Added = 0;
        for (int32 Index = 0; Index < Source->GetInstanceCount(); ++Index)
        {
            FTransform SourceTransform;
            if (!Source->GetInstanceTransform(Index, SourceTransform, true)) continue;
            const bool bUseA = (Index % 2) == 0;
            Added += AddPreservedShapeInstance(bUseA ? TargetA : TargetB, bUseA ? MeshA : MeshB,
                SourceTransform, SourceBoxWorldSize(SourceTransform)) ? 1 : 0;
        }
        return Added;
    }

    int32 TileFenceInstances(UInstancedStaticMeshComponent* Source,
        UInstancedStaticMeshComponent* Target, UStaticMesh* FenceMesh)
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
                Location.Z = TargetBottom - (Bounds.Origin.Z - Bounds.BoxExtent.Z) * HeightScale;
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

    int32 ReplaceFenceFamily(UInstancedStaticMeshComponent* Source, UInstancedStaticMeshComponent* Target,
        UStaticMesh* Mesh, const TCHAR* FamilyName)
    {
        if (!Source) return 0;
        const int32 SourceCount = Source->GetInstanceCount();
        if (SourceCount <= 0) return 0;
        const int32 VisualSegments = TileFenceInstances(Source, Target, Mesh);
        if (VisualSegments > 0)
        {
            HideVisualKeepCollision(Source);
            UE_LOG(LogTemp, Display, TEXT("PASS45_B2_FENCE_FAMILY_READY family=%s source=%d visual_segments=%d"),
                FamilyName, SourceCount, VisualSegments);
            return VisualSegments;
        }
        UE_LOG(LogTemp, Warning, TEXT("PASS45_B2_FENCE_FAMILY_GAP family=%s source=%d visual_segments=0 primitive_hidden=0"),
            FamilyName, SourceCount);
        return 0;
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
    // Run after actor BeginPlay so the historical R11 BasicShape tint cannot overwrite production materials.
    InWorld.GetTimerManager().SetTimer(RetryHandle, this,
        &UOCWorldProductionVisualsSubsystem::TryBuildProductionVisuals, 0.05f, false);
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
    for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It) { Sector = *It; break; }
    if (!Sector)
    {
        ++Attempts;
        if (Attempts >= 20)
        {
            World->GetTimerManager().ClearTimer(RetryHandle);
            UE_LOG(LogTemp, Error, TEXT("PASS45_B2_PRODUCTION_VISUALS_FAIL reason=sector_missing attempts=%d"), Attempts);
            return;
        }
        World->GetTimerManager().SetTimer(RetryHandle, this,
            &UOCWorldProductionVisualsSubsystem::TryBuildProductionVisuals, 0.10f, false);
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
    if (!Owner) { UE_LOG(LogTemp, Error, TEXT("PASS45_B2_PRODUCTION_VISUALS_FAIL reason=owner_spawn_failed")); return; }
    Owner->Tags.AddUnique(ProductionVisualOwnerTag);
    USceneComponent* Root = NewObject<USceneComponent>(Owner, TEXT("ProductionVisualRoot"));
    Owner->AddInstanceComponent(Root);
    Owner->SetRootComponent(Root);
    Root->RegisterComponent();

    UInstancedStaticMeshComponent* HousesA = MakeVisualISM(*Owner, *Root, TEXT("ProductionHousesA"), HouseA, 30000, 65000, true);
    UInstancedStaticMeshComponent* HousesB = MakeVisualISM(*Owner, *Root, TEXT("ProductionHousesB"), HouseB, 30000, 65000, true);
    UInstancedStaticMeshComponent* FencesA = MakeVisualISM(*Owner, *Root, TEXT("ProductionFencesA"), FenceA, 6000, 28000, false);
    UInstancedStaticMeshComponent* FencesB = MakeVisualISM(*Owner, *Root, TEXT("ProductionFencesB"), FenceB, 6000, 28000, false);
    UInstancedStaticMeshComponent* FencesC = MakeVisualISM(*Owner, *Root, TEXT("ProductionFencesC"), FenceC, 6000, 28000, false);
    UInstancedStaticMeshComponent* FencesD = MakeVisualISM(*Owner, *Root, TEXT("ProductionFencesD"), FenceD, 6000, 28000, false);
    if (!HousesA || !HousesB || !FencesA || !FencesB || !FencesC || !FencesD)
    {
        Owner->Destroy();
        UE_LOG(LogTemp, Error, TEXT("PASS45_B2_PRODUCTION_VISUALS_FAIL reason=visual_component_creation_failed"));
        return;
    }

    UInstancedStaticMeshComponent* Buildings = FindISM(Sector, TEXT("Buildings"));
    UInstancedStaticMeshComponent* Roofs = FindISM(Sector, TEXT("ResidentialRoofs"));
    UInstancedStaticMeshComponent* Details = FindISM(Sector, TEXT("ResidentialDetails"));
    const int32 SourceHouseCount = Buildings ? Buildings->GetInstanceCount() : 0;
    const int32 HouseInstances = CopyResidentialHouses(Buildings, HousesA, HouseA, HousesB, HouseB);
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

    const int32 FenceInstances =
        ReplaceFenceFamily(FindISM(Sector, TEXT("Fences")), FencesD, FenceD, TEXT("public")) +
        ReplaceFenceFamily(FindISM(Sector, TEXT("WoodFences")), FencesA, FenceA, TEXT("wood")) +
        ReplaceFenceFamily(FindISM(Sector, TEXT("MetalFences")), FencesB, FenceB, TEXT("metal")) +
        ReplaceFenceFamily(FindISM(Sector, TEXT("LightSheetFences")), FencesC, FenceC, TEXT("sheet"));

    UStaticMeshComponent* Ground = FindStaticMeshComponent(Sector, TEXT("Ground"));
    UInstancedStaticMeshComponent* Roads = FindISM(Sector, TEXT("Roads"));
    UInstancedStaticMeshComponent* Sidewalks = FindISM(Sector, TEXT("Sidewalks"));
    if (Ground) Ground->SetMaterial(0, GroundMaterial);
    if (Roads) Roads->SetMaterial(0, RoadMaterial);
    if (Sidewalks) Sidewalks->SetMaterial(0, SidewalkMaterial);

    bBuilt = true;
    World->GetTimerManager().ClearTimer(RetryHandle);
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_B2_PRODUCTION_VISUALS_READY houses=%d fence_segments=%d real_house_meshes=2 real_fence_meshes=4 house_cull_m=300_650 fence_cull_m=60_280 ground_material=landscape asphalt_material=1 sidewalk_material=1 post_actor_beginplay=1 polling_after_ready=0 collision_backstop_retained=1"),
        HouseInstances, FenceInstances);
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_B2_REMAINING_CONTENT_GAPS generic_landmark_blocks=1 park_detail_proxies=1 water_bridge_instances=0 dedicated_site_owners_override_museum_silpo_culture=1"));
}
