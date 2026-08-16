#include "OCKrushelnytskaVisualSliceSubsystem.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
    constexpr float StreetCenterX = -33500.0f;
    constexpr float StreetStartY = 20500.0f;
    constexpr float StreetStepY = 3000.0f;
    constexpr int32 LotCountPerSide = 10;

    UStaticMesh* LoadMesh(const TCHAR* Path)
    {
        UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, Path);
        if (!Mesh)
        {
            UE_LOG(LogTemp, Warning, TEXT("R12 Krushelnytska: missing mesh %s"), Path);
        }
        return Mesh;
    }

    UInstancedStaticMeshComponent* MakeISM(AActor* Owner, USceneComponent* Root, UStaticMesh* Mesh,
        const FName& Name, bool bCollision)
    {
        if (!Owner || !Root || !Mesh) return nullptr;

        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionProfileName(FName(bCollision ? TEXT("BlockAll") : TEXT("NoCollision")));
        Component->SetCastShadow(true);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void AddInstance(UInstancedStaticMeshComponent* Component, const FVector& Location, const FRotator& Rotation,
        const FVector& Scale = FVector::OneVector)
    {
        if (!Component) return;
        Component->AddInstance(FTransform(Rotation, Location, Scale), true);
    }

    void HideR11ResidentialProxies(UWorld& World)
    {
        static const TSet<FName> HiddenComponentNames =
        {
            TEXT("Sidewalks"),
            TEXT("Buildings"),
            TEXT("ResidentialRoofs"),
            TEXT("ResidentialDetails"),
            TEXT("Fences"),
            TEXT("WoodFences"),
            TEXT("MetalFences"),
            TEXT("LightSheetFences"),
            TEXT("TreeTrunks"),
            TEXT("TreeCrowns"),
            TEXT("SovietPoplarTrunks"),
            TEXT("SovietPoplarCrowns"),
            TEXT("BirchTrunks"),
            TEXT("BirchCrowns"),
            TEXT("PineTrunks"),
            TEXT("PineCrowns"),
            TEXT("GrassMown"),
            TEXT("GrassRough"),
            TEXT("GrassWetland")
        };

        for (TActorIterator<AActor> It(&World); It; ++It)
        {
            AActor* Actor = *It;
            if (!Actor || !Actor->GetClass()->GetName().Contains(TEXT("OCWorldSectorOster"))) continue;

            TInlineComponentArray<UActorComponent*> Components;
            Actor->GetComponents(Components);
            for (UActorComponent* Component : Components)
            {
                UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Component);
                if (!Primitive || !HiddenComponentNames.Contains(Primitive->GetFName())) continue;
                Primitive->SetVisibility(false, true);
                Primitive->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            }
        }
    }
}

bool UOCKrushelnytskaVisualSliceSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    if (!World) return false;
    return World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE;
}

void UOCKrushelnytskaVisualSliceSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    const FString MapName = InWorld.GetMapName();
    if (!MapName.Contains(TEXT("OsterConflict_Runtime"))) return;

    // UWorldSubsystem::OnWorldBeginPlay runs before GameMode::BeginPlay. R11 spawns OCWorldSectorOster from
    // GameMode::BeginPlay, so defer one short tick before hiding its proxy components and placing the R12 slice.
    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle BuildTimer;
    InWorld.GetTimerManager().SetTimer(BuildTimer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get())
            {
                BuildVisualSlice(*World);
            }
        }),
        0.25f, false);
}

