#include "OCR13SilpoFoliageUpgradeSubsystem.h"

#include "OCGameMode.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
    constexpr float SilpoFoliageUpgradeDelaySeconds = 6.35f;

    AActor* FindSilpoModel(UWorld& World)
    {
        for (TActorIterator<AActor> It(&World); It; ++It)
        {
            AActor* Actor = *It;
            if (Actor && Actor->ActorHasTag(TEXT("R13_SilpoPhotoModel"))) return Actor;
        }
        return nullptr;
    }

    UInstancedStaticMeshComponent* AddFoliageComponent(AActor* Owner, USceneComponent* Root,
        UStaticMesh* Mesh, const FName Name)
    {
        if (!Owner || !Root || !Mesh) return nullptr;
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionProfileName(TEXT("NoCollision"));
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        Component->SetCastShadow(true);
        Component->SetCullDistances(0, 45000);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void AddPlant(UInstancedStaticMeshComponent* Component, const float X, const float Y,
        const float Z, const float Scale, const float Yaw)
    {
        if (!Component) return;
        Component->AddInstance(FTransform(FRotator(0.0f, Yaw, 0.0f), FVector(X, Y, Z), FVector(Scale)), false);
    }

    void HideProceduralPlantFallback(AActor* Model)
    {
        if (!Model) return;
        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Model->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (!Component) continue;
            const FName Name = Component->GetFName();
            if (Name == TEXT("R13SilpoSite_FlowerStems") ||
                Name == TEXT("R13SilpoSite_Shrubs") ||
                Name == TEXT("R13SilpoSite_OrangeFlowers") ||
                Name == TEXT("R13SilpoSite_YellowFlowers"))
            {
                Component->SetVisibility(false, true);
                Component->SetHiddenInGame(true, true);
            }
        }
    }
}

bool UOCR13SilpoFoliageUpgradeSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13SilpoFoliageUpgradeSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) UpgradeFoliage(*World);
        }), SilpoFoliageUpgradeDelaySeconds, false);
}

void UOCR13SilpoFoliageUpgradeSubsystem::UpgradeFoliage(UWorld& World)
{
    AActor* Model = FindSilpoModel(World);
    if (!Model || Model->ActorHasTag(TEXT("R13_SilpoFoliageUpgradeApplied"))) return;

    USceneComponent* Root = Model->GetRootComponent();
    if (!Root) return;

    UStaticMesh* FlowerA = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/PN_FoliageCollection/Meshes/flowerMesh/flower_01_01.flower_01_01"));
    UStaticMesh* FlowerB = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/PN_FoliageCollection/Meshes/flowerMesh/flower_02_03.flower_02_03"));
    UStaticMesh* FlowerC = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/PN_FoliageCollection/Meshes/flowerMesh/flower_03_02.flower_03_02"));
    UStaticMesh* GroundA = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/PN_FoliageCollection/Meshes/groundPlantMesh/ground_01_03.ground_01_03"));
    UStaticMesh* GroundB = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/PN_FoliageCollection/Meshes/groundPlantMesh/ground_02_02.ground_02_02"));

    int32 LoadedMeshCount = 0;
    for (UStaticMesh* Mesh : { FlowerA, FlowerB, FlowerC, GroundA, GroundB })
    {
        if (Mesh) ++LoadedMeshCount;
    }

    const bool bHasFlowerMesh = FlowerA || FlowerB || FlowerC;
    const bool bHasGroundMesh = GroundA || GroundB;
    if (!bHasFlowerMesh || !bHasGroundMesh)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("R13 Silpo foliage upgrade: incomplete PN foliage payload (%d/5 meshes, flower=%d ground=%d); procedural fallback kept."),
            LoadedMeshCount, bHasFlowerMesh ? 1 : 0, bHasGroundMesh ? 1 : 0);
        return;
    }

    UInstancedStaticMeshComponent* FlowerCompA = AddFoliageComponent(Model, Root, FlowerA,
        TEXT("R13SilpoFoliage_FlowerA"));
    UInstancedStaticMeshComponent* FlowerCompB = AddFoliageComponent(Model, Root, FlowerB,
        TEXT("R13SilpoFoliage_FlowerB"));
    UInstancedStaticMeshComponent* FlowerCompC = AddFoliageComponent(Model, Root, FlowerC,
        TEXT("R13SilpoFoliage_FlowerC"));
    UInstancedStaticMeshComponent* GroundCompA = AddFoliageComponent(Model, Root, GroundA,
        TEXT("R13SilpoFoliage_GroundA"));
    UInstancedStaticMeshComponent* GroundCompB = AddFoliageComponent(Model, Root, GroundB,
        TEXT("R13SilpoFoliage_GroundB"));

    const float Xs[] = { -900.0f, -785.0f, -655.0f, -515.0f, -360.0f, -205.0f, -45.0f,
        115.0f, 275.0f, 440.0f, 600.0f, 760.0f, 920.0f, 1075.0f, 1225.0f };
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(Xs); ++Index)
    {
        const float Y = -1088.0f + static_cast<float>((Index % 3) - 1) * 19.0f;
        const float Scale = 0.62f + static_cast<float>((Index * 7) % 5) * 0.07f;
        const float Yaw = static_cast<float>((Index * 47) % 360);

        UInstancedStaticMeshComponent* Flower = nullptr;
        switch (Index % 3)
        {
        case 0: Flower = FlowerCompA ? FlowerCompA : (FlowerCompB ? FlowerCompB : FlowerCompC); break;
        case 1: Flower = FlowerCompB ? FlowerCompB : (FlowerCompC ? FlowerCompC : FlowerCompA); break;
        default: Flower = FlowerCompC ? FlowerCompC : (FlowerCompA ? FlowerCompA : FlowerCompB); break;
        }
        AddPlant(Flower, Xs[Index], Y, 18.0f, Scale, Yaw);

        UInstancedStaticMeshComponent* Ground = (Index % 2 == 0)
            ? (GroundCompA ? GroundCompA : GroundCompB)
            : (GroundCompB ? GroundCompB : GroundCompA);
        AddPlant(Ground, Xs[Index] + 34.0f, Y + 9.0f, 16.0f, 0.55f + Scale * 0.28f, Yaw + 71.0f);
    }

    HideProceduralPlantFallback(Model);
    Model->Tags.Add(TEXT("R13_SilpoFoliageUpgradeApplied"));
    UE_LOG(LogTemp, Display,
        TEXT("R13 Silpo foliage upgrade: PN flower/ground meshes loaded=%d/5; procedural plant fallback hidden."),
        LoadedMeshCount);
}
