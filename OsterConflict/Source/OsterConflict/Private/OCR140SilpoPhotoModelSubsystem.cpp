#include "OCR140SilpoPhotoModelSubsystem.h"

#include "OCGameMode.h"
#include "OCGeoReference.h"
#include "OCInteractableDoor.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PointLightComponent.h"
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
    constexpr float SilpoBuildDelaySeconds = 5.35f;
    constexpr float SourceCleanupRadiusCm = 2600.0f;

    // The address-center anchor is verified. The exact parcel/footprint bearing is not survey data yet.
    // The first photo-driven pass therefore keeps the long facade close to east-west until a measured
    // footprint or reliable orthophoto bearing is available.
    constexpr float SilpoYawDegrees = 0.0f;

    constexpr float BuildingLengthCm = 3000.0f;
    constexpr float BuildingDepthCm = 1750.0f;
    constexpr float WallHeightCm = 390.0f;
    constexpr float WallThicknessCm = 26.0f;
    constexpr float HalfLength = BuildingLengthCm * 0.5f;
    constexpr float HalfDepth = BuildingDepthCm * 0.5f;
    constexpr float FrontY = -HalfDepth;
    constexpr float RearY = HalfDepth;

    // The photo set places the public entrance at the far-left end of the long front elevation.
    constexpr float EntranceCenterX = -1315.0f;
    constexpr float EntranceWidthCm = 140.0f;
    constexpr float EntranceHeightCm = 245.0f;

    FVector SilpoAnchor()
    {
        const FOCGeoReferencePoint Ref = FOCGeoReference::Silpo();
        return FOCGeoReference::ToLocalCm(Ref.Latitude, Ref.Longitude, 0.0);
    }

    FVector RotateLocal(const FVector& Local)
    {
        return FRotator(0.0f, SilpoYawDegrees, 0.0f).RotateVector(Local);
    }

    FVector At(const FVector& Origin, const FVector& Local)
    {
        return Origin + RotateLocal(Local);
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
        const int32 CullEndCm = 85000)
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
        Component->AddInstance(FTransform(Rotation, Center, SizeCm / 100.0f), true);
    }

    void AddLocalBox(UInstancedStaticMeshComponent* Component, const FVector& Origin,
        const FVector& LocalCenter, const FVector& SizeCm, const FRotator& LocalRotation = FRotator::ZeroRotator)
    {
        const FRotator WorldRotation(LocalRotation.Pitch,
            LocalRotation.Yaw + SilpoYawDegrees, LocalRotation.Roll);
        AddBox(Component, At(Origin, LocalCenter), SizeCm, WorldRotation);
    }

    void AddShelfRun(UInstancedStaticMeshComponent* Shelves, const FVector& Origin,
        const FVector& LocalCenter, const float RunLengthCm, const float TotalDepthCm, const float HeightCm)
    {
        if (!Shelves) return;
        const float Upright = 7.0f;
        const float Board = 6.0f;
        const float HalfRun = RunLengthCm * 0.5f;
        const float HalfDepthRun = TotalDepthCm * 0.5f;

        for (const float X : { -HalfRun + Upright * 0.5f, HalfRun - Upright * 0.5f })
        {
            AddLocalBox(Shelves, Origin,
                LocalCenter + FVector(X, -HalfDepthRun + Upright * 0.5f, HeightCm * 0.5f),
                FVector(Upright, Upright, HeightCm));
            AddLocalBox(Shelves, Origin,
                LocalCenter + FVector(X, HalfDepthRun - Upright * 0.5f, HeightCm * 0.5f),
                FVector(Upright, Upright, HeightCm));
        }

        for (int32 Level = 0; Level < 5; ++Level)
        {
            const float Z = 18.0f + static_cast<float>(Level) * ((HeightCm - 30.0f) / 4.0f);
            AddLocalBox(Shelves, Origin, LocalCenter + FVector(0.0f, -TotalDepthCm * 0.24f, Z),
                FVector(RunLengthCm, TotalDepthCm * 0.43f, Board));
            AddLocalBox(Shelves, Origin, LocalCenter + FVector(0.0f, TotalDepthCm * 0.24f, Z),
                FVector(RunLengthCm, TotalDepthCm * 0.43f, Board));
        }
    }

    bool RemoveInstancesNear(UInstancedStaticMeshComponent* Component, const FVector& Center, const float RadiusCm)
    {
        if (!Component) return false;
        const float RadiusSq = FMath::Square(RadiusCm);
        bool bChanged = false;
        for (int32 Index = Component->GetInstanceCount() - 1; Index >= 0; --Index)
        {
            FTransform Transform;
            if (!Component->GetInstanceTransform(Index, Transform, true)) continue;
            if (FVector::DistSquared2D(Transform.GetLocation(), Center) > RadiusSq) continue;
            if (Component->RemoveInstance(Index)) bChanged = true;
        }
        if (bChanged) Component->MarkRenderStateDirty();
        return bChanged;
    }

    bool IsSourceBuildingFamily(const FName Name)
    {
        return Name == TEXT("Buildings") || Name == TEXT("ResidentialRoofs") ||
            Name == TEXT("ResidentialDetails") || Name == TEXT("LandmarkBlocks") ||
            Name == TEXT("LandmarkRoofs") || Name == TEXT("LandmarkWindows") ||
            Name == TEXT("LandmarkDetails");
    }

    bool IsSourceFenceFamily(const FName Name)
    {
        return Name == TEXT("Fences") || Name == TEXT("WoodFences") ||
            Name == TEXT("MetalFences") || Name == TEXT("LightSheetFences");
    }

    bool IsSourceTreeFamily(const FName Name)
    {
        return Name == TEXT("TreeTrunks") || Name == TEXT("TreeCrowns") ||
            Name == TEXT("SovietPoplarTrunks") || Name == TEXT("SovietPoplarCrowns") ||
            Name == TEXT("BirchTrunks") || Name == TEXT("BirchCrowns") ||
            Name == TEXT("PineTrunks") || Name == TEXT("PineCrowns");
    }

    bool IsLegacySilpoComponent(const FName Name)
    {
        return Name.ToString().StartsWith(TEXT("R140Silpo_"));
    }
}

