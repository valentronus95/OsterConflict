#include "OCR13SilpoPhotoModelSubsystem.h"

#include "OCGameMode.h"
#include "OCGeoReference.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"

namespace
{
    // Run after the current R13.7 landmark passes so this site replacement owns the final Silpo footprint.
    constexpr float SilpoPhotoModelDelaySeconds = 5.60f;

    // Public-map/OSM point for the Oster Silpo. The official store listing identifies the site as
    // Bohdana Khmelnytskoho 54. This is a location anchor, not a cadastral building-centroid claim.
    constexpr double SilpoLatitude = 50.94907;
    constexpr double SilpoLongitude = 30.87621;

    // The current road/source reconstruction is mostly cardinal in this block. Keep one explicit orientation
    // constant so a later surveyed footprint can be rotated without rewriting photo-model dimensions.
    constexpr float SilpoYawDegrees = 0.0f;

    // Photo-proportioned footprint: approximately 32 m frontage x 18 m depth plus a narrow cleanup margin.
    constexpr float CleanupHalfWidthCm = 1760.0f;
    constexpr float CleanupHalfDepthCm = 1120.0f;

    FVector SilpoAnchor()
    {
        return FOCGeoReference::ToLocalCm(SilpoLatitude, SilpoLongitude, 0.0);
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
        UMaterialInterface* Material, const FName Name, const bool bCollision, const bool bCastShadow,
        const int32 CullEndCm = 90000)
    {
        if (!Owner || !Root || !Mesh) return nullptr;

        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;

        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        if (Material)
        {
            const int32 SlotCount = FMath::Max(1, Mesh->GetStaticMaterials().Num());
            for (int32 Slot = 0; Slot < SlotCount; ++Slot)
            {
                Component->SetMaterial(Slot, Material);
            }
        }

        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionProfileName(FName(bCollision ? TEXT("BlockAll") : TEXT("NoCollision")));
        Component->SetCollisionEnabled(bCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(bCollision);
        Component->SetCastShadow(bCastShadow);
        if (CullEndCm > 0) Component->SetCullDistances(0, CullEndCm);

        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void AddBox(UInstancedStaticMeshComponent* Component, const FVector& Center, const FVector& SizeCm,
        const FRotator& Rotation = FRotator::ZeroRotator)
    {
        if (!Component) return;
        Component->AddInstance(FTransform(Rotation, Center, SizeCm / 100.0f), false);
    }

    void AddPlaque(UInstancedStaticMeshComponent* Component, const FVector& Center,
        const float WidthCm, const float HeightCm, const float DepthCm)
    {
        if (!Component) return;

        // Engine cylinder is Z-axis aligned. Rotating 90 degrees about X turns the thin cylinder into an
        // elliptical facade plaque in the local X/Z plane.
        const FVector Scale(WidthCm / 100.0f, HeightCm / 100.0f, DepthCm / 100.0f);
        Component->AddInstance(FTransform(FRotator(90.0f, 0.0f, 0.0f), Center, Scale), false);
    }

    bool IsSourceBuildingFamily(const FName Name)
    {
        return Name == TEXT("Buildings") ||
            Name == TEXT("ResidentialRoofs") ||
            Name == TEXT("ResidentialDetails") ||
            Name == TEXT("LandmarkBlocks") ||
            Name == TEXT("LandmarkRoofs") ||
            Name == TEXT("LandmarkWindows") ||
            Name == TEXT("LandmarkDetails");
    }

    bool IsInsideSilpoFootprint(const FVector& WorldLocation)
    {
        const FVector Anchor = SilpoAnchor();
        const FVector Delta = WorldLocation - Anchor;
        const FVector Local = FRotator(0.0f, -SilpoYawDegrees, 0.0f).RotateVector(Delta);
        return FMath::Abs(Local.X) <= CleanupHalfWidthCm &&
            FMath::Abs(Local.Y) <= CleanupHalfDepthCm;
    }

    int32 RemoveSourceInstancesInFootprint(UInstancedStaticMeshComponent* Component)
    {
        if (!Component) return 0;

        int32 Removed = 0;
        for (int32 Index = Component->GetInstanceCount() - 1; Index >= 0; --Index)
        {
            FTransform Transform;
            if (!Component->GetInstanceTransform(Index, Transform, true)) continue;
            if (!IsInsideSilpoFootprint(Transform.GetLocation())) continue;
            if (Component->RemoveInstance(Index)) ++Removed;
        }

        if (Removed > 0) Component->MarkRenderStateDirty();
        return Removed;
    }

    void AddBillboard(UInstancedStaticMeshComponent* Frame, UInstancedStaticMeshComponent* Panel,
        const float X, const float Z, const float Width, const float Height)
    {
        constexpr float FrontY = -914.0f;
        AddBox(Frame, FVector(X, FrontY, Z), FVector(Width + 24.0f, 12.0f, Height + 24.0f));
        AddBox(Panel, FVector(X, FrontY - 8.0f, Z), FVector(Width, 8.0f, Height));
    }

    void AddWallLamp(UInstancedStaticMeshComponent* Metal, const float X, const float Z)
    {
        constexpr float WallY = -920.0f;
        AddBox(Metal, FVector(X, WallY - 14.0f, Z), FVector(18.0f, 38.0f, 20.0f));
        AddBox(Metal, FVector(X, WallY - 38.0f, Z - 9.0f), FVector(70.0f, 18.0f, 14.0f));
    }
}

bool UOCR13SilpoPhotoModelSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13SilpoPhotoModelSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) ReplaceSilpo(*World);
        }), SilpoPhotoModelDelaySeconds, false);
}

