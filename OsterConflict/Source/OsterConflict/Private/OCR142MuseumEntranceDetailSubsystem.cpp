#include "OCR142MuseumEntranceDetailSubsystem.h"

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
#include "UObject/UObjectGlobals.h"

namespace
{
    constexpr float R142EntranceDetailDelaySeconds = 6.10f;

    UMaterialInstanceDynamic* MakeMID(AActor* Owner, UMaterialInterface* Base,
        const TCHAR* Name, const FLinearColor& Color)
    {
        if (!Owner || !Base) return nullptr;
        UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(
            Base, Owner, MakeUniqueObjectName(Owner, UMaterialInstanceDynamic::StaticClass(), FName(Name)));
        if (Material) Material->SetVectorParameterValue(TEXT("Color"), Color);
        return Material;
    }

    UInstancedStaticMeshComponent* MakeISM(AActor* Owner, USceneComponent* Root, UStaticMesh* Cube,
        UMaterialInterface* Material, const TCHAR* Name)
    {
        if (!Owner || !Root || !Cube) return nullptr;
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(
            Owner, MakeUniqueObjectName(Owner, UInstancedStaticMeshComponent::StaticClass(), FName(Name)));
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Cube);
        if (Material) Component->SetMaterial(0, Material);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void AddBox(UInstancedStaticMeshComponent* Component, const FVector& Center,
        const FVector& SizeCm, const FRotator& Rotation = FRotator::ZeroRotator)
    {
        if (!Component || SizeCm.GetMin() <= 0.0f) return;
        Component->AddInstance(FTransform(Rotation, Center, SizeCm / 100.0f), true);
    }

    void AddFrontCanopyTrim(UInstancedStaticMeshComponent* Pale, const FVector& Museum)
    {
        // REF-06/17: layered carved fascia under the entrance gable.
        AddBox(Pale, Museum + FVector(0.0f, -694.0f, 392.0f), FVector(540.0f, 12.0f, 18.0f));
        for (int32 Index = -10; Index <= 10; ++Index)
        {
            const float X = static_cast<float>(Index) * 24.0f;
            AddBox(Pale, Museum + FVector(X, -699.0f, 378.0f), FVector(15.0f, 9.0f, 16.0f),
                FRotator(0.0f, 0.0f, (Index % 2 == 0) ? 18.0f : -18.0f));
        }

        // Simple stepped gable outline. The existing roof mesh remains the roof surface.
        AddBox(Pale, Museum + FVector(-126.0f, -696.0f, 430.0f), FVector(285.0f, 10.0f, 14.0f),
            FRotator(-25.0f, 0.0f, 0.0f));
        AddBox(Pale, Museum + FVector(126.0f, -696.0f, 430.0f), FVector(285.0f, 10.0f, 14.0f),
            FRotator(25.0f, 0.0f, 0.0f));
    }

    void AddDormerTrim(UInstancedStaticMeshComponent* Pale, const FVector& Museum)
    {
        // REF-01/17/19: pale carved trim around the blue-grey upper room.
        AddBox(Pale, Museum + FVector(0.0f, -289.0f, 630.0f), FVector(570.0f, 12.0f, 16.0f));
        for (int32 Index = -9; Index <= 9; ++Index)
        {
            const float X = static_cast<float>(Index) * 28.0f;
            AddBox(Pale, Museum + FVector(X, -294.0f, 616.0f), FVector(16.0f, 8.0f, 16.0f),
                FRotator(0.0f, 0.0f, (Index % 2 == 0) ? 16.0f : -16.0f));
        }
    }

    void AddPorchRail(UInstancedStaticMeshComponent* Rail, const FVector& Museum, const float X)
    {
        // Side railing run beside the stairs.
        AddBox(Rail, Museum + FVector(X, -790.0f, 118.0f), FVector(9.0f, 300.0f, 9.0f),
            FRotator(0.0f, 0.0f, 8.0f));
        AddBox(Rail, Museum + FVector(X, -790.0f, 35.0f), FVector(9.0f, 300.0f, 9.0f));
        AddBox(Rail, Museum + FVector(X, -930.0f, 85.0f), FVector(9.0f, 9.0f, 165.0f));
        AddBox(Rail, Museum + FVector(X, -650.0f, 135.0f), FVector(9.0f, 9.0f, 190.0f));

        // REF-17/18: repeated sunburst/spoke pattern. Rings are implied by the perimeter rails;
        // the radial bars preserve the recognizable motif without adding expensive spline meshes.
        for (const float YCenter : { -865.0f, -765.0f, -665.0f })
        {
            const FVector Center = Museum + FVector(X, YCenter, 92.0f);
            for (const float Roll : { -62.0f, -38.0f, -16.0f, 16.0f, 38.0f, 62.0f })
            {
                AddBox(Rail, Center, FVector(7.0f, 66.0f, 5.0f), FRotator(0.0f, 0.0f, Roll));
            }
        }
    }
}

bool UOCR142MuseumEntranceDetailSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR142MuseumEntranceDetailSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) BuildEntranceDetail(*World);
        }), R142EntranceDetailDelaySeconds, false);
}

