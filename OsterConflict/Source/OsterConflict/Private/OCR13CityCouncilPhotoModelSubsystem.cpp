#include "OCR13CityCouncilPhotoModelSubsystem.h"

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
    constexpr float CityCouncilDelaySeconds = 2.46f;
    // Address/anchor and facade are verified. Street-facing parcel rotation is not surveyed yet, so keep it
    // centralized here instead of distributing a guessed yaw through every facade transform.
    constexpr float CityCouncilYawDegrees = 0.0f;

    FVector CityCouncilAnchor()
    {
        const FOCGeoReferencePoint Ref = FOCGeoReference::CityCouncil();
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

    void AddFacadeWindow(UInstancedStaticMeshComponent* Frame, UInstancedStaticMeshComponent* Glass,
        const float X, const float Z, const FVector& FrameSize, const FVector& GlassSize)
    {
        AddBox(Frame, FVector(X, -763.0f, Z), FrameSize);
        AddBox(Glass, FVector(X, -773.0f, Z), GlassSize);
    }
}

bool UOCR13CityCouncilPhotoModelSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13CityCouncilPhotoModelSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    // The shell, terrace and steps are gameplay collision. Build on every authoritative world so dedicated-server
    // physics cannot diverge from the client-side landmark presentation.
    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) BuildCityCouncil(*World);
        }), CityCouncilDelaySeconds, false);
}