void UOCKrushelnytskaVisualSliceSubsystem::BuildVisualSlice(UWorld& World)
{
    HideR11ResidentialProxies(World);

    AActor* VisualRoot = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!VisualRoot) return;
    VisualRoot->SetActorEnableCollision(true);

    USceneComponent* Root = NewObject<USceneComponent>(VisualRoot, TEXT("R12_Krushelnytska_Root"));
    VisualRoot->SetRootComponent(Root);
    VisualRoot->AddInstanceComponent(Root);
    Root->RegisterComponent();

    // These are real content meshes already committed through Git LFS, replacing R11 cube/sphere proxies.
    UStaticMesh* House01 = LoadMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_House_Var01.SM_House_Var01"));
    UStaticMesh* House02 = LoadMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_House_Var02.SM_House_Var02"));
    UStaticMesh* Fence01 = LoadMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Fence_Var01.SM_Fence_Var01"));
    UStaticMesh* Fence02 = LoadMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Fence_Var02.SM_Fence_Var02"));
    UStaticMesh* Fence03 = LoadMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Fence_Var03.SM_Fence_Var03"));
    UStaticMesh* Fence04 = LoadMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Fence_Var04.SM_Fence_Var04"));
    UStaticMesh* Tree01 = LoadMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var01.SM_Tree_Var01"));
    UStaticMesh* Tree02 = LoadMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var02.SM_Tree_Var02"));
    UStaticMesh* Tree03 = LoadMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var03.SM_Tree_Var03"));
    UStaticMesh* Tree04 = LoadMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var04.SM_Tree_Var04"));
    UStaticMesh* Tree05 = LoadMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var05.SM_Tree_Var05"));

    UInstancedStaticMeshComponent* House01ISM = MakeISM(VisualRoot, Root, House01, TEXT("R12_House01"), true);
    UInstancedStaticMeshComponent* House02ISM = MakeISM(VisualRoot, Root, House02, TEXT("R12_House02"), true);
    UInstancedStaticMeshComponent* Fence01ISM = MakeISM(VisualRoot, Root, Fence01, TEXT("R12_Fence01"), true);
    UInstancedStaticMeshComponent* Fence02ISM = MakeISM(VisualRoot, Root, Fence02, TEXT("R12_Fence02"), true);
    UInstancedStaticMeshComponent* Fence03ISM = MakeISM(VisualRoot, Root, Fence03, TEXT("R12_Fence03"), true);
    UInstancedStaticMeshComponent* Fence04ISM = MakeISM(VisualRoot, Root, Fence04, TEXT("R12_Fence04"), true);
    UInstancedStaticMeshComponent* Tree01ISM = MakeISM(VisualRoot, Root, Tree01, TEXT("R12_Tree01"), true);
    UInstancedStaticMeshComponent* Tree02ISM = MakeISM(VisualRoot, Root, Tree02, TEXT("R12_Tree02"), true);
    UInstancedStaticMeshComponent* Tree03ISM = MakeISM(VisualRoot, Root, Tree03, TEXT("R12_Tree03"), true);
    UInstancedStaticMeshComponent* Tree04ISM = MakeISM(VisualRoot, Root, Tree04, TEXT("R12_Tree04"), true);
    UInstancedStaticMeshComponent* Tree05ISM = MakeISM(VisualRoot, Root, Tree05, TEXT("R12_Tree05"), true);

    UInstancedStaticMeshComponent* FenceFamilies[] = { Fence01ISM, Fence02ISM, Fence03ISM, Fence04ISM };
    UInstancedStaticMeshComponent* TreeFamilies[] = { Tree01ISM, Tree02ISM, Tree03ISM, Tree04ISM, Tree05ISM };

    // Street-reference composition: narrow asphalt, sandy/grass verges, detached low houses behind tall frontage,
    // dense mature deciduous trees and irregular spacing. Deliberately non-uniform: Oster is not a suburb grid.
    for (int32 Index = 0; Index < LotCountPerSide; ++Index)
    {
        const float Y = StreetStartY + StreetStepY * static_cast<float>(Index);
        const float Jitter = static_cast<float>((Index % 3) - 1) * 180.0f;

        const FVector WestHouse(StreetCenterX - 4700.0f - (Index % 2) * 280.0f, Y + 650.0f + Jitter, 0.0f);
        const FVector EastHouse(StreetCenterX + 4550.0f + (Index % 3) * 190.0f, Y - 380.0f - Jitter, 0.0f);
        const float WestYaw = 88.0f + static_cast<float>((Index % 3) - 1) * 2.0f;
        const float EastYaw = -89.0f + static_cast<float>((Index % 2) * 3);

        AddInstance((Index % 2 == 0) ? House01ISM : House02ISM,
            WestHouse, FRotator(0.0f, WestYaw, 0.0f), FVector(0.92f + 0.03f * (Index % 3)));
        AddInstance((Index % 3 == 0) ? House01ISM : House02ISM,
            EastHouse, FRotator(0.0f, EastYaw, 0.0f), FVector(0.90f + 0.025f * (Index % 4)));

        // Mixed frontage families instead of one repeated fence. Reference photos show wood, metal and panel fences.
        UInstancedStaticMeshComponent* WestFence = FenceFamilies[Index % UE_ARRAY_COUNT(FenceFamilies)];
        UInstancedStaticMeshComponent* EastFence = FenceFamilies[(Index + 2) % UE_ARRAY_COUNT(FenceFamilies)];
        for (int32 Segment = -2; Segment <= 2; ++Segment)
        {
            const float SegmentY = Y + Segment * 520.0f;
            if (!(Index == 3 && Segment == 0))
            {
                AddInstance(WestFence, FVector(StreetCenterX - 2450.0f, SegmentY, 0.0f),
                    FRotator(0.0f, 90.0f, 0.0f), FVector(1.0f, 1.0f, 1.08f));
            }
            if (!(Index == 6 && (Segment == 0 || Segment == 1)))
            {
                AddInstance(EastFence, FVector(StreetCenterX + 2450.0f, SegmentY + 110.0f, 0.0f),
                    FRotator(0.0f, 90.0f, 0.0f), FVector(1.0f, 1.0f, 1.12f));
            }
        }

        // Mature canopy close to the carriageway, with extra yard trees behind the frontage.
        UInstancedStaticMeshComponent* WestTree = TreeFamilies[Index % UE_ARRAY_COUNT(TreeFamilies)];
        UInstancedStaticMeshComponent* EastTree = TreeFamilies[(Index + 3) % UE_ARRAY_COUNT(TreeFamilies)];
        AddInstance(WestTree, FVector(StreetCenterX - 3150.0f - (Index % 2) * 240.0f, Y - 520.0f, 0.0f),
            FRotator(0.0f, Index * 23.0f, 0.0f), FVector(1.08f + 0.05f * (Index % 4)));
        AddInstance(EastTree, FVector(StreetCenterX + 3250.0f + (Index % 3) * 170.0f, Y + 430.0f, 0.0f),
            FRotator(0.0f, Index * 31.0f, 0.0f), FVector(1.02f + 0.06f * ((Index + 1) % 4)));

        if ((Index % 2) == 0)
        {
            AddInstance(TreeFamilies[(Index + 1) % UE_ARRAY_COUNT(TreeFamilies)],
                FVector(StreetCenterX - 5200.0f, Y + 1280.0f, 0.0f), FRotator(0.0f, Index * 17.0f, 0.0f), FVector(0.90f));
        }
        if ((Index % 3) != 1)
        {
            AddInstance(TreeFamilies[(Index + 4) % UE_ARRAY_COUNT(TreeFamilies)],
                FVector(StreetCenterX + 5350.0f, Y - 1100.0f, 0.0f), FRotator(0.0f, Index * 19.0f, 0.0f), FVector(0.94f));
        }
    }

    UE_LOG(LogTemp, Display, TEXT("R12 Krushelnytska visual slice built with AdvancedVillagePack meshes."));
}
