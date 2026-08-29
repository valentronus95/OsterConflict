#include "OCR137MuseumPhotoModelSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    constexpr float MuseumPhotoModelDelaySeconds = 0.75f;
    constexpr float SourceMuseumCleanupRadiusCm = 5000.0f;

    UInstancedStaticMeshComponent* MakeAuthoredISM(AActor* Owner, USceneComponent* Root, UStaticMesh* Mesh,
        UMaterialInterface* AuthoredMaterial, const TCHAR* RequestedName, const bool bCollision,
        const bool bCastShadow = true)
    {
        if (!Owner || !Root || !Mesh) return nullptr;
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(
            Owner, MakeUniqueObjectName(Owner, UInstancedStaticMeshComponent::StaticClass(), FName(RequestedName)));
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        if (AuthoredMaterial)
        {
            const int32 SlotCount = FMath::Max(1, Mesh->GetStaticMaterials().Num());
            for (int32 Slot = 0; Slot < SlotCount; ++Slot) Component->SetMaterial(Slot, AuthoredMaterial);
        }
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionProfileName(FName(bCollision ? TEXT("BlockAll") : TEXT("NoCollision")));
        Component->SetCollisionEnabled(bCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(bCollision);
        Component->SetCastShadow(bCastShadow);
        Component->SetCullDistances(0, 100000);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    bool AddFittedAuthoredMesh(UInstancedStaticMeshComponent* Component, const FVector& Center,
        const FVector& DesiredSizeCm, const FRotator& Rotation = FRotator::ZeroRotator)
    {
        if (!Component || !Component->GetStaticMesh()) return false;
        UStaticMesh* Mesh = Component->GetStaticMesh();
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        if (NativeSize.X <= 1.0f || NativeSize.Y <= 1.0f || NativeSize.Z <= 1.0f) return false;
        const FVector Scale(DesiredSizeCm.X / NativeSize.X, DesiredSizeCm.Y / NativeSize.Y,
            DesiredSizeCm.Z / NativeSize.Z);
        const FQuat Quat = Rotation.Quaternion();
        const FVector Location = Center - Quat.RotateVector(Bounds.Origin * Scale);
        return Component->AddInstance(FTransform(Quat, Location, Scale), false) != INDEX_NONE;
    }

    void AddAuthoredFrame(UInstancedStaticMeshComponent* Frame, const FVector& Center,
        const FVector& HorizontalDirection, const float WidthCm, const float HeightCm)
    {
        if (!Frame || !Frame->GetStaticMesh()) return;
        const FBoxSphereBounds Bounds = Frame->GetStaticMesh()->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        const float NativeLengths[3] = { NativeSize.X, NativeSize.Y, NativeSize.Z };
        int32 NativeLongestAxis = 0;
        for (int32 Axis = 1; Axis < 3; ++Axis)
            if (NativeLengths[Axis] > NativeLengths[NativeLongestAxis]) NativeLongestAxis = Axis;
        if (NativeLengths[NativeLongestAxis] <= 1.0f) return;

        const FVector UnitAxes[3] = { FVector::ForwardVector, FVector::RightVector, FVector::UpVector };
        auto AddSegment = [&](const FVector& SegmentCenter, const FVector& TargetDirection, const float LengthCm)
        {
            const FQuat Rotation = FQuat::FindBetweenNormals(UnitAxes[NativeLongestAxis], TargetDirection.GetSafeNormal());
            const float UniformScale = LengthCm / NativeLengths[NativeLongestAxis];
            const FVector Location = SegmentCenter - Rotation.RotateVector(Bounds.Origin * UniformScale);
            Frame->AddInstance(FTransform(Rotation, Location, FVector(UniformScale)), false);
        };

        const FVector Horizontal = HorizontalDirection.GetSafeNormal();
        AddSegment(Center + FVector::UpVector * HeightCm * 0.5f, Horizontal, WidthCm);
        AddSegment(Center - FVector::UpVector * HeightCm * 0.5f, Horizontal, WidthCm);
        AddSegment(Center - Horizontal * WidthCm * 0.5f, FVector::UpVector, HeightCm);
        AddSegment(Center + Horizontal * WidthCm * 0.5f, FVector::UpVector, HeightCm);
    }

    bool RemoveInstancesNear(UInstancedStaticMeshComponent* Component, const FVector& Center, const float RadiusCm)
    {
        if (!Component) return false;
        bool bChanged = false;
        const float RadiusSq = FMath::Square(RadiusCm);
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

    bool IsLegacyMuseumComponent(const FName Name)
    {
        return Name.ToString().StartsWith(TEXT("R13_Museum"));
    }

    bool IsSourceMuseumFamily(const FName Name)
    {
        return Name == TEXT("LandmarkBlocks") || Name == TEXT("LandmarkRoofs") ||
            Name == TEXT("LandmarkWindows") || Name == TEXT("LandmarkDetails");
    }
}

bool UOCR137MuseumPhotoModelSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR137MuseumPhotoModelSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
        if (GameMode->IsFrontendOnlySession()) return;

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) ReplaceMuseum(*World);
        }), MuseumPhotoModelDelaySeconds, false);
    UE_LOG(LogTemp, Display, TEXT("PASS42_MUSEUM_EXTERIOR_EARLY_SCHEDULED delay=%.2f"), MuseumPhotoModelDelaySeconds);
}