void UOCR13CityCouncilPhotoModelSubsystem::BuildCityCouncil(UWorld& World)
{
    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Existing = *It;
        if (Existing && Existing->ActorHasTag(TEXT("R13_CityCouncilPhotoModel"))) return;
    }

    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UMaterialInterface* Basic = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    UMaterialInterface* GlassMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Glass_Window.Glass_Window"));
    if (!Cube || !Basic) return;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = TEXT("R13_OsterCityCouncil");
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AActor* Model = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, SpawnParams);
    if (!Model) return;
    Model->SetReplicates(false);
    Model->SetActorEnableCollision(true);
    Model->Tags.Add(TEXT("R13_CityCouncilPhotoModel"));

    USceneComponent* Root = NewObject<USceneComponent>(Model, TEXT("R13_CityCouncilRoot"));
    if (!Root)
    {
        Model->Destroy();
        return;
    }
    Root->SetMobility(EComponentMobility::Static);
    Model->SetRootComponent(Root);
    Model->AddInstanceComponent(Root);
    Root->RegisterComponent();
    Model->SetActorLocationAndRotation(CityCouncilAnchor(), FRotator(0.0f, CityCouncilYawDegrees, 0.0f));

    UMaterialInstanceDynamic* Facade = MakeColor(Model, Basic, TEXT("R13_CityCouncilFacade"),
        FLinearColor(0.48f, 0.36f, 0.24f, 1.0f));
    UMaterialInstanceDynamic* Classical = MakeColor(Model, Basic, TEXT("R13_CityCouncilClassical"),
        FLinearColor(0.78f, 0.77f, 0.69f, 1.0f));
    UMaterialInstanceDynamic* Dark = MakeColor(Model, Basic, TEXT("R13_CityCouncilDark"),
        FLinearColor(0.085f, 0.075f, 0.065f, 1.0f));
    UMaterialInstanceDynamic* RoofMat = MakeColor(Model, Basic, TEXT("R13_CityCouncilRoof"),
        FLinearColor(0.16f, 0.14f, 0.12f, 1.0f));
    UMaterialInstanceDynamic* Stone = MakeColor(Model, Basic, TEXT("R13_CityCouncilStone"),
        FLinearColor(0.36f, 0.36f, 0.34f, 1.0f));
    UMaterialInstanceDynamic* Paving = MakeColor(Model, Basic, TEXT("R13_CityCouncilPaving"),
        FLinearColor(0.29f, 0.29f, 0.27f, 1.0f));

    UInstancedStaticMeshComponent* Shell = MakeISM(Model, Root, Cube, Facade, TEXT("R13_CityCouncilShell"), true);
    UInstancedStaticMeshComponent* Trim = MakeISM(Model, Root, Cube, Classical, TEXT("R13_CityCouncilTrim"), false);
    UInstancedStaticMeshComponent* Frames = MakeISM(Model, Root, Cube, Classical, TEXT("R13_CityCouncilWindowFrames"), false);
    UInstancedStaticMeshComponent* Glass = MakeISM(Model, Root, Cube, GlassMaterial, TEXT("R13_CityCouncilGlass"), false);
    UInstancedStaticMeshComponent* Doors = MakeISM(Model, Root, Cube, Dark, TEXT("R13_CityCouncilDoors"), false);
    UInstancedStaticMeshComponent* Roof = MakeISM(Model, Root, Cube, RoofMat, TEXT("R13_CityCouncilRoof"), false);
    UInstancedStaticMeshComponent* Stonework = MakeISM(Model, Root, Cube, Stone, TEXT("R13_CityCouncilStonework"), true);
    UInstancedStaticMeshComponent* Ground = MakeISM(Model, Root, Cube, Paving, TEXT("R13_CityCouncilGround"), true, false);

    // Two-storey council body from the public facade references. The footprint is intentionally conservative:
    // facade proportions are photo-driven while depth/yaw remain provisional until parcel geometry is surveyed.
    AddBox(Shell, FVector(0.0f, 0.0f, 390.0f), FVector(3900.0f, 1500.0f, 780.0f));
    AddBox(Stonework, FVector(0.0f, 0.0f, 45.0f), FVector(3980.0f, 1560.0f, 90.0f));

    // Horizontal belt and roof cornices visible across the darker side wings.
    AddBox(Trim, FVector(0.0f, -765.0f, 390.0f), FVector(3940.0f, 34.0f, 34.0f));
    AddBox(Trim, FVector(0.0f, -765.0f, 745.0f), FVector(4020.0f, 42.0f, 72.0f));
    AddBox(Trim, FVector(-1930.0f, -765.0f, 390.0f), FVector(48.0f, 40.0f, 700.0f));
    AddBox(Trim, FVector(1930.0f, -765.0f, 390.0f), FVector(48.0f, 40.0f, 700.0f));

    const float WingWindowXs[] = { -1630.0f, -1210.0f, -790.0f, 790.0f, 1210.0f, 1630.0f };
    for (const float X : WingWindowXs)
    {
        AddFacadeWindow(Frames, Glass, X, 220.0f, FVector(210.0f, 28.0f, 245.0f), FVector(172.0f, 9.0f, 205.0f));
        AddFacadeWindow(Frames, Glass, X, 555.0f, FVector(210.0f, 28.0f, 250.0f), FVector(172.0f, 9.0f, 210.0f));
    }

    // Central neoclassical portico: four full-height light columns and a deep entablature.
    AddBox(Stonework, FVector(0.0f, -900.0f, 42.0f), FVector(1660.0f, 390.0f, 84.0f));
    for (const float X : { -610.0f, -205.0f, 205.0f, 610.0f })
    {
        AddBox(Trim, FVector(X, -940.0f, 410.0f), FVector(105.0f, 105.0f, 690.0f));
        AddBox(Trim, FVector(X, -940.0f, 82.0f), FVector(142.0f, 142.0f, 38.0f));
        AddBox(Trim, FVector(X, -940.0f, 755.0f), FVector(148.0f, 148.0f, 44.0f));
    }
    AddBox(Trim, FVector(0.0f, -930.0f, 800.0f), FVector(1660.0f, 250.0f, 92.0f));
    AddBox(Trim, FVector(0.0f, -930.0f, 862.0f), FVector(1700.0f, 258.0f, 32.0f));

    // Door/window rhythm between the columns. No TextRender signage: landmark lettering belongs in proper texture/decal art.
    AddBox(Doors, FVector(0.0f, -1000.0f, 188.0f), FVector(190.0f, 28.0f, 300.0f));
    AddFacadeWindow(Frames, Glass, -405.0f, 235.0f, FVector(180.0f, 28.0f, 230.0f), FVector(144.0f, 9.0f, 192.0f));
    AddFacadeWindow(Frames, Glass, 405.0f, 235.0f, FVector(180.0f, 28.0f, 230.0f), FVector(144.0f, 9.0f, 192.0f));
    for (const float X : { -405.0f, 0.0f, 405.0f })
    {
        AddFacadeWindow(Frames, Glass, X, 565.0f, FVector(180.0f, 28.0f, 250.0f), FVector(144.0f, 9.0f, 210.0f));
    }

    // Pediment outline and shallow roof mass. The triangle is expressed with real sloped members instead of a fake text/UI plane.
    AddBox(Trim, FVector(0.0f, -930.0f, 892.0f), FVector(1610.0f, 245.0f, 36.0f));
    AddBoxRotated(Trim, FVector(-385.0f, -930.0f, 1000.0f), FVector(830.0f, 210.0f, 38.0f),
        FRotator(-15.5f, 0.0f, 0.0f));
    AddBoxRotated(Trim, FVector(385.0f, -930.0f, 1000.0f), FVector(830.0f, 210.0f, 38.0f),
        FRotator(15.5f, 0.0f, 0.0f));
    AddBox(Roof, FVector(0.0f, 80.0f, 805.0f), FVector(4040.0f, 1640.0f, 70.0f));
    AddBoxRotated(Roof, FVector(0.0f, -385.0f, 858.0f), FVector(3980.0f, 820.0f, 58.0f),
        FRotator(0.0f, 0.0f, -9.0f));
    AddBoxRotated(Roof, FVector(0.0f, 385.0f, 858.0f), FVector(3980.0f, 820.0f, 58.0f),
        FRotator(0.0f, 0.0f, 9.0f));

    // Front terrace, shallow stair run and civic-sidewalk apron visible in both reference eras.
    AddBox(Ground, FVector(0.0f, -1160.0f, 12.0f), FVector(2300.0f, 780.0f, 24.0f));
    AddBox(Stonework, FVector(0.0f, -1110.0f, 58.0f), FVector(980.0f, 330.0f, 42.0f));
    AddBox(Stonework, FVector(0.0f, -1270.0f, 38.0f), FVector(1120.0f, 210.0f, 34.0f));
    AddBox(Stonework, FVector(0.0f, -1390.0f, 21.0f), FVector(1260.0f, 160.0f, 24.0f));

    UE_LOG(LogTemp, Display,
        TEXT("R13 Oster City Council built at verified Nezalezhnosti 21 geo anchor (%.1f, %.1f); photo-driven two-storey facade, four-column portico, pediment and authoritative collision active; parcel yaw remains provisional."),
        CityCouncilAnchor().X, CityCouncilAnchor().Y);
}
