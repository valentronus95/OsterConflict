#include "OCEnterableHouse.h"

#include "OCBreakableWindow.h"
#include "OCInteractableLight.h"
#include "OCInteractableGate.h"
#include "OCInteractableDoor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

AOCEnterableHouse::AOCEnterableHouse()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetReplicateMovement(false);

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    Shell = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Shell"));
    Shell->SetupAttachment(SceneRoot);
    Shell->SetCollisionProfileName(TEXT("BlockAll"));

    Interior = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Interior"));
    Interior->SetupAttachment(SceneRoot);
    Interior->SetCollisionProfileName(TEXT("BlockAll"));

    HouseholdFurniture = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("HouseholdFurniture"));
    HouseholdFurniture->SetupAttachment(SceneRoot);
    HouseholdFurniture->SetCollisionProfileName(TEXT("BlockAll"));

    HouseholdElectronics = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("HouseholdElectronics"));
    HouseholdElectronics->SetupAttachment(SceneRoot);
    HouseholdElectronics->SetCollisionProfileName(TEXT("BlockAll"));

    HouseholdClutter = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("HouseholdClutter"));
    HouseholdClutter->SetupAttachment(SceneRoot);
    HouseholdClutter->SetCollisionProfileName(TEXT("NoCollision"));

    YardFences = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("YardFences"));
    YardFences->SetupAttachment(SceneRoot);
    YardFences->SetCollisionProfileName(TEXT("BlockAll"));

    YardPaths = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("YardPaths"));
    YardPaths->SetupAttachment(SceneRoot);
    YardPaths->SetCollisionProfileName(TEXT("BlockAll"));

    DebugLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("DebugLabel"));
    DebugLabel->SetupAttachment(SceneRoot);
    DebugLabel->SetText(FText::FromString(TEXT("S08 ENTERABLE HOUSE")));
    DebugLabel->SetRelativeLocation(FVector(0.0f, 0.0f, 470.0f));
    DebugLabel->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
    DebugLabel->SetHorizontalAlignment(EHTA_Center);
    DebugLabel->SetWorldSize(65.0f);
    DebugLabel->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        Shell->SetStaticMesh(CubeMesh.Object);
        Interior->SetStaticMesh(CubeMesh.Object);
        HouseholdFurniture->SetStaticMesh(CubeMesh.Object);
        HouseholdElectronics->SetStaticMesh(CubeMesh.Object);
        HouseholdClutter->SetStaticMesh(CubeMesh.Object);
        YardFences->SetStaticMesh(CubeMesh.Object);
        YardPaths->SetStaticMesh(CubeMesh.Object);
    }

    BuildShell();
    BuildInterior();
    BuildHouseholdProps();
    BuildYard();
}

void AOCEnterableHouse::BeginPlay()
{
    Super::BeginPlay();
    if (HasAuthority())
    {
        SpawnInteractiveOpeningsServer();
    }
}

void AOCEnterableHouse::AddBox(UInstancedStaticMeshComponent* Component, const FVector& Center,
    const FVector& SizeCm, float YawDegrees)
{
    if (!Component)
    {
        return;
    }

    FTransform Transform;
    Transform.SetLocation(Center);
    Transform.SetRotation(FQuat(FRotator(0.0f, YawDegrees, 0.0f)));
    Transform.SetScale3D(SizeCm / 100.0f);
    Component->AddInstance(Transform);
}

