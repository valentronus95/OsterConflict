#include "OCR13MuseumRearTerrainSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    constexpr float MapHalfCm = 120000.0f;
    constexpr float ValleyHalfWidthCm = 10000.0f;
    constexpr float ValleyStartY = 2800.0f;
    constexpr float ValleyBottomStartY = 14000.0f;
    constexpr float ValleyBottomEndY = 26000.0f;
    constexpr float ValleyEndY = 34000.0f;
    constexpr float ValleyDropCm = 850.0f;
    constexpr float TerrainThicknessCm = 140.0f;
    constexpr float FinalizeDelaySeconds = 3.40f;
    constexpr int32 ExpectedTerrainSlabCount = 7;

    const FName TerrainActorTag(TEXT("R13_MuseumRearTerrain"));
    const FName LowerDistrictActorTag(TEXT("R13_MuseumRearLowerDistrict"));

    bool HasTaggedActor(UWorld& World, const FName Tag)
    {
        for (TActorIterator<AActor> It(&World); It; ++It)
        {
            if (*It && It->ActorHasTag(Tag)) return true;
        }
        return false;
    }

    UInstancedStaticMeshComponent* MakeISM(AActor* Owner, USceneComponent* Root, UStaticMesh* Mesh,
        UMaterialInterface* Material, const FName Name, const bool bCollision, const bool bShadow)
    {
        if (!Owner || !Root || !Mesh) return nullptr;
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        if (Material)
        {
            const int32 Slots = FMath::Max(1, Mesh->GetStaticMaterials().Num());
            for (int32 Slot = 0; Slot < Slots; ++Slot) Component->SetMaterial(Slot, Material);
        }
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionProfileName(FName(bCollision ? TEXT("BlockAll") : TEXT("NoCollision")));
        Component->SetCollisionEnabled(bCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(bCollision);
        Component->SetCastShadow(bShadow);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void AddBox(UInstancedStaticMeshComponent* Component, const FVector& Center, const FVector& SizeCm,
        const FRotator& Rotation = FRotator::ZeroRotator)
    {
        if (!Component) return;
        Component->AddInstance(FTransform(Rotation, Center, SizeCm / 100.0f), true);
    }

    void AddFlatSurface(UInstancedStaticMeshComponent* Component, const float MinX, const float MaxX,
        const float MinY, const float MaxY, const float TopZ)
    {
        const FVector Center((MinX + MaxX) * 0.5f, (MinY + MaxY) * 0.5f,
            TopZ - TerrainThicknessCm * 0.5f);
        AddBox(Component, Center,
            FVector(MaxX - MinX, MaxY - MinY, TerrainThicknessCm));
    }

    void AddYSlopeSurface(UInstancedStaticMeshComponent* Component, const float MinX, const float MaxX,
        const float StartY, const float EndY, const float StartTopZ, const float EndTopZ)
    {
        const float Run = EndY - StartY;
        if (Run <= KINDA_SMALL_NUMBER) return;
        const float DeltaZ = EndTopZ - StartTopZ;
        const float SlopedLength = FMath::Sqrt(Run * Run + DeltaZ * DeltaZ);

        // Positive Unreal roll lowers local +Y. Use the opposite sign of DeltaZ so the top face joins
        // the requested start/end elevations while the cube remains a solid collision slab.
        const float RollDegrees = -FMath::RadiansToDegrees(FMath::Atan2(DeltaZ, Run));
        const float CosRoll = FMath::Cos(FMath::DegreesToRadians(RollDegrees));
        const FVector Center((MinX + MaxX) * 0.5f, (StartY + EndY) * 0.5f,
            (StartTopZ + EndTopZ) * 0.5f - TerrainThicknessCm * 0.5f * CosRoll);
        AddBox(Component, Center,
            FVector(MaxX - MinX, SlopedLength, TerrainThicknessCm),
            FRotator(0.0f, 0.0f, RollDegrees));
    }

    float TerrainZAtY(const float Y)
    {
        if (Y <= ValleyStartY) return 0.0f;
        if (Y < ValleyBottomStartY)
        {
            const float Alpha = (Y - ValleyStartY) / (ValleyBottomStartY - ValleyStartY);
            return FMath::Lerp(0.0f, -ValleyDropCm, Alpha);
        }
        if (Y <= ValleyBottomEndY) return -ValleyDropCm;
        if (Y < ValleyEndY)
        {
            const float Alpha = (Y - ValleyBottomEndY) / (ValleyEndY - ValleyBottomEndY);
            return FMath::Lerp(-ValleyDropCm, 0.0f, Alpha);
        }
        return 0.0f;
    }

    bool IsInsideRearValley(const FVector& Location)
    {
        return FMath::Abs(Location.X) <= ValleyHalfWidthCm - 100.0f &&
            Location.Y >= ValleyStartY - 150.0f && Location.Y <= ValleyEndY + 150.0f;
    }

    bool ShouldTrimGeneratedFamily(const FName Name)
    {
        static const TSet<FName> Exact = {
            TEXT("Roads"), TEXT("Sidewalks"), TEXT("Buildings"), TEXT("ResidentialRoofs"),
            TEXT("ResidentialDetails"), TEXT("Fences"), TEXT("WoodFences"), TEXT("MetalFences"),
            TEXT("LightSheetFences"), TEXT("TreeTrunks"), TEXT("TreeCrowns"),
            TEXT("SovietPoplarTrunks"), TEXT("SovietPoplarCrowns"), TEXT("BirchTrunks"),
            TEXT("BirchCrowns"), TEXT("PineTrunks"), TEXT("PineCrowns"),
            TEXT("GrassMown"), TEXT("GrassRough"), TEXT("GrassWetland"),
            TEXT("R13_House01"), TEXT("R13_House02"), TEXT("R13_Roads"), TEXT("R13_Sidewalks")
        };
        if (Exact.Contains(Name)) return true;

        const FString Value = Name.ToString();
        return Value.StartsWith(TEXT("R13_House01Extra")) || Value == TEXT("R13_House02Extra") ||
            Value.StartsWith(TEXT("R13_Tree")) || Value.StartsWith(TEXT("R13_Pine")) ||
            Value.StartsWith(TEXT("R13_CompanionTree")) || Value.StartsWith(TEXT("R13_ExplicitPine")) ||
            Value.StartsWith(TEXT("R13_DenseGrass")) || Value.StartsWith(TEXT("R13_GroundPlant")) ||
            Value.StartsWith(TEXT("R13_Yard")) || Value.StartsWith(TEXT("R13_OsterBrickHouse")) ||
            Value.StartsWith(TEXT("R13_OsterHouse")) || Value.StartsWith(TEXT("R13_OsterGreyPitchedRoofs"));
    }

    int32 TrimInstancesInsideValley(UInstancedStaticMeshComponent* Component)
    {
        if (!Component || !ShouldTrimGeneratedFamily(Component->GetFName())) return 0;
        int32 Removed = 0;
        for (int32 Index = Component->GetInstanceCount() - 1; Index >= 0; --Index)
        {
            FTransform Transform;
            if (!Component->GetInstanceTransform(Index, Transform, true)) continue;
            if (!IsInsideRearValley(Transform.GetLocation())) continue;
            if (Component->RemoveInstance(Index)) ++Removed;
        }
        if (Removed > 0) Component->MarkRenderStateDirty();
        return Removed;
    }

    float UniformScaleForFootprint(UStaticMesh* Mesh, const float DesiredMaxFootprintCm)
    {
        if (!Mesh) return 1.0f;
        const FVector Native = Mesh->GetBounds().BoxExtent * 2.0f;
        const float NativeFootprint = FMath::Max(1.0f, FMath::Max(Native.X, Native.Y));
        return FMath::Clamp(DesiredMaxFootprintCm / NativeFootprint, 0.20f, 4.0f);
    }

    void AddGroundedMesh(UInstancedStaticMeshComponent* Component, UStaticMesh* Mesh,
        const FVector& GroundPoint, const float Yaw, const float Scale)
    {
        if (!Component || !Mesh) return;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const float LocalBottom = Bounds.Origin.Z - Bounds.BoxExtent.Z;
        const FRotator Rotation(0.0f, Yaw, 0.0f);
        FVector Location = GroundPoint;
        const FVector PivotXY = Rotation.RotateVector(FVector(-Bounds.Origin.X * Scale, -Bounds.Origin.Y * Scale, 0.0f));
        Location += PivotXY;
        Location.Z = GroundPoint.Z - LocalBottom * Scale;
        Component->AddInstance(FTransform(Rotation, Location, FVector(Scale)), true);
    }

    void AddGroundedHeightMesh(UInstancedStaticMeshComponent* Component, UStaticMesh* Mesh,
        const FVector& GroundPoint, const float Yaw, const float DesiredHeightCm)
    {
        if (!Component || !Mesh) return;
        const float NativeHeight = FMath::Max(1.0f, Mesh->GetBounds().BoxExtent.Z * 2.0f);
        AddGroundedMesh(Component, Mesh, GroundPoint, Yaw,
            FMath::Clamp(DesiredHeightCm / NativeHeight, 0.20f, 4.0f));
    }
}

bool UOCR13MuseumRearTerrainSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13MuseumRearTerrainSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);

    FTimerHandle TerrainTimer;
    InWorld.GetTimerManager().SetTimer(TerrainTimer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) BuildTerrainSurface(*World);
        }), 0.05f, false);

    // WholeOsterArt (0.80 s), EnvironmentDressing (1.60 s), legacy residential architecture (1.95 s)
    // and real-mesh restore (2.20 s) all finish first. This final pass then removes only objects that would
    // float over the new valley and authors the lower residential area once, still behind the loading screen.
    FTimerHandle FinalizeTimer;
    InWorld.GetTimerManager().SetTimer(FinalizeTimer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) FinalizeLowerResidentialDistrict(*World);
        }), FinalizeDelaySeconds, false);
}

