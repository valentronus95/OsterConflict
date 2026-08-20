#include "OCAssetModelDecorator.h"

#include "OCGeoReference.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
    UInstancedStaticMeshComponent* MakeVisualISM(AActor* Owner, USceneComponent* Root, const TCHAR* Name)
    {
        UInstancedStaticMeshComponent* Component = Owner->CreateDefaultSubobject<UInstancedStaticMeshComponent>(FName(Name));
        Component->SetupAttachment(Root);
        Component->SetCollisionProfileName(TEXT("NoCollision"));
        Component->SetGenerateOverlapEvents(false);
        return Component;
    }

    FVector GeoPoint(const FOCGeoReferencePoint& Ref)
    {
        return FOCGeoReference::ToLocalCm(Ref.Latitude, Ref.Longitude, 0.0f);
    }
}

AOCAssetModelDecorator::AOCAssetModelDecorator()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    HouseA = MakeVisualISM(this, SceneRoot, TEXT("RealHouseA"));
    HouseB = MakeVisualISM(this, SceneRoot, TEXT("RealHouseB"));
    TreeA = MakeVisualISM(this, SceneRoot, TEXT("RealTreeA"));
    TreeB = MakeVisualISM(this, SceneRoot, TEXT("RealTreeB"));
    TreeC = MakeVisualISM(this, SceneRoot, TEXT("RealTreeC"));
    PineA = MakeVisualISM(this, SceneRoot, TEXT("RealPineA"));
    PineB = MakeVisualISM(this, SceneRoot, TEXT("RealPineB"));
    OldFence = MakeVisualISM(this, SceneRoot, TEXT("RealOldFence"));
    StreetLight = MakeVisualISM(this, SceneRoot, TEXT("RealStreetLight"));
    PowerPole = MakeVisualISM(this, SceneRoot, TEXT("RealPowerPole"));
    Bridge = MakeVisualISM(this, SceneRoot, TEXT("RealBridge"));
    SideShed = MakeVisualISM(this, SceneRoot, TEXT("RealSideShed"));
    Crate = MakeVisualISM(this, SceneRoot, TEXT("RealCrate"));
    MetalBarrel = MakeVisualISM(this, SceneRoot, TEXT("RealMetalBarrel"));
    ShoppingCart = MakeVisualISM(this, SceneRoot, TEXT("RealShoppingCart"));
    PicnicTable = MakeVisualISM(this, SceneRoot, TEXT("RealPicnicTable"));
    Tire = MakeVisualISM(this, SceneRoot, TEXT("RealTire"));
    Bush = MakeVisualISM(this, SceneRoot, TEXT("RealBush"));
    Well = MakeVisualISM(this, SceneRoot, TEXT("RealWell"));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> HouseMeshA(
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_House_Var01.SM_House_Var01"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> HouseMeshB(
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_House_Var02.SM_House_Var02"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> TreeMeshA(
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var01.SM_Tree_Var01"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> TreeMeshB(
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var02.SM_Tree_Var02"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> TreeMeshC(
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var03.SM_Tree_Var03"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PineMeshA(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_01.SM_Pine_Tree_01"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PineMeshB(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_03.SM_Pine_Tree_03"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> FenceMesh(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Fence_Old_1_2m.Fence_Old_1_2m"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> StreetLightMesh(
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_StreetLight.SM_StreetLight"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PowerPoleMesh(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Power_Pole_1.Power_Pole_1"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> BridgeMesh(
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Bridge_Var01.SM_Bridge_Var01"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SideShedMesh(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Side_Shed.Side_Shed"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CrateMesh(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Wooden_Crate.Wooden_Crate"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> BarrelMesh(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Metal_Barrel.Metal_Barrel"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> ShoppingCartMesh(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Shopping_Cart.Shopping_Cart"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PicnicTableMesh(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Picnic_Table.Picnic_Table"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> TireMesh(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Tire.Tire"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> BushMesh(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/Bush_1.Bush_1"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> WellMesh(
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Well.SM_Well"));

    if (HouseMeshA.Succeeded()) HouseA->SetStaticMesh(HouseMeshA.Object);
    if (HouseMeshB.Succeeded()) HouseB->SetStaticMesh(HouseMeshB.Object);
    if (TreeMeshA.Succeeded()) TreeA->SetStaticMesh(TreeMeshA.Object);
    if (TreeMeshB.Succeeded()) TreeB->SetStaticMesh(TreeMeshB.Object);
    if (TreeMeshC.Succeeded()) TreeC->SetStaticMesh(TreeMeshC.Object);
    if (PineMeshA.Succeeded()) PineA->SetStaticMesh(PineMeshA.Object);
    if (PineMeshB.Succeeded()) PineB->SetStaticMesh(PineMeshB.Object);
    if (FenceMesh.Succeeded()) OldFence->SetStaticMesh(FenceMesh.Object);
    if (StreetLightMesh.Succeeded()) StreetLight->SetStaticMesh(StreetLightMesh.Object);
    if (PowerPoleMesh.Succeeded()) PowerPole->SetStaticMesh(PowerPoleMesh.Object);
    if (BridgeMesh.Succeeded()) Bridge->SetStaticMesh(BridgeMesh.Object);
    if (SideShedMesh.Succeeded()) SideShed->SetStaticMesh(SideShedMesh.Object);
    if (CrateMesh.Succeeded()) Crate->SetStaticMesh(CrateMesh.Object);
    if (BarrelMesh.Succeeded()) MetalBarrel->SetStaticMesh(BarrelMesh.Object);
    if (ShoppingCartMesh.Succeeded()) ShoppingCart->SetStaticMesh(ShoppingCartMesh.Object);
    if (PicnicTableMesh.Succeeded()) PicnicTable->SetStaticMesh(PicnicTableMesh.Object);
    if (TireMesh.Succeeded()) Tire->SetStaticMesh(TireMesh.Object);
    if (BushMesh.Succeeded()) Bush->SetStaticMesh(BushMesh.Object);
    if (WellMesh.Succeeded()) Well->SetStaticMesh(WellMesh.Object);
}

void AOCAssetModelDecorator::PopulateForSector(AActor* SectorActor)
{
    if (bPopulated || !SectorActor) return;
    bPopulated = true;

    HideReplacedProxyComponents(SectorActor);
    BuildResidentialModels();
    BuildVegetationModels();
    BuildInfrastructureModels();
    BuildAmbientProps();
}

void AOCAssetModelDecorator::AddMeshInstance(UInstancedStaticMeshComponent* Component, const FVector& Location,
    float YawDegrees, const FVector& Scale)
{
    if (!Component || !Component->GetStaticMesh()) return;

    FTransform Transform;
    Transform.SetLocation(Location);
    Transform.SetRotation(FQuat(FRotator(0.0f, YawDegrees, 0.0f)));
    Transform.SetScale3D(Scale);
    Component->AddInstance(Transform);
}

void AOCAssetModelDecorator::AddFenceLine(UInstancedStaticMeshComponent* Component, const FVector& Center,
    float LengthCm, float YawDegrees, float ZCm)
{
    if (!Component || !Component->GetStaticMesh()) return;

    constexpr float SegmentCm = 195.0f;
    const int32 Count = FMath::Max(1, FMath::RoundToInt(LengthCm / SegmentCm));
    const float UsedLength = static_cast<float>(Count - 1) * SegmentCm;
    const FVector Axis = FRotator(0.0f, YawDegrees, 0.0f).RotateVector(FVector(1.0f, 0.0f, 0.0f));

    for (int32 Index = 0; Index < Count; ++Index)
    {
        const float Offset = -UsedLength * 0.5f + static_cast<float>(Index) * SegmentCm;
        AddMeshInstance(Component, Center + Axis * Offset + FVector(0.0f, 0.0f, ZCm), YawDegrees);
    }
}

void AOCAssetModelDecorator::HideReplacedProxyComponents(AActor* SectorActor) const
{
    TArray<UActorComponent*> Components;
    SectorActor->GetComponents(Components);

    for (UActorComponent* Component : Components)
    {
        UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Component);
        if (!Primitive) continue;

        const FName Name = Primitive->GetFName();
        const bool bReplaced =
            Name == TEXT("ResidentialRoofs") ||
            Name == TEXT("ResidentialDetails") ||
            Name == TEXT("TreeTrunks") ||
            Name == TEXT("TreeCrowns") ||
            Name == TEXT("SovietPoplarTrunks") ||
            Name == TEXT("SovietPoplarCrowns") ||
            Name == TEXT("BirchTrunks") ||
            Name == TEXT("BirchCrowns") ||
            Name == TEXT("PineTrunks") ||
            Name == TEXT("PineCrowns");

        if (bReplaced)
        {
            Primitive->SetVisibility(false, true);
            Primitive->SetHiddenInGame(true, true);
        }
    }
}

void AOCAssetModelDecorator::BuildResidentialModels()
{
    // Solomii Krushelnytskoi: reuse the exact authored house rhythm, but put real meshes over the collision cores.
    constexpr float WestHouseX = -39200.0f;
    constexpr float EastHouseX = -27800.0f;
    constexpr float StartY = 20500.0f;

    for (int32 Index = 0; Index < 8; ++Index)
    {
        const float Y = StartY + static_cast<float>(Index) * 4800.0f;
        const float WestYaw = 87.0f + static_cast<float>((Index % 3) - 1) * 2.0f;
        const float EastYaw = -88.0f + static_cast<float>((Index % 2) * 3);

        if (Index != 2)
        {
            AddMeshInstance((Index % 2 == 0) ? HouseA : HouseB,
                FVector(EastHouseX, Y, 0.0f), EastYaw, FVector(0.95f + 0.04f * (Index % 3)));
        }
        AddMeshInstance((Index % 2 == 0) ? HouseB : HouseA,
            FVector(WestHouseX, Y + 700.0f, 0.0f), WestYaw, FVector(0.94f + 0.03f * (Index % 2)));

        AddMeshInstance(SideShed, FVector(WestHouseX - 1700.0f, Y + 1950.0f, 0.0f), WestYaw);
        AddMeshInstance(SideShed, FVector(EastHouseX + 1600.0f, Y + 1750.0f, 0.0f), EastYaw);
        AddFenceLine(OldFence, FVector(-37100.0f, Y - 1200.0f, 0.0f), 3200.0f, 90.0f);
        AddFenceLine(OldFence, FVector(-29900.0f, Y - 1200.0f, 0.0f), 3200.0f, 90.0f);
    }

    struct FBlockSeed
    {
        FVector Origin;
        int32 Rows;
        int32 Columns;
        FVector Spacing;
        float Yaw;
    };

    const FBlockSeed Blocks[] =
    {
        { FVector(16000, 15000, 0), 3, 4, FVector(4200, 4300, 0), 4.0f },
        { FVector(36500, 12500, 0), 3, 4, FVector(4100, 4400, 0), -3.0f },
        { FVector(35000, -22000, 0), 3, 4, FVector(4200, 4100, 0), 1.0f },
        { FVector(-52000, -21000, 0), 3, 4, FVector(4000, 4300, 0), 2.0f },
        { FVector(-50000, 28000, 0), 3, 4, FVector(4100, 4200, 0), -2.0f },
        { FVector(-12000, -33000, 0), 3, 5, FVector(3900, 4200, 0), 3.0f },
        { FVector(-82000, 15000, 0), 3, 4, FVector(4100, 4300, 0), 8.0f },
        { FVector(-76000, -41000, 0), 3, 4, FVector(4050, 4250, 0), -4.0f },
        { FVector(52000, 33000, 0), 3, 4, FVector(4200, 4400, 0), 5.0f },
        { FVector(47000, -50000, 0), 3, 4, FVector(4100, 4200, 0), -7.0f },
        { FVector(-24000, 76000, 0), 2, 5, FVector(4200, 4100, 0), 12.0f }
    };

    int32 HouseCounter = 0;
    for (const FBlockSeed& Block : Blocks)
    {
        for (int32 Row = 0; Row < Block.Rows; ++Row)
        {
            for (int32 Col = 0; Col < Block.Columns; ++Col)
            {
                const float OffsetJitter = static_cast<float>((HouseCounter % 3) - 1) * 170.0f;
                const FVector Center = Block.Origin + FVector(
                    Col * Block.Spacing.X + OffsetJitter,
                    Row * Block.Spacing.Y - OffsetJitter,
                    0.0f);
                const float HouseYaw = Block.Yaw + (HouseCounter % 2 == 0 ? -5.0f : 5.0f);
                const float UniformScale = 0.92f + static_cast<float>(HouseCounter % 4) * 0.045f;

                AddMeshInstance((HouseCounter % 2 == 0) ? HouseA : HouseB,
                    Center, HouseYaw, FVector(UniformScale));

                const float Width = 1600.0f + static_cast<float>((HouseCounter % 4) * 180);
                const float Depth = 1050.0f + static_cast<float>((HouseCounter % 3) * 130);
                const FVector ShedOffset = FRotator(0.0f, HouseYaw, 0.0f).RotateVector(
                    FVector(-Width * 0.32f, Depth * 1.35f, 0.0f));
                AddMeshInstance(SideShed, Center + ShedOffset, HouseYaw + 90.0f, FVector(0.88f));

                if ((HouseCounter % 11) != 0)
                {
                    const float FenceLength = (HouseCounter % 3 == 0) ? 2100.0f : 2850.0f;
                    const FVector FrontOffset = FRotator(0.0f, HouseYaw, 0.0f).RotateVector(FVector(0.0f, -1500.0f, 0.0f));
                    AddFenceLine(OldFence, Center + FrontOffset, FenceLength, HouseYaw);
                }

                ++HouseCounter;
            }
        }
    }
}

void AOCAssetModelDecorator::BuildVegetationModels()
{
    const FVector Museum = GeoPoint(FOCGeoReference::Museum());
    const FVector Park = GeoPoint(FOCGeoReference::CentralPark());
    const FVector College = GeoPoint(FOCGeoReference::College());
    const FVector Stadium(15000.0f, -1500.0f, 0.0f);

    auto AddBroadleaf = [this](const FVector& Location, int32 Seed, float Scale = 1.0f)
    {
        UInstancedStaticMeshComponent* Family = (Seed % 3 == 0) ? TreeA : ((Seed % 3 == 1) ? TreeB : TreeC);
        AddMeshInstance(Family, Location, static_cast<float>((Seed * 47) % 360), FVector(Scale));
    };

    auto AddPoplar = [this](const FVector& Location, int32 Seed, float Scale = 1.0f)
    {
        AddMeshInstance(TreeB, Location, static_cast<float>((Seed * 61) % 360), FVector(0.76f * Scale, 0.76f * Scale, 1.28f * Scale));
    };

    auto AddBirchLike = [this](const FVector& Location, int32 Seed, float Scale = 1.0f)
    {
        AddMeshInstance(TreeC, Location, static_cast<float>((Seed * 53) % 360), FVector(0.82f * Scale, 0.82f * Scale, 1.02f * Scale));
    };

    auto AddPine = [this](const FVector& Location, int32 Seed, float Scale = 1.0f)
    {
        AddMeshInstance((Seed % 2 == 0) ? PineA : PineB, Location,
            static_cast<float>((Seed * 43) % 360), FVector(Scale));
    };

    // Museum garden.
    for (int32 Index = 0; Index < 16; ++Index)
    {
        const FVector LocalOffset(
            -4700.0f + static_cast<float>(Index % 8) * 1350.0f,
            2500.0f + static_cast<float>(Index / 8) * 1750.0f,
            0.0f);
        const float Scale = 0.88f + 0.07f * static_cast<float>(Index % 3);
        if (Index == 2 || Index == 11) AddPoplar(Museum + LocalOffset, Index, Scale);
        else AddBroadleaf(Museum + LocalOffset, Index, Scale);
    }

    // Stadium perimeter.
    for (int32 I = -6; I <= 6; ++I)
    {
        const FVector Location = Stadium + FVector(I * 1500.0f, 5700.0f + (I % 2) * 350.0f, 0.0f);
        if (I % 3 == 0) AddPoplar(Location, I + 20, 0.9f);
        else if (I % 4 == 0) AddBirchLike(Location, I + 20, 0.9f);
        else AddBroadleaf(Location, I + 20, 0.9f);
    }

    // Central park.
    int32 TreeSeed = 50;
    for (int32 Row = -3; Row <= 3; ++Row)
    {
        for (int32 Col = -4; Col <= 4; ++Col)
        {
            if (FMath::Abs(Row) <= 1 && FMath::Abs(Col) <= 1) continue;
            const float JitterX = static_cast<float>(((Row * 7 + Col * 3) % 5) - 2) * 180.0f;
            const float JitterY = static_cast<float>(((Row * 5 + Col * 11) % 5) - 2) * 160.0f;
            const FVector Location = Park + FVector(Col * 1850.0f + JitterX, Row * 1700.0f + JitterY, 0.0f);
            const int32 Roll = FMath::Abs(Row * 9 + Col * 5) % 12;
            const float Scale = 0.85f + 0.05f * static_cast<float>((Row + Col + 8) % 4);

            if (Roll <= 2) AddPoplar(Location, TreeSeed, Scale);
            else if (Roll == 3 || Roll == 4) AddBirchLike(Location, TreeSeed, Scale);
            else if (Roll == 5) AddPine(Location, TreeSeed, Scale);
            else AddBroadleaf(Location, TreeSeed, Scale);
            ++TreeSeed;
        }
    }

    // College conifers.
    AddPine(College + FVector(-3800, -1100, 0), 101, 1.2f);
    AddPine(College + FVector(3900, -950, 0), 102, 1.15f);
    AddPine(College + FVector(-4600, 1500, 0), 103, 1.0f);
    AddBirchLike(College + FVector(4700, 2100, 0), 104, 0.9f);

    // Selected street rows.
    for (int32 Index = -7; Index <= 7; ++Index)
    {
        const FVector WestEast(Index * 7000.0f, -11500.0f, 0.0f);
        if (Index % 4 == 0) AddPoplar(WestEast, Index + 130, 0.75f);
        else AddBroadleaf(WestEast, Index + 130, 0.75f);

        if ((Index % 2) == 0)
        {
            const FVector NorthSouth(28500.0f, Index * 6500.0f, 0.0f);
            if (Index % 4 == 0) AddPoplar(NorthSouth, Index + 160, 0.8f);
            else AddBirchLike(NorthSouth, Index + 160, 0.8f);
        }
    }

    // Private yards.
    for (int32 I = 0; I < 24; ++I)
    {
        const FVector Location(
            -62000.0f + static_cast<float>(I % 8) * 15000.0f,
            -48000.0f + static_cast<float>(I / 8) * 42000.0f + static_cast<float>((I % 3) - 1) * 1800.0f,
            0.0f);
        AddBroadleaf(Location, I + 200, 0.55f + 0.05f * static_cast<float>(I % 3));
    }
}

void AOCAssetModelDecorator::BuildInfrastructureModels()
{
    // Real bridge art over the existing collision/deck proxies.
    AddMeshInstance(Bridge, FVector(-17000.0f, -100000.0f, 0.0f), 87.0f, FVector(1.0f));
    AddMeshInstance(Bridge, FVector(76000.0f, -65000.0f, 0.0f), 28.0f, FVector(0.95f));

    // Lighting along the central east-west corridor.
    for (int32 I = -6; I <= 6; ++I)
    {
        const float X = -5000.0f + static_cast<float>(I) * 9500.0f;
        AddMeshInstance(StreetLight, FVector(X, -7850.0f, 0.0f), 0.0f);
        if ((I % 2) == 0) AddMeshInstance(StreetLight, FVector(X, -10150.0f, 0.0f), 180.0f);
    }

    // Utility poles along the Krushelnytska corridor.
    for (int32 I = 0; I < 8; ++I)
    {
        AddMeshInstance(PowerPole, FVector(-36000.0f, 15500.0f + static_cast<float>(I) * 7600.0f, 0.0f), 91.5f);
    }
}

void AOCAssetModelDecorator::BuildAmbientProps()
{
    const FVector Park = GeoPoint(FOCGeoReference::CentralPark());

    // Park furniture and low vegetation. Kept sparse so it does not turn into asset-pack soup.
    AddMeshInstance(PicnicTable, Park + FVector(-2900, 1900, 0), 18.0f);
    AddMeshInstance(PicnicTable, Park + FVector(3300, -2400, 0), -21.0f);
    for (int32 I = 0; I < 10; ++I)
    {
        const float Angle = static_cast<float>(I) * 36.0f;
        const FVector Offset = FRotator(0.0f, Angle, 0.0f).RotateVector(FVector(5700.0f + (I % 3) * 350.0f, 0.0f, 0.0f));
        AddMeshInstance(Bush, Park + Offset, Angle + 25.0f, FVector(0.8f + 0.08f * (I % 3)));
    }

    // Residential details. These are deliberately modest and deterministic, not random clutter.
    const FVector YardA(-51500.0f, -17500.0f, 0.0f);
    const FVector YardB(40500.0f, 18500.0f, 0.0f);
    const FVector YardC(-9000.0f, -28500.0f, 0.0f);

    AddMeshInstance(Crate, YardA + FVector(700, 500, 0), 17.0f);
    AddMeshInstance(MetalBarrel, YardA + FVector(1050, 650, 0), -8.0f);
    AddMeshInstance(Tire, YardA + FVector(1350, 420, 0), 42.0f);

    AddMeshInstance(Well, YardB + FVector(1100, 1150, 0), 0.0f);
    AddMeshInstance(Crate, YardB + FVector(-650, 850, 0), -32.0f);

    AddMeshInstance(ShoppingCart, YardC + FVector(900, -600, 0), 75.0f);
    AddMeshInstance(MetalBarrel, YardC + FVector(-700, 700, 0), 12.0f);
}
