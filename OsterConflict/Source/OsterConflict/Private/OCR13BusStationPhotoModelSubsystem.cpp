#include "OCR13BusStationPhotoModelSubsystem.h"

#include "OCGameMode.h"
#include "OCGeoReference.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"

namespace
{
    constexpr float BusStationDelaySeconds = 2.34f;
    // Public sources confirm the station site and historical/current photo existence, but the exact parcel yaw has
    // not yet been surveyed. Keep a single explicit value rather than baking inferred rotation into every detail.
    constexpr float BusStationYawDegrees = 0.0f;

    FVector BusStationAnchor()
    {
        const FOCGeoReferencePoint Ref = FOCGeoReference::BusStation();
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
        Component->SetCullDistances(0, 90000);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void AddBox(UInstancedStaticMeshComponent* Component, const FVector& Center, const FVector& SizeCm)
    {
        if (!Component) return;
        Component->AddInstance(FTransform(FRotator::ZeroRotator, Center, SizeCm / 100.0f), false);
    }

    void AddFittedPlank(UInstancedStaticMeshComponent* Component, UStaticMesh* Mesh,
        const FVector& Center, const FVector& DesiredSizeCm)
    {
        if (!Component || !Mesh) return;
        const FVector NativeSize = Mesh->GetBounds().BoxExtent * 2.0f;
        if (NativeSize.X <= 1.0f || NativeSize.Y <= 1.0f || NativeSize.Z <= 1.0f) return;
        const FVector Scale(DesiredSizeCm.X / NativeSize.X, DesiredSizeCm.Y / NativeSize.Y, DesiredSizeCm.Z / NativeSize.Z);
        Component->AddInstance(FTransform(FRotator::ZeroRotator,
            Center - Mesh->GetBounds().Origin * Scale, Scale), false);
    }
}

bool UOCR13BusStationPhotoModelSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13BusStationPhotoModelSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (InWorld.GetNetMode() == NM_DedicatedServer) return;
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
            if (UWorld* World = WeakWorld.Get()) BuildBusStation(*World);
        }), BusStationDelaySeconds, false);
}