void UOCR13MuseumRearTerrainSubsystem::BuildTerrainSurface(UWorld& World)
{
    if (HasTaggedActor(World, TerrainActorTag)) return;

    AOCWorldSectorOster* Sector = nullptr;
    for (TActorIterator<AOCWorldSectorOster> It(&World); It; ++It)
    {
        Sector = *It;
        if (Sector) break;
    }
    if (!Sector) return;

    UStaticMeshComponent* LegacyGround = FindObjectFast<UStaticMeshComponent>(Sector, TEXT("Ground"));
    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UMaterialInterface* Basic = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!LegacyGround || !Cube || !Basic) return;

    AActor* TerrainActor = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!TerrainActor) return;
    TerrainActor->SetReplicates(false);
    TerrainActor->SetActorEnableCollision(true);

    USceneComponent* Root = NewObject<USceneComponent>(TerrainActor, TEXT("R13_MuseumRearTerrainRoot"));
    if (!Root)
    {
        TerrainActor->Destroy();
        return;
    }
    Root->SetMobility(EComponentMobility::Static);
    TerrainActor->SetRootComponent(Root);
    TerrainActor->AddInstanceComponent(Root);
    Root->RegisterComponent();

    UMaterialInstanceDynamic* GroundMaterial = UMaterialInstanceDynamic::Create(
        Basic, TerrainActor, TEXT("R13_MuseumRearTerrainGroundMID"));
    if (GroundMaterial)
    {
        GroundMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.19f, 0.24f, 0.13f, 1.0f));
    }

    UInstancedStaticMeshComponent* Terrain = MakeISM(TerrainActor, Root, Cube, GroundMaterial,
        TEXT("R13_MuseumRearTerrainCollision"), true, false);
    if (!Terrain)
    {
        TerrainActor->Destroy();
        return;
    }

    // Build the complete replacement first. The legacy Ground remains active until this succeeds, so a missing
    // mesh/component can never leave the playtest world without a walkable floor.
    AddFlatSurface(Terrain, -MapHalfCm, -ValleyHalfWidthCm, -MapHalfCm, MapHalfCm, 0.0f);
    AddFlatSurface(Terrain, ValleyHalfWidthCm, MapHalfCm, -MapHalfCm, MapHalfCm, 0.0f);
    AddFlatSurface(Terrain, -ValleyHalfWidthCm, ValleyHalfWidthCm, -MapHalfCm, ValleyStartY, 0.0f);
    AddYSlopeSurface(Terrain, -ValleyHalfWidthCm, ValleyHalfWidthCm,
        ValleyStartY, ValleyBottomStartY, 0.0f, -ValleyDropCm);
    AddFlatSurface(Terrain, -ValleyHalfWidthCm, ValleyHalfWidthCm,
        ValleyBottomStartY, ValleyBottomEndY, -ValleyDropCm);
    AddYSlopeSurface(Terrain, -ValleyHalfWidthCm, ValleyHalfWidthCm,
        ValleyBottomEndY, ValleyEndY, -ValleyDropCm, 0.0f);
    AddFlatSurface(Terrain, -ValleyHalfWidthCm, ValleyHalfWidthCm, ValleyEndY, MapHalfCm, 0.0f);

    if (Terrain->GetInstanceCount() != ExpectedTerrainSlabCount)
    {
        UE_LOG(LogTemp, Error,
            TEXT("R13 museum rear terrain: replacement incomplete (%d/%d slabs); legacy Ground preserved."),
            Terrain->GetInstanceCount(), ExpectedTerrainSlabCount);
        TerrainActor->Destroy();
        return;
    }

    // Commit the swap only after the replacement collision exists in full.
    LegacyGround->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    LegacyGround->SetVisibility(false, true);
    LegacyGround->SetHiddenInGame(true, true);
    TerrainActor->Tags.Add(TerrainActorTag);

    UE_LOG(LogTemp, Display,
        TEXT("R13 museum rear terrain: replacement verified in-memory; flat authoritative Ground disabled; provisional drop=%.0f cm."),
        ValleyDropCm);
}