void UOCR13SilpoPhotoModelSubsystem::ReplaceSilpo(UWorld& World)
{
    SuppressSourceBuilding(World);
    BuildSilpo(World);
}

void UOCR13SilpoPhotoModelSubsystem::SuppressSourceBuilding(UWorld& World)
{
    int32 RemovedInstances = 0;
    int32 TouchedFamilies = 0;

    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor) continue;

        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (!Component || !IsSourceBuildingFamily(Component->GetFName())) continue;

            const int32 RemovedHere = RemoveSourceInstancesInFootprint(Component);
            if (RemovedHere <= 0) continue;

            RemovedInstances += RemovedHere;
            ++TouchedFamilies;
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("R13 Silpo site replacement: source building instances removed=%d across %d families at %.5f, %.5f."),
        RemovedInstances, TouchedFamilies, SilpoLatitude, SilpoLongitude);
}

void UOCR13SilpoPhotoModelSubsystem::BuildSilpo(UWorld& World)
{
    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    UMaterialInterface* Basic = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!Cube || !Cylinder || !Basic) return;

    AActor* Model = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!Model) return;

    Model->SetReplicates(false);
    Model->SetActorEnableCollision(true);
    Model->Tags.Add(TEXT("R13_SilpoPhotoModel"));

    USceneComponent* Root = NewObject<USceneComponent>(Model, TEXT("R13_SilpoPhotoModelRoot"));
    if (!Root)
    {
        Model->Destroy();
        return;
    }

    Root->SetMobility(EComponentMobility::Static);
    Model->SetRootComponent(Root);
    Model->AddInstanceComponent(Root);
    Root->RegisterComponent();

    Model->SetActorLocationAndRotation(SilpoAnchor(), FRotator(0.0f, SilpoYawDegrees, 0.0f));

    // The reference set spans multiple facade paint states. The dominant supplied views show the warm,
    // faded peach/beige exterior, so this model uses that state while preserving the photographed silhouette.
    UMaterialInstanceDynamic* WallMat = MakeColorMaterial(Model, Basic, TEXT("R13Silpo_WallMat"),
        FLinearColor(0.56f, 0.36f, 0.22f, 1.0f));
    UMaterialInstanceDynamic* WallLightMat = MakeColorMaterial(Model, Basic, TEXT("R13Silpo_WallLightMat"),
        FLinearColor(0.72f, 0.53f, 0.37f, 1.0f));
    UMaterialInstanceDynamic* PlinthMat = MakeColorMaterial(Model, Basic, TEXT("R13Silpo_PlinthMat"),
        FLinearColor(0.24f, 0.25f, 0.23f, 1.0f));
    UMaterialInstanceDynamic* RoofMat = MakeColorMaterial(Model, Basic, TEXT("R13Silpo_RoofMat"),
        FLinearColor(0.10f, 0.11f, 0.11f, 1.0f));
    UMaterialInstanceDynamic* WhiteMat = MakeColorMaterial(Model, Basic, TEXT("R13Silpo_WhiteMat"),
        FLinearColor(0.82f, 0.82f, 0.78f, 1.0f));
    UMaterialInstanceDynamic* GlassMat = MakeColorMaterial(Model, Basic, TEXT("R13Silpo_GlassMat"),
        FLinearColor(0.08f, 0.16f, 0.19f, 1.0f));
    UMaterialInstanceDynamic* MetalMat = MakeColorMaterial(Model, Basic, TEXT("R13Silpo_MetalMat"),
        FLinearColor(0.16f, 0.17f, 0.17f, 1.0f));
    UMaterialInstanceDynamic* AsphaltMat = MakeColorMaterial(Model, Basic, TEXT("R13Silpo_AsphaltMat"),
        FLinearColor(0.07f, 0.075f, 0.075f, 1.0f));
    UMaterialInstanceDynamic* BlueMat = MakeColorMaterial(Model, Basic, TEXT("R13Silpo_BlueMat"),
        FLinearColor(0.035f, 0.12f, 0.34f, 1.0f));
    UMaterialInstanceDynamic* OrangeMat = MakeColorMaterial(Model, Basic, TEXT("R13Silpo_OrangeMat"),
        FLinearColor(0.86f, 0.31f, 0.085f, 1.0f));
    UMaterialInstanceDynamic* AdBlueMat = MakeColorMaterial(Model, Basic, TEXT("R13Silpo_AdBlueMat"),
        FLinearColor(0.08f, 0.31f, 0.47f, 1.0f));
    UMaterialInstanceDynamic* AdGreenMat = MakeColorMaterial(Model, Basic, TEXT("R13Silpo_AdGreenMat"),
        FLinearColor(0.16f, 0.43f, 0.17f, 1.0f));
    UMaterialInstanceDynamic* AdPurpleMat = MakeColorMaterial(Model, Basic, TEXT("R13Silpo_AdPurpleMat"),
        FLinearColor(0.34f, 0.18f, 0.39f, 1.0f));

    UInstancedStaticMeshComponent* Shell = MakeISM(Model, Root, Cube, WallMat,
        TEXT("R13Silpo_MainShell"), true, true);
    UInstancedStaticMeshComponent* Parapet = MakeISM(Model, Root, Cube, WallLightMat,
        TEXT("R13Silpo_SteppedParapet"), true, true);
    UInstancedStaticMeshComponent* Plinth = MakeISM(Model, Root, Cube, PlinthMat,
        TEXT("R13Silpo_Plinth"), true, true);
    UInstancedStaticMeshComponent* Roof = MakeISM(Model, Root, Cube, RoofMat,
        TEXT("R13Silpo_FlatRoof"), true, true);
    UInstancedStaticMeshComponent* Entrance = MakeISM(Model, Root, Cube, WhiteMat,
        TEXT("R13Silpo_EntranceFrame"), true, true);
    UInstancedStaticMeshComponent* Glass = MakeISM(Model, Root, Cube, GlassMat,
        TEXT("R13Silpo_EntranceGlass"), false, true);
    UInstancedStaticMeshComponent* Metal = MakeISM(Model, Root, Cube, MetalMat,
        TEXT("R13Silpo_FacadeMetal"), false, true);
    UInstancedStaticMeshComponent* Sidewalk = MakeISM(Model, Root, Cube, PlinthMat,
        TEXT("R13Silpo_SidewalkAndSteps"), true, true);
    UInstancedStaticMeshComponent* Parking = MakeISM(Model, Root, Cube, AsphaltMat,
        TEXT("R13Silpo_ParkingApron"), true, false);
    UInstancedStaticMeshComponent* ParkingLines = MakeISM(Model, Root, Cube, WhiteMat,
        TEXT("R13Silpo_ParkingLines"), false, false);
    UInstancedStaticMeshComponent* LogoBlue = MakeISM(Model, Root, Cylinder, BlueMat,
        TEXT("R13Silpo_LogoBlueBorder"), false, true);
    UInstancedStaticMeshComponent* LogoOrange = MakeISM(Model, Root, Cylinder, OrangeMat,
        TEXT("R13Silpo_LogoOrangeFace"), false, true);
    UInstancedStaticMeshComponent* AdFrame = MakeISM(Model, Root, Cube, MetalMat,
        TEXT("R13Silpo_AdvertisingFrames"), false, true);
    UInstancedStaticMeshComponent* AdBlue = MakeISM(Model, Root, Cube, AdBlueMat,
        TEXT("R13Silpo_AdvertisingBlue"), false, false);
    UInstancedStaticMeshComponent* AdGreen = MakeISM(Model, Root, Cube, AdGreenMat,
        TEXT("R13Silpo_AdvertisingGreen"), false, false);
    UInstancedStaticMeshComponent* AdPurple = MakeISM(Model, Root, Cube, AdPurpleMat,
        TEXT("R13Silpo_AdvertisingPurple"), false, false);

    // Main one-storey mass and concrete plinth.
    AddBox(Plinth, FVector(0.0f, 0.0f, 38.0f), FVector(3240.0f, 1840.0f, 76.0f));
    AddBox(Shell, FVector(0.0f, 0.0f, 280.0f), FVector(3200.0f, 1800.0f, 480.0f));
    AddBox(Roof, FVector(0.0f, 0.0f, 527.0f), FVector(3180.0f, 1780.0f, 34.0f));

    // Stepped front silhouette visible in the frontal and oblique references.
    AddBox(Parapet, FVector(-1420.0f, -870.0f, 542.0f), FVector(360.0f, 70.0f, 124.0f));
    AddBox(Parapet, FVector(-1080.0f, -870.0f, 568.0f), FVector(340.0f, 70.0f, 176.0f));
    AddBox(Parapet, FVector(-680.0f, -870.0f, 598.0f), FVector(460.0f, 70.0f, 236.0f));
    AddBox(Parapet, FVector(0.0f, -870.0f, 628.0f), FVector(900.0f, 70.0f, 296.0f));
    AddBox(Parapet, FVector(680.0f, -870.0f, 598.0f), FVector(460.0f, 70.0f, 236.0f));
    AddBox(Parapet, FVector(1080.0f, -870.0f, 568.0f), FVector(340.0f, 70.0f, 176.0f));
    AddBox(Parapet, FVector(1420.0f, -870.0f, 542.0f), FVector(360.0f, 70.0f, 124.0f));

    // Repeated raised side parapet piers visible along the long wall.
    for (float Y = -520.0f; Y <= 640.0f; Y += 290.0f)
    {
        AddBox(Parapet, FVector(-1565.0f, Y, 573.0f), FVector(70.0f, 86.0f, 186.0f));
        AddBox(Parapet, FVector(1565.0f, Y, 573.0f), FVector(70.0f, 86.0f, 186.0f));
    }

    // Left-side entrance vestibule and tiled approach.
    AddBox(Entrance, FVector(-1350.0f, -1015.0f, 205.0f), FVector(500.0f, 300.0f, 410.0f));
    AddBox(Glass, FVector(-1350.0f, -1172.0f, 190.0f), FVector(170.0f, 8.0f, 300.0f));
    AddBox(Entrance, FVector(-1450.0f, -1178.0f, 190.0f), FVector(18.0f, 18.0f, 310.0f));
    AddBox(Entrance, FVector(-1250.0f, -1178.0f, 190.0f), FVector(18.0f, 18.0f, 310.0f));
    AddBox(Entrance, FVector(-1350.0f, -1178.0f, 346.0f), FVector(220.0f, 18.0f, 18.0f));
    AddBox(Entrance, FVector(-1350.0f, -1120.0f, 425.0f), FVector(600.0f, 360.0f, 34.0f));
    AddBox(Sidewalk, FVector(-1350.0f, -1260.0f, 18.0f), FVector(650.0f, 470.0f, 36.0f));
    AddBox(Sidewalk, FVector(-1350.0f, -1515.0f, 10.0f), FVector(650.0f, 90.0f, 20.0f));

    // Long pavement strip against the advertising facade.
    AddBox(Sidewalk, FVector(310.0f, -1080.0f, 11.0f), FVector(2520.0f, 340.0f, 22.0f));

    // Parking apron and photographed perpendicular bay rhythm. Existing road remains untouched.
    AddBox(Parking, FVector(150.0f, -2050.0f, 5.0f), FVector(3900.0f, 1700.0f, 10.0f));
    for (float X = -1450.0f; X <= 1550.0f; X += 500.0f)
    {
        AddBox(ParkingLines, FVector(X, -2020.0f, 12.0f), FVector(10.0f, 1350.0f, 4.0f));
    }

    // Framed facade posters. Their artwork is intentionally abstracted: photos establish panel count/placement,
    // while no photographed advertising bitmap is copied into the game.
    AddBillboard(AdFrame, AdPurple, -820.0f, 285.0f, 360.0f, 250.0f);
    AddBillboard(AdFrame, AdBlue, -390.0f, 285.0f, 390.0f, 250.0f);
    AddBillboard(AdFrame, AdGreen, 70.0f, 285.0f, 390.0f, 250.0f);
    AddBillboard(AdFrame, AdBlue, 530.0f, 285.0f, 390.0f, 250.0f);
    AddBillboard(AdFrame, AdGreen, 990.0f, 285.0f, 390.0f, 250.0f);

    // Large raised Silpo facade plaque: blue edge, orange face, then actual store name as local TextRender.
    AddPlaque(LogoBlue, FVector(180.0f, -925.0f, 596.0f), 900.0f, 300.0f, 24.0f);
    AddPlaque(LogoOrange, FVector(180.0f, -941.0f, 596.0f), 840.0f, 250.0f, 18.0f);

    UTextRenderComponent* LogoText = NewObject<UTextRenderComponent>(Model, TEXT("R13Silpo_LogoText"));
    if (LogoText)
    {
        LogoText->SetupAttachment(Root);
        LogoText->SetMobility(EComponentMobility::Static);
        LogoText->SetText(FText::FromString(TEXT("СІЛЬПО")));
        LogoText->SetWorldSize(118.0f);
        LogoText->SetTextRenderColor(FColor(236, 238, 235));
        LogoText->SetRelativeLocation(FVector(-95.0f, -958.0f, 555.0f));
        LogoText->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
        LogoText->SetCastShadow(false);
        Model->AddInstanceComponent(LogoText);
        LogoText->RegisterComponent();
    }

    // Wall-mounted shallow lamps, matching the repeated fixtures above the poster band.
    for (float X : { -950.0f, -500.0f, -40.0f, 480.0f, 930.0f, 1360.0f })
    {
        AddWallLamp(Metal, X, 455.0f);
    }

    // Parking sign and post at the right side of the facade.
    AddBox(Metal, FVector(1180.0f, -1260.0f, 120.0f), FVector(16.0f, 16.0f, 240.0f));
    AddBox(AdBlue, FVector(1180.0f, -1260.0f, 255.0f), FVector(125.0f, 14.0f, 125.0f));
    AddBox(ParkingLines, FVector(1180.0f, -1270.0f, 255.0f), FVector(58.0f, 5.0f, 76.0f));

    UE_LOG(LogTemp, Display,
        TEXT("R13 Silpo photo model built at local anchor (%.1f, %.1f), tag=R13_SilpoPhotoModel."),
        SilpoAnchor().X, SilpoAnchor().Y);
}
