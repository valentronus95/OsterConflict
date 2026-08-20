#include "OCR13BusStationPhotoModelSubsystem.h"

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
    constexpr float BusStationDelaySeconds = 2.34f;
    constexpr float BusStationYawDegrees = 0.0f;

    FVector BusStationAnchor()
    {
        // Verified POI: Oster bus station, Bohdana Khmelnytskoho Street 75.
        return OCGeoReference::FromLatLon(50.948122, 30.881425) + FVector(0.0f, 0.0f, 4.0f);
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
        Component->SetMobility(EComponentMobility::Static);
        Component->SetStaticMesh(Mesh);
        if (Material) Component->SetMaterial(0, Material);
        Component->SetCollisionEnabled(bCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
        Component->SetCollisionResponseToAllChannels(bCollision ? ECR_Block : ECR_Ignore);
        Component->SetCastShadow(bShadow);
        Component->SetCanEverAffectNavigation(bCollision);
        Component->RegisterComponent();
        Owner->AddInstanceComponent(Component);
        return Component;
    }

    void AddBox(UInstancedStaticMeshComponent* Component, const FVector& CenterCm, const FVector& SizeCm,
        const float YawDegrees = BusStationYawDegrees)
    {
        if (!Component) return;
        Component->AddInstance(FTransform(
            FRotator(0.0f, YawDegrees, 0.0f),
            CenterCm,
            FVector(SizeCm.X / 100.0f, SizeCm.Y / 100.0f, SizeCm.Z / 100.0f)));
    }

    AActor* FindNamedActor(UWorld& World, const FName Name)
    {
        for (TActorIterator<AActor> It(&World); It; ++It)
        {
            if (IsValid(*It) && It->GetFName() == Name) return *It;
        }
        return nullptr;
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
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (AOCGameMode* GM = InWorld.GetAuthGameMode<AOCGameMode>(); GM && GM->IsFrontendOnlySession()) return;

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    InWorld.GetTimerManager().SetTimer(BuildTimer, FTimerDelegate::CreateLambda([this, WeakWorld]()
    {
        if (UWorld* World = WeakWorld.Get()) BuildBusStation(*World);
    }), BusStationDelaySeconds, false);
}

void UOCR13BusStationPhotoModelSubsystem::BuildBusStation(UWorld& World)
{
    if (FindNamedActor(World, TEXT("R13_OsterBusStation"))) return;

    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UMaterialInterface* Basic = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Visual/Materials/M_OCBasicColor.M_OCBasicColor"));
    if (!Cube || !Basic)
    {
        UE_LOG(LogTemp, Warning, TEXT("R13 bus station: missing cube/basic-color material."));
        return;
    }

    AActor* Model = World.SpawnActor<AActor>(AActor::StaticClass(), BusStationAnchor(), FRotator::ZeroRotator);
    if (!Model) return;
    Model->SetActorLabel(TEXT("R13_OsterBusStation"));
    Model->Rename(TEXT("R13_OsterBusStation"));

    USceneComponent* Root = NewObject<USceneComponent>(Model, TEXT("R13_BusStationRoot"));
    if (!Root)
    {
        Model->Destroy();
        return;
    }
    Model->SetRootComponent(Root);
    Root->RegisterComponent();
    Model->AddInstanceComponent(Root);

    UMaterialInstanceDynamic* Wall = MakeColor(Model, Basic, TEXT("R13_BusStationWall"),
        FLinearColor(0.23f, 0.17f, 0.12f, 1.0f));
    UMaterialInstanceDynamic* WallLight = MakeColor(Model, Basic, TEXT("R13_BusStationWallLight"),
        FLinearColor(0.54f, 0.44f, 0.30f, 1.0f));
    UMaterialInstanceDynamic* Dark = MakeColor(Model, Basic, TEXT("R13_BusStationDark"),
        FLinearColor(0.035f, 0.040f, 0.042f, 1.0f));
    UMaterialInstanceDynamic* Concrete = MakeColor(Model, Basic, TEXT("R13_BusStationConcreteMaterial"),
        FLinearColor(0.34f, 0.34f, 0.33f, 1.0f));
    UMaterialInstanceDynamic* Asphalt = MakeColor(Model, Basic, TEXT("R13_BusStationAsphalt"),
        FLinearColor(0.12f, 0.12f, 0.115f, 1.0f));
    UMaterialInstanceDynamic* White = MakeColor(Model, Basic, TEXT("R13_BusStationWhite"),
        FLinearColor(0.79f, 0.78f, 0.70f, 1.0f));

    UInstancedStaticMeshComponent* Shell = MakeISM(Model, Root, Cube, Wall, TEXT("R13_BusStationShell"), true);
    UInstancedStaticMeshComponent* Trim = MakeISM(Model, Root, Cube, WallLight, TEXT("R13_BusStationTrim"), false);
    UInstancedStaticMeshComponent* Windows = MakeISM(Model, Root, Cube, Dark, TEXT("R13_BusStationWindows"), false);
    UInstancedStaticMeshComponent* ConcreteParts = MakeISM(Model, Root, Cube, Concrete, TEXT("R13_BusStationConcrete"), true);
    UInstancedStaticMeshComponent* AsphaltParts = MakeISM(Model, Root, Cube, Asphalt, TEXT("R13_BusStationAsphaltParts"), true, false);
    UInstancedStaticMeshComponent* WhiteParts = MakeISM(Model, Root, Cube, White, TEXT("R13_BusStationWhiteParts"), false);

    // Compact one-storey roadside terminal based on the verified bus-station photo massing.
    AddBox(Shell, FVector(0.0f, 0.0f, 170.0f), FVector(2250.0f, 980.0f, 340.0f));
    AddBox(Shell, FVector(-620.0f, -30.0f, 360.0f), FVector(650.0f, 880.0f, 115.0f));
    AddBox(Shell, FVector(520.0f, -45.0f, 360.0f), FVector(720.0f, 850.0f, 115.0f));

    // Shallow front canopy and concrete apron.
    AddBox(ConcreteParts, FVector(0.0f, 575.0f, -2.0f), FVector(2700.0f, 900.0f, 18.0f));
    AddBox(ConcreteParts, FVector(0.0f, 505.0f, 310.0f), FVector(1500.0f, 150.0f, 22.0f));
    AddBox(ConcreteParts, FVector(-660.0f, 470.0f, 160.0f), FVector(22.0f, 22.0f, 300.0f));
    AddBox(ConcreteParts, FVector(660.0f, 470.0f, 160.0f), FVector(22.0f, 22.0f, 300.0f));

    // Front openings and contrasting facade bands.
    AddBox(Windows, FVector(-720.0f, 497.0f, 175.0f), FVector(360.0f, 14.0f, 190.0f));
    AddBox(Windows, FVector(-250.0f, 497.0f, 175.0f), FVector(300.0f, 14.0f, 190.0f));
    AddBox(Windows, FVector(220.0f, 497.0f, 175.0f), FVector(300.0f, 14.0f, 190.0f));
    AddBox(Windows, FVector(690.0f, 497.0f, 175.0f), FVector(360.0f, 14.0f, 190.0f));
    AddBox(Trim, FVector(0.0f, 504.0f, 325.0f), FVector(2200.0f, 18.0f, 34.0f));
    AddBox(Trim, FVector(0.0f, 503.0f, 70.0f), FVector(2200.0f, 16.0f, 24.0f));

    // Central entrance.
    AddBox(Dark ? Windows : Shell, FVector(0.0f, 503.0f, 120.0f), FVector(210.0f, 20.0f, 240.0f));
    AddBox(WhiteParts, FVector(-117.0f, 513.0f, 120.0f), FVector(14.0f, 18.0f, 240.0f));
    AddBox(WhiteParts, FVector(117.0f, 513.0f, 120.0f), FVector(14.0f, 18.0f, 240.0f));
    AddBox(WhiteParts, FVector(0.0f, 513.0f, 242.0f), FVector(250.0f, 18.0f, 14.0f));

    // Roadside pull-in and lane markings in front of the terminal.
    AddBox(AsphaltParts, FVector(0.0f, 1450.0f, -7.0f), FVector(4200.0f, 1400.0f, 14.0f));
    for (int32 Index = -3; Index <= 3; ++Index)
    {
        AddBox(WhiteParts, FVector(static_cast<float>(Index) * 520.0f, 1450.0f, 2.0f), FVector(220.0f, 12.0f, 3.0f));
    }

    UE_LOG(LogTemp, Display, TEXT("R13 bus station photo model: built at verified POI anchor."));
}