void AOCEnterableHouse::BuildShell()
{
    constexpr float WallThickness = 25.0f;
    constexpr float WallHeight = 320.0f;
    constexpr float FrontY = -550.0f;
    constexpr float BackY = 550.0f;
    constexpr float LeftX = -800.0f;
    constexpr float RightX = 800.0f;

    // Floor and flat prototype roof/ceiling.
    AddBox(Shell, FVector(0.0f, 0.0f, 10.0f), FVector(1600.0f, 1100.0f, 20.0f));
    AddBox(Shell, FVector(0.0f, 0.0f, 332.0f), FVector(1700.0f, 1200.0f, 24.0f));

    // Front wall. Real openings: 130 cm door and 200 cm window.
    AddBox(Shell, FVector(-725.0f, FrontY, 160.0f), FVector(150.0f, WallThickness, WallHeight));
    AddBox(Shell, FVector(-135.0f, FrontY, 160.0f), FVector(770.0f, WallThickness, WallHeight));
    AddBox(Shell, FVector(625.0f, FrontY, 160.0f), FVector(350.0f, WallThickness, WallHeight));
    AddBox(Shell, FVector(-585.0f, FrontY, 277.5f), FVector(130.0f, WallThickness, 85.0f));
    AddBox(Shell, FVector(350.0f, FrontY, 45.0f), FVector(200.0f, WallThickness, 90.0f));
    AddBox(Shell, FVector(350.0f, FrontY, 282.5f), FVector(200.0f, WallThickness, 75.0f));

    // Back wall with one true window opening.
    AddBox(Shell, FVector(-580.0f, BackY, 160.0f), FVector(440.0f, WallThickness, WallHeight));
    AddBox(Shell, FVector(330.0f, BackY, 160.0f), FVector(940.0f, WallThickness, WallHeight));
    AddBox(Shell, FVector(-250.0f, BackY, 45.0f), FVector(220.0f, WallThickness, 90.0f));
    AddBox(Shell, FVector(-250.0f, BackY, 282.5f), FVector(220.0f, WallThickness, 75.0f));

    // Left wall remains solid in the first pass.
    AddBox(Shell, FVector(LeftX, 0.0f, 160.0f), FVector(WallThickness, 1100.0f, WallHeight));

    // Right wall with side window opening around Y=170 cm.
    AddBox(Shell, FVector(RightX, -235.0f, 160.0f), FVector(WallThickness, 630.0f, WallHeight));
    AddBox(Shell, FVector(RightX, 415.0f, 160.0f), FVector(WallThickness, 270.0f, WallHeight));
    AddBox(Shell, FVector(RightX, 180.0f, 45.0f), FVector(WallThickness, 200.0f, 90.0f));
    AddBox(Shell, FVector(RightX, 180.0f, 282.5f), FVector(WallThickness, 200.0f, 75.0f));
}

void AOCEnterableHouse::BuildInterior()
{
    // One interior partition with a 140 cm doorway. This makes room-to-room fighting testable.
    AddBox(Interior, FVector(-435.0f, 100.0f, 150.0f), FVector(730.0f, 18.0f, 300.0f));
    AddBox(Interior, FVector(435.0f, 100.0f, 150.0f), FVector(730.0f, 18.0f, 300.0f));
    AddBox(Interior, FVector(0.0f, 100.0f, 272.5f), FVector(140.0f, 18.0f, 55.0f));

}


void AOCEnterableHouse::AddChair(UInstancedStaticMeshComponent* Component, const FVector& Center, float YawDegrees)
{
    AddBox(Component, Center + FVector(0,0,45), FVector(48,48,8), YawDegrees);
    AddBox(Component, Center + FRotator(0,YawDegrees,0).RotateVector(FVector(-20,0,82)), FVector(8,48,80), YawDegrees);
    for (float X : {-18.0f,18.0f}) for (float Y : {-18.0f,18.0f})
        AddBox(Component, Center + FRotator(0,YawDegrees,0).RotateVector(FVector(X,Y,22)), FVector(6,6,44), YawDegrees);
}

void AOCEnterableHouse::AddTable(UInstancedStaticMeshComponent* Component, const FVector& Center, const FVector2D& Size, float YawDegrees)
{
    AddBox(Component, Center + FVector(0,0,76), FVector(Size.X,Size.Y,8), YawDegrees);
    for (float X : {-Size.X*0.42f,Size.X*0.42f}) for (float Y : {-Size.Y*0.38f,Size.Y*0.38f})
        AddBox(Component, Center + FRotator(0,YawDegrees,0).RotateVector(FVector(X,Y,38)), FVector(8,8,76), YawDegrees);
}

void AOCEnterableHouse::AddSofa(UInstancedStaticMeshComponent* Component, const FVector& Center, float YawDegrees, float WidthCm)
{
    AddBox(Component, Center + FVector(0,0,34), FVector(WidthCm,82,42), YawDegrees);
    AddBox(Component, Center + FRotator(0,YawDegrees,0).RotateVector(FVector(0,34,78)), FVector(WidthCm,18,88), YawDegrees);
    AddBox(Component, Center + FRotator(0,YawDegrees,0).RotateVector(FVector(-WidthCm*0.48f,0,55)), FVector(16,82,70), YawDegrees);
    AddBox(Component, Center + FRotator(0,YawDegrees,0).RotateVector(FVector( WidthCm*0.48f,0,55)), FVector(16,82,70), YawDegrees);
}

