#include "OCR13CivicLandscapingSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
    constexpr float CivicLandscapingDelaySeconds = 1.90f;

    UStaticMesh* LoadLandscapeMesh(const TCHAR* Path)
    {
        return LoadObject<UStaticMesh>(nullptr, Path);
    }

    UInstancedStaticMeshComponent* MakeLandscapeISM(AActor* Owner, USceneComponent* Root, UStaticMesh* Mesh,
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

    struct FLandscapeSeed
    {
        FVector Offset;
        float Yaw;
        float Scale;
        int32 Variant;
    };

    void AddSeedSet(const FVector& Anchor, const FLandscapeSeed* Seeds, const int32 SeedCount,
        const TArray<UInstancedStaticMeshComponent*>& Targets, int32& OutCount)
    {
        if (!Seeds || SeedCount <= 0 || Targets.IsEmpty()) return;
        for (int32 Index = 0; Index < SeedCount; ++Index)
        {
            const FLandscapeSeed& Seed = Seeds[Index];
            UInstancedStaticMeshComponent* Target = Targets[Seed.Variant % Targets.Num()];
            if (!Target) continue;
            FVector Location = Anchor + Seed.Offset;
            Location.Z = FMath::Max(2.0f, Location.Z);
            Target->AddInstance(FTransform(
                FRotator(0.0f, Seed.Yaw, 0.0f), Location, FVector(Seed.Scale)), true);
            ++OutCount;
        }
    }

    void AddMuseumGarden(const FVector& Museum,
        const TArray<UInstancedStaticMeshComponent*>& Shrubs,
        const TArray<UInstancedStaticMeshComponent*>& Flowers,
        const TArray<UInstancedStaticMeshComponent*>& Grass,
        int32& ShrubCount, int32& FlowerCount, int32& GrassCount)
    {
        // Keep the glazed porch and stair approach clear. Planting stays inside/along the established garden fence.
        const FLandscapeSeed ShrubSeeds[] = {
            {{-2650, -1700, 2},  15, 0.80f, 0}, {{-2700, -850, 2},  80, 0.92f, 1},
            {{-2720,   250, 2}, 155, 0.82f, 2}, {{-2700,  1350, 2}, 230, 0.88f, 0},
            {{ 2700,  -450, 2},  20, 0.84f, 1}, {{ 2680,   700, 2}, 105, 0.94f, 2},
            {{ 2600,  1600, 2}, 185, 0.80f, 0}, {{-1900,  2050, 2}, 270, 0.92f, 1},
            {{ -650,  2100, 2},  35, 0.86f, 2}, {{  700,  2100, 2}, 115, 0.90f, 0},
            {{ 1850,  2050, 2}, 200, 0.84f, 1}
        };
        const FLandscapeSeed FlowerSeeds[] = {
            {{-2200, -2050, 3},  10, 0.82f, 0}, {{-1450, -2050, 3},  30, 0.76f, 0},
            {{ -650, -2060, 3}, 350, 0.82f, 0}, {{ 2100, -2030, 3},  18, 0.80f, 0},
            {{ 2450,  1500, 3}, 105, 0.72f, 0}, {{-2350,  1550, 3}, 245, 0.76f, 0}
        };
        const FLandscapeSeed GrassSeeds[] = {
            {{-2300,  1850, 2},  12, 0.82f, 0}, {{-1300,  1900, 2},  75, 0.88f, 0},
            {{ 1250,  1900, 2}, 142, 0.84f, 0}, {{ 2250,  1750, 2}, 215, 0.90f, 0}
        };
        AddSeedSet(Museum, ShrubSeeds, UE_ARRAY_COUNT(ShrubSeeds), Shrubs, ShrubCount);
        AddSeedSet(Museum, FlowerSeeds, UE_ARRAY_COUNT(FlowerSeeds), Flowers, FlowerCount);
        AddSeedSet(Museum, GrassSeeds, UE_ARRAY_COUNT(GrassSeeds), Grass, GrassCount);
    }

    void AddCentralParkBorders(const FVector& Park,
        const TArray<UInstancedStaticMeshComponent*>& Shrubs,
        const TArray<UInstancedStaticMeshComponent*>& Flowers,
        const TArray<UInstancedStaticMeshComponent*>& Grass,
        int32& ShrubCount, int32& FlowerCount, int32& GrassCount)
    {
        // Border clusters avoid the main east-west/north-south alleys and the central memorial plaza.
        const FLandscapeSeed ShrubSeeds[] = {
            {{-9000,-6500,2},  20,1.00f,0}, {{-6800,-6750,2},  70,0.92f,1},
            {{-4300,-6900,2}, 120,0.88f,2}, {{-1500,-7000,2}, 165,0.96f,0},
            {{ 1800,-7000,2}, 210,0.92f,1}, {{ 4500,-6850,2}, 255,0.90f,2},
            {{ 8200,-6500,2}, 300,0.98f,0}, {{-9000, 6500,2},  45,0.92f,1},
            {{-6200, 6900,2},  95,0.98f,2}, {{-3400, 7000,2}, 145,0.88f,0},
            {{ 3200, 7000,2}, 205,0.92f,1}, {{ 6000, 6850,2}, 255,0.96f,2},
            {{ 9000, 6400,2}, 315,0.90f,0}, {{-9300,-3500,2},  80,0.92f,1},
            {{-9400, 3200,2}, 130,0.88f,2}, {{ 9400, 3300,2}, 220,0.92f,0}
        };
        const FLandscapeSeed FlowerSeeds[] = {
            {{-7600,-6100,3}, 15,0.78f,0}, {{-3000,-6250,3}, 85,0.82f,0},
            {{ 2800,-6250,3},155,0.78f,0}, {{ 7600,-6000,3},225,0.82f,0},
            {{-7200, 6100,3}, 40,0.76f,0}, {{-2600, 6250,3},110,0.80f,0},
            {{ 2600, 6250,3},180,0.78f,0}, {{ 7200, 6050,3},250,0.82f,0}
        };
        const FLandscapeSeed GrassSeeds[] = {
            {{-8500,-5400,2},  5,0.90f,0}, {{-5200,-5600,2}, 65,0.96f,0},
            {{ 5200,-5550,2},125,0.94f,0}, {{ 8500,-5250,2},185,0.90f,0},
            {{-8300, 5400,2},245,0.94f,0}, {{-5000, 5650,2},305,0.90f,0},
            {{ 5000, 5600,2}, 35,0.96f,0}, {{ 8300, 5300,2}, 95,0.92f,0}
        };
        AddSeedSet(Park, ShrubSeeds, UE_ARRAY_COUNT(ShrubSeeds), Shrubs, ShrubCount);
        AddSeedSet(Park, FlowerSeeds, UE_ARRAY_COUNT(FlowerSeeds), Flowers, FlowerCount);
        AddSeedSet(Park, GrassSeeds, UE_ARRAY_COUNT(GrassSeeds), Grass, GrassCount);
    }

    void AddCollegeCampusPlanting(const FVector& College,
        const TArray<UInstancedStaticMeshComponent*>& Shrubs,
        const TArray<UInstancedStaticMeshComponent*>& Flowers,
        const TArray<UInstancedStaticMeshComponent*>& Grass,
        int32& ShrubCount, int32& FlowerCount, int32& GrassCount)
    {
        // Front planting avoids the entrance/stair axis around X=900 and follows the existing campus perimeter.
        const FLandscapeSeed ShrubSeeds[] = {
            {{-4700,-2050,2},  15,0.84f,0}, {{-3600,-2070,2},  55,0.88f,1},
            {{-2450,-2090,2}, 100,0.82f,2}, {{-1300,-2110,2}, 145,0.90f,0},
            {{ 2850,-2100,2}, 195,0.86f,1}, {{ 4050,-2050,2}, 240,0.90f,2},
            {{-5000, 8200,2}, 285,0.94f,0}, {{-3000, 8500,2}, 325,0.86f,1},
            {{ 1000, 8500,2},  25,0.90f,2}, {{ 3000, 8400,2},  70,0.92f,0},
            {{ 5200, 8200,2}, 115,0.86f,1}
        };
        const FLandscapeSeed FlowerSeeds[] = {
            {{-4150,-1800,3},  5,0.74f,0}, {{-2950,-1820,3}, 55,0.78f,0},
            {{-1750,-1840,3},105,0.74f,0}, {{ 3150,-1830,3},165,0.78f,0},
            {{ 4350,-1800,3},215,0.74f,0}
        };
        const FLandscapeSeed GrassSeeds[] = {
            {{-4700, 7600,2}, 20,0.88f,0}, {{-2500, 7900,2}, 90,0.92f,0},
            {{  500, 7900,2},160,0.90f,0}, {{ 3000, 7800,2},230,0.92f,0}
        };
        AddSeedSet(College, ShrubSeeds, UE_ARRAY_COUNT(ShrubSeeds), Shrubs, ShrubCount);
        AddSeedSet(College, FlowerSeeds, UE_ARRAY_COUNT(FlowerSeeds), Flowers, FlowerCount);
        AddSeedSet(College, GrassSeeds, UE_ARRAY_COUNT(GrassSeeds), Grass, GrassCount);
    }

    void AddStadiumPerimeterPlanting(const FVector& Stadium,
        const TArray<UInstancedStaticMeshComponent*>& Shrubs,
        const TArray<UInstancedStaticMeshComponent*>& Grass,
        int32& ShrubCount, int32& GrassCount)
    {
        // Everything remains outside the authored 12.4 x 8.6 m-equivalent fence rectangle.
        const FLandscapeSeed ShrubSeeds[] = {
            {{-7600,-5200,2},  10,0.90f,0}, {{-6000,-5400,2},  50,0.86f,1},
            {{ 5200,-5600,2}, 100,0.90f,2}, {{ 7200,-5300,2}, 145,0.86f,0},
            {{-7600, 5200,2}, 190,0.92f,1}, {{-5200, 5400,2}, 235,0.88f,2},
            {{-2000, 5550,2}, 280,0.90f,0}, {{ 1800, 5550,2}, 325,0.88f,1},
            {{ 5200, 5400,2},  25,0.92f,2}, {{ 7600, 5100,2},  70,0.86f,0},
            {{-7600,-1800,2}, 115,0.88f,1}, {{-7700, 1800,2}, 160,0.90f,2},
            {{ 7700,-1800,2}, 205,0.88f,0}, {{ 7700, 1800,2}, 250,0.90f,1}
        };
        const FLandscapeSeed GrassSeeds[] = {
            {{-7000,-4700,2}, 15,0.90f,0}, {{ 6500,-4750,2}, 85,0.94f,0},
            {{-6800, 4750,2},155,0.92f,0}, {{ 6800, 4700,2},225,0.90f,0},
            {{-7000,    0,2},295,0.94f,0}, {{ 7000,    0,2},  5,0.92f,0}
        };
        AddSeedSet(Stadium, ShrubSeeds, UE_ARRAY_COUNT(ShrubSeeds), Shrubs, ShrubCount);
        AddSeedSet(Stadium, GrassSeeds, UE_ARRAY_COUNT(GrassSeeds), Grass, GrassCount);
    }
}