void UOCR142MuseumEntranceDetailSubsystem::BuildEntranceDetail(UWorld& World) const
{
    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        if (AActor* Actor = *It; Actor && Actor->ActorHasTag(TEXT("R142_MuseumEntranceDetail"))) return;
    }

    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UMaterialInterface* Basic = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!Cube || !Basic) return;

    AActor* DetailActor = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!DetailActor) return;
    DetailActor->SetReplicates(false);
    DetailActor->Tags.Add(TEXT("R142_MuseumEntranceDetail"));

    USceneComponent* Root = NewObject<USceneComponent>(DetailActor,
        MakeUniqueObjectName(DetailActor, USceneComponent::StaticClass(), FName(TEXT("R142MuseumEntranceRoot"))));
    if (!Root)
    {
        DetailActor->Destroy();
        return;
    }
    DetailActor->SetRootComponent(Root);
    DetailActor->AddInstanceComponent(Root);
    Root->RegisterComponent();

    UMaterialInstanceDynamic* Grey = MakeMID(DetailActor, Basic, TEXT("R142MuseumMID_GreyCladding"),
        FLinearColor(0.35f, 0.38f, 0.39f, 1.0f));
    UMaterialInstanceDynamic* Brick = MakeMID(DetailActor, Basic, TEXT("R142MuseumMID_VestibuleBrick"),
        FLinearColor(0.50f, 0.21f, 0.10f, 1.0f));
    UMaterialInstanceDynamic* Pale = MakeMID(DetailActor, Basic, TEXT("R142MuseumMID_CarvedPale"),
        FLinearColor(0.80f, 0.77f, 0.65f, 1.0f));
    UMaterialInstanceDynamic* Curtain = MakeMID(DetailActor, Basic, TEXT("R142MuseumMID_Curtain"),
        FLinearColor(0.17f, 0.22f, 0.32f, 1.0f));
    UMaterialInstanceDynamic* Paper = MakeMID(DetailActor, Basic, TEXT("R142MuseumMID_InfoBoard"),
        FLinearColor(0.87f, 0.84f, 0.72f, 1.0f));
    UMaterialInstanceDynamic* RailMat = MakeMID(DetailActor, Basic, TEXT("R142MuseumMID_Rail"),
        FLinearColor(0.44f, 0.48f, 0.47f, 1.0f));
    UMaterialInstanceDynamic* HatchMat = MakeMID(DetailActor, Basic, TEXT("R142MuseumMID_BaseHatch"),
        FLinearColor(0.15f, 0.16f, 0.16f, 1.0f));

    UInstancedStaticMeshComponent* GreyCladding = MakeISM(DetailActor, Root, Cube, Grey,
        TEXT("R142Museum_GreyCladding"));
    UInstancedStaticMeshComponent* BrickBase = MakeISM(DetailActor, Root, Cube, Brick,
        TEXT("R142Museum_VestibuleBrickBase"));
    UInstancedStaticMeshComponent* PaleTrim = MakeISM(DetailActor, Root, Cube, Pale,
        TEXT("R142Museum_EntranceCarvedTrim"));
    UInstancedStaticMeshComponent* Curtains = MakeISM(DetailActor, Root, Cube, Curtain,
        TEXT("R142Museum_Curtains"));
    UInstancedStaticMeshComponent* Info = MakeISM(DetailActor, Root, Cube, Paper,
        TEXT("R142Museum_InfoBoard"));
    UInstancedStaticMeshComponent* Rail = MakeISM(DetailActor, Root, Cube, RailMat,
        TEXT("R142Museum_OrnamentalRail"));
    UInstancedStaticMeshComponent* Hatch = MakeISM(DetailActor, Root, Cube, HatchMat,
        TEXT("R142Museum_BaseHatch"));

    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();

    // Front vestibule lower grey cladding under the two side-light windows.
    AddBox(GreyCladding, Museum + FVector(-190.0f, -686.0f, 99.0f), FVector(120.0f, 14.0f, 58.0f));
    AddBox(GreyCladding, Museum + FVector( 190.0f, -686.0f, 99.0f), FVector(120.0f, 14.0f, 58.0f));

    // Side vestibule bases are exposed old brick in REF-17/18.
    AddBox(BrickBase, Museum + FVector(-270.0f, -545.0f, 105.0f), FVector(24.0f, 245.0f, 72.0f));
    AddBox(BrickBase, Museum + FVector( 270.0f, -545.0f, 105.0f), FVector(24.0f, 245.0f, 72.0f));

    // Curtains sit behind breakable glass, so breaking a pane does not delete the interior dressing.
    AddBox(Curtains, Museum + FVector(-190.0f, -664.0f, 220.0f), FVector(92.0f, 5.0f, 150.0f));
    AddBox(Curtains, Museum + FVector( 190.0f, -664.0f, 220.0f), FVector(92.0f, 5.0f, 150.0f));
    AddBox(Curtains, Museum + FVector(-250.0f, -545.0f, 220.0f), FVector(5.0f, 105.0f, 150.0f));
    AddBox(Curtains, Museum + FVector( 250.0f, -545.0f, 220.0f), FVector(5.0f, 105.0f, 150.0f));

    // Information sheet/stand visible behind the right entrance glazing in REF-15/17.
    AddBox(Info, Museum + FVector(198.0f, -661.0f, 205.0f), FVector(48.0f, 3.0f, 72.0f));

    AddFrontCanopyTrim(PaleTrim, Museum);
    AddDormerTrim(PaleTrim, Museum);
    AddPorchRail(Rail, Museum, -246.0f);
    AddPorchRail(Rail, Museum, 246.0f);

    // Small dark access/base hatch under the vestibule side seen in REF-17.
    AddBox(Hatch, Museum + FVector(-278.0f, -545.0f, 42.0f), FVector(14.0f, 88.0f, 68.0f));

    UE_LOG(LogTemp, Display,
        TEXT("R14.2 museum entrance: brick/grey vestibule base, curtains, info board, carved fascia, dormer trim and ornamental porch rails built from REF-06/15/17/18."));
}
