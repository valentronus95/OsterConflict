#include "OCR13MuseumReferenceSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"

namespace
{
    UInstancedStaticMeshComponent* MakeISM(AActor* Owner, USceneComponent* Root, UStaticMesh* Mesh,
        UMaterialInterface* Material, const FName Name, const bool bCollision, const bool bCastShadow)
    {
        if (!Owner || !Root || !Mesh) return nullptr;
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        if (Material) Component->SetMaterial(0, Material);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionProfileName(FName(bCollision ? TEXT("BlockAll") : TEXT("NoCollision")));
        Component->SetCollisionEnabled(bCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        Component->SetCastShadow(bCastShadow);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    UMaterialInstanceDynamic* MakeColorMaterial(AActor* Owner, UMaterialInterface* Base,
        const FName Name, const FLinearColor& Color)
    {
        if (!Owner || !Base) return nullptr;
        UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Base, Owner, Name);
        if (Material) Material->SetVectorParameterValue(TEXT("Color"), Color);
        return Material;
    }

    void AddBox(UInstancedStaticMeshComponent* Component, const FVector& Center, const FVector& SizeCm)
    {
        if (!Component) return;
        Component->AddInstance(FTransform(FRotator::ZeroRotator, Center, SizeCm / 100.0f), true);
    }

    void AddRotatedBox(UInstancedStaticMeshComponent* Component, const FVector& Center,
        const FVector& SizeCm, const FRotator& Rotation)
    {
        if (!Component) return;
        Component->AddInstance(FTransform(Rotation, Center, SizeCm / 100.0f), true);
    }

    void AddGroundedTree(UInstancedStaticMeshComponent* Component, UStaticMesh* Mesh,
        const FVector& XYLocation, const float DesiredHeightCm, const float Yaw)
    {
        if (!Component || !Mesh) return;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector MeshSize = Bounds.BoxExtent * 2.0f;
        if (MeshSize.Z <= 10.0f) return;

        const float Scale = FMath::Clamp(DesiredHeightCm / MeshSize.Z, 0.30f, 4.0f);
        const float LocalBottom = Bounds.Origin.Z - Bounds.BoxExtent.Z;
        FVector Location = XYLocation;
        Location.Z = -LocalBottom * Scale;
        Component->AddInstance(FTransform(FRotator(0.0f, Yaw, 0.0f), Location, FVector(Scale)), true);
    }
}

bool UOCR13MuseumReferenceSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13MuseumReferenceSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    // Run after whole-city replacement + window/roof/chimney bridges. LandmarkSiteDressing remains the owner of
    // the long museum approach path; this pass only owns photo-specific facade/entrance/glazing details.
    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) BuildMuseumReferenceLayer(*World);
        }), 1.55f, false);
}

