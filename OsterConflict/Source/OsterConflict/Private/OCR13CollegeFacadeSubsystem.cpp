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
    constexpr float MainSkinPlinthY = MainFrontY - 9.0f;
    constexpr float MainSkinBandY = MainFrontY - 10.0f;
    constexpr float EntranceCenterX = 900.0f;
    constexpr float EntranceBlockCenterY = -1230.0f;
    constexpr float EntranceFrontY = -1530.0f;
    constexpr float EntranceCanopyCenterY = -1590.0f;
    constexpr float WindowFrontY = -981.0f;
    constexpr float EntranceFrontLocalY = EntranceFrontY - EntranceBlockCenterY;

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

    FVector RotateCollegeVector(const FVector& LocalOffset)
    {
        return FRotator(0.0f, CollegeYawDegrees, 0.0f).RotateVector(LocalOffset);
    }

    // Mirrors AddFacadeWindow(), whose source offsets are explicitly rotated around CollegeAnchor().
    FVector CollegeRotatedLocalToWorld(const FVector& College, const FVector& LocalOffset)
    {
        return College + RotateCollegeVector(LocalOffset);
    }

    // Mirrors BuildCollegeSector AddBox() calls such as College + FVector(900,-1230,...):
    // AddBox rotates the mesh itself but does not rotate the supplied center around CollegeAnchor().
    FVector CollegeAuthoredCenterToWorld(const FVector& College, const FVector& SourceCenterOffset)
    {
        return College + SourceCenterOffset;
    }

    // Starts from one of those direct source centers, then follows the mesh's 1-degree local axes to a face/detail.
    FVector CollegeFaceOffsetFromAuthoredCenter(const FVector& College, const FVector& SourceCenterOffset,
        const FVector& LocalFaceOffset)
    {
        return CollegeAuthoredCenterToWorld(College, SourceCenterOffset) + RotateCollegeVector(LocalFaceOffset);
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
    UMaterialInstanceDynamic* StepMaterial = MakeColor(ArtRoot, BaseMaterial,
        TEXT("R13_CollegeStepMat"), FLinearColor(0.42f, 0.41f, 0.38f, 1.0f));

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
    UInstancedStaticMeshComponent* StairTreads = MakeVisualISM(
        ArtRoot, Root, Cube, StepMaterial, TEXT("R13_CollegeStairTreads"), true);
    UInstancedStaticMeshComponent* StairNosings = MakeVisualISM(
        ArtRoot, Root, Cube, FrameMaterial, TEXT("R13_CollegeStairNosings"), true);

    const FVector College = AOCWorldSectorOster::CollegeAnchor();

    // Mixed source-coordinate contract:
    // - the main facade/window offsets follow AddFacadeWindow and are rotated around CollegeAnchor;
    // - entrance/canopy/stair centers are direct College + FVector(...) source centers, with only their face offsets
    //   rotated along each mesh's 1-degree axes. Keeping these modes explicit prevents 20-40 cm overlay drift.
    AddBox(Plinth, CollegeRotatedLocalToWorld(College, FVector(0.0f, MainSkinPlinthY, 70.0f)),
        FVector(MainWidthCm - 80.0f, 18.0f, 140.0f));
    const float FloorBandZ[] = { 385.0f, 735.0f, 1085.0f, 1430.0f };
    for (const float Z : FloorBandZ)
    {
        AddBox(Bands, CollegeRotatedLocalToWorld(College, FVector(0.0f, MainSkinBandY, Z)),
            FVector(MainWidthCm - 100.0f, 20.0f, 26.0f));
    }
    AddBox(Bands, CollegeRotatedLocalToWorld(College,
        FVector(-3190.0f, MainSkinBandY, MainHeightCm * 0.5f)), FVector(34.0f, 20.0f, MainHeightCm - 30.0f));
    AddBox(Bands, CollegeRotatedLocalToWorld(College,
        FVector(3190.0f, MainSkinBandY, MainHeightCm * 0.5f)), FVector(34.0f, 20.0f, MainHeightCm - 30.0f));

    // Preserve the source 9x4 AddFacadeWindow topology, including the two ground-floor slots omitted for entrance.
    constexpr int32 WindowColumns = 9;
    constexpr int32 WindowRows = 4;
    for (int32 Row = 0; Row < WindowRows; ++Row)
    {
        for (int32 Col = 0; Col < WindowColumns; ++Col)
        {
            if (Row == 0 && (Col == 5 || Col == 6)) continue;
            const float X = -2800.0f + Col * 700.0f;
            const float Z = 255.0f + Row * 340.0f;
            AddBox(WindowGlass, CollegeRotatedLocalToWorld(College, FVector(X, WindowFrontY, Z)),
                FVector(408.0f, 8.0f, 198.0f));
            AddBox(WindowFrames, CollegeRotatedLocalToWorld(College,
                FVector(X, WindowFrontY - 6.0f, Z - 111.0f)), FVector(446.0f, 12.0f, 18.0f));
            AddBox(WindowFrames, CollegeRotatedLocalToWorld(College,
                FVector(X, WindowFrontY - 6.0f, Z + 111.0f)), FVector(446.0f, 12.0f, 18.0f));
            AddBox(WindowFrames, CollegeRotatedLocalToWorld(College,
                FVector(X - 220.0f, WindowFrontY - 6.0f, Z)), FVector(18.0f, 12.0f, 240.0f));
            AddBox(WindowFrames, CollegeRotatedLocalToWorld(College,
                FVector(X + 220.0f, WindowFrontY - 6.0f, Z)), FVector(18.0f, 12.0f, 240.0f));
        }
    }

    // BuildCollegeSector authors this vestibule at the direct center College + FVector(900,-1230,230).
    // We retain its plan center exactly and rotate only offsets to the yawed front face.
    const FVector EntranceSourcePlanCenter(EntranceCenterX, EntranceBlockCenterY, 0.0f);
    const FVector EntranceFront = CollegeFaceOffsetFromAuthoredCenter(College, EntranceSourcePlanCenter,
        FVector(0.0f, EntranceFrontLocalY - 7.0f, 0.0f));
    for (float XOffset : { -420.0f, 0.0f, 420.0f })
    {
        AddBox(Glass, CollegeFaceOffsetFromAuthoredCenter(College, EntranceSourcePlanCenter,
            FVector(XOffset, EntranceFrontLocalY - 7.0f, 255.0f)), FVector(360.0f, 14.0f, 420.0f));
    }
    AddBox(EntranceFrame, CollegeFaceOffsetFromAuthoredCenter(College, EntranceSourcePlanCenter,
        FVector(-625.0f, EntranceFrontLocalY - 9.0f, 255.0f)), FVector(28.0f, 18.0f, 470.0f));
    AddBox(EntranceFrame, CollegeFaceOffsetFromAuthoredCenter(College, EntranceSourcePlanCenter,
        FVector(625.0f, EntranceFrontLocalY - 9.0f, 255.0f)), FVector(28.0f, 18.0f, 470.0f));
    AddBox(EntranceFrame, CollegeFaceOffsetFromAuthoredCenter(College, EntranceSourcePlanCenter,
        FVector(0.0f, EntranceFrontLocalY - 9.0f, 485.0f)), FVector(1280.0f, 18.0f, 28.0f));
    AddBox(EntranceFrame, CollegeFaceOffsetFromAuthoredCenter(College, EntranceSourcePlanCenter,
        FVector(-210.0f, EntranceFrontLocalY - 9.0f, 255.0f)), FVector(24.0f, 18.0f, 440.0f));
    AddBox(EntranceFrame, CollegeFaceOffsetFromAuthoredCenter(College, EntranceSourcePlanCenter,
        FVector(210.0f, EntranceFrontLocalY - 9.0f, 255.0f)), FVector(24.0f, 18.0f, 440.0f));

    // Reuse exact direct source centers for the five collision steps. Tread caps move only in Z; front nosings
    // follow each yawed step's local -Y face. The original LandmarkDetails geometry remains the sole collision owner.
    for (int32 Step = 0; Step < 5; ++Step)
    {
        const float StepCenterY = -1940.0f - Step * 115.0f;
        const float StepCenterZ = 22.0f + Step * 22.0f;
        const float StepWidth = 2750.0f - Step * 100.0f;
        const FVector StepSourceCenter(EntranceCenterX, StepCenterY, StepCenterZ);
        AddBox(StairTreads, CollegeAuthoredCenterToWorld(College,
            FVector(EntranceCenterX, StepCenterY, StepCenterZ + 23.0f)), FVector(StepWidth - 24.0f, 204.0f, 6.0f));
        AddBox(StairNosings, CollegeFaceOffsetFromAuthoredCenter(College, StepSourceCenter,
            FVector(0.0f, -114.0f, 16.0f)), FVector(StepWidth - 18.0f, 8.0f, 8.0f));
    }

    // Source canopy center is direct College + FVector(900,-1590,505), size 2650x920x70 at yaw 1 degree.
    // Its fascia touches the actual yawed box edges: 460+9 cm front, 1325+9 cm on the sides.
    const FVector CanopySourceCenter(EntranceCenterX, EntranceCanopyCenterY, 505.0f);
    const FVector CanopyCenter = CollegeAuthoredCenterToWorld(College, CanopySourceCenter);
    AddBox(Canopy, CollegeFaceOffsetFromAuthoredCenter(College, CanopySourceCenter,
        FVector(0.0f, -469.0f, 0.0f)), FVector(2670.0f, 18.0f, 92.0f));
    AddBox(Canopy, CollegeFaceOffsetFromAuthoredCenter(College, CanopySourceCenter,
        FVector(-1334.0f, 0.0f, 0.0f)), FVector(18.0f, 920.0f, 92.0f));
    AddBox(Canopy, CollegeFaceOffsetFromAuthoredCenter(College, CanopySourceCenter,
        FVector(1334.0f, 0.0f, 0.0f)), FVector(18.0f, 920.0f, 92.0f));

    static_cast<void>(MainDepthCm);
    static_cast<void>(EntranceFront);
    static_cast<void>(CanopyCenter);

    bApplied = true;
    UE_LOG(LogTemp, Display,
        TEXT("R13.5 college facade: source-coordinate contract aligned: rotated main/window offsets plus direct authored entrance/canopy/stair centers; flush trim, framed 9x4 windows and source-matched five-step finish remain visual-only; source collision/footprint preserved."));
}
