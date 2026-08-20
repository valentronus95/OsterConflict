#include "OCR142SilpoInteriorDetailSubsystem.h"

#include "OCGameMode.h"
#include "OCGeoReference.h"

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
    constexpr float DetailBuildDelaySeconds = 6.45f;
    constexpr float BuildingLengthCm = 3000.0f;
    constexpr float BuildingDepthCm = 1750.0f;
    constexpr float HalfLength = BuildingLengthCm * 0.5f;
    constexpr float HalfDepth = BuildingDepthCm * 0.5f;
    constexpr float FrontY = -HalfDepth;
    constexpr float RearY = HalfDepth;
    constexpr float EntranceCenterX = -1315.0f;

    FVector SilpoAnchor()
    {
        const FOCGeoReferencePoint Ref = FOCGeoReference::Silpo();
        return FOCGeoReference::ToLocalCm(Ref.Latitude, Ref.Longitude, 0.0);
    }

    UMaterialInstanceDynamic* MakeColorMaterial(AActor* Owner, UMaterialInterface* Base,
        const FName Name, const FLinearColor& Color)
    {
        if (!Owner || !Base) return nullptr;
        UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Base, Owner, Name);
        if (Material) Material->SetVectorParameterValue(TEXT("Color"), Color);
        return Material;
    }

    UInstancedStaticMeshComponent* MakeISM(AActor* Owner, USceneComponent* Root, UStaticMesh* Mesh,
        UMaterialInterface* Material, const FName Name, const bool bCastShadow = false)
    {
        if (!Owner || !Root || !Mesh) return nullptr;
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        if (Material)
        {
            const int32 SlotCount = FMath::Max(1, Mesh->GetStaticMaterials().Num());
            for (int32 Slot = 0; Slot < SlotCount; ++Slot) Component->SetMaterial(Slot, Material);
        }
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionProfileName(TEXT("NoCollision"));
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        Component->SetCastShadow(bCastShadow);
        Component->SetCullDistances(0, 65000);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void AddLocalBox(UInstancedStaticMeshComponent* Component, const FVector& LocalCenter,
        const FVector& SizeCm, const FRotator& Rotation = FRotator::ZeroRotator)
    {
        if (!Component) return;
        Component->AddInstance(FTransform(Rotation, LocalCenter, SizeCm / 100.0f), false);
    }

    bool HasActorTag(UWorld& World, const FName Tag)
    {
        for (TActorIterator<AActor> It(&World); It; ++It)
        {
            const AActor* Actor = *It;
            if (Actor && Actor->ActorHasTag(Tag)) return true;
        }
        return false;
    }
}

bool UOCR142SilpoInteriorDetailSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR142SilpoInteriorDetailSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) BuildInteriorDetails(*World);
        }), DetailBuildDelaySeconds, false);
}

