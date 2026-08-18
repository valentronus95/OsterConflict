#include "OCR13EnvironmentDressingSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
    constexpr float DressingDelaySeconds = 1.60f;
    constexpr float MownSpacingCm = 450.0f;
    constexpr float RoughSpacingCm = 600.0f;
    constexpr float WetlandSpacingCm = 700.0f;
    constexpr int32 MaxCellsPerZone = 2200;

    enum class EGrassZone : uint8
    {
        Mown,
        Rough,
        Wetland,
    };

    struct FExclusionZone
    {
        FVector Center = FVector::ZeroVector;
        float HalfX = 0.0f;
        float HalfY = 0.0f;
        float Yaw = 0.0f;

        bool Contains(const FVector& Point, const float PaddingCm) const
        {
            const FVector Delta = Point - Center;
            const FVector Local = FRotator(0.0f, -Yaw, 0.0f).RotateVector(Delta);
            return FMath::Abs(Local.X) <= HalfX + PaddingCm &&
                FMath::Abs(Local.Y) <= HalfY + PaddingCm;
        }
    };

    UStaticMesh* LoadDressingMesh(const TCHAR* Path)
    {
        return LoadObject<UStaticMesh>(nullptr, Path);
    }

    uint32 StableSeed(const FVector& Location, const int32 Index, const int32 Salt)
    {
        uint32 Hash = 2166136261u;
        const int32 X = FMath::RoundToInt(Location.X * 0.01f);
        const int32 Y = FMath::RoundToInt(Location.Y * 0.01f);
        Hash = (Hash ^ static_cast<uint32>(X)) * 16777619u;
        Hash = (Hash ^ static_cast<uint32>(Y)) * 16777619u;
        Hash = (Hash ^ static_cast<uint32>(Index + 1)) * 16777619u;
        Hash = (Hash ^ static_cast<uint32>(Salt + 31)) * 16777619u;
        return Hash;
    }

    float HashUnit(const uint32 Seed)
    {
        return static_cast<float>(Seed & 0x00ffffffu) / static_cast<float>(0x00ffffffu);
    }

    bool IsInsideKrushelnytskaSlice(const FVector& Location)
    {
        // Keep the dedicated R12 street slice as its own visual owner. Whole-Oster art uses the same contract.
        return FMath::Abs(Location.X + 3400.0f) < 7000.0f &&
            Location.Y > -14500.0f && Location.Y < 17500.0f;
    }

    UInstancedStaticMeshComponent* MakeVisualISM(AActor* Owner, USceneComponent* Root, UStaticMesh* Mesh,
        const FName Name, const bool bCastShadow, const int32 CullEndCm)
    {
        if (!Owner || !Root || !Mesh) return nullptr;

        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;

        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        Component->SetCastShadow(bCastShadow);
        if (CullEndCm > 0) Component->SetCullDistances(0, CullEndCm);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    UInstancedStaticMeshComponent* FindISMOnActor(AActor* Actor, const FName Name)
    {
        if (!Actor) return nullptr;
        TArray<UInstancedStaticMeshComponent*> Components;
        Actor->GetComponents<UInstancedStaticMeshComponent>(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (Component && Component->GetFName() == Name) return Component;
        }
        return nullptr;
    }

    TArray<UInstancedStaticMeshComponent*> FindISMsInWorld(UWorld& World, const FName Name)
    {
        TArray<UInstancedStaticMeshComponent*> Matches;
        for (TActorIterator<AActor> It(&World); It; ++It)
        {
            AActor* Actor = *It;
            if (!Actor) continue;
            TArray<UInstancedStaticMeshComponent*> Components;
            Actor->GetComponents<UInstancedStaticMeshComponent>(Components);
            for (UInstancedStaticMeshComponent* Component : Components)
            {
                if (Component && Component->GetFName() == Name) Matches.Add(Component);
            }
        }
        return Matches;
    }

    void CollectExclusions(AOCWorldSectorOster* Sector, TArray<FExclusionZone>& OutZones)
    {
        if (!Sector) return;

        static const FName Families[] = {
            TEXT("Roads"), TEXT("Sidewalks"), TEXT("Buildings"), TEXT("LandmarkBlocks"),
            TEXT("StadiumGeometry"), TEXT("StadiumDetails"), TEXT("ParkDetails"), TEXT("Bridges")
        };

        for (const FName Family : Families)
        {
            UInstancedStaticMeshComponent* Component = FindISMOnActor(Sector, Family);
            if (!Component) continue;

            for (int32 Index = 0; Index < Component->GetInstanceCount(); ++Index)
            {
                FTransform Transform;
                if (!Component->GetInstanceTransform(Index, Transform, true)) continue;
                const FVector Scale = Transform.GetScale3D().GetAbs();
                FExclusionZone Zone;
                Zone.Center = Transform.GetLocation();
                Zone.HalfX = FMath::Max(25.0f, Scale.X * 50.0f);
                Zone.HalfY = FMath::Max(25.0f, Scale.Y * 50.0f);
                Zone.Yaw = Transform.Rotator().Yaw;
                OutZones.Add(Zone);
            }
        }
    }

    bool IsExcluded(const FVector& Location, const TArray<FExclusionZone>& Zones, const float PaddingCm)
    {
        for (const FExclusionZone& Zone : Zones)
        {
            if (Zone.Contains(Location, PaddingCm)) return true;
        }
        return false;
    }

    float GrassVisualScale(UStaticMesh* Mesh, const EGrassZone Zone, const uint32 Seed)
    {
        if (!Mesh) return 1.0f;
        const FVector Size = Mesh->GetBounds().BoxExtent * 2.0f;
        const float Footprint = FMath::Max(1.0f, FMath::Max(Size.X, Size.Y));
        const float Random = HashUnit(Seed ^ 0x9e3779b9u);
        float DesiredCm = 250.0f + Random * 70.0f;
        if (Zone == EGrassZone::Rough) DesiredCm = 310.0f + Random * 100.0f;
        if (Zone == EGrassZone::Wetland) DesiredCm = 370.0f + Random * 120.0f;
        return FMath::Clamp(DesiredCm / Footprint, 0.35f, 6.0f);
    }

    void AddAdaptiveGrass(UInstancedStaticMeshComponent* Proxy, const EGrassZone Zone,
        const TArray<UInstancedStaticMeshComponent*>& GrassTargets,
        const TArray<UInstancedStaticMeshComponent*>& PlantTargets,
        const TArray<FExclusionZone>& Exclusions,
        int32& OutGrassCount, int32& OutPlantCount)
    {
        if (!Proxy || GrassTargets.IsEmpty()) return;

        const float BaseSpacing = Zone == EGrassZone::Mown ? MownSpacingCm :
            (Zone == EGrassZone::Rough ? RoughSpacingCm : WetlandSpacingCm);
        const int32 PlantDivisor = Zone == EGrassZone::Mown ? 32 : (Zone == EGrassZone::Rough ? 11 : 7);
        const float PaddingCm = Zone == EGrassZone::Mown ? 80.0f : 130.0f;

        for (int32 ZoneIndex = 0; ZoneIndex < Proxy->GetInstanceCount(); ++ZoneIndex)
        {
            FTransform ZoneTransform;
            if (!Proxy->GetInstanceTransform(ZoneIndex, ZoneTransform, true)) continue;

            const FVector Scale = ZoneTransform.GetScale3D().GetAbs();
            const float WidthCm = FMath::Max(500.0f, Scale.X * 100.0f);
            const float DepthCm = FMath::Max(500.0f, Scale.Y * 100.0f);

            float EffectiveSpacing = BaseSpacing;
            int32 CellsX = FMath::Max(1, FMath::CeilToInt(WidthCm / EffectiveSpacing));
            int32 CellsY = FMath::Max(1, FMath::CeilToInt(DepthCm / EffectiveSpacing));
            const int32 RequestedCells = CellsX * CellsY;
            if (RequestedCells > MaxCellsPerZone)
            {
                EffectiveSpacing *= FMath::Sqrt(static_cast<float>(RequestedCells) /
                    static_cast<float>(MaxCellsPerZone));
                CellsX = FMath::Max(1, FMath::CeilToInt(WidthCm / EffectiveSpacing));
                CellsY = FMath::Max(1, FMath::CeilToInt(DepthCm / EffectiveSpacing));
            }

            const float StepX = WidthCm / static_cast<float>(CellsX);
            const float StepY = DepthCm / static_cast<float>(CellsY);
            const FQuat ZoneRotation = ZoneTransform.GetRotation();
            const FVector Center = ZoneTransform.GetLocation();

            for (int32 X = 0; X < CellsX; ++X)
            {
                for (int32 Y = 0; Y < CellsY; ++Y)
                {
                    const int32 CellIndex = X * CellsY + Y;
                    const uint32 Seed = StableSeed(Center, CellIndex, ZoneIndex + static_cast<int32>(Zone) * 101);
                    const float JitterX = (HashUnit(Seed ^ 0x68bc21ebu) - 0.5f) * StepX * 0.50f;
                    const float JitterY = (HashUnit(Seed ^ 0x02e5be93u) - 0.5f) * StepY * 0.50f;
                    const FVector Local(
                        -WidthCm * 0.5f + (static_cast<float>(X) + 0.5f) * StepX + JitterX,
                        -DepthCm * 0.5f + (static_cast<float>(Y) + 0.5f) * StepY + JitterY,
                        0.0f);
                    FVector Location = Center + ZoneRotation.RotateVector(Local);
                    Location.Z = FMath::Max(3.0f, Center.Z + 1.0f);

                    if (IsInsideKrushelnytskaSlice(Location)) continue;
                    if (IsExcluded(Location, Exclusions, PaddingCm)) continue;

                    UInstancedStaticMeshComponent* Grass = GrassTargets[Seed % GrassTargets.Num()];
                    if (!Grass || !Grass->GetStaticMesh()) continue;
                    const float GrassScale = GrassVisualScale(Grass->GetStaticMesh(), Zone, Seed);
                    const float Yaw = static_cast<float>((Seed >> 8) % 360u);
                    Grass->AddInstance(FTransform(FRotator(0.0f, Yaw, 0.0f), Location, FVector(GrassScale)), true);
                    ++OutGrassCount;

                    if (!PlantTargets.IsEmpty() && static_cast<int32>(Seed % static_cast<uint32>(PlantDivisor)) == 0)
                    {
                        UInstancedStaticMeshComponent* Plant = PlantTargets[(Seed >> 16) % PlantTargets.Num()];
                        if (Plant)
                        {
                            const float PlantScale = Zone == EGrassZone::Wetland
                                ? 0.95f + HashUnit(Seed ^ 0xa511e9b3u) * 0.35f
                                : 0.72f + HashUnit(Seed ^ 0x63d83595u) * 0.30f;
                            const FVector PlantLocation = Location + FVector(
                                (HashUnit(Seed ^ 0x7f4a7c15u) - 0.5f) * 120.0f,
                                (HashUnit(Seed ^ 0x94d049bbu) - 0.5f) * 120.0f,
                                0.0f);
                            Plant->AddInstance(FTransform(
                                FRotator(0.0f, static_cast<float>((Seed >> 3) % 360u), 0.0f),
                                PlantLocation, FVector(PlantScale)), true);
                            ++OutPlantCount;
                        }
                    }
                }
            }
        }
    }

    void GatherTransforms(const TArray<UInstancedStaticMeshComponent*>& Components, TArray<FTransform>& OutTransforms)
    {
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (!Component) continue;
            for (int32 Index = 0; Index < Component->GetInstanceCount(); ++Index)
            {
                FTransform Transform;
                if (Component->GetInstanceTransform(Index, Transform, true)) OutTransforms.Add(Transform);
            }
        }
    }

    FVector OffsetFromTransform(const FTransform& Transform, const FVector& LocalOffset)
    {
        return Transform.GetLocation() + Transform.GetRotation().RotateVector(LocalOffset);
    }

    bool FindYardLocation(const FTransform& HouseTransform, const uint32 Seed,
        const TArray<FExclusionZone>& Exclusions, FVector& OutLocation)
    {
        static const FVector CandidateOffsets[] = {
            FVector(-760.0f, 820.0f, 0.0f), FVector(760.0f, 820.0f, 0.0f),
            FVector(-920.0f, 120.0f, 0.0f), FVector(920.0f, 120.0f, 0.0f),
            FVector(-650.0f, 1180.0f, 0.0f), FVector(650.0f, 1180.0f, 0.0f),
        };

        for (int32 Attempt = 0; Attempt < UE_ARRAY_COUNT(CandidateOffsets); ++Attempt)
        {
            const int32 Index = (static_cast<int32>(Seed % UE_ARRAY_COUNT(CandidateOffsets)) + Attempt) %
                UE_ARRAY_COUNT(CandidateOffsets);
            FVector Candidate = OffsetFromTransform(HouseTransform, CandidateOffsets[Index]);
            Candidate.Z = FMath::Max(4.0f, HouseTransform.GetLocation().Z + 4.0f);
            if (!IsExcluded(Candidate, Exclusions, 30.0f))
            {
                OutLocation = Candidate;
                return true;
            }
        }
        return false;
    }
}