void UOCR13MuseumRearTerrainSubsystem::FinalizeLowerResidentialDistrict(UWorld& World)
{
    if (HasTaggedActor(World, LowerDistrictActorTag)) return;

    int32 RemovedFloatingInstances = 0;
    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor || Actor->ActorHasTag(TerrainActorTag)) continue;
        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            RemovedFloatingInstances += TrimInstancesInsideValley(Component);
        }
    }

    UStaticMesh* House01 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_House_Var01.SM_House_Var01"));
    UStaticMesh* House02 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_House_Var02.SM_House_Var02"));
    UStaticMesh* Fence01 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Fence_Var01.SM_Fence_Var01"));
    UStaticMesh* Fence03 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Fence_Var03.SM_Fence_Var03"));
    UStaticMesh* Tree01 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var01.SM_Tree_Var01"));
    UStaticMesh* Tree04 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var04.SM_Tree_Var04"));
    UStaticMesh* Well = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Well.SM_Well"));
    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));

    if (!House01 && !House02) return;

    AActor* District = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!District) return;
    District->Tags.Add(LowerDistrictActorTag);
    District->SetReplicates(false);
    District->SetActorEnableCollision(true);

    USceneComponent* Root = NewObject<USceneComponent>(District, TEXT("R13_MuseumRearLowerDistrictRoot"));
    if (!Root)
    {
        District->Destroy();
        return;
    }
    District->SetRootComponent(Root);
    District->AddInstanceComponent(Root);
    Root->RegisterComponent();

    UInstancedStaticMeshComponent* Houses01 = MakeISM(District, Root, House01, nullptr,
        TEXT("R13_MuseumRearHouse01"), true, true);
    UInstancedStaticMeshComponent* Houses02 = MakeISM(District, Root, House02, nullptr,
        TEXT("R13_MuseumRearHouse02"), true, true);
    UInstancedStaticMeshComponent* Fences01 = MakeISM(District, Root, Fence01, nullptr,
        TEXT("R13_MuseumRearFence01"), true, true);
    UInstancedStaticMeshComponent* Fences03 = MakeISM(District, Root, Fence03, nullptr,
        TEXT("R13_MuseumRearFence03"), true, true);
    UInstancedStaticMeshComponent* Trees01 = MakeISM(District, Root, Tree01, nullptr,
        TEXT("R13_MuseumRearTree01"), false, true);
    UInstancedStaticMeshComponent* Trees04 = MakeISM(District, Root, Tree04, nullptr,
        TEXT("R13_MuseumRearTree04"), false, true);
    UInstancedStaticMeshComponent* Wells = MakeISM(District, Root, Well, nullptr,
        TEXT("R13_MuseumRearWell"), true, true);
    UInstancedStaticMeshComponent* HouseCollision = MakeISM(District, Root, Cube, nullptr,
        TEXT("R13_MuseumRearHouseCollision"), true, false);
    if (HouseCollision)
    {
        HouseCollision->SetVisibility(false, true);
        HouseCollision->SetHiddenInGame(true, true);
    }

    struct FHouseSeed { float X; float Y; float Yaw; bool bVariant02; float Footprint; };
    const FHouseSeed HouseSeeds[] = {
        { -6000.0f, 16400.0f,   7.0f, false, 1120.0f },
        { -2150.0f, 19200.0f,  -4.0f, true,  1040.0f },
        {  2350.0f, 16600.0f,   3.0f, false, 1080.0f },
        {  6100.0f, 20500.0f,  -8.0f, true,  1050.0f },
        { -5200.0f, 23900.0f,   2.0f, true,  1020.0f },
        {   650.0f, 23150.0f,   6.0f, false, 1140.0f },
    };

    int32 HouseCount = 0;
    for (const FHouseSeed& Seed : HouseSeeds)
    {
        UStaticMesh* Mesh = Seed.bVariant02 ? House02 : House01;
        UInstancedStaticMeshComponent* Target = Seed.bVariant02 ? Houses02 : Houses01;
        if (!Mesh || !Target) continue;
        const float GroundZ = TerrainZAtY(Seed.Y);
        const float Scale = UniformScaleForFootprint(Mesh, Seed.Footprint);
        AddGroundedMesh(Target, Mesh, FVector(Seed.X, Seed.Y, GroundZ), Seed.Yaw, Scale);
        if (HouseCollision)
        {
            AddBox(HouseCollision,
                FVector(Seed.X, Seed.Y, GroundZ + 180.0f), FVector(820.0f, 650.0f, 360.0f),
                FRotator(0.0f, Seed.Yaw, 0.0f));
        }
        ++HouseCount;
    }

    struct FFenceSeed { float X; float Y; float Yaw; bool bVariant03; float Span; };
    const FFenceSeed FenceSeeds[] = {
        { -6550.0f, 18250.0f, 90.0f, false, 850.0f },
        { -4350.0f, 21150.0f,  0.0f, true,  900.0f },
        { -1100.0f, 21100.0f, 90.0f, false, 820.0f },
        {  3300.0f, 18750.0f, 90.0f, true,  880.0f },
        {  5600.0f, 22800.0f,  0.0f, false, 920.0f },
        { -3500.0f, 24900.0f,  0.0f, true,  860.0f },
    };
    int32 FenceCount = 0;
    for (const FFenceSeed& Seed : FenceSeeds)
    {
        UStaticMesh* Mesh = Seed.bVariant03 ? Fence03 : Fence01;
        UInstancedStaticMeshComponent* Target = Seed.bVariant03 ? Fences03 : Fences01;
        if (!Mesh || !Target) continue;
        AddGroundedMesh(Target, Mesh, FVector(Seed.X, Seed.Y, TerrainZAtY(Seed.Y)), Seed.Yaw,
            UniformScaleForFootprint(Mesh, Seed.Span));
        ++FenceCount;
    }

    struct FTreeSeed { float X; float Y; float Height; float Yaw; bool bVar04; };
    const FTreeSeed TreeSeeds[] = {
        { -8200.0f,  7600.0f, 780.0f,  15.0f, false },
        {  7900.0f,  9200.0f, 860.0f, 145.0f, true  },
        { -7700.0f, 13600.0f, 900.0f, 260.0f, true  },
        {  8300.0f, 15100.0f, 820.0f,  70.0f, false },
        { -7800.0f, 22200.0f, 760.0f, 205.0f, false },
        {  8100.0f, 24700.0f, 900.0f, 330.0f, true  },
        { -7200.0f, 29200.0f, 850.0f, 120.0f, true  },
        {  7600.0f, 30900.0f, 790.0f, 280.0f, false },
    };
    int32 TreeCount = 0;
    for (const FTreeSeed& Seed : TreeSeeds)
    {
        UStaticMesh* Mesh = Seed.bVar04 ? Tree04 : Tree01;
        UInstancedStaticMeshComponent* Target = Seed.bVar04 ? Trees04 : Trees01;
        if (!Mesh || !Target) continue;
        AddGroundedHeightMesh(Target, Mesh,
            FVector(Seed.X, Seed.Y, TerrainZAtY(Seed.Y)), Seed.Yaw, Seed.Height);
        ++TreeCount;
    }

    if (Wells && Well)
    {
        AddGroundedHeightMesh(Wells, Well,
            FVector(4350.0f, 23800.0f, TerrainZAtY(23800.0f)), 22.0f, 210.0f);
    }

    UE_LOG(LogTemp, Display,
        TEXT("R13 museum rear lower district: removed floating generated instances=%d; real houses=%d, real fence pieces=%d, real trees=%d, well=%s; lower terrace Z=%.0f cm."),
        RemovedFloatingInstances, HouseCount, FenceCount, TreeCount, Wells && Well ? TEXT("yes") : TEXT("no"), -ValleyDropCm);
}
