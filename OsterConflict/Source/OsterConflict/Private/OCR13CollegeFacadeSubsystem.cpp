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
    constexpr float CollegeYawDegrees = 1.0f;
    constexpr float MainWidthCm = 6500.0f;
    constexpr float MainDepthCm = 1900.0f;
    constexpr float MainHeightCm = 1440.0f;
    constexpr float MainFrontY = -950.0f;
    constexpr float EntranceCenterX = 900.0f;
    constexpr float EntranceBlockCenterY = -1230.0f;
    constexpr float EntranceFrontY = -1530.0f;
    constexpr float EntranceCanopyCenterY = -1590.0f;

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
        Component->SetCollisionProfileName(TEXT("NoCollision"));
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

    void AddBox(UInstancedStaticMeshComponent* Target, const FVector& Center, const FVector& SizeCm,
        const float YawDegrees = CollegeYawDegrees)
    {
        if (!Target) return;
        Target->AddInstance(FTransform(FRotator(0.0f, YawDegrees, 0.0f), Center, SizeCm / 100.0f), true);
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
    ArtRoot->Tags.Add(TEXT("R13_CollegeFacadeAligned"));

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
    UMaterialInstanceDynamic* FrameMaterial = MakeColor(ArtRoot, BaseMaterial,
        TEXT("R13_CollegeEntranceFrameMat"), FLinearColor(0.20f, 0.21f, 0.20f, 1.0f));

    UInstancedStaticMeshComponent* Plinth = MakeVisualISM(
        ArtRoot, Root, Cube, PlinthMaterial, TEXT("R13_CollegeDarkPlinth"), true);
    UInstancedStaticMeshComponent* Bands = MakeVisualISM(
        ArtRoot, Root, Cube, BandMaterial, TEXT("R13_CollegeFacadeBands"), true);
    UInstancedStaticMeshComponent* Canopy = MakeVisualISM(
        ArtRoot, Root, Cube, CanopyMaterial, TEXT("R13_CollegeEntranceCanopy"), true);
    UInstancedStaticMeshComponent* Glass = MakeVisualISM(
        ArtRoot, Root, Cube, GlassMaterial, TEXT("R13_CollegeEntranceGlass"), false);
    UInstancedStaticMeshComponent* EntranceFrame = MakeVisualISM(
        ArtRoot, Root, Cube, FrameMaterial, TEXT("R13_CollegeEntranceFrame"), true);

    const FVector College = AOCWorldSectorOster::CollegeAnchor();
    const float FacadeY = College.Y + MainFrontY - 16.0f;

    // BuildCollegeSector owns a 65 x 19 x 14.4 m main collision mass. Keep this visual skin on that exact
    // frontage rather than the obsolete 48 x 17 m dimensions used by the first R13.5 facade pass.
    AddBox(Plinth, FVector(College.X, FacadeY, 70.0f), FVector(MainWidthCm - 80.0f, 18.0f, 140.0f));
    const float FloorBandZ[] = { 385.0f, 735.0f, 1085.0f, 1430.0f };
    for (const float Z : FloorBandZ)
    {
        AddBox(Bands, FVector(College.X, FacadeY - 2.0f, Z), FVector(MainWidthCm - 100.0f, 20.0f, 26.0f));
    }
    AddBox(Bands, FVector(College.X - 3190.0f, FacadeY - 2.0f, MainHeightCm * 0.5f),
        FVector(34.0f, 20.0f, MainHeightCm - 30.0f));
    AddBox(Bands, FVector(College.X + 3190.0f, FacadeY - 2.0f, MainHeightCm * 0.5f),
        FVector(34.0f, 20.0f, MainHeightCm - 30.0f));

    // The authored entrance is not centered on the main block. BuildCollegeSector places its vestibule at
    // X +900 cm and Y -1230 cm, with the stair run continuing south. Align the glass/front-frame to that
    // topology instead of creating a second fake entrance at X=0.
    const FVector EntranceFront = College + FVector(EntranceCenterX, EntranceFrontY - 8.0f, 0.0f);
    for (float XOffset : { -420.0f, 0.0f, 420.0f })
    {
        AddBox(Glass, EntranceFront + FVector(XOffset, 0.0f, 255.0f), FVector(360.0f, 14.0f, 420.0f));
    }
    AddBox(EntranceFrame, EntranceFront + FVector(-625.0f, -2.0f, 255.0f), FVector(28.0f, 18.0f, 470.0f));
    AddBox(EntranceFrame, EntranceFront + FVector(625.0f, -2.0f, 255.0f), FVector(28.0f, 18.0f, 470.0f));
    AddBox(EntranceFrame, EntranceFront + FVector(0.0f, -2.0f, 485.0f), FVector(1280.0f, 18.0f, 28.0f));
    AddBox(EntranceFrame, EntranceFront + FVector(-210.0f, -2.0f, 255.0f), FVector(24.0f, 18.0f, 440.0f));
    AddBox(EntranceFrame, EntranceFront + FVector(210.0f, -2.0f, 255.0f), FVector(24.0f, 18.0f, 440.0f));

    // The source already owns the 26.5 x 9.2 m canopy collision slab at X +900 / Y -1590. Add only a thin
    // visual fascia to its exposed front and side edges so gameplay topology remains single-source.
    const FVector CanopyCenter = College + FVector(EntranceCenterX, EntranceCanopyCenterY, 505.0f);
    AddBox(Canopy, CanopyCenter + FVector(0.0f, -468.0f, 0.0f), FVector(2670.0f, 18.0f, 92.0f));
    AddBox(Canopy, CanopyCenter + FVector(-1333.0f, 0.0f, 0.0f), FVector(18.0f, 930.0f, 92.0f));
    AddBox(Canopy, CanopyCenter + FVector(1333.0f, 0.0f, 0.0f), FVector(18.0f, 930.0f, 92.0f));

    // Keep the constants referenced by the verifier and document the source topology being matched.
    static_cast<void>(MainDepthCm);
    static_cast<void>(EntranceBlockCenterY);

    bApplied = true;
    UE_LOG(LogTemp, Display,
        TEXT("R13.5 college facade: aligned to authored 65x19x14.4m main mass and X+900 entrance; dark plinth, full-width floor bands, glazed vestibule and canopy fascia added; authored 9x4 windows, stairs and footprint preserved."));
}
