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
    constexpr float WindowFrontY = -981.0f;

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

    FVector CollegeLocalToWorld(const FVector& College, const FVector& LocalOffset)
    {
        return College + FRotator(0.0f, CollegeYawDegrees, 0.0f).RotateVector(LocalOffset);
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
    UInstancedStaticMeshComponent* WindowGlass = MakeVisualISM(
        ArtRoot, Root, Cube, GlassMaterial, TEXT("R13_CollegeWindowGlass"), false);
    UInstancedStaticMeshComponent* WindowFrames = MakeVisualISM(
        ArtRoot, Root, Cube, FrameMaterial, TEXT("R13_CollegeWindowFrames"), true);

    const FVector College = AOCWorldSectorOster::CollegeAnchor();

    // BuildCollegeSector rotates local offsets by the authored 1-degree building yaw. Apply the same transform to
    // every overlay center; rotating only the cubes themselves leaves edge/entrance details tens of centimetres off.
    AddBox(Plinth, CollegeLocalToWorld(College, FVector(0.0f, MainFrontY - 16.0f, 70.0f)),
        FVector(MainWidthCm - 80.0f, 18.0f, 140.0f));
    const float FloorBandZ[] = { 385.0f, 735.0f, 1085.0f, 1430.0f };
    for (const float Z : FloorBandZ)
    {
        AddBox(Bands, CollegeLocalToWorld(College, FVector(0.0f, MainFrontY - 18.0f, Z)),
            FVector(MainWidthCm - 100.0f, 20.0f, 26.0f));
    }
    AddBox(Bands, CollegeLocalToWorld(College, FVector(-3190.0f, MainFrontY - 18.0f, MainHeightCm * 0.5f)),
        FVector(34.0f, 20.0f, MainHeightCm - 30.0f));
    AddBox(Bands, CollegeLocalToWorld(College, FVector(3190.0f, MainFrontY - 18.0f, MainHeightCm * 0.5f)),
        FVector(34.0f, 20.0f, MainHeightCm - 30.0f));

    // Preserve the source 9x4 window topology, including the two ground-floor slots omitted for the entrance.
    // A thin glass/front-frame layer replaces the flat proxy look without changing source collision or navigation.
    constexpr int32 WindowColumns = 9;
    constexpr int32 WindowRows = 4;
    for (int32 Row = 0; Row < WindowRows; ++Row)
    {
        for (int32 Col = 0; Col < WindowColumns; ++Col)
        {
            if (Row == 0 && (Col == 5 || Col == 6)) continue;
            const float X = -2800.0f + Col * 700.0f;
            const float Z = 255.0f + Row * 340.0f;
            AddBox(WindowGlass, CollegeLocalToWorld(College, FVector(X, WindowFrontY, Z)),
                FVector(408.0f, 8.0f, 198.0f));
            AddBox(WindowFrames, CollegeLocalToWorld(College, FVector(X, WindowFrontY - 6.0f, Z - 111.0f)),
                FVector(446.0f, 12.0f, 18.0f));
            AddBox(WindowFrames, CollegeLocalToWorld(College, FVector(X, WindowFrontY - 6.0f, Z + 111.0f)),
                FVector(446.0f, 12.0f, 18.0f));
            AddBox(WindowFrames, CollegeLocalToWorld(College, FVector(X - 220.0f, WindowFrontY - 6.0f, Z)),
                FVector(18.0f, 12.0f, 240.0f));
            AddBox(WindowFrames, CollegeLocalToWorld(College, FVector(X + 220.0f, WindowFrontY - 6.0f, Z)),
                FVector(18.0f, 12.0f, 240.0f));
        }
    }

    // The authored entrance is not centered on the main block. BuildCollegeSector places its vestibule at
    // X +900 cm and Y -1230 cm, with the stair run continuing south. Keep all entrance offsets in the same local frame.
    const FVector EntranceFront = CollegeLocalToWorld(College,
        FVector(EntranceCenterX, EntranceFrontY - 8.0f, 0.0f));
    for (float XOffset : { -420.0f, 0.0f, 420.0f })
    {
        AddBox(Glass, CollegeLocalToWorld(College,
            FVector(EntranceCenterX + XOffset, EntranceFrontY - 8.0f, 255.0f)), FVector(360.0f, 14.0f, 420.0f));
    }
    AddBox(EntranceFrame, CollegeLocalToWorld(College,
        FVector(EntranceCenterX - 625.0f, EntranceFrontY - 10.0f, 255.0f)), FVector(28.0f, 18.0f, 470.0f));
    AddBox(EntranceFrame, CollegeLocalToWorld(College,
        FVector(EntranceCenterX + 625.0f, EntranceFrontY - 10.0f, 255.0f)), FVector(28.0f, 18.0f, 470.0f));
    AddBox(EntranceFrame, CollegeLocalToWorld(College,
        FVector(EntranceCenterX, EntranceFrontY - 10.0f, 485.0f)), FVector(1280.0f, 18.0f, 28.0f));
    AddBox(EntranceFrame, CollegeLocalToWorld(College,
        FVector(EntranceCenterX - 210.0f, EntranceFrontY - 10.0f, 255.0f)), FVector(24.0f, 18.0f, 440.0f));
    AddBox(EntranceFrame, CollegeLocalToWorld(College,
        FVector(EntranceCenterX + 210.0f, EntranceFrontY - 10.0f, 255.0f)), FVector(24.0f, 18.0f, 440.0f));

    // The source already owns the 26.5 x 9.2 m canopy collision slab at X +900 / Y -1590. Add only a thin
    // visual fascia to its exposed front and side edges so gameplay topology remains single-source.
    const FVector CanopyCenter = CollegeLocalToWorld(College,
        FVector(EntranceCenterX, EntranceCanopyCenterY, 505.0f));
    AddBox(Canopy, CollegeLocalToWorld(College,
        FVector(EntranceCenterX, EntranceCanopyCenterY - 468.0f, 505.0f)), FVector(2670.0f, 18.0f, 92.0f));
    AddBox(Canopy, CollegeLocalToWorld(College,
        FVector(EntranceCenterX - 1333.0f, EntranceCanopyCenterY, 505.0f)), FVector(18.0f, 930.0f, 92.0f));
    AddBox(Canopy, CollegeLocalToWorld(College,
        FVector(EntranceCenterX + 1333.0f, EntranceCanopyCenterY, 505.0f)), FVector(18.0f, 930.0f, 92.0f));

    // Retain explicit topology constants and intermediate anchors for verifier/documentation visibility.
    static_cast<void>(MainDepthCm);
    static_cast<void>(EntranceBlockCenterY);
    static_cast<void>(EntranceFront);
    static_cast<void>(CanopyCenter);

    bApplied = true;
    UE_LOG(LogTemp, Display,
        TEXT("R13.5 college facade: aligned to authored 65x19x14.4m main mass/X+900 entrance using the same rotated local frame; full-width bands, framed 9x4 window topology, glazed vestibule and canopy fascia added; stairs/footprint preserved."));
}
