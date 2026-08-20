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
    // R12.2: keep the first visual slice around the normal gameplay spawn instead of hiding it tens of metres away.
    // The street now crosses the origin area, so a fresh player should see real houses/trees/grass almost immediately.
    constexpr float StreetCenterX = -3400.0f;
    constexpr float StreetStartY = -12000.0f;
    constexpr float StreetStepY = 3000.0f;
    constexpr int32 LotCountPerSide = 10;

    UStaticMesh* LoadMesh(const TCHAR* Path, const bool bWarn = true)
    {
        UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, Path);
        if (!Mesh && bWarn)
        {
            UE_LOG(LogTemp, Warning, TEXT("R13 Krushelnytska: missing mesh %s"), Path);
        }
        return Mesh;
    }

    UStaticMesh* LoadPreferredMesh(const TCHAR* Preferred, const TCHAR* Fallback)
    {
        if (UStaticMesh* Mesh = LoadMesh(Preferred, false)) return Mesh;
        return LoadMesh(Fallback, true);
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
        Component->SetCastShadow(bCollision);
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

    void AddGroundedTree(UInstancedStaticMeshComponent* Component, UStaticMesh* Mesh,
        FVector Location, const float DesiredHeightCm, const float Yaw)
    {
        if (!Component || !Mesh) return;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector Size = Bounds.BoxExtent * 2.0f;
        if (Size.Z <= 10.0f) return;
        const float Scale = FMath::Clamp(DesiredHeightCm / Size.Z, 0.30f, 4.0f);
        const float LocalBottom = Bounds.Origin.Z - Bounds.BoxExtent.Z;
        Location.Z = -LocalBottom * Scale;
        Component->AddInstance(FTransform(FRotator(0.0f, Yaw, 0.0f), Location, FVector(Scale)), true);
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

    // R13.4: use the same committed foliage family as the whole-city pass so this street no longer reads like
    // a separate prototype map. AdvancedVillage grass remains a fallback for old archives without PN payloads.
    UStaticMesh* Grass01 = LoadPreferredMesh(
        TEXT("/Game/PN_FoliageCollection/Meshes/grassMesh/grass_01_01_mesh.grass_01_01_mesh"),
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_GrassPatch_Var01.SM_GrassPatch_Var01"));
    UStaticMesh* Grass02 = LoadPreferredMesh(
        TEXT("/Game/PN_FoliageCollection/Meshes/grassMesh/grass_01_02_mesh.grass_01_02_mesh"),
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_GrassPatch_Var02.SM_GrassPatch_Var02"));
    UStaticMesh* Grass03 = LoadPreferredMesh(
        TEXT("/Game/PN_FoliageCollection/Meshes/grassMesh/grass_01_03_mesh.grass_01_03_mesh"),
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_GrassPatch_Var03.SM_GrassPatch_Var03"));

    UStaticMesh* Pine01 = LoadMesh(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_01.SM_Pine_Tree_01"), false);
    UStaticMesh* Pine03 = LoadMesh(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_03.SM_Pine_Tree_03"), false);
    UStaticMesh* Pine05 = LoadMesh(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_05.SM_Pine_Tree_05"), false);

    UStaticMesh* Plant = LoadMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Plant.SM_Plant"));
    UStaticMesh* StreetLight = LoadMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_StreetLight.SM_StreetLight"));
    UStaticMesh* Barrel = LoadMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Barrel.SM_Barrel"));
    UStaticMesh* Crate = LoadMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Crate_Closed.SM_Crate_Closed"));

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
    UInstancedStaticMeshComponent* Pine01ISM = MakeISM(VisualRoot, Root, Pine01, TEXT("R13_KrushelnytskaPine01"), true);
    UInstancedStaticMeshComponent* Pine03ISM = MakeISM(VisualRoot, Root, Pine03, TEXT("R13_KrushelnytskaPine03"), true);
    UInstancedStaticMeshComponent* Pine05ISM = MakeISM(VisualRoot, Root, Pine05, TEXT("R13_KrushelnytskaPine05"), true);
    UInstancedStaticMeshComponent* Grass01ISM = MakeISM(VisualRoot, Root, Grass01, TEXT("R13_KrushelnytskaGrass01"), false);
    UInstancedStaticMeshComponent* Grass02ISM = MakeISM(VisualRoot, Root, Grass02, TEXT("R13_KrushelnytskaGrass02"), false);
    UInstancedStaticMeshComponent* Grass03ISM = MakeISM(VisualRoot, Root, Grass03, TEXT("R13_KrushelnytskaGrass03"), false);
    UInstancedStaticMeshComponent* PlantISM = MakeISM(VisualRoot, Root, Plant, TEXT("R12_Plants"), false);
    UInstancedStaticMeshComponent* StreetLightISM = MakeISM(VisualRoot, Root, StreetLight, TEXT("R12_StreetLights"), false);
    UInstancedStaticMeshComponent* BarrelISM = MakeISM(VisualRoot, Root, Barrel, TEXT("R12_Barrels"), true);
    UInstancedStaticMeshComponent* CrateISM = MakeISM(VisualRoot, Root, Crate, TEXT("R12_Crates"), true);

    UInstancedStaticMeshComponent* FenceFamilies[] = { Fence01ISM, Fence02ISM, Fence03ISM, Fence04ISM };
    UInstancedStaticMeshComponent* TreeFamilies[] = { Tree01ISM, Tree02ISM, Tree03ISM, Tree04ISM, Tree05ISM };
    UInstancedStaticMeshComponent* GrassFamilies[] = { Grass01ISM, Grass02ISM, Grass03ISM };
    UInstancedStaticMeshComponent* PineFamilies[] = { Pine01ISM, Pine03ISM, Pine05ISM };
    UStaticMesh* PineMeshes[] = { Pine01, Pine03, Pine05 };

    // Street-reference composition: narrow residential corridor, detached low houses behind mixed frontage,
    // dense mature deciduous trees, grass/sandy verges and irregular yards. Deliberately non-uniform.
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

        // A few real conifers break up the deciduous wall without turning every residential yard into a pine forest.
        if ((Index % 3) == 0)
        {
            const int32 PineIndex = (Index / 3) % UE_ARRAY_COUNT(PineFamilies);
            AddGroundedTree(PineFamilies[PineIndex], PineMeshes[PineIndex],
                FVector(StreetCenterX - 5650.0f, Y - 650.0f, 0.0f),
                1550.0f + 90.0f * static_cast<float>(Index % 4), static_cast<float>(Index * 41));
        }
        if ((Index % 4) == 1)
        {
            const int32 PineIndex = (Index + 1) % UE_ARRAY_COUNT(PineFamilies);
            AddGroundedTree(PineFamilies[PineIndex], PineMeshes[PineIndex],
                FVector(StreetCenterX + 5750.0f, Y + 900.0f, 0.0f),
                1650.0f + 70.0f * static_cast<float>(Index % 3), static_cast<float>(Index * 53));
        }

        // Denser PN foliage on shoulders and inside yards. The central carriageway remains clear.
        for (int32 Patch = 0; Patch < 6; ++Patch)
        {
            const float PatchY = Y - 1150.0f + Patch * 470.0f + static_cast<float>((Index + Patch) % 3) * 55.0f;
            const float WestX = StreetCenterX - 2850.0f - static_cast<float>((Patch + Index) % 3) * 420.0f;
            const float EastX = StreetCenterX + 2850.0f + static_cast<float>((Patch + Index + 1) % 3) * 410.0f;
            UInstancedStaticMeshComponent* WestGrass = GrassFamilies[(Index + Patch) % UE_ARRAY_COUNT(GrassFamilies)];
            UInstancedStaticMeshComponent* EastGrass = GrassFamilies[(Index + Patch + 1) % UE_ARRAY_COUNT(GrassFamilies)];
            AddInstance(WestGrass, FVector(WestX, PatchY, 2.0f), FRotator(0.0f, 31.0f * (Index + Patch), 0.0f),
                FVector(0.92f + 0.07f * (Patch % 3)));
            AddInstance(EastGrass, FVector(EastX, PatchY + 100.0f, 2.0f), FRotator(0.0f, 27.0f * (Index + Patch), 0.0f),
                FVector(0.90f + 0.08f * ((Patch + 1) % 3)));
        }

        AddInstance(PlantISM, FVector(StreetCenterX - 4050.0f, Y + 1050.0f, 3.0f),
            FRotator(0.0f, Index * 37.0f, 0.0f), FVector(0.9f + 0.08f * (Index % 3)));
        AddInstance(PlantISM, FVector(StreetCenterX + 4150.0f, Y - 950.0f, 3.0f),
            FRotator(0.0f, Index * 29.0f, 0.0f), FVector(0.85f + 0.07f * ((Index + 1) % 3)));

        // Kept as a legacy source marker; the R13 whole-Oster pass suppresses this fantasy light family.
        if ((Index % 2) == 0)
        {
            AddInstance(StreetLightISM, FVector(StreetCenterX + 2050.0f, Y - 1100.0f, 0.0f),
                FRotator(0.0f, -90.0f, 0.0f), FVector(1.0f));
        }

        if (Index == 2 || Index == 7)
        {
            AddInstance(BarrelISM, FVector(StreetCenterX - 4200.0f, Y + 1350.0f, 0.0f),
                FRotator(0.0f, Index * 21.0f, 0.0f), FVector(0.90f));
            AddInstance(CrateISM, FVector(StreetCenterX - 4450.0f, Y + 1500.0f, 0.0f),
                FRotator(0.0f, Index * 13.0f, 0.0f), FVector(0.92f));
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("R13.4 Krushelnytska visual slice built with unified PN grass, real conifers, houses, yard trees and sparse props."));
}
