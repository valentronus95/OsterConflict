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

    // The public address point is stable. Exact footprint bearing is not survey data; keep the first-pass
    // long facade almost east-west and document the confidence separately in the Silpo TZ.
    constexpr float SilpoYawDegrees = 0.0f;

    constexpr float BuildingLengthCm = 3000.0f;
    constexpr float BuildingDepthCm = 1750.0f;
    constexpr float WallHeightCm = 430.0f;
    constexpr float WallThicknessCm = 26.0f;

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

    void AddShelfUnit(UInstancedStaticMeshComponent* Shelves, const FVector& Origin,
        const FVector& LocalCenter, const float LengthCm, const float DepthCm, const float HeightCm)
    {
        if (!Shelves) return;
        const float Upright = 7.0f;
        const float Board = 6.0f;
        const float HalfLength = LengthCm * 0.5f;
        const float HalfDepth = DepthCm * 0.5f;

        for (const float X : { -HalfLength + Upright * 0.5f, HalfLength - Upright * 0.5f })
        {
            AddLocalBox(Shelves, Origin, LocalCenter + FVector(X, -HalfDepth + Upright * 0.5f, HeightCm * 0.5f),
                FVector(Upright, Upright, HeightCm));
            AddLocalBox(Shelves, Origin, LocalCenter + FVector(X, HalfDepth - Upright * 0.5f, HeightCm * 0.5f),
                FVector(Upright, Upright, HeightCm));
        }

        for (int32 Level = 0; Level < 5; ++Level)
        {
            const float Z = 18.0f + static_cast<float>(Level) * ((HeightCm - 30.0f) / 4.0f);
            AddLocalBox(Shelves, Origin, LocalCenter + FVector(0.0f, -DepthCm * 0.24f, Z),
                FVector(LengthCm, DepthCm * 0.43f, Board));
            AddLocalBox(Shelves, Origin, LocalCenter + FVector(0.0f, DepthCm * 0.24f, Z),
                FVector(LengthCm, DepthCm * 0.43f, Board));
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
            if (IsSourceTreeFamily(Name) && RemoveInstancesNear(Component, Site, 1700.0f))
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

    AActor* Model = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
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

    UMaterialInstanceDynamic* Brick = MakeColorMaterial(Model, Basic, TEXT("R140Silpo_BrickMat"),
        FLinearColor(0.52f, 0.43f, 0.31f, 1.0f));
    UMaterialInstanceDynamic* BrickDark = MakeColorMaterial(Model, Basic, TEXT("R140Silpo_BrickDarkMat"),
        FLinearColor(0.34f, 0.27f, 0.20f, 1.0f));
    UMaterialInstanceDynamic* Concrete = MakeColorMaterial(Model, Basic, TEXT("R140Silpo_ConcreteMat"),
        FLinearColor(0.40f, 0.41f, 0.39f, 1.0f));
    UMaterialInstanceDynamic* RoofMat = MakeColorMaterial(Model, Basic, TEXT("R140Silpo_RoofMat"),
        FLinearColor(0.23f, 0.25f, 0.25f, 1.0f));
    UMaterialInstanceDynamic* WindowMat = MakeColorMaterial(Model, Basic, TEXT("R140Silpo_WindowMat"),
        FLinearColor(0.16f, 0.24f, 0.27f, 1.0f));
    UMaterialInstanceDynamic* FrameMat = MakeColorMaterial(Model, Basic, TEXT("R140Silpo_FrameMat"),
        FLinearColor(0.78f, 0.78f, 0.73f, 1.0f));
    UMaterialInstanceDynamic* SignMat = MakeColorMaterial(Model, Basic, TEXT("R140Silpo_SignMat"),
        FLinearColor(0.34f, 0.08f, 0.15f, 1.0f));
    UMaterialInstanceDynamic* ShelfMat = MakeColorMaterial(Model, Basic, TEXT("R140Silpo_ShelfMat"),
        FLinearColor(0.49f, 0.50f, 0.48f, 1.0f));
    UMaterialInstanceDynamic* CheckoutMat = MakeColorMaterial(Model, Basic, TEXT("R140Silpo_CheckoutMat"),
        FLinearColor(0.29f, 0.30f, 0.28f, 1.0f));
    UMaterialInstanceDynamic* CoolerMat = MakeColorMaterial(Model, Basic, TEXT("R140Silpo_CoolerMat"),
        FLinearColor(0.63f, 0.66f, 0.65f, 1.0f));
    UMaterialInstanceDynamic* UtilityMat = MakeColorMaterial(Model, Basic, TEXT("R140Silpo_UtilityMat"),
        FLinearColor(0.19f, 0.20f, 0.19f, 1.0f));

    UInstancedStaticMeshComponent* Walls = MakeISM(Model, Root, Cube, Brick,
        TEXT("R140Silpo_Walls"), true, true);
    UInstancedStaticMeshComponent* WallDetail = MakeISM(Model, Root, Cube, BrickDark,
        TEXT("R140Silpo_BrickDetail"), false, true);
    UInstancedStaticMeshComponent* Floor = MakeISM(Model, Root, Cube, Concrete,
        TEXT("R140Silpo_FloorAndForecourt"), true, false);
    UInstancedStaticMeshComponent* Roof = MakeISM(Model, Root, Cube, RoofMat,
        TEXT("R140Silpo_Roof"), true, true);
    UInstancedStaticMeshComponent* Windows = MakeISM(Model, Root, Cube, WindowMat,
        TEXT("R140Silpo_Windows"), false, false);
    UInstancedStaticMeshComponent* Frames = MakeISM(Model, Root, Cube, FrameMat,
        TEXT("R140Silpo_Frames"), false, true);
    UInstancedStaticMeshComponent* Sign = MakeISM(Model, Root, Cube, SignMat,
        TEXT("R140Silpo_FacadeSign"), false, true);
    UInstancedStaticMeshComponent* Shelves = MakeISM(Model, Root, Cube, ShelfMat,
        TEXT("R140Silpo_EmptyShelves"), true, true);
    UInstancedStaticMeshComponent* Checkouts = MakeISM(Model, Root, Cube, CheckoutMat,
        TEXT("R140Silpo_Checkouts"), true, true);
    UInstancedStaticMeshComponent* Coolers = MakeISM(Model, Root, Cube, CoolerMat,
        TEXT("R140Silpo_WallCoolers"), true, true);
    UInstancedStaticMeshComponent* Utilities = MakeISM(Model, Root, Cube, UtilityMat,
        TEXT("R140Silpo_Utilities"), true, true);

    // Enterable shell. The geocoder point is treated as the building center until a survey-grade footprint is available.
    AddLocalBox(Floor, Site, FVector(0.0f, 0.0f, 8.0f), FVector(BuildingLengthCm, BuildingDepthCm, 16.0f));
    AddLocalBox(Roof, Site, FVector(0.0f, -430.0f, 458.0f), FVector(3120.0f, 1020.0f, 20.0f),
        FRotator(0.0f, 0.0f, -8.0f));
    AddLocalBox(Roof, Site, FVector(0.0f, 430.0f, 458.0f), FVector(3120.0f, 1020.0f, 20.0f),
        FRotator(0.0f, 0.0f, 8.0f));

    const float HalfLength = BuildingLengthCm * 0.5f;
    const float HalfDepth = BuildingDepthCm * 0.5f;
    const float WallZ = WallHeightCm * 0.5f;

    AddLocalBox(Walls, Site, FVector(0.0f, HalfDepth, WallZ),
        FVector(BuildingLengthCm, WallThicknessCm, WallHeightCm));
    AddLocalBox(Walls, Site, FVector(-HalfLength, 0.0f, WallZ),
        FVector(WallThicknessCm, BuildingDepthCm, WallHeightCm));
    AddLocalBox(Walls, Site, FVector(HalfLength, 0.0f, WallZ),
        FVector(WallThicknessCm, BuildingDepthCm, WallHeightCm));

    // Front wall is split around a 3 m entrance opening so the player can physically enter the store.
    AddLocalBox(Walls, Site, FVector(-825.0f, -HalfDepth, WallZ),
        FVector(1350.0f, WallThicknessCm, WallHeightCm));
    AddLocalBox(Walls, Site, FVector(825.0f, -HalfDepth, WallZ),
        FVector(1350.0f, WallThicknessCm, WallHeightCm));
    AddLocalBox(Walls, Site, FVector(0.0f, -HalfDepth, 382.0f),
        FVector(300.0f, WallThicknessCm, 96.0f));

    // Modest brick rhythm/cornice from the photographed older-town supermarket exterior.
    AddLocalBox(WallDetail, Site, FVector(0.0f, -HalfDepth - 16.0f, 400.0f),
        FVector(BuildingLengthCm, 18.0f, 38.0f));
    AddLocalBox(WallDetail, Site, FVector(0.0f, HalfDepth + 16.0f, 400.0f),
        FVector(BuildingLengthCm, 18.0f, 38.0f));

    // Front facade windows. They remain uncut facade details in this first mesh pass; entrance opening is real geometry.
    for (const float X : { -1180.0f, -760.0f, 760.0f, 1180.0f })
    {
        AddLocalBox(Frames, Site, FVector(X, -HalfDepth - 20.0f, 225.0f), FVector(300.0f, 18.0f, 215.0f));
        AddLocalBox(Windows, Site, FVector(X, -HalfDepth - 31.0f, 225.0f), FVector(260.0f, 8.0f, 175.0f));
    }

    // Side/rear barred-window rhythm seen in the reference set.
    for (const float Y : { -500.0f, 0.0f, 500.0f })
    {
        for (const float XSide : { -HalfLength - 20.0f, HalfLength + 20.0f })
        {
            AddLocalBox(Frames, Site, FVector(XSide, Y, 220.0f), FVector(18.0f, 270.0f, 210.0f));
            AddLocalBox(Windows, Site, FVector(XSide + (XSide < 0.0f ? -11.0f : 11.0f), Y, 220.0f),
                FVector(8.0f, 235.0f, 172.0f));
            for (int32 Bar = -1; Bar <= 1; ++Bar)
            {
                AddLocalBox(Frames, Site, FVector(XSide + (XSide < 0.0f ? -18.0f : 18.0f),
                    Y + static_cast<float>(Bar) * 70.0f, 220.0f), FVector(5.0f, 5.0f, 168.0f));
            }
        }
    }
    for (const float X : { -1100.0f, -550.0f, 550.0f, 1100.0f })
    {
        AddLocalBox(Frames, Site, FVector(X, HalfDepth + 20.0f, 220.0f), FVector(270.0f, 18.0f, 210.0f));
        AddLocalBox(Windows, Site, FVector(X, HalfDepth + 31.0f, 220.0f), FVector(235.0f, 8.0f, 172.0f));
    }

    // Front canopy, threshold and broad paved forecourt toward the street.
    AddLocalBox(Roof, Site, FVector(0.0f, -HalfDepth - 165.0f, 335.0f), FVector(560.0f, 360.0f, 18.0f));
    AddLocalBox(Floor, Site, FVector(0.0f, -HalfDepth - 145.0f, 10.0f), FVector(620.0f, 310.0f, 20.0f));
    AddLocalBox(Floor, Site, FVector(0.0f, -HalfDepth - 610.0f, 6.0f), FVector(3500.0f, 900.0f, 12.0f));

    // Maroon facade sign. Text is separate so the geometry remains readable even if the font fallback changes.
    AddLocalBox(Sign, Site, FVector(500.0f, -HalfDepth - 32.0f, 354.0f), FVector(760.0f, 20.0f, 118.0f));
    UTextRenderComponent* SilpoText = NewObject<UTextRenderComponent>(Model, TEXT("R140Silpo_FacadeText"));
    if (SilpoText)
    {
        SilpoText->SetupAttachment(Root);
        SilpoText->SetText(FText::FromString(TEXT("СІЛЬПО")));
        SilpoText->SetTextRenderColor(FColor(236, 226, 220));
        SilpoText->SetHorizontalAlignment(EHTA_Center);
        SilpoText->SetWorldSize(70.0f);
        SilpoText->SetRelativeLocation(At(Site, FVector(500.0f, -HalfDepth - 45.0f, 352.0f)));
        SilpoText->SetRelativeRotation(FRotator(0.0f, 90.0f + SilpoYawDegrees, 0.0f));
        SilpoText->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Model->AddInstanceComponent(SilpoText);
        SilpoText->RegisterComponent();
    }

    // Sparse phase-one interior: ordinary empty shelves, four checkout lanes and wall refrigeration shells.
    for (const float X : { -950.0f, -570.0f, -190.0f, 190.0f, 570.0f, 950.0f })
    {
        AddShelfUnit(Shelves, Site, FVector(X, 80.0f, 0.0f), 105.0f, 820.0f, 185.0f);
    }
    for (int32 Lane = 0; Lane < 4; ++Lane)
    {
        const float X = 420.0f + static_cast<float>(Lane) * 275.0f;
        AddLocalBox(Checkouts, Site, FVector(X, -610.0f, 48.0f), FVector(220.0f, 72.0f, 96.0f));
        AddLocalBox(Checkouts, Site, FVector(X - 55.0f, -610.0f, 104.0f), FVector(95.0f, 92.0f, 16.0f));
    }
    for (const float X : { -1100.0f, -650.0f, -200.0f, 250.0f, 700.0f, 1150.0f })
    {
        AddLocalBox(Coolers, Site, FVector(X, HalfDepth - 65.0f, 120.0f), FVector(390.0f, 110.0f, 240.0f));
    }

    // Basic back-of-house markers and a service door shell, deliberately not a stocked warehouse yet.
    AddLocalBox(Utilities, Site, FVector(-1220.0f, 645.0f, 120.0f), FVector(260.0f, 210.0f, 240.0f));
    AddLocalBox(Utilities, Site, FVector(1220.0f, 650.0f, 105.0f), FVector(220.0f, 180.0f, 210.0f));
    AddLocalBox(Utilities, Site, FVector(-1180.0f, HalfDepth + 18.0f, 118.0f), FVector(150.0f, 18.0f, 236.0f));

    // Neutral supermarket lighting, intentionally plain. Product-specific/brand-specific lighting is a later pass.
    for (int32 Row = -1; Row <= 1; ++Row)
    {
        for (int32 Col = -2; Col <= 2; ++Col)
        {
            UPointLightComponent* Light = NewObject<UPointLightComponent>(Model,
                FName(*FString::Printf(TEXT("R140Silpo_CeilingLight_%d_%d"), Row, Col)));
            if (!Light) continue;
            Light->SetupAttachment(Root);
            Light->SetMobility(EComponentMobility::Static);
            Light->SetIntensity(2100.0f);
            Light->SetAttenuationRadius(900.0f);
            Light->SetLightColor(FLinearColor(0.95f, 0.94f, 0.88f));
            Light->SetCastShadows(false);
            Light->SetRelativeLocation(At(Site, FVector(static_cast<float>(Col) * 520.0f,
                static_cast<float>(Row) * 430.0f, 355.0f)));
            Model->AddInstanceComponent(Light);
            Light->RegisterComponent();
        }
    }

    // Two independently interactive entrance leaves. Only the authoritative world spawns replicated doors.
    if (World.GetNetMode() != NM_Client)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        const FRotator DoorRotation(0.0f, SilpoYawDegrees, 0.0f);
        const FVector LeftHinge = At(Site, FVector(-135.0f, -HalfDepth - 8.0f, 14.0f));
        const FVector RightHinge = At(Site, FVector(5.0f, -HalfDepth - 8.0f, 14.0f));

        if (AOCInteractableDoor* LeftDoor = World.SpawnActor<AOCInteractableDoor>(AOCInteractableDoor::StaticClass(),
            LeftHinge, DoorRotation, SpawnParams))
        {
            LeftDoor->Tags.Add(TEXT("R140_SilpoEntranceDoor"));
            LeftDoor->Tags.Add(TEXT("SilpoEntranceLeft"));
        }
        if (AOCInteractableDoor* RightDoor = World.SpawnActor<AOCInteractableDoor>(AOCInteractableDoor::StaticClass(),
            RightHinge, DoorRotation, SpawnParams))
        {
            RightDoor->Tags.Add(TEXT("R140_SilpoEntranceDoor"));
            RightDoor->Tags.Add(TEXT("SilpoEntranceRight"));
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("R14.0 Silpo photo model built at WGS84 %.9f, %.9f -> local [%.0f %.0f], enterable shell + sparse interior."),
        FOCGeoReference::Silpo().Latitude, FOCGeoReference::Silpo().Longitude, Site.X, Site.Y);
}