bool UOCR140SilpoPhotoModelSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR140SilpoPhotoModelSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
        }), SilpoBuildDelaySeconds, false);
}

void UOCR140SilpoPhotoModelSubsystem::ReplaceSilpo(UWorld& World)
{
    SuppressSourceSite(World);
    BuildSilpo(World);
}

void UOCR140SilpoPhotoModelSubsystem::SuppressSourceSite(UWorld& World)
{
    const FVector Site = SilpoAnchor();
    int32 HiddenLegacy = 0;
    int32 TrimmedBuildings = 0;
    int32 TrimmedFences = 0;
    int32 TrimmedTrees = 0;

    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor) continue;

        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (!Component) continue;
            const FName Name = Component->GetFName();

            if (IsLegacySilpoComponent(Name))
            {
                Component->SetVisibility(false, true);
                Component->SetHiddenInGame(true, true);
                Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                ++HiddenLegacy;
                continue;
            }

            if (IsSourceBuildingFamily(Name) && RemoveInstancesNear(Component, Site, SourceCleanupRadiusCm))
            {
                ++TrimmedBuildings;
                continue;
            }
            if (IsSourceFenceFamily(Name) && RemoveInstancesNear(Component, Site, SourceCleanupRadiusCm))
            {
                ++TrimmedFences;
                continue;
            }
            if (IsSourceTreeFamily(Name) && RemoveInstancesNear(Component, Site, 1500.0f))
            {
                ++TrimmedTrees;
            }
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("R14.0 Silpo site cleanup: legacy=%d buildingFamilies=%d fenceFamilies=%d treeFamilies=%d at [%.0f %.0f]."),
        HiddenLegacy, TrimmedBuildings, TrimmedFences, TrimmedTrees, Site.X, Site.Y);
}