bool UOCR13EnvironmentDressingSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13EnvironmentDressingSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) ApplyEnvironmentDressing(*World);
        }), DressingDelaySeconds, false);
}

void UOCR13EnvironmentDressingSubsystem::ApplyEnvironmentDressing(UWorld& World)
{
    if (bApplied) return;

    AOCWorldSectorOster* Sector = nullptr;
    for (TActorIterator<AOCWorldSectorOster> It(&World); It; ++It)
    {
        Sector = *It;
        if (Sector) break;
    }
    if (!Sector) return;

    UStaticMesh* GrassMeshes[] = {
        LoadDressingMesh(TEXT("/Game/PN_FoliageCollection/Meshes/grassMesh/grass_01_01_mesh.grass_01_01_mesh")),
        LoadDressingMesh(TEXT("/Game/PN_FoliageCollection/Meshes/grassMesh/grass_01_02_mesh.grass_01_02_mesh")),
        LoadDressingMesh(TEXT("/Game/PN_FoliageCollection/Meshes/grassMesh/grass_01_03_mesh.grass_01_03_mesh")),
    };
    const TCHAR* GrassFallbacks[] = {
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_GrassPatch_Var01.SM_GrassPatch_Var01"),
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_GrassPatch_Var02.SM_GrassPatch_Var02"),
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_GrassPatch_Var03.SM_GrassPatch_Var03"),
    };
    for (int32 Index = 0; Index < 3; ++Index)
    {
        if (!GrassMeshes[Index]) GrassMeshes[Index] = LoadDressingMesh(GrassFallbacks[Index]);
    }

    UStaticMesh* PlantMeshes[] = {
        LoadDressingMesh(TEXT("/Game/PN_FoliageCollection/Meshes/groundPlantMesh/ground_01_01.ground_01_01")),
        LoadDressingMesh(TEXT("/Game/PN_FoliageCollection/Meshes/groundPlantMesh/ground_01_02.ground_01_02")),
        LoadDressingMesh(TEXT("/Game/PN_FoliageCollection/Meshes/groundPlantMesh/ground_01_03.ground_01_03")),
    };
    UStaticMesh* PlantFallback = LoadDressingMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Plant.SM_Plant"));
    for (UStaticMesh*& Plant : PlantMeshes)
    {
        if (!Plant) Plant = PlantFallback;
    }

    TArray<UStaticMesh*> House01ExtraMeshes;
    for (int32 Index = 1; Index <= 8; ++Index)
    {
        const FString Path = FString::Printf(
            TEXT("/Game/AdvancedVillagePack/Meshes/SM_House_Var01_Extra%02d.SM_House_Var01_Extra%02d"), Index, Index);
        if (UStaticMesh* Mesh = LoadDressingMesh(*Path)) House01ExtraMeshes.Add(Mesh);
    }
    UStaticMesh* House02ExtraMesh = LoadDressingMesh(
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_House_Var02_Extra.SM_House_Var02_Extra"));

    UStaticMesh* LogsMesh = LoadDressingMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Logs_Var01.SM_Logs_Var01"));
    UStaticMesh* CrateMesh = LoadDressingMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Crate_Closed.SM_Crate_Closed"));
    UStaticMesh* BarrelMesh = LoadDressingMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Barrel.SM_Barrel"));
    UStaticMesh* CartMesh = LoadDressingMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Cart_Var02.SM_Cart_Var02"));
    UStaticMesh* WellMesh = LoadDressingMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Well.SM_Well"));
    UStaticMesh* StonePathMesh = LoadDressingMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Stonepath_Var02.SM_Stonepath_Var02"));

    UStaticMesh* CompanionTreeMeshes[] = {
        LoadDressingMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var03.SM_Tree_Var03")),
        LoadDressingMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var04.SM_Tree_Var04")),
        LoadDressingMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var05.SM_Tree_Var05")),
    };
    UStaticMesh* StumpMeshes[] = {
        LoadDressingMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Treestump_Var01.SM_Treestump_Var01")),
        LoadDressingMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Treestump_Var02.SM_Treestump_Var02")),
    };

    bool bAnyVisualAsset = House02ExtraMesh || LogsMesh || CompanionTreeMeshes[0] || PlantFallback;
    for (UStaticMesh* Mesh : GrassMeshes) bAnyVisualAsset |= Mesh != nullptr;
    for (UStaticMesh* Mesh : House01ExtraMeshes) bAnyVisualAsset |= Mesh != nullptr;
    if (!bAnyVisualAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("R13 environment dressing: bundled environment art unavailable; pass skipped."));
        return;
    }

    AActor* DressingRoot = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!DressingRoot) return;
    DressingRoot->SetReplicates(false);
    DressingRoot->SetActorEnableCollision(false);

    USceneComponent* Root = NewObject<USceneComponent>(DressingRoot, TEXT("R13_EnvironmentDressingRoot"));
    if (!Root) return;
    DressingRoot->SetRootComponent(Root);
    DressingRoot->AddInstanceComponent(Root);
    Root->RegisterComponent();

    TArray<UInstancedStaticMeshComponent*> GrassTargets;
    for (int32 Index = 0; Index < 3; ++Index)
    {
        if (UInstancedStaticMeshComponent* Component = MakeVisualISM(DressingRoot, Root, GrassMeshes[Index],
            FName(*FString::Printf(TEXT("R13_DenseGrass%02d"), Index + 1)), false, 26000))
        {
            GrassTargets.Add(Component);
        }
    }

    TArray<UInstancedStaticMeshComponent*> PlantTargets;
    for (int32 Index = 0; Index < 3; ++Index)
    {
        if (UInstancedStaticMeshComponent* Component = MakeVisualISM(DressingRoot, Root, PlantMeshes[Index],
            FName(*FString::Printf(TEXT("R13_GroundPlant%02d"), Index + 1)), false, 22000))
        {
            PlantTargets.Add(Component);
        }
    }

    TArray<UInstancedStaticMeshComponent*> House01Extras;
    for (int32 Index = 0; Index < House01ExtraMeshes.Num(); ++Index)
    {
        if (UInstancedStaticMeshComponent* Component = MakeVisualISM(DressingRoot, Root, House01ExtraMeshes[Index],
            FName(*FString::Printf(TEXT("R13_House01Extra%02d"), Index + 1)), true, 90000))
        {
            House01Extras.Add(Component);
        }
    }
    UInstancedStaticMeshComponent* House02Extra = MakeVisualISM(
        DressingRoot, Root, House02ExtraMesh, TEXT("R13_House02Extra"), true, 90000);

    UInstancedStaticMeshComponent* Logs = MakeVisualISM(DressingRoot, Root, LogsMesh, TEXT("R13_YardLogs"), true, 50000);
    UInstancedStaticMeshComponent* Crates = MakeVisualISM(DressingRoot, Root, CrateMesh, TEXT("R13_YardCrates"), true, 45000);
    UInstancedStaticMeshComponent* Barrels = MakeVisualISM(DressingRoot, Root, BarrelMesh, TEXT("R13_YardBarrels"), true, 45000);
    UInstancedStaticMeshComponent* Carts = MakeVisualISM(DressingRoot, Root, CartMesh, TEXT("R13_YardCarts"), true, 55000);
    UInstancedStaticMeshComponent* Wells = MakeVisualISM(DressingRoot, Root, WellMesh, TEXT("R13_YardWells"), true, 65000);
    UInstancedStaticMeshComponent* StonePaths = MakeVisualISM(DressingRoot, Root, StonePathMesh, TEXT("R13_YardStonePaths"), false, 40000);

    TArray<UInstancedStaticMeshComponent*> CompanionTrees;
    for (int32 Index = 0; Index < 3; ++Index)
    {
        if (UInstancedStaticMeshComponent* Component = MakeVisualISM(DressingRoot, Root, CompanionTreeMeshes[Index],
            FName(*FString::Printf(TEXT("R13_CompanionTree%02d"), Index + 1)), true, 80000))
        {
            CompanionTrees.Add(Component);
        }
    }
    TArray<UInstancedStaticMeshComponent*> Stumps;
    for (int32 Index = 0; Index < 2; ++Index)
    {
        if (UInstancedStaticMeshComponent* Component = MakeVisualISM(DressingRoot, Root, StumpMeshes[Index],
            FName(*FString::Printf(TEXT("R13_TreeStump%02d"), Index + 1)), true, 40000))
        {
            Stumps.Add(Component);
        }
    }

    TArray<FExclusionZone> Exclusions;
    CollectExclusions(Sector, Exclusions);

    int32 GrassCount = 0;
    int32 PlantCount = 0;
    AddAdaptiveGrass(FindISMOnActor(Sector, TEXT("GrassMown")), EGrassZone::Mown,
        GrassTargets, PlantTargets, Exclusions, GrassCount, PlantCount);
    AddAdaptiveGrass(FindISMOnActor(Sector, TEXT("GrassRough")), EGrassZone::Rough,
        GrassTargets, PlantTargets, Exclusions, GrassCount, PlantCount);
    AddAdaptiveGrass(FindISMOnActor(Sector, TEXT("GrassWetland")), EGrassZone::Wetland,
        GrassTargets, PlantTargets, Exclusions, GrassCount, PlantCount);

    TArray<FTransform> House01Transforms;
    TArray<FTransform> House02Transforms;
    GatherTransforms(FindISMsInWorld(World, TEXT("R13_House01")), House01Transforms);
    GatherTransforms(FindISMsInWorld(World, TEXT("R12_House01")), House01Transforms);
    GatherTransforms(FindISMsInWorld(World, TEXT("R13_House02")), House02Transforms);
    GatherTransforms(FindISMsInWorld(World, TEXT("R12_House02")), House02Transforms);

    int32 HouseExtraCount = 0;
    int32 YardPropCount = 0;
    auto DressHouses = [&](const TArray<FTransform>& Houses, const bool bHouse01)
    {
        for (int32 Index = 0; Index < Houses.Num(); ++Index)
        {
            const FTransform& House = Houses[Index];
            const FVector HouseLocation = House.GetLocation();
            const uint32 Seed = StableSeed(HouseLocation, Index, bHouse01 ? 401 : 509);

            if (bHouse01 && !House01Extras.IsEmpty() && Seed % 5u != 0u)
            {
                UInstancedStaticMeshComponent* Extra = House01Extras[(Seed >> 4) % House01Extras.Num()];
                if (Extra)
                {
                    Extra->AddInstance(House, true);
                    ++HouseExtraCount;
                }
            }
            else if (!bHouse01 && House02Extra && Seed % 3u != 0u)
            {
                House02Extra->AddInstance(House, true);
                ++HouseExtraCount;
            }

            FVector YardLocation;
            if (!FindYardLocation(House, Seed, Exclusions, YardLocation)) continue;
            FRotator PropRotation = House.Rotator();
            PropRotation.Pitch = 0.0f;
            PropRotation.Roll = 0.0f;

            auto AddProp = [&](UInstancedStaticMeshComponent* Target, const uint32 Divisor,
                const float Scale, const float YawOffset)
            {
                if (!Target || Divisor == 0u || Seed % Divisor != 0u) return;
                FRotator Rotation = PropRotation;
                Rotation.Yaw += YawOffset + static_cast<float>((Seed >> 12) % 25u) - 12.0f;
                Target->AddInstance(FTransform(Rotation, YardLocation, FVector(Scale)), true);
                ++YardPropCount;
            };

            AddProp(Logs, 4u, 0.86f, 90.0f);
            AddProp(Crates, 6u, 0.86f, 0.0f);
            AddProp(Barrels, 9u, 0.90f, 0.0f);
            AddProp(Carts, 13u, 0.88f, 35.0f);
            AddProp(Wells, 19u, 0.92f, 0.0f);
            AddProp(StonePaths, 5u, 0.90f, 0.0f);
        }
    };
    DressHouses(House01Transforms, true);
    DressHouses(House02Transforms, false);

    TArray<FTransform> PrimaryTrees;
    for (int32 Variant = 1; Variant <= 5; ++Variant)
    {
        GatherTransforms(FindISMsInWorld(World, FName(*FString::Printf(TEXT("R13_Tree%02d"), Variant))), PrimaryTrees);
        GatherTransforms(FindISMsInWorld(World, FName(*FString::Printf(TEXT("R12_Tree%02d"), Variant))), PrimaryTrees);
    }

    int32 CompanionTreeCount = 0;
    int32 StumpCount = 0;
    for (int32 Index = 0; Index < PrimaryTrees.Num(); ++Index)
    {
        const FTransform& Primary = PrimaryTrees[Index];
        const FVector BaseLocation = Primary.GetLocation();
        const uint32 Seed = StableSeed(BaseLocation, Index, 733);

        if (!CompanionTrees.IsEmpty() && Seed % 3u == 0u)
        {
            const float Radius = 480.0f + HashUnit(Seed ^ 0x4cf5ad43u) * 440.0f;
            const float Angle = FMath::DegreesToRadians(static_cast<float>((Seed >> 7) % 360u));
            FVector Location = BaseLocation + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.0f);
            Location.Z = FMath::Max(0.0f, BaseLocation.Z);
            if (!IsExcluded(Location, Exclusions, 90.0f))
            {
                UInstancedStaticMeshComponent* Target = CompanionTrees[(Seed >> 13) % CompanionTrees.Num()];
                const float Scale = 0.55f + HashUnit(Seed ^ 0x1b56c4e9u) * 0.22f;
                Target->AddInstance(FTransform(
                    FRotator(0.0f, static_cast<float>((Seed >> 3) % 360u), 0.0f),
                    Location, FVector(Scale)), true);
                ++CompanionTreeCount;
            }
        }

        if (!Stumps.IsEmpty() && Seed % 17u == 0u)
        {
            const float Angle = FMath::DegreesToRadians(static_cast<float>((Seed >> 10) % 360u));
            FVector Location = BaseLocation + FVector(FMath::Cos(Angle) * 330.0f, FMath::Sin(Angle) * 330.0f, 0.0f);
            Location.Z = FMath::Max(0.0f, BaseLocation.Z);
            if (!IsExcluded(Location, Exclusions, 40.0f))
            {
                UInstancedStaticMeshComponent* Target = Stumps[(Seed >> 18) % Stumps.Num()];
                Target->AddInstance(FTransform(
                    FRotator(0.0f, static_cast<float>((Seed >> 5) % 360u), 0.0f),
                    Location, FVector(0.80f + HashUnit(Seed ^ 0x846ca68bu) * 0.18f)), true);
                ++StumpCount;
            }
        }
    }

    bApplied = true;
    UE_LOG(LogTemp, Display,
        TEXT("R13 environment dressing: grass=%d plants=%d house extras=%d yard props=%d companion trees=%d stumps=%d."),
        GrassCount, PlantCount, HouseExtraCount, YardPropCount, CompanionTreeCount, StumpCount);
}