void AOCEnterableHouse::BuildHouseholdProps()
{
    if (!HouseholdFurniture || !HouseholdElectronics || !HouseholdClutter) return;
    FRandomStream R(InteriorSeed + LayoutVariant * 7919);
    const float WearYaw = HouseCondition == EOCHouseCondition::Worn ? 8.0f : HouseCondition == EOCHouseCondition::Ordinary ? 3.5f : 1.0f;

    // Living area: ordinary inexpensive sofa, small table/TV cabinet and optional low storage.
    AddSofa(HouseholdFurniture, FVector(-520, -250, 0), -90.0f + R.FRandRange(-WearYaw, WearYaw), 270.0f + R.FRandRange(-25, 20));
    AddBox(HouseholdFurniture, FVector(-170,-275,42), FVector(150,55,84), R.FRandRange(-WearYaw,WearYaw));
    AddBox(HouseholdElectronics, FVector(-170,-275,105), FVector(100,12,58), R.FRandRange(-WearYaw,WearYaw)); // TV/monitor proxy

    // Dining table and mismatched chairs. Worn houses intentionally miss/rotate one chair.
    AddTable(HouseholdFurniture, FVector(300,-245,0), FVector2D(180,90), R.FRandRange(-WearYaw,WearYaw));
    AddChair(HouseholdFurniture, FVector(300,-340,0), 0 + R.FRandRange(-WearYaw,WearYaw));
    AddChair(HouseholdFurniture, FVector(300,-150,0), 180 + R.FRandRange(-WearYaw,WearYaw));
    AddChair(HouseholdFurniture, FVector(190,-245,0), 90 + R.FRandRange(-WearYaw,WearYaw));
    if (HouseCondition != EOCHouseCondition::Worn || (InteriorSeed % 2)==0)
        AddChair(HouseholdFurniture, FVector(410,-245,0), -90 + R.FRandRange(-WearYaw,WearYaw));

    // Kitchen: worktop segments, stove and fridge. Nothing is interactive in S14A.
    AddBox(HouseholdFurniture, FVector(500,430,48), FVector(450,65,96), 0);
    AddBox(HouseholdFurniture, FVector(670,365,48), FVector(65,190,96), 0);
    AddBox(HouseholdFurniture, FVector(700,500,95), FVector(75,75,190), R.FRandRange(-2,2)); // fridge
    AddBox(HouseholdElectronics, FVector(440,430,103), FVector(72,62,18), 0); // stove/hob proxy

    // Work corner: cheap desk, monitor, PC tower and laptop. Geometry only, no gameplay interaction.
    AddTable(HouseholdFurniture, FVector(-470,380,0), FVector2D(200,75), 0 + R.FRandRange(-WearYaw,WearYaw));
    AddBox(HouseholdElectronics, FVector(-470,395,112), FVector(75,10,52), R.FRandRange(-3,3));
    AddBox(HouseholdElectronics, FVector(-545,400,45), FVector(35,48,90), 0); // PC tower
    AddBox(HouseholdElectronics, FVector(-390,365,88), FVector(52,36,4), R.FRandRange(-8,8)); // laptop
    AddChair(HouseholdFurniture, FVector(-470,285,0), 0 + R.FRandRange(-WearYaw,WearYaw));

    // Storage/furniture with condition-dependent irregularity.
    AddBox(HouseholdFurniture, FVector(-710,360,95), FVector(110,62,190), R.FRandRange(-WearYaw,WearYaw)); // wardrobe
    AddBox(HouseholdFurniture, FVector(600,110,55), FVector(280,55,110), R.FRandRange(-WearYaw,WearYaw)); // low cabinet

    // Low-cost household clutter proxies: boxes, bags/books, small appliances. Cosmetic, no collision.
    const int32 ClutterCount = HouseCondition == EOCHouseCondition::Worn ? 16 : HouseCondition == EOCHouseCondition::Ordinary ? 10 : 6;
    for (int32 I=0; I<ClutterCount; ++I)
    {
        const FVector P(R.FRandRange(-690,690), R.FRandRange(-430,470), R.FRandRange(6,28));
        const FVector S(R.FRandRange(12,45), R.FRandRange(12,55), R.FRandRange(8,35));
        AddBox(HouseholdClutter, P, S, R.FRandRange(-25,25));
    }
}

