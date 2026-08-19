#include "OCR13MuseumStadiumPhotoFidelitySubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"
#include "OCR13VerifiedOsterGeographySubsystem.h"

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
    constexpr float PhotoFidelityDelaySeconds = 4.15f;
    constexpr float MuseumLegacyRadiusCm = 7200.0f;
    constexpr float MuseumLegacyTreeRadiusCm = 6600.0f;
    constexpr float StadiumLegacyRadiusCm = 9200.0f;

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
        const float YawDegrees = 0.0f)
    {
        if (!Component) return;
        Component->AddInstance(FTransform(FRotator(0.0f, YawDegrees, 0.0f), Center, SizeCm / 100.0f), true);
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

    void AddGroundedTree(UInstancedStaticMeshComponent* Component, UStaticMesh* Mesh,
        const FVector& GroundLocation, const float DesiredHeightCm, const float YawDegrees)
    {
        if (!Component || !Mesh) return;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        if (NativeSize.Z <= 10.0f) return;

        const float Scale = FMath::Clamp(DesiredHeightCm / NativeSize.Z, 0.25f, 4.5f);
        const float LocalBottom = Bounds.Origin.Z - Bounds.BoxExtent.Z;
        FVector Location = GroundLocation;
        Location.Z = -LocalBottom * Scale;
        Component->AddInstance(FTransform(
            FRotator(0.0f, YawDegrees, 0.0f), Location, FVector(Scale)), true);
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

    bool IsSourceMuseumFamily(const FName Name)
    {
        return Name == TEXT("LandmarkBlocks") || Name == TEXT("LandmarkRoofs") ||
            Name == TEXT("LandmarkWindows") || Name == TEXT("LandmarkDetails") || Name == TEXT("Fences");
    }

    bool IsWholeCityTreeFamily(const FName Name)
    {
        const FString Value = Name.ToString();
        return Value.StartsWith(TEXT("R13_Tree")) || Value.StartsWith(TEXT("R13_Pine")) ||
            Value.StartsWith(TEXT("R13_CivicTree")) || Value.StartsWith(TEXT("R13_CivicShrub"));
    }

    bool IsLegacyMuseumDedicatedFamily(const FName Name)
    {
        return Name.ToString().StartsWith(TEXT("R13_Museum"));
    }

    bool IsLegacyStadiumDedicatedFamily(const FName Name)
    {
        return Name.ToString().StartsWith(TEXT("R13_Stadium"));
    }

    void AddFrontWindow(UInstancedStaticMeshComponent* Trim, UInstancedStaticMeshComponent* Glass,
        const FVector& Museum, const float X, const float Y, const float Z, const float Width, const float Height)
    {
        AddBox(Trim, Museum + FVector(X - Width * 0.5f - 9.0f, Y, Z), FVector(18.0f, 18.0f, Height + 28.0f));
        AddBox(Trim, Museum + FVector(X + Width * 0.5f + 9.0f, Y, Z), FVector(18.0f, 18.0f, Height + 28.0f));
        AddBox(Trim, Museum + FVector(X, Y, Z + Height * 0.5f + 9.0f), FVector(Width + 36.0f, 18.0f, 18.0f));
        AddBox(Trim, Museum + FVector(X, Y, Z - Height * 0.5f - 9.0f), FVector(Width + 36.0f, 18.0f, 18.0f));
        AddBox(Glass, Museum + FVector(X, Y + 4.0f, Z), FVector(Width, 10.0f, Height));
    }

    void AddSideWindow(UInstancedStaticMeshComponent* Trim, UInstancedStaticMeshComponent* Glass,
        const FVector& Museum, const float X, const float Y, const float Z, const float Width, const float Height)
    {
        AddBox(Trim, Museum + FVector(X, Y - Width * 0.5f - 9.0f, Z), FVector(18.0f, 18.0f, Height + 28.0f));
        AddBox(Trim, Museum + FVector(X, Y + Width * 0.5f + 9.0f, Z), FVector(18.0f, 18.0f, Height + 28.0f));
        AddBox(Trim, Museum + FVector(X, Y, Z + Height * 0.5f + 9.0f), FVector(18.0f, Width + 36.0f, 18.0f));
        AddBox(Trim, Museum + FVector(X, Y, Z - Height * 0.5f - 9.0f), FVector(18.0f, Width + 36.0f, 18.0f));
        AddBox(Glass, Museum + FVector(X + 4.0f, Y, Z), FVector(10.0f, Width, Height));
    }

    void AddGoal(UInstancedStaticMeshComponent* Goal, const FVector& Center, const bool bAlongX)
    {
        if (!Goal) return;
        if (bAlongX)
        {
            AddBox(Goal, Center + FVector(0.0f, -360.0f, 120.0f), FVector(26.0f, 26.0f, 240.0f));
            AddBox(Goal, Center + FVector(0.0f,  360.0f, 120.0f), FVector(26.0f, 26.0f, 240.0f));
            AddBox(Goal, Center + FVector(0.0f, 0.0f, 240.0f), FVector(26.0f, 746.0f, 26.0f));
        }
        else
        {
            AddBox(Goal, Center + FVector(-360.0f, 0.0f, 120.0f), FVector(26.0f, 26.0f, 240.0f));
            AddBox(Goal, Center + FVector( 360.0f, 0.0f, 120.0f), FVector(26.0f, 26.0f, 240.0f));
            AddBox(Goal, Center + FVector(0.0f, 0.0f, 240.0f), FVector(746.0f, 26.0f, 26.0f));
        }
    }
}

bool UOCR13MuseumStadiumPhotoFidelitySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13MuseumStadiumPhotoFidelitySubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) ApplyPhotoFidelity(*World);
        }), PhotoFidelityDelaySeconds, false);
}

