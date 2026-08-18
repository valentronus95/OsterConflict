#include "OCR13CentralParkDressingSubsystem.h"

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
    constexpr float ParkDressingDelaySeconds = 2.00f;

    struct FParkSeed
    {
        FVector Offset;
        float Yaw;
        float Scale;
        int32 Variant;
    };

    UStaticMesh* LoadParkMesh(const TCHAR* Path)
    {
        return LoadObject<UStaticMesh>(nullptr, Path);
    }

    UInstancedStaticMeshComponent* MakeParkISM(AActor* Owner, USceneComponent* Root, UStaticMesh* Mesh,
        const FName Name, const int32 CullEndCm)
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
        Component->SetCastShadow(false);
        if (CullEndCm > 0) Component->SetCullDistances(0, CullEndCm);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void AddSeeds(const FVector& Park, const FParkSeed* Seeds, const int32 SeedCount,
        const TArray<UInstancedStaticMeshComponent*>& Targets, int32& OutCount)
    {
        if (!Seeds || SeedCount <= 0 || Targets.IsEmpty()) return;
        for (int32 Index = 0; Index < SeedCount; ++Index)
        {
            const FParkSeed& Seed = Seeds[Index];
            UInstancedStaticMeshComponent* Target = Targets[Seed.Variant % Targets.Num()];
            if (!Target) continue;
            FVector Location = Park + Seed.Offset;
            Location.Z = FMath::Max(2.0f, Location.Z);
            Target->AddInstance(FTransform(FRotator(0.0f, Seed.Yaw, 0.0f), Location, FVector(Seed.Scale)), true);
            ++OutCount;
        }
    }
}

bool UOCR13CentralParkDressingSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13CentralParkDressingSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) ApplyCentralParkDressing(*World);
        }), ParkDressingDelaySeconds, false);
}

void UOCR13CentralParkDressingSubsystem::ApplyCentralParkDressing(UWorld& World)
{
    if (bApplied) return;

    UStaticMesh* ShrubMeshes[] = {
        LoadParkMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/Shrubs_1.Shrubs_1")),
        LoadParkMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/Shrubs_1_Single.Shrubs_1_Single")),
        LoadParkMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/Bush_1.Bush_1")),
    };
    UStaticMesh* FlowerMesh = LoadParkMesh(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/Flower_Patch_1.Flower_Patch_1"));
    UStaticMesh* GrassMesh = LoadParkMesh(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/Grass_Patch_Long.Grass_Patch_Long"));

    bool bAnyMesh = FlowerMesh != nullptr || GrassMesh != nullptr;
    for (UStaticMesh* Mesh : ShrubMeshes) bAnyMesh |= Mesh != nullptr;
    if (!bAnyMesh) return;

    AActor* ArtRoot = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!ArtRoot) return;
    ArtRoot->SetReplicates(false);
    ArtRoot->SetActorEnableCollision(false);

    USceneComponent* Root = NewObject<USceneComponent>(ArtRoot, TEXT("R13_CentralParkDressingRoot"));
    if (!Root)
    {
        ArtRoot->Destroy();
        return;
    }
    ArtRoot->SetRootComponent(Root);
    ArtRoot->AddInstanceComponent(Root);
    Root->SetMobility(EComponentMobility::Static);
    Root->RegisterComponent();

    TArray<UInstancedStaticMeshComponent*> Shrubs;
    for (int32 Index = 0; Index < 3; ++Index)
    {
        if (UInstancedStaticMeshComponent* Component = MakeParkISM(ArtRoot, Root, ShrubMeshes[Index],
            FName(*FString::Printf(TEXT("R13_CentralParkShrub%02d"), Index + 1)), 38000))
        {
            Shrubs.Add(Component);
        }
    }

    TArray<UInstancedStaticMeshComponent*> Flowers;
    if (UInstancedStaticMeshComponent* Component = MakeParkISM(
        ArtRoot, Root, FlowerMesh, TEXT("R13_CentralParkFlowers"), 26000))
    {
        Flowers.Add(Component);
    }
    TArray<UInstancedStaticMeshComponent*> Grass;
    if (UInstancedStaticMeshComponent* Component = MakeParkISM(
        ArtRoot, Root, GrassMesh, TEXT("R13_CentralParkLongGrass"), 30000))
    {
        Grass.Add(Component);
    }

    const FParkSeed ShrubSeeds[] = {
        {{-9000,-6500,2},  20,1.00f,0}, {{-6800,-6750,2},  70,0.92f,1},
        {{-4300,-6900,2}, 120,0.88f,2}, {{-1500,-7000,2}, 165,0.96f,0},
        {{ 1800,-7000,2}, 210,0.92f,1}, {{ 4500,-6850,2}, 255,0.90f,2},
        {{ 8200,-6500,2}, 300,0.98f,0}, {{-9000, 6500,2},  45,0.92f,1},
        {{-6200, 6900,2},  95,0.98f,2}, {{-3400, 7000,2}, 145,0.88f,0},
        {{ 3200, 7000,2}, 205,0.92f,1}, {{ 6000, 6850,2}, 255,0.96f,2},
        {{ 9000, 6400,2}, 315,0.90f,0}, {{-9300,-3500,2},  80,0.92f,1},
        {{-9400, 3200,2}, 130,0.88f,2}, {{ 9400, 3300,2}, 220,0.92f,0}
    };
    const FParkSeed FlowerSeeds[] = {
        {{-7600,-6100,3}, 15,0.78f,0}, {{-3000,-6250,3}, 85,0.82f,0},
        {{ 2800,-6250,3},155,0.78f,0}, {{ 7600,-6000,3},225,0.82f,0},
        {{-7200, 6100,3}, 40,0.76f,0}, {{-2600, 6250,3},110,0.80f,0},
        {{ 2600, 6250,3},180,0.78f,0}, {{ 7200, 6050,3},250,0.82f,0}
    };
    const FParkSeed GrassSeeds[] = {
        {{-8500,-5400,2},  5,0.90f,0}, {{-5200,-5600,2}, 65,0.96f,0},
        {{ 5200,-5550,2},125,0.94f,0}, {{ 8500,-5250,2},185,0.90f,0},
        {{-8300, 5400,2},245,0.94f,0}, {{-5000, 5650,2},305,0.90f,0},
        {{ 5000, 5600,2}, 35,0.96f,0}, {{ 8300, 5300,2}, 95,0.92f,0}
    };

    const FVector Park = AOCWorldSectorOster::ParkAnchor();
    int32 ShrubCount = 0;
    int32 FlowerCount = 0;
    int32 GrassCount = 0;
    AddSeeds(Park, ShrubSeeds, UE_ARRAY_COUNT(ShrubSeeds), Shrubs, ShrubCount);
    AddSeeds(Park, FlowerSeeds, UE_ARRAY_COUNT(FlowerSeeds), Flowers, FlowerCount);
    AddSeeds(Park, GrassSeeds, UE_ARRAY_COUNT(GrassSeeds), Grass, GrassCount);

    if (ShrubCount + FlowerCount + GrassCount <= 0)
    {
        ArtRoot->Destroy();
        return;
    }

    bApplied = true;
    UE_LOG(LogTemp, Display,
        TEXT("R13 central park dressing: shrubs=%d flowers=%d long-grass accents=%d; alleys/memorial/nav unchanged."),
        ShrubCount, FlowerCount, GrassCount);
}