void UOCR13MuseumReferenceSubsystem::BuildMuseumReferenceLayer(UWorld& World)
{
    AOCWorldSectorOster* WorldSector = nullptr;
    for (TActorIterator<AOCWorldSectorOster> It(&World); It; ++It)
    {
        WorldSector = *It;
        if (WorldSector) break;
    }
    if (!WorldSector) return;

    UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!CubeMesh || !BaseMaterial) return;

    UStaticMesh* Pine01 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_01.SM_Pine_Tree_01"));
    UStaticMesh* Pine03 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_03.SM_Pine_Tree_03"));
    UStaticMesh* Pine05 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_05.SM_Pine_Tree_05"));
    UMaterialInterface* GlassMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Glass_Window.Glass_Window"));
    UMaterialInterface* PaintedBlueWood = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Wood_Planks_Painted_Blue.Wood_Planks_Painted_Blue"));

    AActor* ArtRoot = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!ArtRoot) return;
    ArtRoot->SetReplicates(false);
    ArtRoot->SetActorEnableCollision(false);

    USceneComponent* Root = NewObject<USceneComponent>(ArtRoot, TEXT("R13_MuseumReferenceRoot"));
    if (!Root)
    {
        ArtRoot->Destroy();
        return;
    }
    Root->SetMobility(EComponentMobility::Static);
    ArtRoot->SetRootComponent(Root);
    ArtRoot->AddInstanceComponent(Root);
    Root->RegisterComponent();

    UMaterialInstanceDynamic* PlinthMaterial = MakeColorMaterial(ArtRoot, BaseMaterial,
        TEXT("R13_MuseumPlinthMat"), FLinearColor(0.045f, 0.042f, 0.038f, 1.0f));
    UMaterialInstanceDynamic* BrickMaterial = MakeColorMaterial(ArtRoot, BaseMaterial,
        TEXT("R13_MuseumBrickMat"), FLinearColor(0.43f, 0.17f, 0.095f, 1.0f));
    UMaterialInstanceDynamic* UpperFallbackMaterial = MakeColorMaterial(ArtRoot, BaseMaterial,
        TEXT("R13_MuseumUpperFallbackMat"), FLinearColor(0.28f, 0.34f, 0.36f, 1.0f));
    UMaterialInterface* UpperSurfaceMaterial = PaintedBlueWood ? PaintedBlueWood : UpperFallbackMaterial;
    UMaterialInstanceDynamic* TrimMaterial = MakeColorMaterial(ArtRoot, BaseMaterial,
        TEXT("R13_MuseumTrimMat"), FLinearColor(0.78f, 0.76f, 0.68f, 1.0f));
    UMaterialInstanceDynamic* DoorMaterial = MakeColorMaterial(ArtRoot, BaseMaterial,
        TEXT("R13_MuseumDoorMat"), FLinearColor(0.22f, 0.24f, 0.24f, 1.0f));
    UMaterialInstanceDynamic* StepMaterial = MakeColorMaterial(ArtRoot, BaseMaterial,
        TEXT("R13_MuseumStepMat"), FLinearColor(0.34f, 0.35f, 0.33f, 1.0f));

    UInstancedStaticMeshComponent* Plinth = MakeISM(ArtRoot, Root, CubeMesh, PlinthMaterial,
        TEXT("R13_MuseumDarkPlinth"), false, true);
    UInstancedStaticMeshComponent* BrickAccents = MakeISM(ArtRoot, Root, CubeMesh, BrickMaterial,
        TEXT("R13_MuseumBrickAccents"), false, true);
    UInstancedStaticMeshComponent* UpperCladding = MakeISM(ArtRoot, Root, CubeMesh, UpperSurfaceMaterial,
        TEXT("R13_MuseumBlueGreyUpper"), false, true);
    UInstancedStaticMeshComponent* Trim = MakeISM(ArtRoot, Root, CubeMesh, TrimMaterial,
        TEXT("R13_MuseumPaleTrim"), false, true);
    UInstancedStaticMeshComponent* Doors = MakeISM(ArtRoot, Root, CubeMesh, DoorMaterial,
        TEXT("R13_MuseumGreyDoors"), false, true);
    UInstancedStaticMeshComponent* Steps = MakeISM(ArtRoot, Root, CubeMesh, StepMaterial,
        TEXT("R13_MuseumEntranceSteps"), false, false);
    UInstancedStaticMeshComponent* Glass = MakeISM(ArtRoot, Root, CubeMesh, GlassMaterial,
        TEXT("R13_MuseumSideGlazing"), false, false);

    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();

    AddBox(Plinth, Museum + FVector(0.0f, -883.0f, 43.0f), FVector(3400.0f, 18.0f, 86.0f));
    AddBox(Plinth, Museum + FVector(-2050.0f, -633.0f, 43.0f), FVector(1100.0f, 18.0f, 86.0f));
    AddBox(Plinth, Museum + FVector(2050.0f, -633.0f, 43.0f), FVector(1100.0f, 18.0f, 86.0f));

    AddBox(BrickAccents, Museum + FVector(0.0f, -894.0f, 525.0f), FVector(3400.0f, 22.0f, 52.0f));
    AddBox(BrickAccents, Museum + FVector(-2050.0f, -644.0f, 500.0f), FVector(1100.0f, 22.0f, 48.0f));
    AddBox(BrickAccents, Museum + FVector(2050.0f, -644.0f, 500.0f), FVector(1100.0f, 22.0f, 48.0f));

    // The raised upper volume is painted blue-grey timber in the supplied museum reference photos.
    AddBox(UpperCladding, Museum + FVector(50.0f, -566.0f, 720.0f), FVector(1540.0f, 20.0f, 410.0f));
    AddBox(UpperCladding, Museum + FVector(-728.0f, 80.0f, 720.0f), FVector(18.0f, 1260.0f, 410.0f));
    AddBox(UpperCladding, Museum + FVector(828.0f, 80.0f, 720.0f), FVector(18.0f, 1260.0f, 410.0f));

    AddRotatedBox(Trim, Museum + FVector(-315.0f, -580.0f, 962.0f), FVector(720.0f, 20.0f, 28.0f),
        FRotator(0.0f, 0.0f, -28.0f));
    AddRotatedBox(Trim, Museum + FVector(415.0f, -580.0f, 962.0f), FVector(720.0f, 20.0f, 28.0f),
        FRotator(0.0f, 0.0f, 28.0f));
    AddBox(Trim, Museum + FVector(50.0f, -581.0f, 792.0f), FVector(1260.0f, 20.0f, 24.0f));

    AddBox(Doors, Museum + FVector(1092.0f, -1438.0f, 265.0f), FVector(166.0f, 16.0f, 410.0f));
    AddBox(Doors, Museum + FVector(1268.0f, -1438.0f, 265.0f), FVector(166.0f, 16.0f, 410.0f));
    AddBox(Trim, Museum + FVector(995.0f, -1448.0f, 265.0f), FVector(28.0f, 18.0f, 450.0f));
    AddBox(Trim, Museum + FVector(1365.0f, -1448.0f, 265.0f), FVector(28.0f, 18.0f, 450.0f));
    AddBox(Trim, Museum + FVector(1180.0f, -1448.0f, 485.0f), FVector(398.0f, 18.0f, 28.0f));

    // The long approach belongs to LandmarkSiteDressing. These are only the photographed local steps/landing.
    AddBox(Steps, Museum + FVector(1180.0f, -1700.0f, 9.0f), FVector(720.0f, 210.0f, 18.0f));
    AddBox(Steps, Museum + FVector(1180.0f, -1600.0f, 21.0f), FVector(650.0f, 170.0f, 42.0f));
    AddBox(Steps, Museum + FVector(1180.0f, -1518.0f, 36.0f), FVector(580.0f, 120.0f, 72.0f));
    AddBox(Steps, Museum + FVector(1180.0f, -1460.0f, 54.0f), FVector(520.0f, 120.0f, 108.0f));

    const float GlassX = -2725.0f;
    const float GlassY = 280.0f;
    AddBox(Trim, Museum + FVector(GlassX, GlassY - 540.0f, 250.0f), FVector(28.0f, 28.0f, 500.0f));
    AddBox(Trim, Museum + FVector(GlassX, GlassY + 540.0f, 250.0f), FVector(28.0f, 28.0f, 500.0f));
    AddBox(Trim, Museum + FVector(GlassX, GlassY, 492.0f), FVector(28.0f, 1108.0f, 28.0f));
    AddBox(Trim, Museum + FVector(GlassX, GlassY, 20.0f), FVector(28.0f, 1108.0f, 40.0f));
    AddBox(Trim, Museum + FVector(GlassX, GlassY - 180.0f, 250.0f), FVector(32.0f, 24.0f, 470.0f));
    AddBox(Trim, Museum + FVector(GlassX, GlassY + 180.0f, 250.0f), FVector(32.0f, 24.0f, 470.0f));
    AddBox(Glass, Museum + FVector(GlassX + 2.0f, GlassY - 360.0f, 255.0f), FVector(12.0f, 330.0f, 420.0f));
    AddBox(Glass, Museum + FVector(GlassX + 2.0f, GlassY, 255.0f), FVector(12.0f, 330.0f, 420.0f));
    AddBox(Glass, Museum + FVector(GlassX + 2.0f, GlassY + 360.0f, 255.0f), FVector(12.0f, 330.0f, 420.0f));

    struct FPinePlacement
    {
        FVector Offset;
        float HeightCm;
        float Yaw;
        int32 Variant;
    };
    const FPinePlacement PinePlacements[] = {
        { FVector(-1450.0f, -2550.0f, 0.0f), 1750.0f,  12.0f, 0 },
        { FVector( 3650.0f, -2850.0f, 0.0f), 1880.0f,  61.0f, 1 },
        { FVector(-2100.0f, -4300.0f, 0.0f), 2050.0f, 113.0f, 2 },
        { FVector( 4200.0f, -4650.0f, 0.0f), 1940.0f, 172.0f, 0 },
        { FVector(-2800.0f, -6100.0f, 0.0f), 2200.0f, 227.0f, 1 },
        { FVector( 4700.0f, -6350.0f, 0.0f), 2110.0f, 288.0f, 2 },
        { FVector(-3600.0f,  1700.0f, 0.0f), 1860.0f, 329.0f, 0 },
        { FVector( 3900.0f,  1850.0f, 0.0f), 2010.0f,  43.0f, 1 },
    };

    UStaticMesh* PineMeshes[] = { Pine01, Pine03, Pine05 };
    UInstancedStaticMeshComponent* PineComponents[] = {
        MakeISM(ArtRoot, Root, Pine01, nullptr, TEXT("R13_MuseumPine01"), false, true),
        MakeISM(ArtRoot, Root, Pine03, nullptr, TEXT("R13_MuseumPine03"), false, true),
        MakeISM(ArtRoot, Root, Pine05, nullptr, TEXT("R13_MuseumPine05"), false, true),
    };

    int32 PineCount = 0;
    for (const FPinePlacement& Placement : PinePlacements)
    {
        const int32 Variant = FMath::Clamp(Placement.Variant, 0, 2);
        if (!PineMeshes[Variant] || !PineComponents[Variant]) continue;
        AddGroundedTree(PineComponents[Variant], PineMeshes[Variant], Museum + Placement.Offset,
            Placement.HeightCm, Placement.Yaw);
        ++PineCount;
    }

    UE_LOG(LogTemp, Display,
        TEXT("R13.4 museum reference: unique photo-driven plinth/cornice/textured blue-grey timber upper gable/entrance steps/side glazing applied; long approach remains owned by LandmarkSiteDressing; mature pines=%d."),
        PineCount);
}