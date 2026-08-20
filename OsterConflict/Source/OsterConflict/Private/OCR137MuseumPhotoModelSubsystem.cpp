#include "OCR137MuseumPhotoModelSubsystem.h"

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

namespace
{
    constexpr float MuseumPhotoModelDelaySeconds = 5.10f;
    constexpr float SourceMuseumCleanupRadiusCm = 3600.0f;
    const FName FinalMuseumTag(TEXT("R137_MuseumPhotoModel"));

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
        const int32 CullEndCm = 100000)
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

    void AddFittedMesh(UInstancedStaticMeshComponent* Component, UStaticMesh* Mesh,
        const FVector& Center, const FVector& DesiredSizeCm, const float YawDegrees = 0.0f)
    {
        if (!Component || !Mesh) return;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        if (NativeSize.X <= 1.0f || NativeSize.Y <= 1.0f || NativeSize.Z <= 1.0f) return;

        const FVector Scale(
            DesiredSizeCm.X / NativeSize.X,
            DesiredSizeCm.Y / NativeSize.Y,
            DesiredSizeCm.Z / NativeSize.Z);
        const FQuat Rotation = FRotator(0.0f, YawDegrees, 0.0f).Quaternion();
        const FVector Location = Center - Rotation.RotateVector(Bounds.Origin * Scale);
        Component->AddInstance(FTransform(Rotation, Location, Scale), true);
    }

    void AddFrontWindow(UInstancedStaticMeshComponent* Trim, UInstancedStaticMeshComponent* Glass,
        UInstancedStaticMeshComponent* Grille, const FVector& Origin, const float X, const float Y,
        const float Z, const float Width, const float Height, const bool bRear = false)
    {
        const float Depth = 14.0f;
        AddBox(Trim, Origin + FVector(X - Width * 0.5f - 7.0f, Y, Z), FVector(14.0f, Depth, Height + 24.0f));
        AddBox(Trim, Origin + FVector(X + Width * 0.5f + 7.0f, Y, Z), FVector(14.0f, Depth, Height + 24.0f));
        AddBox(Trim, Origin + FVector(X, Y, Z + Height * 0.5f + 7.0f), FVector(Width + 28.0f, Depth, 14.0f));
        AddBox(Trim, Origin + FVector(X, Y, Z - Height * 0.5f - 7.0f), FVector(Width + 28.0f, Depth, 14.0f));
        const float GlassOffset = bRear ? -4.0f : 4.0f;
        AddBox(Glass, Origin + FVector(X, Y + GlassOffset, Z), FVector(Width, 8.0f, Height));

        for (int32 Bar = -1; Bar <= 1; ++Bar)
        {
            AddBox(Grille, Origin + FVector(X + static_cast<float>(Bar) * Width * 0.25f,
                Y + (bRear ? -10.0f : 10.0f), Z), FVector(5.0f, 5.0f, Height - 14.0f));
        }
        AddBox(Grille, Origin + FVector(X, Y + (bRear ? -10.0f : 10.0f), Z),
            FVector(Width - 12.0f, 5.0f, 5.0f));
    }

    void AddSideWindow(UInstancedStaticMeshComponent* Trim, UInstancedStaticMeshComponent* Glass,
        UInstancedStaticMeshComponent* Grille, const FVector& Origin, const float X, const float Y,
        const float Z, const float Width, const float Height, const bool bLeft = false)
    {
        const float Depth = 14.0f;
        AddBox(Trim, Origin + FVector(X, Y - Width * 0.5f - 7.0f, Z), FVector(Depth, 14.0f, Height + 24.0f));
        AddBox(Trim, Origin + FVector(X, Y + Width * 0.5f + 7.0f, Z), FVector(Depth, 14.0f, Height + 24.0f));
        AddBox(Trim, Origin + FVector(X, Y, Z + Height * 0.5f + 7.0f), FVector(Depth, Width + 28.0f, 14.0f));
        AddBox(Trim, Origin + FVector(X, Y, Z - Height * 0.5f - 7.0f), FVector(Depth, Width + 28.0f, 14.0f));
        const float GlassOffset = bLeft ? 4.0f : -4.0f;
        AddBox(Glass, Origin + FVector(X + GlassOffset, Y, Z), FVector(8.0f, Width, Height));

        for (int32 Bar = -1; Bar <= 1; ++Bar)
        {
            AddBox(Grille, Origin + FVector(X + (bLeft ? 10.0f : -10.0f),
                Y + static_cast<float>(Bar) * Width * 0.25f, Z), FVector(5.0f, 5.0f, Height - 14.0f));
        }
        AddBox(Grille, Origin + FVector(X + (bLeft ? 10.0f : -10.0f), Y, Z),
            FVector(5.0f, Width - 12.0f, 5.0f));
    }

    void AddDoorLeafDetail(UInstancedStaticMeshComponent* Detail, const FVector& Origin, const float X, const float Y)
    {
        // Shallow raised rails make the photographed grey double doors read as paneled timber rather than two slabs.
        for (const float Z : { 125.0f, 205.0f, 285.0f })
        {
            AddBox(Detail, Origin + FVector(X, Y - 10.0f, Z), FVector(82.0f, 7.0f, 8.0f));
        }
        AddBox(Detail, Origin + FVector(X - 38.0f, Y - 10.0f, 205.0f), FVector(7.0f, 7.0f, 165.0f));
        AddBox(Detail, Origin + FVector(X + 38.0f, Y - 10.0f, 205.0f), FVector(7.0f, 7.0f, 165.0f));
    }

    void AddFrontSill(UInstancedStaticMeshComponent* Trim, const FVector& Origin,
        const float X, const float Y, const float Z, const float Width)
    {
        AddBox(Trim, Origin + FVector(X, Y - 4.0f, Z), FVector(Width + 34.0f, 32.0f, 10.0f));
    }

    void AddRailingRun(UInstancedStaticMeshComponent* Rails, const FVector& Origin, const float X)
    {
        // The real entrance uses light open metal railings, not solid rectangular barriers.
        for (int32 Post = 0; Post < 6; ++Post)
        {
            const float Alpha = static_cast<float>(Post) / 5.0f;
            const float Y = FMath::Lerp(-920.0f, -690.0f, Alpha);
            const float BaseZ = FMath::Lerp(65.0f, 150.0f, Alpha);
            AddBox(Rails, Origin + FVector(X, Y, BaseZ + 70.0f), FVector(8.0f, 8.0f, 140.0f));
        }
        AddBox(Rails, Origin + FVector(X, -805.0f, 205.0f), FVector(10.0f, 260.0f, 10.0f),
            FRotator(0.0f, 0.0f, 20.0f));

        for (int32 Panel = 0; Panel < 5; ++Panel)
        {
            const float Y = -895.0f + static_cast<float>(Panel) * 47.0f;
            const float Z = 105.0f + static_cast<float>(Panel) * 17.0f;
            AddBox(Rails, Origin + FVector(X, Y, Z), FVector(7.0f, 58.0f, 7.0f), FRotator(0.0f, 0.0f, 38.0f));
            AddBox(Rails, Origin + FVector(X, Y, Z), FVector(7.0f, 58.0f, 7.0f), FRotator(0.0f, 0.0f, -38.0f));
        }
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

    void AddGroundedTree(UInstancedStaticMeshComponent* Component, UStaticMesh* Mesh,
        const FVector& GroundLocation, const float DesiredHeightCm, const float YawDegrees)
    {
        if (!Component || !Mesh) return;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        if (NativeSize.Z <= 10.0f) return;
        const float Scale = FMath::Clamp(DesiredHeightCm / NativeSize.Z, 0.25f, 4.0f);
        const float LocalBottom = Bounds.Origin.Z - Bounds.BoxExtent.Z;
        FVector Location = GroundLocation;
        Location.Z = -LocalBottom * Scale;
        Component->AddInstance(FTransform(FRotator(0.0f, YawDegrees, 0.0f), Location, FVector(Scale)), true);
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
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) ReplaceMuseum(*World);
        }), MuseumPhotoModelDelaySeconds, false);
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
                continue;
            }
            if (IsSourceMuseumFamily(Name) && RemoveInstancesNear(Component, Museum, SourceMuseumCleanupRadiusCm))
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
    {
        AActor* Existing = *It;
        if (Existing && Existing->ActorHasTag(FinalMuseumTag)) return;
    }

    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UStaticMesh* RoofMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Roof_Both_Ends_4m.Roof_Both_Ends_4m"));
    UStaticMesh* Pine01 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_01.SM_Pine_Tree_01"));
    UStaticMesh* Pine03 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_03.SM_Pine_Tree_03"));
    UMaterialInterface* Basic = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    UMaterialInterface* MetalRoof = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Metal_Roof.Metal_Roof"));
    UMaterialInterface* BlueWoodAsset = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Wood_Planks_Painted_Blue.Wood_Planks_Painted_Blue"));
    if (!Cube || !Basic || !RoofMesh) return;

    AActor* Model = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!Model) return;
    Model->SetReplicates(false);
    Model->SetActorEnableCollision(true);
    Model->Tags.Add(FinalMuseumTag);

    USceneComponent* Root = NewObject<USceneComponent>(Model, TEXT("R137_MuseumPhotoModelRoot"));
    if (!Root)
    {
        Model->Destroy();
        return;
    }
    Root->SetMobility(EComponentMobility::Static);
    Model->SetRootComponent(Root);
    Model->AddInstanceComponent(Root);
    Root->RegisterComponent();

    UMaterialInstanceDynamic* Brick = MakeColorMaterial(Model, Basic, TEXT("R137_Brick"),
        FLinearColor(0.34f, 0.115f, 0.045f, 1.0f));
    UMaterialInstanceDynamic* BrickDark = MakeColorMaterial(Model, Basic, TEXT("R137_BrickDark"),
        FLinearColor(0.22f, 0.065f, 0.025f, 1.0f));
    UMaterialInstanceDynamic* PlinthMat = MakeColorMaterial(Model, Basic, TEXT("R137_Plinth"),
        FLinearColor(0.035f, 0.032f, 0.030f, 1.0f));
    UMaterialInstanceDynamic* PaleTrim = MakeColorMaterial(Model, Basic, TEXT("R137_PaleTrim"),
        FLinearColor(0.62f, 0.64f, 0.59f, 1.0f));
    UMaterialInstanceDynamic* BlueWoodFallback = MakeColorMaterial(Model, Basic, TEXT("R137_BlueWoodFallback"),
        FLinearColor(0.21f, 0.29f, 0.29f, 1.0f));
    UMaterialInstanceDynamic* RedGableWood = MakeColorMaterial(Model, Basic, TEXT("R137_RedGableWood"),
        FLinearColor(0.33f, 0.10f, 0.055f, 1.0f));
    UMaterialInstanceDynamic* DoorMat = MakeColorMaterial(Model, Basic, TEXT("R137_Door"),
        FLinearColor(0.25f, 0.28f, 0.29f, 1.0f));
    UMaterialInstanceDynamic* ConcreteMat = MakeColorMaterial(Model, Basic, TEXT("R137_Concrete"),
        FLinearColor(0.20f, 0.21f, 0.20f, 1.0f));
    UMaterialInstanceDynamic* RailMat = MakeColorMaterial(Model, Basic, TEXT("R137_Rail"),
        FLinearColor(0.24f, 0.28f, 0.28f, 1.0f));
    UMaterialInstanceDynamic* GrilleMat = MakeColorMaterial(Model, Basic, TEXT("R137_Grille"),
        FLinearColor(0.30f, 0.33f, 0.32f, 1.0f));
    UMaterialInstanceDynamic* GasMat = MakeColorMaterial(Model, Basic, TEXT("R137_Gas"),
        FLinearColor(0.62f, 0.46f, 0.015f, 1.0f));
    UMaterialInstanceDynamic* AnnexMat = MakeColorMaterial(Model, Basic, TEXT("R137_Annex"),
        FLinearColor(0.42f, 0.25f, 0.19f, 1.0f));
    UMaterialInstanceDynamic* GlassFallback = MakeColorMaterial(Model, Basic, TEXT("R137_GlassFallback"),
        FLinearColor(0.12f, 0.18f, 0.19f, 1.0f));
    UMaterialInstanceDynamic* PlaqueMat = MakeColorMaterial(Model, Basic, TEXT("R137_Plaque"),
        FLinearColor(0.075f, 0.070f, 0.060f, 1.0f));

    UMaterialInterface* BlueWood = BlueWoodAsset ? BlueWoodAsset : BlueWoodFallback;
    UMaterialInterface* GlassMaterial = GlassFallback;
    UMaterialInterface* RoofMaterial = MetalRoof ? MetalRoof : Basic;

    UInstancedStaticMeshComponent* Plinth = MakeISM(Model, Root, Cube, PlinthMat,
        TEXT("R137Museum_Plinth"), true, true);
    UInstancedStaticMeshComponent* BrickBody = MakeISM(Model, Root, Cube, Brick,
        TEXT("R137Museum_BrickBody"), true, true);
    UInstancedStaticMeshComponent* BrickDetail = MakeISM(Model, Root, Cube, BrickDark,
        TEXT("R137Museum_BrickCornice"), false, true);
    UInstancedStaticMeshComponent* Wood = MakeISM(Model, Root, Cube, BlueWood,
        TEXT("R137Museum_BlueGreyTimber"), true, true);
    UInstancedStaticMeshComponent* GableWood = MakeISM(Model, Root, Cube, RedGableWood,
        TEXT("R137Museum_RedTimberGable"), false, true);
    UInstancedStaticMeshComponent* Roof = MakeISM(Model, Root, RoofMesh, RoofMaterial,
        TEXT("R137Museum_SheetMetalRoof"), false, true);
    UInstancedStaticMeshComponent* Trim = MakeISM(Model, Root, Cube, PaleTrim,
        TEXT("R137Museum_CarvedPaleTrim"), false, true);
    UInstancedStaticMeshComponent* Glass = MakeISM(Model, Root, Cube, GlassMaterial,
        TEXT("R137Museum_WindowGlass"), false, false);
    UInstancedStaticMeshComponent* Grilles = MakeISM(Model, Root, Cube, GrilleMat,
        TEXT("R137Museum_WindowGrilles"), false, false, 45000);
    UInstancedStaticMeshComponent* Doors = MakeISM(Model, Root, Cube, DoorMat,
        TEXT("R137Museum_GreyDoors"), false, true);
    UInstancedStaticMeshComponent* DoorDetail = MakeISM(Model, Root, Cube, DoorMat,
        TEXT("R137Museum_DoorPanelRelief"), false, true, 45000);
    UInstancedStaticMeshComponent* Concrete = MakeISM(Model, Root, Cube, ConcreteMat,
        TEXT("R137Museum_StepsAndSlabs"), true, false);
    UInstancedStaticMeshComponent* Rails = MakeISM(Model, Root, Cube, RailMat,
        TEXT("R137Museum_MetalRailings"), false, true, 50000);
    UInstancedStaticMeshComponent* GasPipe = MakeISM(Model, Root, Cube, GasMat,
        TEXT("R137Museum_YellowGasPipe"), false, false, 50000);
    UInstancedStaticMeshComponent* Annex = MakeISM(Model, Root, Cube, AnnexMat,
        TEXT("R137Museum_RearAnnex"), true, true);
    UInstancedStaticMeshComponent* Plaques = MakeISM(Model, Root, Cube, PlaqueMat,
        TEXT("R137Museum_PlaquesAndVents"), false, true, 45000);

    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();

    // Photo-proportioned exterior, not survey geometry.
    AddBox(Plinth, Museum + FVector(0.0f, 0.0f, 35.0f), FVector(1760.0f, 900.0f, 70.0f));
    AddBox(BrickBody, Museum + FVector(0.0f, 0.0f, 230.0f), FVector(1700.0f, 840.0f, 320.0f));

    AddFittedMesh(Roof, RoofMesh, Museum + FVector(-595.0f, 0.0f, 500.0f),
        FVector(650.0f, 1010.0f, 250.0f));
    AddFittedMesh(Roof, RoofMesh, Museum + FVector(595.0f, 0.0f, 500.0f),
        FVector(650.0f, 1010.0f, 250.0f));

    AddBox(Wood, Museum + FVector(0.0f, -35.0f, 510.0f), FVector(570.0f, 470.0f, 250.0f));
    AddFittedMesh(Roof, RoofMesh, Museum + FVector(0.0f, -35.0f, 690.0f),
        FVector(660.0f, 570.0f, 190.0f));

    for (int32 Course = 0; Course < 6; ++Course)
    {
        const float WidthY = 700.0f - static_cast<float>(Course) * 95.0f;
        AddBox(GableWood, Museum + FVector(-858.0f, 0.0f, 422.0f + static_cast<float>(Course) * 34.0f),
            FVector(14.0f, WidthY, 30.0f));
    }

    AddBox(Wood, Museum + FVector(0.0f, -535.0f, 220.0f), FVector(520.0f, 250.0f, 300.0f));
    AddFittedMesh(Roof, RoofMesh, Museum + FVector(0.0f, -535.0f, 415.0f),
        FVector(610.0f, 340.0f, 145.0f));
    AddBox(Wood, Museum + FVector(-975.0f, 125.0f, 215.0f), FVector(250.0f, 560.0f, 290.0f));
    AddFittedMesh(Roof, RoofMesh, Museum + FVector(-975.0f, 125.0f, 405.0f),
        FVector(340.0f, 650.0f, 145.0f), 90.0f);

    AddBox(Annex, Museum + FVector(1020.0f, 235.0f, 160.0f), FVector(430.0f, 470.0f, 250.0f));
    AddFittedMesh(Roof, RoofMesh, Museum + FVector(1020.0f, 235.0f, 335.0f),
        FVector(500.0f, 560.0f, 130.0f), 90.0f);

    AddBox(BrickDetail, Museum + FVector(-560.0f, 150.0f, 625.0f), FVector(105.0f, 105.0f, 330.0f));
    AddBox(BrickDetail, Museum + FVector(-560.0f, 150.0f, 798.0f), FVector(125.0f, 125.0f, 22.0f));

    AddBox(BrickDetail, Museum + FVector(0.0f, -426.0f, 385.0f), FVector(1700.0f, 22.0f, 55.0f));
    AddBox(BrickDetail, Museum + FVector(0.0f, 426.0f, 385.0f), FVector(1700.0f, 22.0f, 55.0f));
    AddBox(BrickDetail, Museum + FVector(-856.0f, 0.0f, 385.0f), FVector(22.0f, 840.0f, 55.0f));
    AddBox(BrickDetail, Museum + FVector(856.0f, 0.0f, 385.0f), FVector(22.0f, 840.0f, 55.0f));

    for (int32 Index = -7; Index <= 7; ++Index)
    {
        const float X = static_cast<float>(Index) * 105.0f;
        AddBox(Trim, Museum + FVector(X, -439.0f, 370.0f), FVector(34.0f, 12.0f, 38.0f));
        AddBox(Trim, Museum + FVector(X, 439.0f, 370.0f), FVector(34.0f, 12.0f, 38.0f));
    }
    for (int32 Index = -3; Index <= 3; ++Index)
    {
        const float Y = static_cast<float>(Index) * 105.0f;
        AddBox(Trim, Museum + FVector(-869.0f, Y, 370.0f), FVector(12.0f, 34.0f, 38.0f));
        AddBox(Trim, Museum + FVector(869.0f, Y, 370.0f), FVector(12.0f, 34.0f, 38.0f));
    }

    const float FrontY = -426.0f;
    const float RearY = 426.0f;
    for (const float X : { -650.0f, -355.0f, 355.0f, 650.0f })
    {
        AddFrontWindow(Trim, Glass, Grilles, Museum, X, FrontY, 235.0f, 140.0f, 205.0f, false);
        AddFrontSill(Trim, Museum, X, -444.0f, 125.0f, 140.0f);
        AddBox(BrickDetail, Museum + FVector(X, -441.0f, 352.0f), FVector(190.0f, 18.0f, 22.0f));
    }
    for (const float X : { -650.0f, -330.0f, 0.0f, 330.0f, 650.0f })
        AddFrontWindow(Trim, Glass, Grilles, Museum, X, RearY, 235.0f, 140.0f, 205.0f, true);

    // The facade photos show pale relief blocks both below the eaves and near the lower brick band.
    for (const float X : { -790.0f, -515.0f, -205.0f, 205.0f, 515.0f, 790.0f })
    {
        AddBox(Trim, Museum + FVector(X, -442.0f, 92.0f), FVector(38.0f, 12.0f, 34.0f));
    }

    for (const float Y : { -270.0f, 20.0f, 300.0f })
        AddSideWindow(Trim, Glass, Grilles, Museum, 856.0f, Y, 235.0f, 135.0f, 205.0f, false);

    // Raised timber center: two side windows plus the taller central glazed opening visible on the frontal reference.
    AddFrontWindow(Trim, Glass, Grilles, Museum, -190.0f, -277.0f, 520.0f, 115.0f, 165.0f, false);
    AddFrontWindow(Trim, Glass, Grilles, Museum, 0.0f, -277.0f, 520.0f, 130.0f, 178.0f, false);
    AddFrontWindow(Trim, Glass, Grilles, Museum, 190.0f, -277.0f, 520.0f, 115.0f, 165.0f, false);
    AddSideWindow(Trim, Glass, Grilles, Museum, 292.0f, -115.0f, 520.0f, 110.0f, 165.0f, false);
    AddSideWindow(Trim, Glass, Grilles, Museum, 292.0f, 110.0f, 520.0f, 110.0f, 165.0f, false);

    // Carved timber gable. Add the main outline and the repeated diagonal braces seen around the upper windows.
    AddBox(Trim, Museum + FVector(0.0f, -287.0f, 635.0f), FVector(590.0f, 16.0f, 16.0f));
    AddBox(Trim, Museum + FVector(-145.0f, -289.0f, 707.0f), FVector(330.0f, 16.0f, 18.0f), FRotator(-25.0f, 0.0f, 0.0f));
    AddBox(Trim, Museum + FVector(145.0f, -289.0f, 707.0f), FVector(330.0f, 16.0f, 18.0f), FRotator(25.0f, 0.0f, 0.0f));
    AddBox(Trim, Museum + FVector(0.0f, -291.0f, 734.0f), FVector(15.0f, 12.0f, 105.0f));
    AddBox(Trim, Museum + FVector(-255.0f, -291.0f, 648.0f), FVector(150.0f, 12.0f, 13.0f), FRotator(-30.0f, 0.0f, 0.0f));
    AddBox(Trim, Museum + FVector(255.0f, -291.0f, 648.0f), FVector(150.0f, 12.0f, 13.0f), FRotator(30.0f, 0.0f, 0.0f));
    AddBox(Trim, Museum + FVector(-868.0f, -175.0f, 555.0f), FVector(16.0f, 390.0f, 18.0f), FRotator(0.0f, 0.0f, -27.0f));
    AddBox(Trim, Museum + FVector(-868.0f, 175.0f, 555.0f), FVector(16.0f, 390.0f, 18.0f), FRotator(0.0f, 0.0f, 27.0f));

    // Entrance vestibule facade: framed glazing, paneled double doors and a decorated low gable fascia.
    AddFrontWindow(Trim, Glass, Grilles, Museum, -190.0f, -664.0f, 225.0f, 125.0f, 205.0f, false);
    AddFrontWindow(Trim, Glass, Grilles, Museum, 190.0f, -664.0f, 225.0f, 125.0f, 205.0f, false);
    AddBox(Doors, Museum + FVector(-62.0f, -672.0f, 205.0f), FVector(116.0f, 16.0f, 270.0f));
    AddBox(Doors, Museum + FVector(62.0f, -672.0f, 205.0f), FVector(116.0f, 16.0f, 270.0f));
    AddDoorLeafDetail(DoorDetail, Museum, -62.0f, -682.0f);
    AddDoorLeafDetail(DoorDetail, Museum, 62.0f, -682.0f);
    AddBox(Plaques, Museum + FVector(-12.0f, -689.0f, 213.0f), FVector(8.0f, 8.0f, 18.0f));
    AddBox(Plaques, Museum + FVector(12.0f, -689.0f, 213.0f), FVector(8.0f, 8.0f, 18.0f));
    AddBox(Trim, Museum + FVector(0.0f, -710.0f, 414.0f), FVector(565.0f, 15.0f, 14.0f));
    AddBox(Trim, Museum + FVector(-142.0f, -712.0f, 468.0f), FVector(315.0f, 15.0f, 14.0f), FRotator(-21.0f, 0.0f, 0.0f));
    AddBox(Trim, Museum + FVector(142.0f, -712.0f, 468.0f), FVector(315.0f, 15.0f, 14.0f), FRotator(21.0f, 0.0f, 0.0f));

    for (int32 Step = 0; Step < 6; ++Step)
    {
        const float Width = 520.0f - static_cast<float>(Step) * 32.0f;
        const float Y = -875.0f + static_cast<float>(Step) * 47.0f;
        const float Height = 18.0f + static_cast<float>(Step) * 14.0f;
        AddBox(Concrete, Museum + FVector(0.0f, Y, Height * 0.5f), FVector(Width, 92.0f, Height));
    }
    AddBox(Concrete, Museum + FVector(-300.0f, -775.0f, 92.0f), FVector(70.0f, 330.0f, 184.0f));
    AddBox(Concrete, Museum + FVector(300.0f, -775.0f, 92.0f), FVector(70.0f, 330.0f, 184.0f));
    AddRailingRun(Rails, Museum, -245.0f);
    AddRailingRun(Rails, Museum, 245.0f);

    for (const float Y : { -55.0f, 85.0f, 225.0f, 365.0f })
        AddSideWindow(Trim, Glass, Grilles, Museum, -1105.0f, Y, 220.0f, 115.0f, 195.0f, true);

    AddBox(Doors, Museum + FVector(864.0f, -155.0f, 210.0f), FVector(16.0f, 145.0f, 275.0f));
    AddFittedMesh(Roof, RoofMesh, Museum + FVector(965.0f, -155.0f, 385.0f),
        FVector(250.0f, 290.0f, 105.0f), 90.0f);
    AddBox(Trim, Museum + FVector(870.0f, -155.0f, 365.0f), FVector(20.0f, 190.0f, 18.0f));

    AddBox(GasPipe, Museum + FVector(870.0f, -330.0f, 240.0f), FVector(10.0f, 10.0f, 330.0f));
    AddBox(GasPipe, Museum + FVector(870.0f, 10.0f, 360.0f), FVector(10.0f, 690.0f, 10.0f));

    // Dark museum plaque on the left front wing and subtle plinth vents are visible in frontal photography.
    AddBox(Plaques, Museum + FVector(-505.0f, -444.0f, 245.0f), FVector(88.0f, 10.0f, 58.0f));
    AddBox(Plaques, Museum + FVector(-590.0f, -454.0f, 52.0f), FVector(70.0f, 8.0f, 22.0f));
    AddBox(Plaques, Museum + FVector(590.0f, -454.0f, 52.0f), FVector(70.0f, 8.0f, 22.0f));

    for (int32 Slab = 0; Slab < 30; ++Slab)
    {
        const float Y = -940.0f - static_cast<float>(Slab) * 145.0f;
        AddBox(Concrete, Museum + FVector(0.0f, Y, 4.0f), FVector(165.0f, 132.0f, 8.0f));
    }

    UInstancedStaticMeshComponent* Pine01ISM = MakeISM(Model, Root, Pine01, nullptr,
        TEXT("R137Museum_Pine01"), true, true, 100000);
    UInstancedStaticMeshComponent* Pine03ISM = MakeISM(Model, Root, Pine03, nullptr,
        TEXT("R137Museum_Pine03"), true, true, 100000);

    struct FTreeSeed { FVector Offset; float Height; float Yaw; int32 Family; };
    const FTreeSeed Trees[] = {
        { FVector(-720, -1450, 0), 1900, 10, 0 }, { FVector(760, -1500, 0), 2050, 46, 1 },
        { FVector(-930, -2350, 0), 2200, 88, 1 }, { FVector(960, -2400, 0), 2150, 142, 0 },
        { FVector(-1020, -3350, 0), 2300, 188, 0 }, { FVector(1080, -3400, 0), 2250, 236, 1 },
        { FVector(-1150, -4300, 0), 2350, 278, 1 }, { FVector(1180, -4250, 0), 2400, 318, 0 }
    };
    UInstancedStaticMeshComponent* Families[] = { Pine01ISM, Pine03ISM };
    UStaticMesh* Meshes[] = { Pine01, Pine03 };
    int32 TreeCount = 0;
    for (const FTreeSeed& Seed : Trees)
    {
        if (Seed.Family < 0 || Seed.Family >= UE_ARRAY_COUNT(Families)) continue;
        if (!Families[Seed.Family] || !Meshes[Seed.Family]) continue;
        AddGroundedTree(Families[Seed.Family], Meshes[Seed.Family], Museum + Seed.Offset, Seed.Height, Seed.Yaw);
        ++TreeCount;
    }

    UE_LOG(LogTemp, Display,
        TEXT("R13.7 museum detail pass: photo-driven facade now includes finer upper/porch gable carpentry, paneled double doors, window sills/lintels, lower brick relief blocks, open ornamental stair railings, stair cheeks, front plaque and plinth vents; existing veranda, annex, chimney, gas line, slab approach and supported conifer site trees=%d retained."),
        TreeCount);
}