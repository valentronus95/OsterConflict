#include "OCR13CollegeFacadeSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"

namespace
{
    constexpr float CollegeFacadeDelaySeconds = 2.25f;

    UInstancedStaticMeshComponent* MakeVisualISM(AActor* Owner, USceneComponent* Root,
        UStaticMesh* Mesh, UMaterialInterface* Material, const FName Name, const bool bCastShadow)
    {
        if (!Owner || !Root || !Mesh) return nullptr;
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        if (Material) Component->SetMaterial(0, Material);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        Component->SetCastShadow(bCastShadow);
        Component->SetCullDistances(0, 85000);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    UMaterialInstanceDynamic* MakeColor(AActor* Owner, UMaterialInterface* Base,
        const FName Name, const FLinearColor& Color)
    {
        if (!Owner || !Base) return nullptr;
        UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Base, Owner, Name);
        if (Material) Material->SetVectorParameterValue(TEXT("Color"), Color);
        return Material;
    }

    void AddBox(UInstancedStaticMeshComponent* Target, const FVector& Center, const FVector& SizeCm)
    {
        if (!Target) return;
        Target->AddInstance(FTransform(FRotator::ZeroRotator, Center, SizeCm / 100.0f), true);
    }
}

bool UOCR13CollegeFacadeSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13CollegeFacadeSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) ApplyCollegeFacade(*World);
        }), CollegeFacadeDelaySeconds, false);
}

void UOCR13CollegeFacadeSubsystem::ApplyCollegeFacade(UWorld& World)
{
    if (bApplied) return;

    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    UMaterialInterface* GlassMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Glass_Window.Glass_Window"));
    if (!Cube || !BaseMaterial) return;

    AActor* ArtRoot = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!ArtRoot) return;
    ArtRoot->SetReplicates(false);
    ArtRoot->SetActorEnableCollision(false);

    USceneComponent* Root = NewObject<USceneComponent>(ArtRoot, TEXT("R13_CollegeFacadeRoot"));
    if (!Root)
    {
        ArtRoot->Destroy();
        return;
    }
    Root->SetMobility(EComponentMobility::Static);
    ArtRoot->SetRootComponent(Root);
    ArtRoot->AddInstanceComponent(Root);
    Root->RegisterComponent();

    UMaterialInstanceDynamic* PlinthMaterial = MakeColor(ArtRoot, BaseMaterial,
        TEXT("R13_CollegePlinthMat"), FLinearColor(0.10f, 0.105f, 0.10f, 1.0f));
    UMaterialInstanceDynamic* BandMaterial = MakeColor(ArtRoot, BaseMaterial,
        TEXT("R13_CollegeBandMat"), FLinearColor(0.61f, 0.59f, 0.52f, 1.0f));
    UMaterialInstanceDynamic* CanopyMaterial = MakeColor(ArtRoot, BaseMaterial,
        TEXT("R13_CollegeCanopyMat"), FLinearColor(0.18f, 0.20f, 0.20f, 1.0f));

    UInstancedStaticMeshComponent* Plinth = MakeVisualISM(
        ArtRoot, Root, Cube, PlinthMaterial, TEXT("R13_CollegeDarkPlinth"), true);
    UInstancedStaticMeshComponent* Bands = MakeVisualISM(
        ArtRoot, Root, Cube, BandMaterial, TEXT("R13_CollegeFacadeBands"), true);
    UInstancedStaticMeshComponent* Canopy = MakeVisualISM(
        ArtRoot, Root, Cube, CanopyMaterial, TEXT("R13_CollegeEntranceCanopy"), true);
    UInstancedStaticMeshComponent* Glass = MakeVisualISM(
        ArtRoot, Root, Cube, GlassMaterial, TEXT("R13_CollegeEntranceGlass"), false);

    const FVector College = AOCWorldSectorOster::CollegeAnchor();
    const float FrontY = College.Y - 860.0f;

    // Main authored block is 48 x 17 x 15.5 m. Thin overlays sit just in front of its south facade,
    // leaving the existing 9x4 landmark-window bridge visible between horizontal floor bands.
    AddBox(Plinth, FVector(College.X, FrontY - 8.0f, 70.0f), FVector(4740.0f, 18.0f, 140.0f));
    const float FloorBandZ[] = { 385.0f, 735.0f, 1085.0f, 1430.0f };
    for (const float Z : FloorBandZ)
    {
        AddBox(Bands, FVector(College.X, FrontY - 10.0f, Z), FVector(4720.0f, 20.0f, 26.0f));
    }
    AddBox(Bands, FVector(College.X - 2260.0f, FrontY - 10.0f, 780.0f), FVector(34.0f, 20.0f, 1410.0f));
    AddBox(Bands, FVector(College.X + 2260.0f, FrontY - 10.0f, 780.0f), FVector(34.0f, 20.0f, 1410.0f));

    // Source entrance is centered at Y -970. Add a real glazed vestibule skin and a darker canopy without
    // moving the existing stair/door topology or adding collision.
    const FVector Entrance = College + FVector(0.0f, -1040.0f, 0.0f);
    AddBox(Glass, Entrance + FVector(-220.0f, -85.0f, 300.0f), FVector(190.0f, 16.0f, 470.0f));
    AddBox(Glass, Entrance + FVector(0.0f, -85.0f, 300.0f), FVector(190.0f, 16.0f, 470.0f));
    AddBox(Glass, Entrance + FVector(220.0f, -85.0f, 300.0f), FVector(190.0f, 16.0f, 470.0f));
    AddBox(Bands, Entrance + FVector(-325.0f, -94.0f, 300.0f), FVector(26.0f, 18.0f, 520.0f));
    AddBox(Bands, Entrance + FVector(325.0f, -94.0f, 300.0f), FVector(26.0f, 18.0f, 520.0f));
    AddBox(Bands, Entrance + FVector(0.0f, -94.0f, 550.0f), FVector(680.0f, 18.0f, 28.0f));
    AddBox(Canopy, Entrance + FVector(0.0f, -165.0f, 585.0f), FVector(920.0f, 260.0f, 34.0f));

    bApplied = true;
    UE_LOG(LogTemp, Display,
        TEXT("R13.5 college facade: dark plinth + floor bands + glazed vestibule + entrance canopy added; authored 9x4 windows, stairs and footprint preserved."));
}