void UOCR142SilpoInteriorDetailSubsystem::BuildInteriorDetails(UWorld& World)
{
    if (HasActorTag(World, TEXT("R142_SilpoInteriorDetails"))) return;

    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UMaterialInterface* Basic = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!Cube || !Basic) return;

    const FVector Site = SilpoAnchor();
    AActor* Details = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform(FRotator::ZeroRotator, Site));
    if (!Details) return;
    Details->SetReplicates(false);
    Details->Tags.Add(TEXT("R142_SilpoInteriorDetails"));
    Details->Tags.Add(TEXT("SilpoOster_BohdanaKhmelnytskoho54"));

    USceneComponent* Root = NewObject<USceneComponent>(Details, TEXT("R142Silpo_InteriorDetailRoot"));
    if (!Root)
    {
        Details->Destroy();
        return;
    }
    Root->SetMobility(EComponentMobility::Static);
    Details->SetRootComponent(Root);
    Details->AddInstanceComponent(Root);
    Root->RegisterComponent();
    Details->SetActorLocation(Site);

    UMaterialInstanceDynamic* GroutMat = MakeColorMaterial(Details, Basic, TEXT("R142Silpo_GroutMat"),
        FLinearColor(0.46f, 0.46f, 0.43f, 1.0f));
    UMaterialInstanceDynamic* ShelfEdgeMat = MakeColorMaterial(Details, Basic, TEXT("R142Silpo_ShelfEdgeMat"),
        FLinearColor(0.64f, 0.64f, 0.59f, 1.0f));
    UMaterialInstanceDynamic* CoolerGlassMat = MakeColorMaterial(Details, Basic, TEXT("R142Silpo_CoolerGlassMat"),
        FLinearColor(0.19f, 0.27f, 0.29f, 1.0f));
    UMaterialInstanceDynamic* CoolerFrameMat = MakeColorMaterial(Details, Basic, TEXT("R142Silpo_CoolerFrameMat"),
        FLinearColor(0.30f, 0.31f, 0.30f, 1.0f));
    UMaterialInstanceDynamic* BeltMat = MakeColorMaterial(Details, Basic, TEXT("R142Silpo_CheckoutBeltMat"),
        FLinearColor(0.09f, 0.10f, 0.10f, 1.0f));
    UMaterialInstanceDynamic* CounterTrimMat = MakeColorMaterial(Details, Basic, TEXT("R142Silpo_CheckoutTrimMat"),
        FLinearColor(0.76f, 0.39f, 0.12f, 1.0f));
    UMaterialInstanceDynamic* ProduceTrimMat = MakeColorMaterial(Details, Basic, TEXT("R142Silpo_ProduceTrimMat"),
        FLinearColor(0.50f, 0.39f, 0.24f, 1.0f));
    UMaterialInstanceDynamic* MatMat = MakeColorMaterial(Details, Basic, TEXT("R142Silpo_EntranceMatMat"),
        FLinearColor(0.18f, 0.19f, 0.18f, 1.0f));

    UInstancedStaticMeshComponent* FloorGrout = MakeISM(Details, Root, Cube, GroutMat,
        TEXT("R142Silpo_FloorTileGrout"));
    UInstancedStaticMeshComponent* ShelfEdges = MakeISM(Details, Root, Cube, ShelfEdgeMat,
        TEXT("R142Silpo_ShelfEndTrim"), true);
    UInstancedStaticMeshComponent* CoolerGlass = MakeISM(Details, Root, Cube, CoolerGlassMat,
        TEXT("R142Silpo_CoolerDoorGlass"));
    UInstancedStaticMeshComponent* CoolerFrames = MakeISM(Details, Root, Cube, CoolerFrameMat,
        TEXT("R142Silpo_CoolerDoorFrames"), true);
    UInstancedStaticMeshComponent* CheckoutBelts = MakeISM(Details, Root, Cube, BeltMat,
        TEXT("R142Silpo_CheckoutBelts"));
    UInstancedStaticMeshComponent* CheckoutTrim = MakeISM(Details, Root, Cube, CounterTrimMat,
        TEXT("R142Silpo_CheckoutTrim"));
    UInstancedStaticMeshComponent* ProduceTrim = MakeISM(Details, Root, Cube, ProduceTrimMat,
        TEXT("R142Silpo_ProduceBinDividers"), true);
    UInstancedStaticMeshComponent* EntranceMat = MakeISM(Details, Root, Cube, MatMat,
        TEXT("R142Silpo_EntranceMat"));

    // Square tile-joint grid. Thin non-colliding strips sit just above the base floor to avoid gameplay changes.
    constexpr float TileStepCm = 62.0f;
    for (float X = -HalfLength + TileStepCm; X < HalfLength; X += TileStepCm)
    {
        AddLocalBox(FloorGrout, FVector(X, 0.0f, 14.4f), FVector(1.2f, BuildingDepthCm - 50.0f, 0.8f));
    }
    for (float Y = -HalfDepth + TileStepCm; Y < HalfDepth; Y += TileStepCm)
    {
        AddLocalBox(FloorGrout, FVector(0.0f, Y, 14.4f), FVector(BuildingLengthCm - 50.0f, 1.2f, 0.8f));
    }

    // Six shelf runs from R14.0. Add end plates and kick rails without changing the collision volume.
    for (const float X : { -920.0f, -570.0f, -220.0f, 130.0f, 480.0f, 830.0f })
    {
        AddLocalBox(ShelfEdges, FVector(X, -278.0f, 88.0f), FVector(112.0f, 8.0f, 176.0f));
        AddLocalBox(ShelfEdges, FVector(X, 498.0f, 88.0f), FVector(112.0f, 8.0f, 176.0f));
        AddLocalBox(ShelfEdges, FVector(X, 110.0f, 11.0f), FVector(112.0f, 760.0f, 12.0f));
    }

    // Rear refrigeration receives dark glass faces and regular vertical door/frame divisions.
    for (const float X : { -1100.0f, -650.0f, -200.0f, 250.0f, 700.0f, 1150.0f })
    {
        AddLocalBox(CoolerGlass, FVector(X, RearY - 122.0f, 128.0f), FVector(350.0f, 5.0f, 205.0f));
        for (const float Offset : { -118.0f, 0.0f, 118.0f })
        {
            AddLocalBox(CoolerFrames, FVector(X + Offset, RearY - 126.0f, 128.0f), FVector(5.0f, 8.0f, 215.0f));
        }
        AddLocalBox(CoolerFrames, FVector(X, RearY - 126.0f, 232.0f), FVector(360.0f, 8.0f, 6.0f));
        AddLocalBox(CoolerFrames, FVector(X, RearY - 126.0f, 24.0f), FVector(360.0f, 8.0f, 6.0f));
    }

    // Checkout surfaces read better with a dark conveyor and a narrow warm accent strip.
    for (int32 Lane = 0; Lane < 4; ++Lane)
    {
        const float X = 300.0f + static_cast<float>(Lane) * 270.0f;
        AddLocalBox(CheckoutBelts, FVector(X - 18.0f, -600.0f, 94.0f), FVector(138.0f, 56.0f, 4.0f));
        AddLocalBox(CheckoutTrim, FVector(X + 93.0f, -600.0f, 76.0f), FVector(10.0f, 76.0f, 48.0f));
    }

    // Empty produce table: add shallow dividers that imply bins while keeping the island intentionally unstocked.
    for (const float XOffset : { -105.0f, 0.0f, 105.0f })
    {
        AddLocalBox(ProduceTrim, FVector(-760.0f + XOffset, -360.0f, 112.0f), FVector(5.0f, 135.0f, 28.0f));
    }
    AddLocalBox(ProduceTrim, FVector(-760.0f, -426.0f, 112.0f), FVector(325.0f, 5.0f, 28.0f));
    AddLocalBox(ProduceTrim, FVector(-760.0f, -294.0f, 112.0f), FVector(325.0f, 5.0f, 28.0f));

    // Dark entrance mat just inside the photo-derived left-side public doorway.
    AddLocalBox(EntranceMat, FVector(EntranceCenterX, FrontY + 105.0f, 15.0f), FVector(118.0f, 165.0f, 2.0f));

    UE_LOG(LogTemp, Display,
        TEXT("R14.2 Silpo interior detail pass built at [%.0f %.0f]: tile grid, shelf/cooler detail, checkout surfaces and produce bins."),
        Site.X, Site.Y);
}