void UOCR137MuseumPhotoModelSubsystem::ReplaceMuseum(UWorld& World)
{
    SuppressLegacyMuseum(World);
    BuildMuseum(World);
}

void UOCR137MuseumPhotoModelSubsystem::SuppressLegacyMuseum(UWorld& World)
{
    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();
    int32 HiddenComponents = 0;
    int32 TrimmedSourceComponents = 0;
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
            if (IsLegacyMuseumComponent(Name))
            {
                Component->SetVisibility(false, true);
                Component->SetHiddenInGame(true, true);
                Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                ++HiddenComponents;
            }
            else if (IsSourceMuseumFamily(Name) && RemoveInstancesNear(Component, Museum, SourceMuseumCleanupRadiusCm))
            {
                ++TrimmedSourceComponents;
            }
        }
    }
    UE_LOG(LogTemp, Display,
        TEXT("R13.7 museum model: legacy museum components hidden=%d, source landmark families trimmed=%d."),
        HiddenComponents, TrimmedSourceComponents);
}

void UOCR137MuseumPhotoModelSubsystem::BuildMuseum(UWorld& World)
{
    for (TActorIterator<AActor> It(&World); It; ++It)
        if (AActor* Existing = *It; Existing && Existing->ActorHasTag(TEXT("R137_MuseumPhotoModel"))) return;

    UStaticMesh* Wall8 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Wall_8m.Wall_8m"));
    UStaticMesh* WindowWall4 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Wall_Window_4m.Wall_Window_4m"));
    UStaticMesh* DoorWindowWall8 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Wall_Door_Windows_8m.Wall_Door_Windows_8m"));
    UStaticMesh* WallTop4 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Wall_Top_4m.Wall_Top_4m"));
    UStaticMesh* RoofMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Roof_Both_Ends_4m.Roof_Both_Ends_4m"));
    UStaticMesh* BottomExtender = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Bottom_Extender_4m.Bottom_Extender_4m"));
    UStaticMesh* Porch = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Porch_4x4m.Porch_4x4m"));
    UStaticMesh* WindowFramePart = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Window_Frame_Part.Window_Frame_Part"));
    UMaterialInterface* MetalRoof = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Metal_Roof.Metal_Roof"));
    UMaterialInterface* BlueWood = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Wood_Planks_Painted_Blue.Wood_Planks_Painted_Blue"));

    // Pass45 item 32 source slice. These are committed authored assets, not Engine BasicShape stand-ins.
    // The supplied photo pack proves a pale straight pedestrian axis and a mature conifer corridor, but it
    // does not prove exact paving module dimensions or exact tree species/coordinates. Keep those claims provisional.
    UStaticMesh* ApproachSlabMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/KiteDemo/LevelContent/Architecture/SM_1Meter_01.SM_1Meter_01"));
    UMaterialInterface* ApproachConcrete = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Materials/Instances/M_Concrete_2_Inst.M_Concrete_2_Inst"));
    UStaticMesh* ConiferMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/KiteDemo/Environments/Trees/ScotsPineTall_01/ScotsPineTall_01.ScotsPineTall_01"));

    if (!Wall8 || !WindowWall4 || !DoorWindowWall8 || !WallTop4 || !RoofMesh ||
        !BottomExtender || !Porch || !WindowFramePart)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_MUSEUM_AUTHORED_SHELL_FAIL reason=missing_committed_modular_asset basicshape_fallback=0 runtime_acceptance=0"));
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = MakeUniqueObjectName(&World, AActor::StaticClass(), TEXT("R137_OsterMuseumSolonyna"));
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AActor* Model = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, SpawnParams);
    if (!Model) return;
    Model->SetReplicates(false);
    Model->SetActorEnableCollision(true);
    Model->Tags.Add(TEXT("R137_MuseumPhotoModel"));
    Model->Tags.Add(TEXT("MuseumSolonyna_ReferenceExterior"));

    USceneComponent* Root = NewObject<USceneComponent>(Model,
        MakeUniqueObjectName(Model, USceneComponent::StaticClass(), TEXT("R137_MuseumPhotoModelRoot")));
    if (!Root) { Model->Destroy(); return; }
    Root->SetMobility(EComponentMobility::Static);
    Model->SetRootComponent(Root);
    Model->AddInstanceComponent(Root);
    Root->RegisterComponent();
    Model->SetActorLocation(AOCWorldSectorOster::MuseumAnchor());

    UInstancedStaticMeshComponent* BrickBody = MakeAuthoredISM(Model, Root, Wall8, nullptr,
        TEXT("R137Museum_BrickBody"), true);
    UInstancedStaticMeshComponent* WindowWalls = MakeAuthoredISM(Model, Root, WindowWall4, nullptr,
        TEXT("R137Museum_AuthoredWindowWalls"), true);
    UInstancedStaticMeshComponent* Entrance = MakeAuthoredISM(Model, Root, DoorWindowWall8, BlueWood,
        TEXT("R137Museum_AuthoredEntrance"), true);
    UInstancedStaticMeshComponent* Wood = MakeAuthoredISM(Model, Root, WindowWall4, BlueWood,
        TEXT("R137Museum_BlueGreyTimber"), true);
    UInstancedStaticMeshComponent* Roof = MakeAuthoredISM(Model, Root, RoofMesh, MetalRoof,
        TEXT("R137Museum_SheetMetalRoof"), false);
    UInstancedStaticMeshComponent* TopTrim = MakeAuthoredISM(Model, Root, WallTop4, nullptr,
        TEXT("R137Museum_AuthoredTopTrim"), false);
    UInstancedStaticMeshComponent* Frames = MakeAuthoredISM(Model, Root, WindowFramePart, nullptr,
        TEXT("R137Museum_AuthoredWindowFrames"), false);
    UInstancedStaticMeshComponent* Foundation = MakeAuthoredISM(Model, Root, BottomExtender, nullptr,
        TEXT("R137Museum_AuthoredFoundation"), true);
    UInstancedStaticMeshComponent* Steps = MakeAuthoredISM(Model, Root, Porch, nullptr,
        TEXT("R137Museum_AuthoredEntrancePorch"), true, false);
    UInstancedStaticMeshComponent* Annex = MakeAuthoredISM(Model, Root, Wall8, nullptr,
        TEXT("R137Museum_AuthoredRearAnnex"), true);
    if (!BrickBody || !WindowWalls || !Entrance || !Wood || !Roof || !TopTrim || !Frames ||
        !Foundation || !Steps || !Annex)
    {
        Model->Destroy();
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_MUSEUM_AUTHORED_SHELL_FAIL reason=component_creation basicshape_fallback=0 runtime_acceptance=0"));
        return;
    }

    UInstancedStaticMeshComponent* Approach = nullptr;
    UInstancedStaticMeshComponent* Conifers = nullptr;
    if (ApproachSlabMesh && ApproachConcrete && ConiferMesh)
    {
        Approach = MakeAuthoredISM(Model, Root, ApproachSlabMesh, ApproachConcrete,
            TEXT("R137Museum_AuthoredConcreteApproach"), true, false);
        Conifers = MakeAuthoredISM(Model, Root, ConiferMesh, nullptr,
            TEXT("R137Museum_ReferenceConiferCorridor"), false, true);
    }
    if (!Approach || !Conifers)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_MUSEUM_SITE_REFERENCE_SLICE_FAIL reason=missing_or_unusable_authored_site_asset basicshape_fallback=0 runtime_visual_acceptance=pending"));
    }

    const float FrontXs[] = { -660.0f, -220.0f, 220.0f, 660.0f };
    for (const float X : FrontXs)
    {
        AddFittedAuthoredMesh(WindowWalls, FVector(X, -420.0f, 230.0f), FVector(440.0f, 52.0f, 320.0f));
        AddFittedAuthoredMesh(BrickBody, FVector(X, 420.0f, 230.0f), FVector(440.0f, 52.0f, 320.0f), FRotator(0,180,0));
        AddFittedAuthoredMesh(Foundation, FVector(X, -420.0f, 40.0f), FVector(440.0f, 95.0f, 80.0f));
        AddFittedAuthoredMesh(Foundation, FVector(X, 420.0f, 40.0f), FVector(440.0f, 95.0f, 80.0f), FRotator(0,180,0));
        AddFittedAuthoredMesh(TopTrim, FVector(X, -445.0f, 390.0f), FVector(440.0f, 70.0f, 95.0f));
        AddFittedAuthoredMesh(TopTrim, FVector(X, 445.0f, 390.0f), FVector(440.0f, 70.0f, 95.0f), FRotator(0,180,0));
    }
    for (const float Y : { -280.0f, 0.0f, 280.0f })
    {
        AddFittedAuthoredMesh(WindowWalls, FVector(-850.0f, Y, 230.0f), FVector(280.0f, 52.0f, 320.0f), FRotator(0,-90,0));
        AddFittedAuthoredMesh(WindowWalls, FVector(850.0f, Y, 230.0f), FVector(280.0f, 52.0f, 320.0f), FRotator(0,90,0));
    }

    AddFittedAuthoredMesh(Roof, FVector(0,0,505), FVector(1840,1010,270));
    AddFittedAuthoredMesh(Wood, FVector(0,-35,520), FVector(570,470,250));
    AddFittedAuthoredMesh(Roof, FVector(0,-35,690), FVector(660,570,190));
    AddFittedAuthoredMesh(Entrance, FVector(0,-545,220), FVector(520,250,300));
    AddFittedAuthoredMesh(Roof, FVector(0,-545,415), FVector(610,340,145));
    AddFittedAuthoredMesh(Wood, FVector(-975,125,215), FVector(250,560,290), FRotator(0,90,0));
    AddFittedAuthoredMesh(Roof, FVector(-975,125,405), FVector(340,650,145), FRotator(0,90,0));
    AddFittedAuthoredMesh(Annex, FVector(1020,235,160), FVector(430,470,250), FRotator(0,90,0));
    AddFittedAuthoredMesh(Roof, FVector(1020,235,335), FVector(500,560,130), FRotator(0,90,0));

    for (const float Y : { -760.0f, -1080.0f })
        AddFittedAuthoredMesh(Steps, FVector(0,Y,18), FVector(520,320,36));

    for (const float X : FrontXs)
    {
        AddAuthoredFrame(Frames, FVector(X,-450,235), FVector::ForwardVector, 150,205);
        AddAuthoredFrame(Frames, FVector(X,450,235), FVector::ForwardVector, 150,205);
    }
    for (const float Y : { -270.0f, 20.0f, 300.0f })
        AddAuthoredFrame(Frames, FVector(880,Y,235), FVector::RightVector, 140,205);
    for (const float X : { -190.0f, 0.0f, 190.0f })
        AddAuthoredFrame(Frames, FVector(X,-285,520), FVector::ForwardVector, 115,165);

    if (Approach && Conifers)
    {
        // Reference-backed macro composition only. The path stays narrow, straight and entrance-aligned.
        // Six modular sections make joints readable without pretending the source pack gives survey dimensions.
        for (const float Y : { -1320.0f, -1720.0f, -2120.0f, -2520.0f, -2920.0f, -3320.0f })
            AddFittedAuthoredMesh(Approach, FVector(0.0f, Y, 4.0f), FVector(220.0f, 390.0f, 8.0f));

        struct FConiferPlacement
        {
            float X;
            float Y;
            float Width;
            float Height;
            float Yaw;
        };
        const FConiferPlacement Corridor[] = {
            { -690.0f, -1420.0f, 500.0f, 1650.0f, -9.0f }, { 730.0f, -1490.0f, 530.0f, 1720.0f, 13.0f },
            { -760.0f, -1990.0f, 560.0f, 1810.0f, 7.0f },  { 810.0f, -2070.0f, 520.0f, 1690.0f, -12.0f },
            { -720.0f, -2580.0f, 510.0f, 1740.0f, -4.0f }, { 780.0f, -2660.0f, 570.0f, 1840.0f, 10.0f },
            { -800.0f, -3160.0f, 550.0f, 1790.0f, 12.0f }, { 710.0f, -3240.0f, 500.0f, 1680.0f, -8.0f },
        };
        for (const FConiferPlacement& Placement : Corridor)
        {
            AddFittedAuthoredMesh(Conifers, FVector(Placement.X, Placement.Y, Placement.Height * 0.5f),
                FVector(Placement.Width, Placement.Width, Placement.Height), FRotator(0.0f, Placement.Yaw, 0.0f));
        }

        UE_LOG(LogTemp, Display,
            TEXT("PASS45_MUSEUM_SITE_REFERENCE_SLICE_READY owner=R137 path=straight_concrete_pedestrian slabs=6 conifer_corridor=8 exact_tree_species_claim=0 exact_site_coordinates_claim=0 basicshape=0 stadium_owner_duplicated=0 runtime_visual_acceptance=pending"));
    }

    // Pass45 single-visible-owner compatibility contract: breakable actor owns visible glass.
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_MUSEUM_SINGLE_VISIBLE_OWNER_READY visible_shell_owner=R137 static_glass=0 prototype_doors=0 prototype_trees=0 prototype_service_gable=0 breakable actor owns visible glass"));

    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_MUSEUM_AUTHORED_SHELL_READY owner=R137_MuseumPhotoModel x=%.1f y=%.1f authored_wall=1 authored_window_wall=1 authored_roof=1 authored_foundation=1 authored_porch=1 basicshape_structural=0 basicshape_material=0 museum_columns=0 runtime_visual_acceptance=pending"),
        Museum.X, Museum.Y);
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_MUSEUM_R137_PRIMARY_EXTERIOR_READY owner=R137_MuseumPhotoModel anchor=%s canonical_reference=Solonyna_colonel_house source_authored_shell=1 runtime_photo_acceptance=0"),
        *Museum.ToCompactString());
}
