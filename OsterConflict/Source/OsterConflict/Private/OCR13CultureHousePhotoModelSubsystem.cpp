#include "OCR13CultureHousePhotoModelSubsystem.h"

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
    constexpr float CultureHouseDelaySeconds = 2.52f;
    // The public point and Hranovskoho 3 address identify the site, but exact cadastral facade yaw is not surveyed.
    constexpr float CultureHouseYawDegrees = 0.0f;

    FVector CultureHouseAnchor()
    {
        const FOCGeoReferencePoint Ref = FOCGeoReference::CultureHouse();
        return FOCGeoReference::ToLocalCm(Ref.Latitude, Ref.Longitude, 0.0);
    }

    UMaterialInstanceDynamic* MakeColor(AActor* Owner, UMaterialInterface* Base,
        const FName Name, const FLinearColor& Color)
    {
        if (!Owner || !Base) return nullptr;
        UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Base, Owner, Name);
        if (Material) Material->SetVectorParameterValue(TEXT("Color"), Color);
        return Material;
    }

    UInstancedStaticMeshComponent* MakeISM(AActor* Owner, USceneComponent* Root, UStaticMesh* Mesh,
        UMaterialInterface* Material, const FName Name, const bool bCollision, const bool bShadow = true)
    {
        if (!Owner || !Root || !Mesh) return nullptr;
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        if (Material)
        {
            const int32 Slots = FMath::Max(1, Mesh->GetStaticMaterials().Num());
            for (int32 Slot = 0; Slot < Slots; ++Slot) Component->SetMaterial(Slot, Material);
        }
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionProfileName(FName(bCollision ? TEXT("BlockAll") : TEXT("NoCollision")));
        Component->SetCollisionEnabled(bCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(bCollision);
        Component->SetCastShadow(bShadow);
        Component->SetCullDistances(0, 100000);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void AddBox(UInstancedStaticMeshComponent* Component, const FVector& Center, const FVector& SizeCm)
    {
        if (!Component) return;
        Component->AddInstance(FTransform(FRotator::ZeroRotator, Center, SizeCm / 100.0f), false);
    }

    void AddBoxRotated(UInstancedStaticMeshComponent* Component, const FVector& Center,
        const FVector& SizeCm, const FRotator& Rotation)
    {
        if (!Component) return;
        Component->AddInstance(FTransform(Rotation, Center, SizeCm / 100.0f), false);
    }

    void AddCylinder(UInstancedStaticMeshComponent* Component, const FVector& Center,
        const float DiameterCm, const float HeightCm, const FRotator& Rotation = FRotator::ZeroRotator)
    {
        if (!Component) return;
        Component->AddInstance(FTransform(
            Rotation, Center, FVector(DiameterCm / 100.0f, DiameterCm / 100.0f, HeightCm / 100.0f)), false);
    }
}

bool UOCR13CultureHousePhotoModelSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13CultureHousePhotoModelSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    // Structural shell, entrance deck and stair run are gameplay collision, so dedicated-server placement must match.
    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) BuildCultureHouse(*World);
        }), CultureHouseDelaySeconds, false);
}

