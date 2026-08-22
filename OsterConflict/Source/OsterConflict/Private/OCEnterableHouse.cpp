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

namespace
{
    UInstancedStaticMeshComponent* MakeHousePropISM(AOCEnterableHouse* Owner, USceneComponent* Root, const TCHAR* Name)
    {
        UInstancedStaticMeshComponent* Component = Owner->CreateDefaultSubobject<UInstancedStaticMeshComponent>(FName(Name));
        Component->SetupAttachment(Root);
        Component->SetCollisionProfileName(TEXT("NoCollision"));
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        return Component;
    }
}

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

    // Interior dressing is a visual/model concern in this pass. Shell/interior geometry still owns
    // house collision; decorative furniture must not create invisible blocking volumes while its
    // production model is being calibrated.
    HouseholdFurniture = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("HouseholdFurniture"));
    HouseholdFurniture->SetupAttachment(SceneRoot);
    HouseholdFurniture->SetCollisionProfileName(TEXT("NoCollision"));

    HouseholdElectronics = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("HouseholdElectronics"));
    HouseholdElectronics->SetupAttachment(SceneRoot);
    HouseholdElectronics->SetCollisionProfileName(TEXT("NoCollision"));

    HouseholdClutter = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("HouseholdClutter"));
    HouseholdClutter->SetupAttachment(SceneRoot);
    HouseholdClutter->SetCollisionProfileName(TEXT("NoCollision"));

    YardFences = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("YardFences"));
    YardFences->SetupAttachment(SceneRoot);
    YardFences->SetCollisionProfileName(TEXT("BlockAll"));

    YardPaths = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("YardPaths"));
    YardPaths->SetupAttachment(SceneRoot);
    YardPaths->SetCollisionProfileName(TEXT("BlockAll"));

    RealSofa = MakeHousePropISM(this, SceneRoot, TEXT("RealSofa"));
    RealTable = MakeHousePropISM(this, SceneRoot, TEXT("RealTable"));
    RealPlasticChair = MakeHousePropISM(this, SceneRoot, TEXT("RealPlasticChair"));
    RealOfficeChair = MakeHousePropISM(this, SceneRoot, TEXT("RealOfficeChair"));
    RealFridge = MakeHousePropISM(this, SceneRoot, TEXT("RealFridge"));
    RealCrate = MakeHousePropISM(this, SceneRoot, TEXT("RealCrate"));
    RealMetalBarrel = MakeHousePropISM(this, SceneRoot, TEXT("RealMetalBarrel"));
    RealWheelBarrow = MakeHousePropISM(this, SceneRoot, TEXT("RealWheelBarrow"));
    RealYardFence = MakeHousePropISM(this, SceneRoot, TEXT("RealYardFence"));
    RealSideShed = MakeHousePropISM(this, SceneRoot, TEXT("RealSideShed"));

    DebugLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("DebugLabel"));
    DebugLabel->SetupAttachment(SceneRoot);
    DebugLabel->SetText(FText::FromString(TEXT("S08 ENTERABLE HOUSE")));
    DebugLabel->SetRelativeLocation(FVector(0.0f, 0.0f, 470.0f));
    DebugLabel->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
    DebugLabel->SetHorizontalAlignment(EHTA_Center);
    DebugLabel->SetWorldSize(65.0f);
    DebugLabel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    DebugLabel->SetHiddenInGame(true);

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

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SofaMesh(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Old_Sofa.Old_Sofa"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> TableMesh(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Wooden_Table_Small.Wooden_Table_Small"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlasticChairMesh(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Plastic_Chair.Plastic_Chair"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> OfficeChairMesh(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Office_Chair.Office_Chair"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> FridgeMesh(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Refrigerator_Old.Refrigerator_Old"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CrateMesh(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Wooden_Crate.Wooden_Crate"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> BarrelMesh(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Metal_Barrel.Metal_Barrel"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> WheelBarrowMesh(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Wheel_Barrow.Wheel_Barrow"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> FenceMesh(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Fence_Old_1_2m.Fence_Old_1_2m"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SideShedMesh(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Side_Shed.Side_Shed"));

    if (SofaMesh.Succeeded()) RealSofa->SetStaticMesh(SofaMesh.Object);
    if (TableMesh.Succeeded()) RealTable->SetStaticMesh(TableMesh.Object);
    if (PlasticChairMesh.Succeeded()) RealPlasticChair->SetStaticMesh(PlasticChairMesh.Object);
    if (OfficeChairMesh.Succeeded()) RealOfficeChair->SetStaticMesh(OfficeChairMesh.Object);
    if (FridgeMesh.Succeeded()) RealFridge->SetStaticMesh(FridgeMesh.Object);
    if (CrateMesh.Succeeded()) RealCrate->SetStaticMesh(CrateMesh.Object);
    if (BarrelMesh.Succeeded()) RealMetalBarrel->SetStaticMesh(BarrelMesh.Object);
    if (WheelBarrowMesh.Succeeded()) RealWheelBarrow->SetStaticMesh(WheelBarrowMesh.Object);
    if (FenceMesh.Succeeded()) RealYardFence->SetStaticMesh(FenceMesh.Object);
    if (SideShedMesh.Succeeded()) RealSideShed->SetStaticMesh(SideShedMesh.Object);

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

void AOCEnterableHouse::AddFittedGroundProp(UInstancedStaticMeshComponent* Component,
    const FVector& FootprintCenter, float TargetLongestDimensionCm, float YawDegrees, float GroundZCm)
{
    if (!Component || !Component->GetStaticMesh() || TargetLongestDimensionCm <= 1.0f)
    {
        return;
    }

    const FBoxSphereBounds Bounds = Component->GetStaticMesh()->GetBounds();
    const FVector NativeSize = Bounds.BoxExtent * 2.0f;
    const float NativeLongestDimension = FMath::Max3(NativeSize.X, NativeSize.Y, NativeSize.Z);
    if (NativeLongestDimension <= 1.0f)
    {
        return;
    }

    const float UniformScale = TargetLongestDimensionCm / NativeLongestDimension;
    const FQuat Rotation = FRotator(0.0f, YawDegrees, 0.0f).Quaternion();
    const FVector TargetBoundsCenter(
        FootprintCenter.X,
        FootprintCenter.Y,
        GroundZCm + Bounds.BoxExtent.Z * UniformScale);
    const FVector InstanceLocation = TargetBoundsCenter - Rotation.RotateVector(Bounds.Origin * UniformScale);

    FTransform Transform;
    Transform.SetLocation(InstanceLocation);
    Transform.SetRotation(Rotation);
    Transform.SetScale3D(FVector(UniformScale));
    Component->AddInstance(Transform);
}

void AOCEnterableHouse::AddFittedFenceLine(UInstancedStaticMeshComponent* Component, const FVector& Center,
    float LengthCm, float YawDegrees, float GroundZCm)
{
    if (!Component || !Component->GetStaticMesh() || LengthCm <= 1.0f)
    {
        return;
    }

    constexpr float SegmentCm = 195.0f;
    const int32 Count = FMath::Max(1, FMath::RoundToInt(LengthCm / SegmentCm));
    const float UsedLength = static_cast<float>(Count - 1) * SegmentCm;
    const FVector Axis = FRotator(0.0f, YawDegrees, 0.0f).RotateVector(FVector::ForwardVector);

    for (int32 Index = 0; Index < Count; ++Index)
    {
        const float Offset = -UsedLength * 0.5f + static_cast<float>(Index) * SegmentCm;
        AddFittedGroundProp(Component, Center + Axis * Offset, SegmentCm, YawDegrees, GroundZCm);
    }
}

void AOCEnterableHouse::BuildShell()
{
    constexpr float WallThickness = 25.0f;
    constexpr float WallHeight = 320.0f;
    constexpr float FrontY = -550.0f;
    constexpr float BackY = 550.0f;
    constexpr float LeftX = -800.0f;
    constexpr float RightX = 800.0f;

    // Floor and flat prototype roof/ceiling. The building shell is still a blockout owner; the current
    // world-model pass is replacing household/yard props first because those real meshes already exist.
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
    // One interior partition with a 140 cm doorway.
    AddBox(Interior, FVector(-435.0f, 100.0f, 150.0f), FVector(730.0f, 18.0f, 300.0f));
    AddBox(Interior, FVector(435.0f, 100.0f, 150.0f), FVector(730.0f, 18.0f, 300.0f));
    AddBox(Interior, FVector(0.0f, 100.0f, 272.5f), FVector(140.0f, 18.0f, 55.0f));
}

void AOCEnterableHouse::AddChair(UInstancedStaticMeshComponent* Component, const FVector& Center, float YawDegrees)
{
    AddBox(Component, Center + FVector(0, 0, 45), FVector(48, 48, 8), YawDegrees);
    AddBox(Component, Center + FRotator(0, YawDegrees, 0).RotateVector(FVector(-20, 0, 82)), FVector(8, 48, 80), YawDegrees);
    for (float X : {-18.0f, 18.0f})
    {
        for (float Y : {-18.0f, 18.0f})
        {
            AddBox(Component, Center + FRotator(0, YawDegrees, 0).RotateVector(FVector(X, Y, 22)), FVector(6, 6, 44), YawDegrees);
        }
    }
}

void AOCEnterableHouse::AddTable(UInstancedStaticMeshComponent* Component, const FVector& Center,
    const FVector2D& Size, float YawDegrees)
{
    AddBox(Component, Center + FVector(0, 0, 76), FVector(Size.X, Size.Y, 8), YawDegrees);
    for (float X : {-Size.X * 0.42f, Size.X * 0.42f})
    {
        for (float Y : {-Size.Y * 0.38f, Size.Y * 0.38f})
        {
            AddBox(Component, Center + FRotator(0, YawDegrees, 0).RotateVector(FVector(X, Y, 38)), FVector(8, 8, 76), YawDegrees);
        }
    }
}

void AOCEnterableHouse::AddSofa(UInstancedStaticMeshComponent* Component, const FVector& Center,
    float YawDegrees, float WidthCm)
{
    AddBox(Component, Center + FVector(0, 0, 34), FVector(WidthCm, 82, 42), YawDegrees);
    AddBox(Component, Center + FRotator(0, YawDegrees, 0).RotateVector(FVector(0, 34, 78)), FVector(WidthCm, 18, 88), YawDegrees);
    AddBox(Component, Center + FRotator(0, YawDegrees, 0).RotateVector(FVector(-WidthCm * 0.48f, 0, 55)), FVector(16, 82, 70), YawDegrees);
    AddBox(Component, Center + FRotator(0, YawDegrees, 0).RotateVector(FVector(WidthCm * 0.48f, 0, 55)), FVector(16, 82, 70), YawDegrees);
}

void AOCEnterableHouse::ClearRealInteriorProps()
{
    UInstancedStaticMeshComponent* Components[] =
    {
        RealSofa,
        RealTable,
        RealPlasticChair,
        RealOfficeChair,
        RealFridge,
        RealCrate,
        RealMetalBarrel
    };

    for (UInstancedStaticMeshComponent* Component : Components)
    {
        if (Component)
        {
            Component->ClearInstances();
        }
    }
}

void AOCEnterableHouse::BuildHouseholdProps()
{
    if (!HouseholdFurniture || !HouseholdElectronics || !HouseholdClutter) return;

    FRandomStream R(InteriorSeed + LayoutVariant * 7919);
    const float WearYaw = HouseCondition == EOCHouseCondition::Worn ? 8.0f :
        HouseCondition == EOCHouseCondition::Ordinary ? 3.5f : 1.0f;

    const float SofaYaw = -90.0f + R.FRandRange(-WearYaw, WearYaw);
    const float SofaWidth = 270.0f + R.FRandRange(-25.0f, 20.0f);
    if (RealSofa && RealSofa->GetStaticMesh())
    {
        AddFittedGroundProp(RealSofa, FVector(-520, -250, 0), SofaWidth, SofaYaw);
    }
    else
    {
        AddSofa(HouseholdFurniture, FVector(-520, -250, 0), SofaYaw, SofaWidth);
    }

    // A small TV cabinet remains simple geometry until an authored cabinet/television model is present.
    AddBox(HouseholdFurniture, FVector(-170, -275, 42), FVector(150, 55, 84), R.FRandRange(-WearYaw, WearYaw));
    AddBox(HouseholdElectronics, FVector(-170, -275, 105), FVector(100, 12, 58), R.FRandRange(-WearYaw, WearYaw));

    const float DiningYaw = R.FRandRange(-WearYaw, WearYaw);
    if (RealTable && RealTable->GetStaticMesh())
    {
        AddFittedGroundProp(RealTable, FVector(300, -245, 0), 180.0f, DiningYaw);
    }
    else
    {
        AddTable(HouseholdFurniture, FVector(300, -245, 0), FVector2D(180, 90), DiningYaw);
    }

    struct FChairSeed
    {
        FVector Location;
        float Yaw;
    };
    const FChairSeed DiningChairs[] =
    {
        { FVector(300, -340, 0), 0.0f },
        { FVector(300, -150, 0), 180.0f },
        { FVector(190, -245, 0), 90.0f },
        { FVector(410, -245, 0), -90.0f }
    };

    const int32 DiningChairCount = (HouseCondition == EOCHouseCondition::Worn && (InteriorSeed % 2) != 0) ? 3 : 4;
    for (int32 Index = 0; Index < DiningChairCount; ++Index)
    {
        const float ChairYaw = DiningChairs[Index].Yaw + R.FRandRange(-WearYaw, WearYaw);
        if (RealPlasticChair && RealPlasticChair->GetStaticMesh())
        {
            AddFittedGroundProp(RealPlasticChair, DiningChairs[Index].Location, 90.0f, ChairYaw);
        }
        else
        {
            AddChair(HouseholdFurniture, DiningChairs[Index].Location, ChairYaw);
        }
    }

    // Kitchen worktop remains modular geometry; the fridge uses the actual rural-cabin prop.
    AddBox(HouseholdFurniture, FVector(500, 430, 48), FVector(450, 65, 96), 0);
    AddBox(HouseholdFurniture, FVector(670, 365, 48), FVector(65, 190, 96), 0);
    if (RealFridge && RealFridge->GetStaticMesh())
    {
        AddFittedGroundProp(RealFridge, FVector(700, 500, 0), 190.0f, R.FRandRange(-2.0f, 2.0f));
    }
    else
    {
        AddBox(HouseholdFurniture, FVector(700, 500, 95), FVector(75, 75, 190), R.FRandRange(-2, 2));
    }
    AddBox(HouseholdElectronics, FVector(440, 430, 103), FVector(72, 62, 18), 0);

    // Work corner: real wooden desk + real office chair; only the actual electronic devices remain proxy geometry.
    const float DeskYaw = R.FRandRange(-WearYaw, WearYaw);
    if (RealTable && RealTable->GetStaticMesh())
    {
        AddFittedGroundProp(RealTable, FVector(-470, 380, 0), 200.0f, DeskYaw);
    }
    else
    {
        AddTable(HouseholdFurniture, FVector(-470, 380, 0), FVector2D(200, 75), DeskYaw);
    }
    AddBox(HouseholdElectronics, FVector(-470, 395, 112), FVector(75, 10, 52), R.FRandRange(-3, 3));
    AddBox(HouseholdElectronics, FVector(-545, 400, 45), FVector(35, 48, 90), 0);
    AddBox(HouseholdElectronics, FVector(-390, 365, 88), FVector(52, 36, 4), R.FRandRange(-8, 8));

    const float OfficeChairYaw = R.FRandRange(-WearYaw, WearYaw);
    if (RealOfficeChair && RealOfficeChair->GetStaticMesh())
    {
        AddFittedGroundProp(RealOfficeChair, FVector(-470, 285, 0), 105.0f, OfficeChairYaw);
    }
    else
    {
        AddChair(HouseholdFurniture, FVector(-470, 285, 0), OfficeChairYaw);
    }

    // Storage stays lightweight for now. These two pieces are explicitly still blockout geometry and
    // are not falsely labelled as finished production furniture.
    AddBox(HouseholdFurniture, FVector(-710, 360, 95), FVector(110, 62, 190), R.FRandRange(-WearYaw, WearYaw));
    AddBox(HouseholdFurniture, FVector(600, 110, 55), FVector(280, 55, 110), R.FRandRange(-WearYaw, WearYaw));

    // Replace random colored cube clutter with a small number of actual cheap household/yard props.
    const int32 CrateCount = HouseCondition == EOCHouseCondition::Worn ? 5 :
        HouseCondition == EOCHouseCondition::Ordinary ? 3 : 2;
    for (int32 Index = 0; Index < CrateCount; ++Index)
    {
        const FVector P(
            R.FRandRange(-650.0f, 620.0f),
            R.FRandRange(-410.0f, 450.0f),
            0.0f);
        if (RealCrate && RealCrate->GetStaticMesh())
        {
            AddFittedGroundProp(RealCrate, P, R.FRandRange(38.0f, 52.0f), R.FRandRange(-30.0f, 30.0f));
        }
        else
        {
            AddBox(HouseholdClutter, P + FVector(0, 0, 18), FVector(36, 36, 36), R.FRandRange(-30, 30));
        }
    }

    if (HouseCondition == EOCHouseCondition::Worn)
    {
        if (RealMetalBarrel && RealMetalBarrel->GetStaticMesh())
        {
            AddFittedGroundProp(RealMetalBarrel, FVector(610, 320, 0), 88.0f, R.FRandRange(-15.0f, 15.0f));
        }
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
    ClearRealInteriorProps();
    BuildHouseholdProps();
}

void AOCEnterableHouse::BuildYard()
{
    // Preserve the authored pedestrian opening, but render the fence with the actual rural-cabin mesh.
    if (RealYardFence && RealYardFence->GetStaticMesh())
    {
        AddFittedFenceLine(RealYardFence, FVector(-1450.0f, -1750.0f, 0.0f), 1050.0f, 0.0f);
        AddFittedFenceLine(RealYardFence, FVector(850.0f, -1750.0f, 0.0f), 2500.0f, 0.0f);
        AddFittedFenceLine(RealYardFence, FVector(0.0f, 1750.0f, 0.0f), 4000.0f, 0.0f);
        AddFittedFenceLine(RealYardFence, FVector(-2000.0f, 0.0f, 0.0f), 3500.0f, 90.0f);
        AddFittedFenceLine(RealYardFence, FVector(2000.0f, 0.0f, 0.0f), 3500.0f, 90.0f);
    }
    else
    {
        AddBox(YardFences, FVector(-1450.0f, -1750.0f, 85.0f), FVector(1050.0f, 35.0f, 170.0f));
        AddBox(YardFences, FVector(850.0f, -1750.0f, 85.0f), FVector(2500.0f, 35.0f, 170.0f));
        AddBox(YardFences, FVector(0.0f, 1750.0f, 85.0f), FVector(4000.0f, 35.0f, 170.0f));
        AddBox(YardFences, FVector(-2000.0f, 0.0f, 85.0f), FVector(35.0f, 3500.0f, 170.0f));
        AddBox(YardFences, FVector(2000.0f, 0.0f, 85.0f), FVector(35.0f, 3500.0f, 170.0f));
    }

    // Path from the gate to the front door.
    AddBox(YardPaths, FVector(-585.0f, -1150.0f, 5.0f), FVector(170.0f, 1200.0f, 10.0f));

    // Replace the old cube backyard shed where the real asset is available.
    if (RealSideShed && RealSideShed->GetStaticMesh())
    {
        AddFittedGroundProp(RealSideShed, FVector(1250.0f, 1100.0f, 0.0f), 650.0f, 0.0f);
    }
    else
    {
        AddBox(Interior, FVector(1250.0f, 1100.0f, 150.0f), FVector(700.0f, 600.0f, 300.0f));
    }

    if (RealWheelBarrow && RealWheelBarrow->GetStaticMesh())
    {
        AddFittedGroundProp(RealWheelBarrow, FVector(1180.0f, 430.0f, 0.0f), 145.0f, -28.0f);
    }
    if (RealMetalBarrel && RealMetalBarrel->GetStaticMesh())
    {
        AddFittedGroundProp(RealMetalBarrel, FVector(1500.0f, 760.0f, 0.0f), 88.0f, 7.0f);
    }
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

    GetWorld()->SpawnActor<AOCInteractableLight>(AOCInteractableLight::StaticClass(),
        MakeWorldTransform(FVector(0.0f, -50.0f, 300.0f)), SpawnParams);
    GetWorld()->SpawnActor<AOCInteractableGate>(AOCInteractableGate::StaticClass(),
        MakeWorldTransform(FVector(-585.0f, -1750.0f, 0.0f)), SpawnParams);
}