void UOCR13MuseumStadiumPhotoFidelitySubsystem::ApplyPhotoFidelity(UWorld& World)
{
    SuppressLegacyMuseumPresentation(World);
    SuppressLegacyStadiumPresentation(World);
    BuildMuseum(World);
    BuildStadium(World);
}

void UOCR13MuseumStadiumPhotoFidelitySubsystem::SuppressLegacyMuseumPresentation(UWorld& World)
{
    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();
    int32 Touched = 0;

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
            if (IsLegacyMuseumDedicatedFamily(Name))
            {
                Component->SetVisibility(false, true);
                Component->SetHiddenInGame(true, true);
                Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                ++Touched;
                continue;
            }
            if (IsSourceMuseumFamily(Name) && RemoveInstancesNear(Component, Museum, MuseumLegacyRadiusCm))
            {
                ++Touched;
                continue;
            }
            if (IsWholeCityTreeFamily(Name) && RemoveInstancesNear(Component, Museum, MuseumLegacyTreeRadiusCm))
            {
                ++Touched;
            }
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("R13.6 museum photo fidelity: suppressed/trimmed %d legacy museum presentation components before final rebuild."),
        Touched);
}

void UOCR13MuseumStadiumPhotoFidelitySubsystem::SuppressLegacyStadiumPresentation(UWorld& World)
{
    const FVector Stadium = UOCR13VerifiedOsterGeographySubsystem::VerifiedStadiumAnchor();
    int32 Touched = 0;

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
            if (IsLegacyStadiumDedicatedFamily(Name))
            {
                Component->SetVisibility(false, true);
                Component->SetHiddenInGame(true, true);
                Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                ++Touched;
                continue;
            }
            if ((Name == TEXT("StadiumGeometry") || Name == TEXT("StadiumDetails") || Name == TEXT("Fences")) &&
                RemoveInstancesNear(Component, Stadium, StadiumLegacyRadiusCm))
            {
                ++Touched;
            }
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("R13.6 stadium photo fidelity: removed old track/stand/proxy presentation around the verified adjacent stadium; touched=%d."),
        Touched);
}