bool UOCR13CivicLandscapingSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13CivicLandscapingSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) ApplyCivicLandscaping(*World);
        }), CivicLandscapingDelaySeconds, false);
}

void UOCR13CivicLandscapingSubsystem::ApplyCivicLandscaping(UWorld& World)
{
    if (bApplied) return;

    UStaticMesh* ShrubMesh01 = LoadLandscapeMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/Shrubs_1.Shrubs_1"));
    UStaticMesh* ShrubMesh02 = LoadLandscapeMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/Shrubs_1_Single.Shrubs_1_Single"));
    UStaticMesh* ShrubMesh03 = LoadLandscapeMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/Bush_1.Bush_1"));
    UStaticMesh* FlowerMesh = LoadLandscapeMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/Flower_Patch_1.Flower_Patch_1"));
    UStaticMesh* LongGrassMesh = LoadLandscapeMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/Grass_Patch_Long.Grass_Patch_Long"));

    if (!ShrubMesh01 && !ShrubMesh02 && !ShrubMesh03 && !FlowerMesh && !LongGrassMesh) return;

    AActor* LandscapeRoot = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!LandscapeRoot) return;
    LandscapeRoot->SetReplicates(false);
    LandscapeRoot->SetActorEnableCollision(false);

    USceneComponent* Root = NewObject<USceneComponent>(LandscapeRoot, TEXT("R13_CivicLandscapingRoot"));
    if (!Root) return;
    LandscapeRoot->SetRootComponent(Root);
    LandscapeRoot->AddInstanceComponent(Root);
    Root->SetMobility(EComponentMobility::Static);
    Root->RegisterComponent();

    TArray<UInstancedStaticMeshComponent*> Shrubs;
    if (UInstancedStaticMeshComponent* C = MakeLandscapeISM(LandscapeRoot, Root, ShrubMesh01, TEXT("R13_CivicShrub01"), false, 42000)) Shrubs.Add(C);
    if (UInstancedStaticMeshComponent* C = MakeLandscapeISM(LandscapeRoot, Root, ShrubMesh02, TEXT("R13_CivicShrub02"), false, 42000)) Shrubs.Add(C);
    if (UInstancedStaticMeshComponent* C = MakeLandscapeISM(LandscapeRoot, Root, ShrubMesh03, TEXT("R13_CivicShrub03"), false, 42000)) Shrubs.Add(C);

    TArray<UInstancedStaticMeshComponent*> Flowers;
    if (UInstancedStaticMeshComponent* C = MakeLandscapeISM(LandscapeRoot, Root, FlowerMesh, TEXT("R13_CivicFlowerPatch"), false, 26000)) Flowers.Add(C);

    TArray<UInstancedStaticMeshComponent*> Grass;
    if (UInstancedStaticMeshComponent* C = MakeLandscapeISM(LandscapeRoot, Root, LongGrassMesh, TEXT("R13_CivicLongGrass"), false, 30000)) Grass.Add(C);

    int32 ShrubCount = 0;
    int32 FlowerCount = 0;
    int32 GrassCount = 0;

    AddMuseumGarden(AOCWorldSectorOster::MuseumAnchor(), Shrubs, Flowers, Grass,
        ShrubCount, FlowerCount, GrassCount);
    AddCentralParkBorders(AOCWorldSectorOster::ParkAnchor(), Shrubs, Flowers, Grass,
        ShrubCount, FlowerCount, GrassCount);
    AddCollegeCampusPlanting(AOCWorldSectorOster::CollegeAnchor(), Shrubs, Flowers, Grass,
        ShrubCount, FlowerCount, GrassCount);
    AddStadiumPerimeterPlanting(AOCWorldSectorOster::StadiumAnchor(), Shrubs, Grass,
        ShrubCount, GrassCount);

    if (ShrubCount + FlowerCount + GrassCount <= 0)
    {
        LandscapeRoot->Destroy();
        return;
    }

    bApplied = true;
    UE_LOG(LogTemp, Display,
        TEXT("R13 civic landscaping: shrubs=%d flowers=%d grass accents=%d; landmark collision/massing unchanged."),
        ShrubCount, FlowerCount, GrassCount);
}