void AOCEnterableHouse::ConfigureInteriorVariantServer(int32 NewSeed, EOCHouseCondition NewCondition, int32 NewLayoutVariant)
{
    if (!HasAuthority()) return;
    InteriorSeed = NewSeed;
    HouseCondition = NewCondition;
    LayoutVariant = FMath::Clamp(NewLayoutVariant, 0, 5);
    HouseholdFurniture->ClearInstances();
    HouseholdElectronics->ClearInstances();
    HouseholdClutter->ClearInstances();
    BuildHouseholdProps();
}

void AOCEnterableHouse::BuildYard()
{
    // Front fence has a pedestrian opening aligned with the front door path.
    AddBox(YardFences, FVector(-1450.0f, -1750.0f, 85.0f), FVector(1050.0f, 35.0f, 170.0f));
    AddBox(YardFences, FVector(850.0f, -1750.0f, 85.0f), FVector(2500.0f, 35.0f, 170.0f));
    AddBox(YardFences, FVector(0.0f, 1750.0f, 85.0f), FVector(4000.0f, 35.0f, 170.0f));
    AddBox(YardFences, FVector(-2000.0f, 0.0f, 85.0f), FVector(35.0f, 3500.0f, 170.0f));
    AddBox(YardFences, FVector(2000.0f, 0.0f, 85.0f), FVector(35.0f, 3500.0f, 170.0f));

    // Path from the gate to the front door and a small backyard shed.
    AddBox(YardPaths, FVector(-585.0f, -1150.0f, 5.0f), FVector(170.0f, 1200.0f, 10.0f));
    AddBox(Interior, FVector(1250.0f, 1100.0f, 150.0f), FVector(700.0f, 600.0f, 300.0f));
}

FTransform AOCEnterableHouse::MakeWorldTransform(const FVector& LocalLocation, float LocalYawDegrees) const
{
    const FVector WorldLocation = GetActorTransform().TransformPosition(LocalLocation);
    const FRotator WorldRotation(0.0f, GetActorRotation().Yaw + LocalYawDegrees, 0.0f);
    return FTransform(WorldRotation, WorldLocation);
}

void AOCEnterableHouse::SpawnInteractiveOpeningsServer()
{
    if (!HasAuthority() || !GetWorld())
    {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // Door root is its left hinge. It fills the actual opening in the front shell.
    GetWorld()->SpawnActor<AOCInteractableDoor>(AOCInteractableDoor::StaticClass(),
        MakeWorldTransform(FVector(-650.0f, -562.5f, 0.0f)), SpawnParams);

    struct FWindowSeed
    {
        FVector LocalLocation;
        float LocalYaw;
    };

    const FWindowSeed Windows[] =
    {
        { FVector(350.0f, -562.5f, 167.5f), 0.0f },
        { FVector(-250.0f, 562.5f, 167.5f), 0.0f },
        { FVector(812.5f, 180.0f, 167.5f), 90.0f }
    };

    for (const FWindowSeed& Seed : Windows)
    {
        GetWorld()->SpawnActor<AOCBreakableWindow>(AOCBreakableWindow::StaticClass(),
            MakeWorldTransform(Seed.LocalLocation, Seed.LocalYaw), SpawnParams);
    }

    // S12 Sandbox interaction coverage: real replicated light + yard gate.
    GetWorld()->SpawnActor<AOCInteractableLight>(AOCInteractableLight::StaticClass(),
        MakeWorldTransform(FVector(0.0f, -50.0f, 300.0f)), SpawnParams);
    GetWorld()->SpawnActor<AOCInteractableGate>(AOCInteractableGate::StaticClass(),
        MakeWorldTransform(FVector(-585.0f, -1750.0f, 0.0f)), SpawnParams);
}