void UOCR13MuseumStadiumPhotoFidelitySubsystem::BuildMuseum(UWorld& World)
{
    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UStaticMesh* RoofMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Roof_Both_Ends_4m.Roof_Both_Ends_4m"));
    UStaticMesh* Pine01 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_01.SM_Pine_Tree_01"));
    UStaticMesh* Pine03 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_03.SM_Pine_Tree_03"));
    UStaticMesh* Pine05 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_05.SM_Pine_Tree_05"));
    UStaticMesh* Tree01 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var01.SM_Tree_Var01"));
    UStaticMesh* Tree04 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var04.SM_Tree_Var04"));
    UMaterialInterface* BasicMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    UMaterialInterface* RoofMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Metal_Roof.Metal_Roof"));
    UMaterialInterface* WoodMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Wood_Planks_Painted_Blue.Wood_Planks_Painted_Blue"));
    UMaterialInterface* GlassMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Glass_Window.Glass_Window"));
    if (!Cube || !BasicMaterial || !RoofMesh) return;

    AActor* ArtRoot = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!ArtRoot) return;
    ArtRoot->SetReplicates(false);
    ArtRoot->SetActorEnableCollision(true);

    USceneComponent* Root = NewObject<USceneComponent>(ArtRoot, TEXT("R13_MuseumPhotoFidelityRoot"));
    if (!Root)
    {
        ArtRoot->Destroy();
        return;
    }
    Root->SetMobility(EComponentMobility::Static);
    ArtRoot->SetRootComponent(Root);
    ArtRoot->AddInstanceComponent(Root);
    Root->RegisterComponent();

    UMaterialInstanceDynamic* Brick = MakeColorMaterial(ArtRoot, BasicMaterial,
        TEXT("R13_MuseumPhotoBrick"), FLinearColor(0.47f, 0.19f, 0.085f, 1.0f));
    UMaterialInstanceDynamic* BrickDark = MakeColorMaterial(ArtRoot, BasicMaterial,
        TEXT("R13_MuseumPhotoBrickDark"), FLinearColor(0.31f, 0.11f, 0.055f, 1.0f));
    UMaterialInstanceDynamic* PlinthMat = MakeColorMaterial(ArtRoot, BasicMaterial,
        TEXT("R13_MuseumPhotoPlinth"), FLinearColor(0.035f, 0.032f, 0.030f, 1.0f));
    UMaterialInstanceDynamic* TrimMat = MakeColorMaterial(ArtRoot, BasicMaterial,
        TEXT("R13_MuseumPhotoTrim"), FLinearColor(0.78f, 0.74f, 0.64f, 1.0f));
    UMaterialInstanceDynamic* DoorMat = MakeColorMaterial(ArtRoot, BasicMaterial,
        TEXT("R13_MuseumPhotoDoor"), FLinearColor(0.28f, 0.31f, 0.31f, 1.0f));
    UMaterialInstanceDynamic* ConcreteMat = MakeColorMaterial(ArtRoot, BasicMaterial,
        TEXT("R13_MuseumPhotoConcrete"), FLinearColor(0.39f, 0.40f, 0.37f, 1.0f));
    UMaterialInstanceDynamic* GasMat = MakeColorMaterial(ArtRoot, BasicMaterial,
        TEXT("R13_MuseumPhotoGasPipe"), FLinearColor(0.82f, 0.63f, 0.035f, 1.0f));
    UMaterialInstanceDynamic* SignBlueMat = MakeColorMaterial(ArtRoot, BasicMaterial,
        TEXT("R13_MuseumPhotoSignBlue"), FLinearColor(0.03f, 0.42f, 0.66f, 1.0f));
    UMaterialInstanceDynamic* AnnexMat = MakeColorMaterial(ArtRoot, BasicMaterial,
        TEXT("R13_MuseumPhotoAnnex"), FLinearColor(0.52f, 0.34f, 0.26f, 1.0f));
    UMaterialInstanceDynamic* RailMat = MakeColorMaterial(ArtRoot, BasicMaterial,
        TEXT("R13_MuseumPhotoRail"), FLinearColor(0.32f, 0.38f, 0.38f, 1.0f));

    UInstancedStaticMeshComponent* Plinth = MakeISM(ArtRoot, Root, Cube, PlinthMat,
        TEXT("R13_MuseumPhotoPlinthISM"), true, true);
    UInstancedStaticMeshComponent* BrickBody = MakeISM(ArtRoot, Root, Cube, Brick,
        TEXT("R13_MuseumPhotoBrickBody"), true, true);
    UInstancedStaticMeshComponent* BrickDetail = MakeISM(ArtRoot, Root, Cube, BrickDark,
        TEXT("R13_MuseumPhotoBrickDetail"), false, true);
    UInstancedStaticMeshComponent* Wood = MakeISM(ArtRoot, Root, Cube, WoodMaterial,
        TEXT("R13_MuseumPhotoWoodUpper"), true, true);
    UInstancedStaticMeshComponent* Roof = MakeISM(ArtRoot, Root, RoofMesh, RoofMaterial,
        TEXT("R13_MuseumPhotoMetalRoof"), false, true);
    UInstancedStaticMeshComponent* Trim = MakeISM(ArtRoot, Root, Cube, TrimMat,
        TEXT("R13_MuseumPhotoCarvedTrim"), false, true);
    UInstancedStaticMeshComponent* Glass = MakeISM(ArtRoot, Root, Cube, GlassMaterial,
        TEXT("R13_MuseumPhotoGlass"), false, false);
    UInstancedStaticMeshComponent* Door = MakeISM(ArtRoot, Root, Cube, DoorMat,
        TEXT("R13_MuseumPhotoDoors"), false, true);
    UInstancedStaticMeshComponent* Concrete = MakeISM(ArtRoot, Root, Cube, ConcreteMat,
        TEXT("R13_MuseumPhotoConcrete"), true, false);
    UInstancedStaticMeshComponent* GasPipe = MakeISM(ArtRoot, Root, Cube, GasMat,
        TEXT("R13_MuseumPhotoGasPipe"), false, false, 50000);
    UInstancedStaticMeshComponent* Sign = MakeISM(ArtRoot, Root, Cube, SignBlueMat,
        TEXT("R13_MuseumPhotoEntranceSign"), false, false, 50000);
    UInstancedStaticMeshComponent* Annex = MakeISM(ArtRoot, Root, Cube, AnnexMat,
        TEXT("R13_MuseumPhotoRearAnnex"), true, true);
    UInstancedStaticMeshComponent* Rails = MakeISM(ArtRoot, Root, Cube, RailMat,
        TEXT("R13_MuseumPhotoRailings"), false, true);

    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();

    // Photo set: long one-storey brick body, black plinth, light sheet-metal gable roof.
    AddBox(Plinth, Museum + FVector(0.0f, 0.0f, 55.0f), FVector(5750.0f, 2700.0f, 110.0f));
    AddBox(BrickBody, Museum + FVector(0.0f, 0.0f, 385.0f), FVector(5650.0f, 2600.0f, 550.0f));
    AddFittedMesh(Roof, RoofMesh, Museum + FVector(0.0f, 0.0f, 830.0f),
        FVector(6000.0f, 3100.0f, 880.0f));

    // Central timber upper volume visible head-on in the supplied photos.
    AddBox(Wood, Museum + FVector(0.0f, -80.0f, 860.0f), FVector(1780.0f, 1500.0f, 500.0f));
    AddFittedMesh(Roof, RoofMesh, Museum + FVector(0.0f, -80.0f, 1220.0f),
        FVector(2080.0f, 1820.0f, 620.0f));

    // Central glazed entrance vestibule and the left-side enclosed veranda.
    AddBox(Wood, Museum + FVector(0.0f, -1710.0f, 265.0f), FVector(1760.0f, 820.0f, 430.0f));
    AddFittedMesh(Roof, RoofMesh, Museum + FVector(0.0f, -1710.0f, 575.0f),
        FVector(1980.0f, 1120.0f, 420.0f));
    AddBox(Wood, Museum + FVector(-3300.0f, 120.0f, 245.0f), FVector(1000.0f, 1780.0f, 420.0f));
    AddFittedMesh(Roof, RoofMesh, Museum + FVector(-3300.0f, 120.0f, 530.0f),
        FVector(1260.0f, 2020.0f, 390.0f), 90.0f);

    // Small low rear annex shown beyond the brick wall in several side photographs.
    AddBox(Annex, Museum + FVector(3350.0f, 720.0f, 205.0f), FVector(1200.0f, 1550.0f, 410.0f));
    AddFittedMesh(Roof, RoofMesh, Museum + FVector(3350.0f, 720.0f, 480.0f),
        FVector(1400.0f, 1780.0f, 360.0f), 90.0f);

    // Brick cornice/dentils and pale decorative square inserts beneath the eaves.
    AddBox(BrickDetail, Museum + FVector(0.0f, -1312.0f, 625.0f), FVector(5650.0f, 34.0f, 70.0f));
    AddBox(BrickDetail, Museum + FVector(2822.0f, 0.0f, 625.0f), FVector(34.0f, 2600.0f, 70.0f));
    for (int32 Index = -6; Index <= 6; ++Index)
    {
        const float X = static_cast<float>(Index) * 395.0f;
        AddBox(Trim, Museum + FVector(X, -1332.0f, 583.0f), FVector(54.0f, 16.0f, 62.0f));
    }
    for (int32 Index = -2; Index <= 2; ++Index)
    {
        const float Y = static_cast<float>(Index) * 430.0f;
        AddBox(Trim, Museum + FVector(2840.0f, Y, 583.0f), FVector(16.0f, 54.0f, 62.0f));
    }

    // Main facade window rhythm around the central entrance.
    AddFrontWindow(Trim, Glass, Museum, -2120.0f, -1318.0f, 365.0f, 340.0f, 290.0f);
    AddFrontWindow(Trim, Glass, Museum, -1220.0f, -1318.0f, 365.0f, 340.0f, 290.0f);
    AddFrontWindow(Trim, Glass, Museum,  1220.0f, -1318.0f, 365.0f, 340.0f, 290.0f);
    AddFrontWindow(Trim, Glass, Museum,  2120.0f, -1318.0f, 365.0f, 340.0f, 290.0f);

    // Right side wall: tall repeated windows visible in the oblique photo sequence.
    for (int32 Index = -2; Index <= 2; ++Index)
    {
        AddSideWindow(Trim, Glass, Museum, 2835.0f, static_cast<float>(Index) * 430.0f,
            365.0f, 300.0f, 290.0f);
    }

    // Three-window upper group and pale decorative triangular cues around the timber volume.
    AddFrontWindow(Trim, Glass, Museum, -430.0f, -838.0f, 900.0f, 280.0f, 285.0f);
    AddFrontWindow(Trim, Glass, Museum,    0.0f, -838.0f, 900.0f, 280.0f, 285.0f);
    AddFrontWindow(Trim, Glass, Museum,  430.0f, -838.0f, 900.0f, 280.0f, 285.0f);
    for (int32 Index = -1; Index <= 1; ++Index)
    {
        const float X = static_cast<float>(Index) * 430.0f;
        AddBox(Trim, Museum + FVector(X - 105.0f, -850.0f, 1070.0f), FVector(250.0f, 16.0f, 20.0f), -34.0f);
        AddBox(Trim, Museum + FVector(X + 105.0f, -850.0f, 1070.0f), FVector(250.0f, 16.0f, 20.0f),  34.0f);
    }

    // Entrance glazing and grey double doors.
    AddFrontWindow(Trim, Glass, Museum, -560.0f, -2125.0f, 285.0f, 350.0f, 330.0f);
    AddFrontWindow(Trim, Glass, Museum,  560.0f, -2125.0f, 285.0f, 350.0f, 330.0f);
    AddBox(Door, Museum + FVector(-155.0f, -2133.0f, 270.0f), FVector(300.0f, 18.0f, 420.0f));
    AddBox(Door, Museum + FVector( 155.0f, -2133.0f, 270.0f), FVector(300.0f, 18.0f, 420.0f));

    // Side veranda: large glass bays rather than a solid side cube.
    for (int32 Index = -2; Index <= 2; ++Index)
    {
        AddSideWindow(Trim, Glass, Museum, -3808.0f, 120.0f + static_cast<float>(Index) * 330.0f,
            255.0f, 275.0f, 320.0f);
    }

    // Steps and simple metal railings from the head-on entrance photographs.
    for (int32 Step = 0; Step < 6; ++Step)
    {
        const float Width = 1500.0f - static_cast<float>(Step) * 90.0f;
        const float Y = -2570.0f + static_cast<float>(Step) * 115.0f;
        const float Height = 22.0f + static_cast<float>(Step) * 18.0f;
        AddBox(Concrete, Museum + FVector(0.0f, Y, Height * 0.5f), FVector(Width, 220.0f, Height));
    }
    AddBox(Rails, Museum + FVector(-720.0f, -2380.0f, 185.0f), FVector(24.0f, 700.0f, 330.0f));
    AddBox(Rails, Museum + FVector( 720.0f, -2380.0f, 185.0f), FVector(24.0f, 700.0f, 330.0f));
    AddBox(Rails, Museum + FVector(-720.0f, -2380.0f, 350.0f), FVector(24.0f, 720.0f, 24.0f));
    AddBox(Rails, Museum + FVector( 720.0f, -2380.0f, 350.0f), FVector(24.0f, 720.0f, 24.0f));

    // Yellow exposed gas line visible on the brick side wall.
    AddBox(GasPipe, Museum + FVector(2848.0f, -780.0f, 360.0f), FVector(18.0f, 18.0f, 520.0f));
    AddBox(GasPipe, Museum + FVector(2848.0f, 280.0f, 505.0f), FVector(18.0f, 2100.0f, 18.0f));

    // Long narrow slab approach. Previous forest-road mesh is intentionally replaced by individual concrete slabs.
    for (int32 Slab = 0; Slab < 26; ++Slab)
    {
        const float Y = -2770.0f - static_cast<float>(Slab) * 145.0f;
        AddBox(Concrete, Museum + FVector(0.0f, Y, 5.0f), FVector(165.0f, 132.0f, 10.0f));
    }
    for (int32 Slab = 0; Slab < 8; ++Slab)
    {
        const float X = -105.0f - static_cast<float>(Slab) * 145.0f;
        AddBox(Concrete, Museum + FVector(X, -4760.0f, 5.0f), FVector(132.0f, 165.0f, 10.0f));
    }

    // Street-side museum sign/plinths from the supplied site photo. Text stays out of geometry until a proper sign asset exists.
    AddBox(Concrete, Museum + FVector(2350.0f, -6200.0f, 115.0f), FVector(390.0f, 52.0f, 230.0f));
    AddBox(Sign, Museum + FVector(2350.0f, -6228.0f, 135.0f), FVector(300.0f, 12.0f, 105.0f));
    AddBox(Concrete, Museum + FVector(2020.0f, -6100.0f, 55.0f), FVector(220.0f, 180.0f, 110.0f));
    AddBox(Concrete, Museum + FVector(2680.0f, -6100.0f, 55.0f), FVector(220.0f, 180.0f, 110.0f));

    UInstancedStaticMeshComponent* Pine01ISM = MakeISM(ArtRoot, Root, Pine01, nullptr,
        TEXT("R13_MuseumPhotoPine01"), true, true, 120000);
    UInstancedStaticMeshComponent* Pine03ISM = MakeISM(ArtRoot, Root, Pine03, nullptr,
        TEXT("R13_MuseumPhotoPine03"), true, true, 120000);
    UInstancedStaticMeshComponent* Pine05ISM = MakeISM(ArtRoot, Root, Pine05, nullptr,
        TEXT("R13_MuseumPhotoPine05"), true, true, 120000);
    UInstancedStaticMeshComponent* Tree01ISM = MakeISM(ArtRoot, Root, Tree01, nullptr,
        TEXT("R13_MuseumPhotoDeciduous01"), true, true, 110000);
    UInstancedStaticMeshComponent* Tree04ISM = MakeISM(ArtRoot, Root, Tree04, nullptr,
        TEXT("R13_MuseumPhotoDeciduous04"), true, true, 110000);

    struct FTreeSeed { FVector Offset; float Height; float Yaw; int32 Family; };
    const FTreeSeed Trees[] = {
        { FVector(-3300, -3000, 0), 2350,  14, 0 }, { FVector( 3500, -3100, 0), 2450,  55, 1 },
        { FVector(-3850, -4450, 0), 2600, 103, 2 }, { FVector( 4050, -4550, 0), 2520, 157, 0 },
        { FVector(-4300, -5850, 0), 2720, 211, 1 }, { FVector( 4450, -5750, 0), 2650, 278, 2 },
        { FVector(-3900,  2500, 0), 2250, 319, 0 }, { FVector( 4100,  2450, 0), 2400,  35, 1 },
        { FVector(-5200,  -700, 0), 2050,  78, 3 }, { FVector( 5050,  -300, 0), 2150, 132, 4 },
        { FVector(-5150,  3200, 0), 2200, 196, 4 }, { FVector( 5000,  3300, 0), 2180, 248, 3 },
    };
    UInstancedStaticMeshComponent* Families[] = { Pine01ISM, Pine03ISM, Pine05ISM, Tree01ISM, Tree04ISM };
    UStaticMesh* Meshes[] = { Pine01, Pine03, Pine05, Tree01, Tree04 };
    int32 TreeCount = 0;
    for (const FTreeSeed& Seed : Trees)
    {
        if (Seed.Family < 0 || Seed.Family >= UE_ARRAY_COUNT(Families)) continue;
        if (!Families[Seed.Family] || !Meshes[Seed.Family]) continue;
        AddGroundedTree(Families[Seed.Family], Meshes[Seed.Family], Museum + Seed.Offset, Seed.Height, Seed.Yaw);
        ++TreeCount;
    }

    UE_LOG(LogTemp, Display,
        TEXT("R13.6 museum photo fidelity: rebuilt brick Solonyna house with central vestibule/timber upper room/side veranda/metal roof/concrete slab approach/gas pipe/sign zone; mature site trees=%d."),
        TreeCount);
}