void UOCR13BusStationPhotoModelSubsystem::BuildBusStation(UWorld& World)
{
    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        if (AActor* Existing = *It; Existing && Existing->ActorHasTag(TEXT("R13_BusStationPhotoModel"))) return;
    }

    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UStaticMesh* Plank = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Old_Planks_Plank_1.Old_Planks_Plank_1"));
    UMaterialInterface* Basic = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    UMaterialInterface* GlassMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Glass_Window.Glass_Window"));
    if (!Cube || !Basic) return;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = TEXT("R13_OsterBusStation");
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AActor* Model = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, SpawnParams);
    if (!Model) return;
    Model->SetReplicates(false);
    Model->SetActorEnableCollision(true);
    Model->Tags.Add(TEXT("R13_BusStationPhotoModel"));

    USceneComponent* Root = NewObject<USceneComponent>(Model, TEXT("R13_BusStationRoot"));
    if (!Root)
    {
        Model->Destroy();
        return;
    }
    Root->SetMobility(EComponentMobility::Static);
    Model->SetRootComponent(Root);
    Model->AddInstanceComponent(Root);
    Root->RegisterComponent();
    Model->SetActorLocationAndRotation(BusStationAnchor(), FRotator(0.0f, BusStationYawDegrees, 0.0f));

    UMaterialInstanceDynamic* Wall = MakeColor(Model, Basic, TEXT("R13_BusStationWall"),
        FLinearColor(0.55f, 0.52f, 0.43f, 1.0f));
    UMaterialInstanceDynamic* WallLight = MakeColor(Model, Basic, TEXT("R13_BusStationWallLight"),
        FLinearColor(0.72f, 0.70f, 0.62f, 1.0f));
    UMaterialInstanceDynamic* Dark = MakeColor(Model, Basic, TEXT("R13_BusStationDark"),
        FLinearColor(0.10f, 0.11f, 0.11f, 1.0f));
    UMaterialInstanceDynamic* Concrete = MakeColor(Model, Basic, TEXT("R13_BusStationConcrete"),
        FLinearColor(0.30f, 0.30f, 0.28f, 1.0f));
    UMaterialInstanceDynamic* Asphalt = MakeColor(Model, Basic, TEXT("R13_BusStationAsphalt"),
        FLinearColor(0.065f, 0.068f, 0.067f, 1.0f));
    UMaterialInstanceDynamic* White = MakeColor(Model, Basic, TEXT("R13_BusStationWhite"),
        FLinearColor(0.83f, 0.83f, 0.78f, 1.0f));

    UInstancedStaticMeshComponent* Shell = MakeISM(Model, Root, Cube, Wall, TEXT("R13_BusStationShell"), true);
    UInstancedStaticMeshComponent* Trim = MakeISM(Model, Root, Cube, WallLight, TEXT("R13_BusStationTrim"), false);
    UInstancedStaticMeshComponent* DarkParts = MakeISM(Model, Root, Cube, Dark, TEXT("R13_BusStationDarkParts"), false);
    UInstancedStaticMeshComponent* Glass = MakeISM(Model, Root, Cube, GlassMaterial, TEXT("R13_BusStationGlass"), false);
    UInstancedStaticMeshComponent* ConcreteParts = MakeISM(Model, Root, Cube, Concrete, TEXT("R13_BusStationConcrete"), true);
    UInstancedStaticMeshComponent* Forecourt = MakeISM(Model, Root, Cube, Asphalt, TEXT("R13_BusStationForecourt"), true, false);
    UInstancedStaticMeshComponent* Markings = MakeISM(Model, Root, Cube, White, TEXT("R13_BusStationBayMarkings"), false, false);
    UInstancedStaticMeshComponent* BenchWood = MakeISM(Model, Root, Plank, nullptr, TEXT("R13_BusStationBenchWood"), false);

    // Conservative low regional-station mass. The historical and 2009 references establish a small bus-station site,
    // not a large modern terminal, so the landmark deliberately stays one storey and modest in scale.
    AddBox(Shell, FVector(0.0f, 180.0f, 190.0f), FVector(2300.0f, 820.0f, 380.0f));
    AddBox(Shell, FVector(-760.0f, -310.0f, 155.0f), FVector(780.0f, 220.0f, 310.0f));
    AddBox(Trim, FVector(0.0f, -238.0f, 385.0f), FVector(2360.0f, 28.0f, 44.0f));
    AddBox(Trim, FVector(0.0f, 600.0f, 385.0f), FVector(2360.0f, 28.0f, 44.0f));

    // Front waiting-room glazing: several narrow bays plus a central double entrance rather than one giant glass wall.
    const float WindowXs[] = { -850.0f, -560.0f, 410.0f, 700.0f, 990.0f };
    for (const float X : WindowXs)
    {
        AddBox(DarkParts, FVector(X, -244.0f, 195.0f), FVector(218.0f, 14.0f, 238.0f));
        AddBox(Glass, FVector(X, -253.0f, 195.0f), FVector(190.0f, 7.0f, 210.0f));
    }
    for (const float DoorX : { -160.0f, 75.0f })
    {
        AddBox(DarkParts, FVector(DoorX, -250.0f, 150.0f), FVector(205.0f, 18.0f, 300.0f));
        AddBox(Glass, FVector(DoorX, -261.0f, 155.0f), FVector(175.0f, 7.0f, 265.0f));
    }

    // Shallow covered waiting edge and robust columns. No font-rendered sign here: the current TextRender path already
    // demonstrated visible shimmer at Silpo, so signage remains a blank physical plate until a proper texture/decal exists.
    AddBox(ConcreteParts, FVector(70.0f, -570.0f, 337.0f), FVector(2550.0f, 620.0f, 34.0f));
    for (const float X : { -1010.0f, -480.0f, 50.0f, 580.0f, 1110.0f })
    {
        AddBox(ConcreteParts, FVector(X, -640.0f, 168.0f), FVector(26.0f, 26.0f, 336.0f));
    }
    AddBox(Trim, FVector(-650.0f, -292.0f, 455.0f), FVector(980.0f, 24.0f, 105.0f));

    // Passenger platform and compact bus manoeuvring apron. This is intentionally bounded so it cannot swallow nearby roads.
    AddBox(ConcreteParts, FVector(80.0f, -760.0f, 9.0f), FVector(2760.0f, 520.0f, 18.0f));
    AddBox(Forecourt, FVector(80.0f, -1500.0f, 4.0f), FVector(3300.0f, 1050.0f, 8.0f));
    for (const float X : { -1120.0f, -380.0f, 360.0f, 1100.0f })
    {
        AddBox(Markings, FVector(X, -1540.0f, 10.0f), FVector(12.0f, 790.0f, 4.0f));
    }
    AddBox(Markings, FVector(0.0f, -1930.0f, 10.0f), FVector(2400.0f, 12.0f, 4.0f));

    // Two restrained waiting benches. Use the existing old-plank asset when available; otherwise the station still builds.
    if (BenchWood && Plank)
    {
        AddFittedPlank(BenchWood, Plank, FVector(-520.0f, -735.0f, 52.0f), FVector(210.0f, 38.0f, 7.0f));
        AddFittedPlank(BenchWood, Plank, FVector(420.0f, -735.0f, 52.0f), FVector(210.0f, 38.0f, 7.0f));
    }
    for (const float X : { -610.0f, -430.0f, 330.0f, 510.0f })
    {
        AddBox(DarkParts, FVector(X, -735.0f, 25.0f), FVector(8.0f, 32.0f, 50.0f));
    }

    UE_LOG(LogTemp, Display,
        TEXT("R13 Oster bus station built at verified geo anchor (%.1f, %.1f); one-storey photo-informed model and compact forecourt active, parcel yaw remains explicit provisional value."),
        BusStationAnchor().X, BusStationAnchor().Y);
}