void UOCR140SilpoPhotoModelSubsystem::BuildSilpo(UWorld& World)
{
    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UMaterialInterface* Basic = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!Cube || !Basic) return;

    const FVector Site = SilpoAnchor();

    AActor* Model = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform(FRotator::ZeroRotator, Site));
    if (!Model) return;
    Model->SetReplicates(false);
    Model->SetActorEnableCollision(true);
    Model->Tags.Add(TEXT("R140_SilpoPhotoModel"));
    Model->Tags.Add(TEXT("SilpoOster_BohdanaKhmelnytskoho54"));

    USceneComponent* Root = NewObject<USceneComponent>(Model, TEXT("R140Silpo_ModelRoot"));
    if (!Root)
    {
        Model->Destroy();
        return;
    }
    Root->SetMobility(EComponentMobility::Static);
    Model->SetRootComponent(Root);
    Model->AddInstanceComponent(Root);
    Root->RegisterComponent();
    Model->SetActorLocation(Site);

    UMaterialInstanceDynamic* Stucco = MakeColorMaterial(Model, Basic, TEXT("R140Silpo_StuccoMat"),
        FLinearColor(0.72f, 0.57f, 0.42f, 1.0f));
    UMaterialInstanceDynamic* StuccoLight = MakeColorMaterial(Model, Basic, TEXT("R140Silpo_StuccoLightMat"),
        FLinearColor(0.83f, 0.69f, 0.54f, 1.0f));
    UMaterialInstanceDynamic* FoundationMat = MakeColorMaterial(Model, Basic, TEXT("R140Silpo_FoundationMat"),
        FLinearColor(0.28f, 0.29f, 0.28f, 1.0f));
    UMaterialInstanceDynamic* RoofMat = MakeColorMaterial(Model, Basic, TEXT("R140Silpo_RoofMat"),
        FLinearColor(0.22f, 0.23f, 0.22f, 1.0f));
    UMaterialInstanceDynamic* GlassMat = MakeColorMaterial(Model, Basic, TEXT("R140Silpo_GlassMat"),
        FLinearColor(0.14f, 0.20f, 0.22f, 1.0f));
    UMaterialInstanceDynamic* FrameMat = MakeColorMaterial(Model, Basic, TEXT("R140Silpo_FrameMat"),
        FLinearColor(0.20f, 0.21f, 0.20f, 1.0f));
    UMaterialInstanceDynamic* SignOrange = MakeColorMaterial(Model, Basic, TEXT("R140Silpo_SignOrangeMat"),
        FLinearColor(0.89f, 0.36f, 0.08f, 1.0f));
    UMaterialInstanceDynamic* SignBlue = MakeColorMaterial(Model, Basic, TEXT("R140Silpo_SignBlueMat"),
        FLinearColor(0.05f, 0.29f, 0.43f, 1.0f));
    UMaterialInstanceDynamic* PromoBlue = MakeColorMaterial(Model, Basic, TEXT("R140Silpo_PromoBlueMat"),
        FLinearColor(0.11f, 0.32f, 0.47f, 1.0f));
    UMaterialInstanceDynamic* PromoGreen = MakeColorMaterial(Model, Basic, TEXT("R140Silpo_PromoGreenMat"),
        FLinearColor(0.30f, 0.45f, 0.20f, 1.0f));
    UMaterialInstanceDynamic* PromoNeutral = MakeColorMaterial(Model, Basic, TEXT("R140Silpo_PromoNeutralMat"),
        FLinearColor(0.65f, 0.58f, 0.48f, 1.0f));
    UMaterialInstanceDynamic* StoreFloorMat = MakeColorMaterial(Model, Basic, TEXT("R140Silpo_StoreFloorMat"),
        FLinearColor(0.66f, 0.66f, 0.62f, 1.0f));
    UMaterialInstanceDynamic* AsphaltMat = MakeColorMaterial(Model, Basic, TEXT("R140Silpo_AsphaltMat"),
        FLinearColor(0.16f, 0.16f, 0.16f, 1.0f));
    UMaterialInstanceDynamic* ParkingMarkMat = MakeColorMaterial(Model, Basic, TEXT("R140Silpo_ParkingMarkMat"),
        FLinearColor(0.74f, 0.73f, 0.66f, 1.0f));
    UMaterialInstanceDynamic* PlantMat = MakeColorMaterial(Model, Basic, TEXT("R140Silpo_PlantStripMat"),
        FLinearColor(0.18f, 0.32f, 0.12f, 1.0f));
    UMaterialInstanceDynamic* FixtureMat = MakeColorMaterial(Model, Basic, TEXT("R140Silpo_FixtureMat"),
        FLinearColor(0.90f, 0.90f, 0.84f, 1.0f));
    UMaterialInstanceDynamic* ShelfMat = MakeColorMaterial(Model, Basic, TEXT("R140Silpo_ShelfMat"),
        FLinearColor(0.47f, 0.48f, 0.45f, 1.0f));
    UMaterialInstanceDynamic* CheckoutMat = MakeColorMaterial(Model, Basic, TEXT("R140Silpo_CheckoutMat"),
        FLinearColor(0.29f, 0.30f, 0.28f, 1.0f));
    UMaterialInstanceDynamic* CoolerMat = MakeColorMaterial(Model, Basic, TEXT("R140Silpo_CoolerMat"),
        FLinearColor(0.60f, 0.63f, 0.63f, 1.0f));
    UMaterialInstanceDynamic* ProduceMat = MakeColorMaterial(Model, Basic, TEXT("R140Silpo_ProduceMat"),
        FLinearColor(0.43f, 0.37f, 0.25f, 1.0f));
    UMaterialInstanceDynamic* UtilityMat = MakeColorMaterial(Model, Basic, TEXT("R140Silpo_UtilityMat"),
        FLinearColor(0.19f, 0.20f, 0.19f, 1.0f));

    UInstancedStaticMeshComponent* Walls = MakeISM(Model, Root, Cube, Stucco,
        TEXT("R140Silpo_Walls"), true, true);
    UInstancedStaticMeshComponent* Parapet = MakeISM(Model, Root, Cube, StuccoLight,
        TEXT("R140Silpo_SteppedParapet"), true, true);
    UInstancedStaticMeshComponent* EntranceShell = MakeISM(Model, Root, Cube, StuccoLight,
        TEXT("R140Silpo_EntranceShell"), true, true);
    UInstancedStaticMeshComponent* Foundation = MakeISM(Model, Root, Cube, FoundationMat,
        TEXT("R140Silpo_Foundation"), true, false);
    UInstancedStaticMeshComponent* Roof = MakeISM(Model, Root, Cube, RoofMat,
        TEXT("R140Silpo_Roof"), true, true);
    UInstancedStaticMeshComponent* Windows = MakeISM(Model, Root, Cube, GlassMat,
        TEXT("R140Silpo_Windows"), false, false);
    UInstancedStaticMeshComponent* Frames = MakeISM(Model, Root, Cube, FrameMat,
        TEXT("R140Silpo_Frames"), false, true);
    UInstancedStaticMeshComponent* SignBack = MakeISM(Model, Root, Cube, SignOrange,
        TEXT("R140Silpo_FacadeSignBack"), false, true);
    UInstancedStaticMeshComponent* SignAccent = MakeISM(Model, Root, Cube, SignBlue,
        TEXT("R140Silpo_FacadeSignAccent"), false, true);
    UInstancedStaticMeshComponent* PromoA = MakeISM(Model, Root, Cube, PromoBlue,
        TEXT("R140Silpo_PromoBlue"), false, false);
    UInstancedStaticMeshComponent* PromoB = MakeISM(Model, Root, Cube, PromoGreen,
        TEXT("R140Silpo_PromoGreen"), false, false);
    UInstancedStaticMeshComponent* PromoC = MakeISM(Model, Root, Cube, PromoNeutral,
        TEXT("R140Silpo_PromoNeutral"), false, false);
    UInstancedStaticMeshComponent* StoreFloor = MakeISM(Model, Root, Cube, StoreFloorMat,
        TEXT("R140Silpo_StoreFloor"), true, false);
    UInstancedStaticMeshComponent* Asphalt = MakeISM(Model, Root, Cube, AsphaltMat,
        TEXT("R140Silpo_ForecourtAsphalt"), true, false);
    UInstancedStaticMeshComponent* ParkingMarks = MakeISM(Model, Root, Cube, ParkingMarkMat,
        TEXT("R140Silpo_ParkingMarks"), false, false);
    UInstancedStaticMeshComponent* PlantStrip = MakeISM(Model, Root, Cube, PlantMat,
        TEXT("R140Silpo_FacadePlantStrip"), false, false);
    UInstancedStaticMeshComponent* CeilingFixtures = MakeISM(Model, Root, Cube, FixtureMat,
        TEXT("R140Silpo_CeilingFixtures"), false, false);
    UInstancedStaticMeshComponent* Shelves = MakeISM(Model, Root, Cube, ShelfMat,
        TEXT("R140Silpo_EmptyShelves"), true, true);
    UInstancedStaticMeshComponent* Checkouts = MakeISM(Model, Root, Cube, CheckoutMat,
        TEXT("R140Silpo_Checkouts"), true, true);
    UInstancedStaticMeshComponent* Coolers = MakeISM(Model, Root, Cube, CoolerMat,
        TEXT("R140Silpo_WallCoolers"), true, true);
    UInstancedStaticMeshComponent* Produce = MakeISM(Model, Root, Cube, ProduceMat,
        TEXT("R140Silpo_EmptyProduceIsland"), true, true);
    UInstancedStaticMeshComponent* Utilities = MakeISM(Model, Root, Cube, UtilityMat,
        TEXT("R140Silpo_Utilities"), true, true);

    // Store floor and low hidden roof slab. The photo set supports a low/flat roof behind the front parapet.
    AddLocalBox(StoreFloor, Site, FVector(0.0f, 0.0f, 7.0f),
        FVector(BuildingLengthCm, BuildingDepthCm, 14.0f));
    AddLocalBox(Roof, Site, FVector(0.0f, 0.0f, WallHeightCm + 8.0f),
        FVector(BuildingLengthCm + 30.0f, BuildingDepthCm + 30.0f, 18.0f));

    const float WallZ = WallHeightCm * 0.5f;
    AddLocalBox(Walls, Site, FVector(0.0f, RearY, WallZ),
        FVector(BuildingLengthCm, WallThicknessCm, WallHeightCm));
    AddLocalBox(Walls, Site, FVector(-HalfLength, 0.0f, WallZ),
        FVector(WallThicknessCm, BuildingDepthCm, WallHeightCm));
    AddLocalBox(Walls, Site, FVector(HalfLength, 0.0f, WallZ),
        FVector(WallThicknessCm, BuildingDepthCm, WallHeightCm));

    // Photo-derived front elevation: one compact entrance at far left and a mostly solid advertising wall.
    const float EntranceLeft = EntranceCenterX - EntranceWidthCm * 0.5f;
    const float EntranceRight = EntranceCenterX + EntranceWidthCm * 0.5f;
    const float LeftSegmentWidth = EntranceLeft - (-HalfLength);
    const float RightSegmentWidth = HalfLength - EntranceRight;
    if (LeftSegmentWidth > 1.0f)
    {
        AddLocalBox(Walls, Site, FVector(-HalfLength + LeftSegmentWidth * 0.5f, FrontY, WallZ),
            FVector(LeftSegmentWidth, WallThicknessCm, WallHeightCm));
    }
    if (RightSegmentWidth > 1.0f)
    {
        AddLocalBox(Walls, Site, FVector(EntranceRight + RightSegmentWidth * 0.5f, FrontY, WallZ),
            FVector(RightSegmentWidth, WallThicknessCm, WallHeightCm));
    }
    AddLocalBox(Walls, Site,
        FVector(EntranceCenterX, FrontY, EntranceHeightCm + (WallHeightCm - EntranceHeightCm) * 0.5f),
        FVector(EntranceWidthCm, WallThicknessCm, WallHeightCm - EntranceHeightCm));

    // Grey lower plinth visible along the photographed facade.
    AddLocalBox(Foundation, Site, FVector(70.0f, FrontY - 15.0f, 26.0f),
        FVector(2700.0f, 20.0f, 52.0f));

    // The front silhouette is defined by a symmetrical stepped parapet, with the center sign at the high point.
    AddLocalBox(Parapet, Site, FVector(-1170.0f, FrontY - 10.0f, 415.0f), FVector(660.0f, 24.0f, 70.0f));
    AddLocalBox(Parapet, Site, FVector(-620.0f, FrontY - 10.0f, 440.0f), FVector(440.0f, 24.0f, 120.0f));
    AddLocalBox(Parapet, Site, FVector(0.0f, FrontY - 10.0f, 470.0f), FVector(820.0f, 24.0f, 180.0f));
    AddLocalBox(Parapet, Site, FVector(620.0f, FrontY - 10.0f, 440.0f), FVector(440.0f, 24.0f, 120.0f));
    AddLocalBox(Parapet, Site, FVector(1170.0f, FrontY - 10.0f, 415.0f), FVector(660.0f, 24.0f, 70.0f));

    // Compact left entrance porch/canopy and visible raised threshold from the close entrance references.
    AddLocalBox(EntranceShell, Site, FVector(EntranceCenterX, FrontY - 18.0f, 306.0f),
        FVector(210.0f, 36.0f, 42.0f));
    AddLocalBox(Roof, Site, FVector(EntranceCenterX, FrontY - 155.0f, 302.0f),
        FVector(300.0f, 300.0f, 18.0f));
    AddLocalBox(Foundation, Site, FVector(EntranceCenterX, FrontY - 72.0f, 10.0f),
        FVector(250.0f, 135.0f, 20.0f));
    AddLocalBox(Foundation, Site, FVector(EntranceCenterX, FrontY - 145.0f, 5.0f),
        FVector(290.0f, 85.0f, 10.0f));

    // Front wall photo set is dominated by promo boards rather than a row of windows.
    struct FPromoSlot { float X; float Width; int32 Style; };
    const FPromoSlot PromoSlots[] =
    {
        { -980.0f, 300.0f, 0 },
        { -610.0f, 310.0f, 1 },
        { -230.0f, 300.0f, 2 },
        { 260.0f, 360.0f, 0 },
        { 760.0f, 400.0f, 1 },
    };
    for (const FPromoSlot& Slot : PromoSlots)
    {
        UInstancedStaticMeshComponent* Target = Slot.Style == 0 ? PromoA : (Slot.Style == 1 ? PromoB : PromoC);
        AddLocalBox(Target, Site, FVector(Slot.X, FrontY - 23.0f, 175.0f),
            FVector(Slot.Width, 12.0f, 210.0f));
        AddLocalBox(Frames, Site, FVector(Slot.X, FrontY - 18.0f, 175.0f),
            FVector(Slot.Width + 18.0f, 10.0f, 228.0f));
    }

    // Sparse side/rear openings only. The long front remains solid as in the supplied exterior views.
    for (const float Y : { -420.0f, 130.0f, 580.0f })
    {
        AddLocalBox(Frames, Site, FVector(HalfLength + 16.0f, Y, 205.0f), FVector(16.0f, 230.0f, 190.0f));
        AddLocalBox(Windows, Site, FVector(HalfLength + 25.0f, Y, 205.0f), FVector(7.0f, 205.0f, 160.0f));
    }
    for (const float X : { -900.0f, -300.0f, 300.0f, 900.0f })
    {
        AddLocalBox(Frames, Site, FVector(X, RearY + 16.0f, 205.0f), FVector(245.0f, 16.0f, 190.0f));
        AddLocalBox(Windows, Site, FVector(X, RearY + 25.0f, 205.0f), FVector(215.0f, 7.0f, 160.0f));
    }

    // Large centered sign zone. Geometry uses source-only primitives; final logo art can replace this without moving the facade.
    AddLocalBox(SignBack, Site, FVector(50.0f, FrontY - 31.0f, 466.0f), FVector(760.0f, 20.0f, 132.0f));
    AddLocalBox(SignAccent, Site, FVector(-260.0f, FrontY - 44.0f, 466.0f), FVector(54.0f, 8.0f, 82.0f));
    UTextRenderComponent* SilpoText = NewObject<UTextRenderComponent>(Model, TEXT("R140Silpo_FacadeText"));
    if (SilpoText)
    {
        SilpoText->SetupAttachment(Root);
        SilpoText->SetText(FText::FromString(TEXT("СІЛЬПО")));
        SilpoText->SetTextRenderColor(FColor(244, 240, 226));
        SilpoText->SetHorizontalAlignment(EHTA_Center);
        SilpoText->SetWorldSize(64.0f);
        SilpoText->SetRelativeLocation(RotateLocal(FVector(70.0f, FrontY - 48.0f, 458.0f)));
        SilpoText->SetRelativeRotation(FRotator(0.0f, 90.0f + SilpoYawDegrees, 0.0f));
        SilpoText->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Model->AddInstanceComponent(SilpoText);
        SilpoText->RegisterComponent();
    }

    // Exterior wall lamps above the ad panels. These are simple source-only placeholders for the photographed lamps.
    for (const float X : { -980.0f, -600.0f, -220.0f, 260.0f, 760.0f })
    {
        AddLocalBox(Frames, Site, FVector(X, FrontY - 36.0f, 306.0f), FVector(34.0f, 28.0f, 18.0f));
    }

    // Immediate site: sidewalk strip, asphalt apron/parking, painted stall hints and a narrow planting strip.
    AddLocalBox(Foundation, Site, FVector(110.0f, FrontY - 105.0f, 5.0f), FVector(2750.0f, 165.0f, 10.0f));
    AddLocalBox(Asphalt, Site, FVector(100.0f, FrontY - 610.0f, 1.0f), FVector(3450.0f, 850.0f, 4.0f));
    for (const float X : { -720.0f, -270.0f, 180.0f, 630.0f, 1080.0f })
    {
        AddLocalBox(ParkingMarks, Site, FVector(X, FrontY - 545.0f, 4.0f), FVector(10.0f, 560.0f, 3.0f));
    }
    AddLocalBox(PlantStrip, Site, FVector(620.0f, FrontY - 125.0f, 8.0f), FVector(980.0f, 70.0f, 14.0f));

    // Interior shell furniture from the supplied interior references: long empty aisles and wide walkways.
    for (const float X : { -920.0f, -570.0f, -220.0f, 130.0f, 480.0f, 830.0f })
    {
        AddShelfRun(Shelves, Site, FVector(X, 110.0f, 0.0f), 105.0f, 760.0f, 180.0f);
    }

    // Empty produce island, visible as a low central merchandising table in the photo set.
    AddLocalBox(Produce, Site, FVector(-760.0f, -360.0f, 45.0f), FVector(360.0f, 165.0f, 90.0f));
    AddLocalBox(Produce, Site, FVector(-760.0f, -360.0f, 96.0f), FVector(330.0f, 138.0f, 16.0f));

    // Four cashier positions remain a practical gameplay blockout; the references show a compact multi-lane checkout zone.
    for (int32 Lane = 0; Lane < 4; ++Lane)
    {
        const float X = 300.0f + static_cast<float>(Lane) * 270.0f;
        AddLocalBox(Checkouts, Site, FVector(X, -600.0f, 46.0f), FVector(210.0f, 72.0f, 92.0f));
        AddLocalBox(Checkouts, Site, FVector(X - 50.0f, -600.0f, 100.0f), FVector(92.0f, 92.0f, 16.0f));
    }

    // Refrigerated wall runs are kept against the rear/side perimeter and deliberately empty in this pass.
    for (const float X : { -1100.0f, -650.0f, -200.0f, 250.0f, 700.0f, 1150.0f })
    {
        AddLocalBox(Coolers, Site, FVector(X, RearY - 66.0f, 118.0f), FVector(390.0f, 108.0f, 236.0f));
    }

    // Basic back-of-house/service markers only, so the main sales floor remains traversable.
    AddLocalBox(Utilities, Site, FVector(-1230.0f, 650.0f, 112.0f), FVector(245.0f, 190.0f, 224.0f));
    AddLocalBox(Utilities, Site, FVector(1230.0f, 650.0f, 102.0f), FVector(220.0f, 180.0f, 204.0f));

    // Long fluorescent/LED-style ceiling fixtures seen in the interior references.
    for (int32 Row = -1; Row <= 1; ++Row)
    {
        for (int32 Col = -2; Col <= 2; ++Col)
        {
            const FVector LocalLightLocation(static_cast<float>(Col) * 500.0f,
                static_cast<float>(Row) * 430.0f, 345.0f);
            AddLocalBox(CeilingFixtures, Site, LocalLightLocation, FVector(260.0f, 14.0f, 7.0f));

            UPointLightComponent* Light = NewObject<UPointLightComponent>(Model,
                FName(*FString::Printf(TEXT("R140Silpo_CeilingLight_%d_%d"), Row, Col)));
            if (!Light) continue;
            Light->SetupAttachment(Root);
            Light->SetMobility(EComponentMobility::Movable);
            Light->SetIntensity(1750.0f);
            Light->SetAttenuationRadius(760.0f);
            Light->SetLightColor(FLinearColor(0.96f, 0.95f, 0.90f));
            Light->SetCastShadows(false);
            Light->SetRelativeLocation(RotateLocal(LocalLightLocation - FVector(0.0f, 0.0f, 12.0f)));
            Model->AddInstanceComponent(Light);
            Light->RegisterComponent();
        }
    }

    // One working public entrance door at the photographed far-left location. Only authority spawns it.
    if (World.GetNetMode() != NM_Client)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        const FRotator DoorRotation(0.0f, SilpoYawDegrees, 0.0f);
        const FVector Hinge = At(Site,
            FVector(EntranceCenterX - EntranceWidthCm * 0.5f, FrontY - 10.0f, 10.0f));

        if (AOCInteractableDoor* Door = World.SpawnActor<AOCInteractableDoor>(AOCInteractableDoor::StaticClass(),
            Hinge, DoorRotation, SpawnParams))
        {
            Door->Tags.Add(TEXT("R140_SilpoEntranceDoor"));
            Door->Tags.Add(TEXT("SilpoEntranceMain"));
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("R14.0 Silpo photo model built at WGS84 %.9f, %.9f -> local [%.0f %.0f], stepped facade + left entrance + sparse interior."),
        FOCGeoReference::Silpo().Latitude, FOCGeoReference::Silpo().Longitude, Site.X, Site.Y);
}