void UOCR13MuseumStadiumPhotoFidelitySubsystem::BuildStadium(UWorld& World)
{
    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UStaticMesh* Pine03 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_03.SM_Pine_Tree_03"));
    UStaticMesh* Tree01 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var01.SM_Tree_Var01"));
    UMaterialInterface* BasicMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!Cube || !BasicMaterial) return;

    AActor* ArtRoot = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!ArtRoot) return;
    ArtRoot->SetReplicates(false);
    ArtRoot->SetActorEnableCollision(false);

    USceneComponent* Root = NewObject<USceneComponent>(ArtRoot, TEXT("R13_StadiumPhotoFidelityRoot"));
    if (!Root)
    {
        ArtRoot->Destroy();
        return;
    }
    Root->SetMobility(EComponentMobility::Static);
    ArtRoot->SetRootComponent(Root);
    ArtRoot->AddInstanceComponent(Root);
    Root->RegisterComponent();

    UMaterialInstanceDynamic* Grass = MakeColorMaterial(ArtRoot, BasicMaterial,
        TEXT("R13_StadiumPhotoGrass"), FLinearColor(0.22f, 0.31f, 0.12f, 1.0f));
    UMaterialInstanceDynamic* GoalMat = MakeColorMaterial(ArtRoot, BasicMaterial,
        TEXT("R13_StadiumPhotoGoal"), FLinearColor(0.66f, 0.68f, 0.64f, 1.0f));
    UMaterialInstanceDynamic* ExerciseMat = MakeColorMaterial(ArtRoot, BasicMaterial,
        TEXT("R13_StadiumPhotoExercise"), FLinearColor(0.11f, 0.22f, 0.18f, 1.0f));

    UInstancedStaticMeshComponent* Field = MakeISM(ArtRoot, Root, Cube, Grass,
        TEXT("R13_StadiumPhotoOpenGrass"), false, false, 120000);
    UInstancedStaticMeshComponent* Goal = MakeISM(ArtRoot, Root, Cube, GoalMat,
        TEXT("R13_StadiumPhotoGoals"), false, false, 100000);
    UInstancedStaticMeshComponent* Exercise = MakeISM(ArtRoot, Root, Cube, ExerciseMat,
        TEXT("R13_StadiumPhotoExerciseBars"), false, false, 80000);
    UInstancedStaticMeshComponent* Pine = MakeISM(ArtRoot, Root, Pine03, nullptr,
        TEXT("R13_StadiumPhotoPines"), false, true, 120000);
    UInstancedStaticMeshComponent* Tree = MakeISM(ArtRoot, Root, Tree01, nullptr,
        TEXT("R13_StadiumPhotoTrees"), false, true, 110000);

    const FVector Stadium = UOCR13VerifiedOsterGeographySubsystem::VerifiedStadiumAnchor();

    // Supplied stadium photo shows a simple open grass field, not the old artificial-turf/track/stand composition.
    AddBox(Field, Stadium + FVector(0.0f, 0.0f, 3.0f), FVector(10600.0f, 6700.0f, 6.0f));
    AddGoal(Goal, Stadium + FVector(-5050.0f, 0.0f, 0.0f), true);
    AddGoal(Goal, Stadium + FVector( 5050.0f, 0.0f, 0.0f), true);
    AddGoal(Goal, Stadium + FVector(-1600.0f, 1850.0f, 0.0f), true);

    // Sparse outdoor exercise bars along one edge, matching the simple community-sports character in the photo.
    for (int32 Index = 0; Index < 5; ++Index)
    {
        const float X = -3300.0f + static_cast<float>(Index) * 1200.0f;
        AddBox(Exercise, Stadium + FVector(X, 2850.0f, 115.0f), FVector(24.0f, 24.0f, 230.0f));
        AddBox(Exercise, Stadium + FVector(X + 380.0f, 2850.0f, 115.0f), FVector(24.0f, 24.0f, 230.0f));
        AddBox(Exercise, Stadium + FVector(X + 190.0f, 2850.0f, 220.0f), FVector(400.0f, 24.0f, 24.0f));
    }

    if (Pine && Pine03)
    {
        AddGroundedTree(Pine, Pine03, Stadium + FVector(-5900.0f, -2800.0f, 0.0f), 2300.0f, 27.0f);
        AddGroundedTree(Pine, Pine03, Stadium + FVector(-5900.0f,  2600.0f, 0.0f), 2450.0f, 118.0f);
    }
    if (Tree && Tree01)
    {
        AddGroundedTree(Tree, Tree01, Stadium + FVector(6100.0f, -2500.0f, 0.0f), 2050.0f, 205.0f);
        AddGroundedTree(Tree, Tree01, Stadium + FVector(6200.0f,  2500.0f, 0.0f), 2150.0f, 291.0f);
    }

    UE_LOG(LogTemp, Display,
        TEXT("R13.6 stadium photo fidelity: adjacent stadium simplified to open grass, basic goals and sparse exercise bars; old track/stands intentionally removed."));
}