void UOCR13CultureHousePhotoModelSubsystem::BuildCultureHouse(UWorld& World)
{
    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Existing = *It;
        if (Existing && Existing->ActorHasTag(TEXT("R13_CultureHousePhotoModel"))) return;
    }

    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    UMaterialInterface* Basic = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    UMaterialInterface* GlassMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Glass_Window.Glass_Window"));
    if (!Cube || !Cylinder || !Basic) return;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = TEXT("R13_OsterCultureHouse");
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AActor* Model = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, SpawnParams);
    if (!Model) return;
    Model->SetReplicates(false);
    Model->SetActorEnableCollision(true);
    Model->Tags.Add(TEXT("R13_CultureHousePhotoModel"));

    USceneComponent* Root = NewObject<USceneComponent>(Model, TEXT("R13_CultureHouseRoot"));
    if (!Root)
    {
        Model->Destroy();
        return;
    }
    Root->SetMobility(EComponentMobility::Static);
    Model->SetRootComponent(Root);
    Model->AddInstanceComponent(Root);
    Root->RegisterComponent();
    Model->SetActorLocationAndRotation(CultureHouseAnchor(), FRotator(0.0f, CultureHouseYawDegrees, 0.0f));

    UMaterialInstanceDynamic* Facade = MakeColor(Model, Basic, TEXT("R13_CultureHouseFacade"),
        FLinearColor(0.63f, 0.45f, 0.26f, 1.0f));
    UMaterialInstanceDynamic* Classical = MakeColor(Model, Basic, TEXT("R13_CultureHouseClassical"),
        FLinearColor(0.80f, 0.78f, 0.68f, 1.0f));
    UMaterialInstanceDynamic* DoorWood = MakeColor(Model, Basic, TEXT("R13_CultureHouseDoorWood"),
        FLinearColor(0.30f, 0.13f, 0.07f, 1.0f));
    UMaterialInstanceDynamic* RoofMat = MakeColor(Model, Basic, TEXT("R13_CultureHouseRoof"),
        FLinearColor(0.18f, 0.15f, 0.12f, 1.0f));
    UMaterialInstanceDynamic* Stone = MakeColor(Model, Basic, TEXT("R13_CultureHouseStone"),
        FLinearColor(0.41f, 0.39f, 0.35f, 1.0f));
    UMaterialInstanceDynamic* Path = MakeColor(Model, Basic, TEXT("R13_CultureHousePath"),
        FLinearColor(0.27f, 0.27f, 0.25f, 1.0f));

    UInstancedStaticMeshComponent* Shell = MakeISM(Model, Root, Cube, Facade, TEXT("R13_CultureHouseShell"), true);
    UInstancedStaticMeshComponent* Trim = MakeISM(Model, Root, Cube, Classical, TEXT("R13_CultureHouseTrim"), false);
    UInstancedStaticMeshComponent* Columns = MakeISM(Model, Root, Cylinder, Classical, TEXT("R13_CultureHouseColumns"), false);
    UInstancedStaticMeshComponent* DoorFrames = MakeISM(Model, Root, Cube, Classical, TEXT("R13_CultureHouseDoorFrames"), false);
    UInstancedStaticMeshComponent* Doors = MakeISM(Model, Root, Cube, DoorWood, TEXT("R13_CultureHouseDoors"), false);
    UInstancedStaticMeshComponent* Glass = MakeISM(Model, Root, Cube, GlassMaterial, TEXT("R13_CultureHouseGlass"), false);
    UInstancedStaticMeshComponent* RoundGlass = MakeISM(Model, Root, Cylinder, GlassMaterial, TEXT("R13_CultureHouseArchedGlass"), false);
    UInstancedStaticMeshComponent* Roof = MakeISM(Model, Root, Cube, RoofMat, TEXT("R13_CultureHouseRoof"), false);
    UInstancedStaticMeshComponent* Stonework = MakeISM(Model, Root, Cube, Stone, TEXT("R13_CultureHouseStonework"), true);
    UInstancedStaticMeshComponent* Ground = MakeISM(Model, Root, Cube, Path, TEXT("R13_CultureHouseGround"), true, false);

    // Tall one-storey hall from the current facade photo. Depth remains conservative because only the principal facade
    // is strongly documented in public imagery.
    AddBox(Shell, FVector(0.0f, 0.0f, 365.0f), FVector(3200.0f, 1850.0f, 730.0f));
    AddBox(Stonework, FVector(0.0f, 0.0f, 42.0f), FVector(3280.0f, 1910.0f, 84.0f));

    // Main facade cornice and side pilasters.
    AddBox(Trim, FVector(0.0f, -940.0f, 690.0f), FVector(3260.0f, 42.0f, 72.0f));
    AddBox(Trim, FVector(-1560.0f, -940.0f, 365.0f), FVector(56.0f, 42.0f, 620.0f));
    AddBox(Trim, FVector(1560.0f, -940.0f, 365.0f), FVector(56.0f, 42.0f, 620.0f));

    // Six-column portico visible in modern and historic views.
    const float ColumnXs[] = { -1130.0f, -680.0f, -230.0f, 230.0f, 680.0f, 1130.0f };
    for (const float X : ColumnXs)
    {
        AddCylinder(Columns, FVector(X, -1080.0f, 365.0f), 92.0f, 610.0f);
        AddCylinder(Columns, FVector(X, -1080.0f, 75.0f), 126.0f, 34.0f);
        AddCylinder(Columns, FVector(X, -1080.0f, 660.0f), 132.0f, 40.0f);
    }
    AddBox(Trim, FVector(0.0f, -1070.0f, 708.0f), FVector(2650.0f, 225.0f, 86.0f));

    // Three tall entrance bays with glazed arched heads. The circular caps intentionally avoid a false rectangular look
    // until a dedicated arched-window mesh/decal is authored.
    for (const float X : { -650.0f, 0.0f, 650.0f })
    {
        AddBox(DoorFrames, FVector(X, -954.0f, 300.0f), FVector(250.0f, 30.0f, 420.0f));
        AddBox(Doors, FVector(X, -972.0f, 215.0f), FVector(195.0f, 24.0f, 250.0f));
        AddBox(Glass, FVector(X, -977.0f, 415.0f), FVector(182.0f, 10.0f, 150.0f));
        AddCylinder(RoundGlass, FVector(X, -982.0f, 520.0f), 176.0f, 9.0f, FRotator(0.0f, 0.0f, 90.0f));
    }

    // Triangular pediment and shallow pitched roof.
    AddBox(Trim, FVector(0.0f, -1065.0f, 760.0f), FVector(2740.0f, 235.0f, 34.0f));
    AddBoxRotated(Trim, FVector(-650.0f, -1065.0f, 925.0f), FVector(1420.0f, 220.0f, 42.0f),
        FRotator(-14.0f, 0.0f, 0.0f));
    AddBoxRotated(Trim, FVector(650.0f, -1065.0f, 925.0f), FVector(1420.0f, 220.0f, 42.0f),
        FRotator(14.0f, 0.0f, 0.0f));
    AddBox(Roof, FVector(0.0f, 60.0f, 752.0f), FVector(3300.0f, 1960.0f, 66.0f));
    AddBoxRotated(Roof, FVector(0.0f, -470.0f, 815.0f), FVector(3260.0f, 1040.0f, 56.0f),
        FRotator(0.0f, 0.0f, -8.0f));
    AddBoxRotated(Roof, FVector(0.0f, 470.0f, 815.0f), FVector(3260.0f, 1040.0f, 56.0f),
        FRotator(0.0f, 0.0f, 8.0f));

    // Broad stepped entrance and straight park approach, both clear in the current photo.
    AddBox(Stonework, FVector(0.0f, -1170.0f, 64.0f), FVector(2500.0f, 460.0f, 44.0f));
    AddBox(Stonework, FVector(0.0f, -1375.0f, 43.0f), FVector(2660.0f, 190.0f, 34.0f));
    AddBox(Stonework, FVector(0.0f, -1500.0f, 25.0f), FVector(2780.0f, 150.0f, 26.0f));
    AddBox(Ground, FVector(0.0f, -2450.0f, 10.0f), FVector(760.0f, 1900.0f, 20.0f));

    UE_LOG(LogTemp, Display,
        TEXT("R13 Oster Culture House built at public Hranovskoho 3 site anchor (%.1f, %.1f); current-photo six-column facade, three arched entrance bays, pediment and authoritative collision active; parcel yaw remains provisional."),
        CultureHouseAnchor().X, CultureHouseAnchor().Y);
}
