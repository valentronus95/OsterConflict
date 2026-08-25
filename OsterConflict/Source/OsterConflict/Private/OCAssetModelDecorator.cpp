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

    TreeA = MakeVisualISM(this, SceneRoot, TEXT("RealTreeA"));
    TreeB = MakeVisualISM(this, SceneRoot, TEXT("RealTreeB"));
    TreeC = MakeVisualISM(this, SceneRoot, TEXT("RealTreeC"));
    TreeD = MakeVisualISM(this, SceneRoot, TEXT("RealTreeD"));
    TreeE = MakeVisualISM(this, SceneRoot, TEXT("RealTreeE"));
    PineA = MakeVisualISM(this, SceneRoot, TEXT("RealPineA"));
    PineB = MakeVisualISM(this, SceneRoot, TEXT("RealPineB"));

    StreetLight = MakeVisualISM(this, SceneRoot, TEXT("RealStreetLight"));
    PowerPole = MakeVisualISM(this, SceneRoot, TEXT("RealPowerPole"));
    BridgeA = MakeVisualISM(this, SceneRoot, TEXT("RealBridgeA"));
    BridgeB = MakeVisualISM(this, SceneRoot, TEXT("RealBridgeB"));
    BridgeC = MakeVisualISM(this, SceneRoot, TEXT("RealBridgeC"));
    BridgeD = MakeVisualISM(this, SceneRoot, TEXT("RealBridgeD"));
    Crate = MakeVisualISM(this, SceneRoot, TEXT("RealCrate"));
    MetalBarrel = MakeVisualISM(this, SceneRoot, TEXT("RealMetalBarrel"));
    ShoppingCart = MakeVisualISM(this, SceneRoot, TEXT("RealShoppingCart"));
    PicnicTable = MakeVisualISM(this, SceneRoot, TEXT("RealPicnicTable"));
    Tire = MakeVisualISM(this, SceneRoot, TEXT("RealTire"));
    Bush = MakeVisualISM(this, SceneRoot, TEXT("RealBush"));
    Well = MakeVisualISM(this, SceneRoot, TEXT("RealWell"));
    WellExtra01 = MakeVisualISM(this, SceneRoot, TEXT("RealWellExtra01"));
    WellExtra02 = MakeVisualISM(this, SceneRoot, TEXT("RealWellExtra02"));
    WellExtra03 = MakeVisualISM(this, SceneRoot, TEXT("RealWellExtra03"));
    WellExtra04 = MakeVisualISM(this, SceneRoot, TEXT("RealWellExtra04"));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> TreeMeshA(
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var01.SM_Tree_Var01"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> TreeMeshB(
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var02.SM_Tree_Var02"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> TreeMeshC(
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var03.SM_Tree_Var03"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> TreeMeshD(
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var04.SM_Tree_Var04"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> TreeMeshE(
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var05.SM_Tree_Var05"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PineMeshA(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_01.SM_Pine_Tree_01"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PineMeshB(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_03.SM_Pine_Tree_03"));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> StreetLightMesh(
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_StreetLight.SM_StreetLight"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PowerPoleMesh(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Power_Pole_1.Power_Pole_1"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> BridgeMeshA(
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Bridge_Var01.SM_Bridge_Var01"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> BridgeMeshB(
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Bridge_Var02.SM_Bridge_Var02"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> BridgeMeshC(
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Bridge_Var03.SM_Bridge_Var03"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> BridgeMeshD(
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Bridge_Var04.SM_Bridge_Var04"));
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
    static ConstructorHelpers::FObjectFinder<UStaticMesh> WellExtraMesh01(
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Well_Extra01.SM_Well_Extra01"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> WellExtraMesh02(
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Well_Extra02.SM_Well_Extra02"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> WellExtraMesh03(
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Well_Extra03.SM_Well_Extra03"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> WellExtraMesh04(
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Well_Extra04.SM_Well_Extra04"));

    if (TreeMeshA.Succeeded()) TreeA->SetStaticMesh(TreeMeshA.Object);
    if (TreeMeshB.Succeeded()) TreeB->SetStaticMesh(TreeMeshB.Object);
    if (TreeMeshC.Succeeded()) TreeC->SetStaticMesh(TreeMeshC.Object);
    if (TreeMeshD.Succeeded()) TreeD->SetStaticMesh(TreeMeshD.Object);
    if (TreeMeshE.Succeeded()) TreeE->SetStaticMesh(TreeMeshE.Object);
    if (PineMeshA.Succeeded()) PineA->SetStaticMesh(PineMeshA.Object);
    if (PineMeshB.Succeeded()) PineB->SetStaticMesh(PineMeshB.Object);

    if (StreetLightMesh.Succeeded()) StreetLight->SetStaticMesh(StreetLightMesh.Object);
    if (PowerPoleMesh.Succeeded()) PowerPole->SetStaticMesh(PowerPoleMesh.Object);
    if (BridgeMeshA.Succeeded()) BridgeA->SetStaticMesh(BridgeMeshA.Object);
    if (BridgeMeshB.Succeeded()) BridgeB->SetStaticMesh(BridgeMeshB.Object);
    if (BridgeMeshC.Succeeded()) BridgeC->SetStaticMesh(BridgeMeshC.Object);
    if (BridgeMeshD.Succeeded()) BridgeD->SetStaticMesh(BridgeMeshD.Object);
    if (CrateMesh.Succeeded()) Crate->SetStaticMesh(CrateMesh.Object);
    if (BarrelMesh.Succeeded()) MetalBarrel->SetStaticMesh(BarrelMesh.Object);
    if (ShoppingCartMesh.Succeeded()) ShoppingCart->SetStaticMesh(ShoppingCartMesh.Object);
    if (PicnicTableMesh.Succeeded()) PicnicTable->SetStaticMesh(PicnicTableMesh.Object);
    if (TireMesh.Succeeded()) Tire->SetStaticMesh(TireMesh.Object);
    if (BushMesh.Succeeded()) Bush->SetStaticMesh(BushMesh.Object);
    if (WellMesh.Succeeded()) Well->SetStaticMesh(WellMesh.Object);
    if (WellExtraMesh01.Succeeded()) WellExtra01->SetStaticMesh(WellExtraMesh01.Object);
    if (WellExtraMesh02.Succeeded()) WellExtra02->SetStaticMesh(WellExtraMesh02.Object);
    if (WellExtraMesh03.Succeeded()) WellExtra03->SetStaticMesh(WellExtraMesh03.Object);
    if (WellExtraMesh04.Succeeded()) WellExtra04->SetStaticMesh(WellExtraMesh04.Object);
}

void AOCAssetModelDecorator::PopulateForSector(AActor* SectorActor)
{
    if (bPopulated || !SectorActor) return;
    bPopulated = true;

    HideReplacedProxyComponents(SectorActor);
    BuildVegetationModels();
    BuildInfrastructureModels();
    BuildAmbientProps();

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_GENERIC_RESIDENTIAL_REPLACEMENT_RETIRED semantic_baseline=1 advanced_village_houses=0 village_fences=0 side_sheds=0 runtime_house_replacement=0"));
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

UInstancedStaticMeshComponent* AOCAssetModelDecorator::SelectBridge(int32 VariantSeed) const
{
    UInstancedStaticMeshComponent* Bridges[] = { BridgeA, BridgeB, BridgeC, BridgeD };
    const int32 StartIndex = FMath::Abs(VariantSeed) % UE_ARRAY_COUNT(Bridges);
    for (int32 Offset = 0; Offset < UE_ARRAY_COUNT(Bridges); ++Offset)
    {
        UInstancedStaticMeshComponent* Candidate = Bridges[(StartIndex + Offset) % UE_ARRAY_COUNT(Bridges)];
        if (Candidate && Candidate->GetStaticMesh()) return Candidate;
    }
    return nullptr;
}

void AOCAssetModelDecorator::AddAuthoredWell(const FVector& Location, float YawDegrees,
    const FVector& Scale, int32 VariantSeed)
{
    AddMeshInstance(Well, Location, YawDegrees, Scale);
    UInstancedStaticMeshComponent* Extras[] = { WellExtra01, WellExtra02, WellExtra03, WellExtra04 };
    const int32 Seed = FMath::Abs(VariantSeed);
    AddMeshInstance(Extras[Seed % UE_ARRAY_COUNT(Extras)], Location, YawDegrees, Scale);
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

void AOCAssetModelDecorator::BuildVegetationModels()
{
    const FVector Museum = GeoPoint(FOCGeoReference::Museum());
    const FVector Park = GeoPoint(FOCGeoReference::CentralPark());
    const FVector College = GeoPoint(FOCGeoReference::College());
    const FVector Stadium(15000.0f, -1500.0f, 0.0f);

    auto AddBroadleaf = [this](const FVector& Location, int32 Seed, float Scale = 1.0f)
    {
        UInstancedStaticMeshComponent* Families[] = { TreeA, TreeB, TreeC, TreeD, TreeE };
        const int32 FamilyIndex = FMath::Abs(Seed) % UE_ARRAY_COUNT(Families);
        AddMeshInstance(Families[FamilyIndex], Location, static_cast<float>((Seed * 47) % 360), FVector(Scale));
    };

    auto AddPoplar = [this](const FVector& Location, int32 Seed, float Scale = 1.0f)
    {
        UInstancedStaticMeshComponent* Family = (FMath::Abs(Seed) % 2 == 0) ? TreeB : TreeD;
        AddMeshInstance(Family, Location, static_cast<float>((Seed * 61) % 360),
            FVector(0.76f * Scale, 0.76f * Scale, 1.28f * Scale));
    };

    auto AddBirchLike = [this](const FVector& Location, int32 Seed, float Scale = 1.0f)
    {
        UInstancedStaticMeshComponent* Family = (FMath::Abs(Seed) % 2 == 0) ? TreeC : TreeE;
        AddMeshInstance(Family, Location, static_cast<float>((Seed * 53) % 360),
            FVector(0.82f * Scale, 0.82f * Scale, 1.02f * Scale));
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
    // Keep the two existing bridge sites exactly where they were. Only the authored model family varies.
    AddMeshInstance(SelectBridge(0), FVector(-17000.0f, -100000.0f, 0.0f), 87.0f, FVector(1.0f));
    AddMeshInstance(SelectBridge(1), FVector(76000.0f, -65000.0f, 0.0f), 28.0f, FVector(0.95f));

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
        const FVector Offset = FRotator(0.0f, Angle, 0.0f).RotateVector(
            FVector(5700.0f + (I % 3) * 350.0f, 0.0f, 0.0f));
        AddMeshInstance(Bush, Park + Offset, Angle + 25.0f, FVector(0.8f + 0.08f * (I % 3)));
    }

    // Low-scale yard props only. Residential building/fence replacement is retired under Pass45.
    const FVector YardA(-51500.0f, -17500.0f, 0.0f);
    const FVector YardB(40500.0f, 18500.0f, 0.0f);
    const FVector YardC(-9000.0f, -28500.0f, 0.0f);

    AddMeshInstance(Crate, YardA + FVector(700, 500, 0), 17.0f);
    AddMeshInstance(MetalBarrel, YardA + FVector(1050, 650, 0), -8.0f);
    AddMeshInstance(Tire, YardA + FVector(1350, 420, 0), 42.0f);

    AddAuthoredWell(YardB + FVector(1100, 1150, 0), 0.0f, FVector(1.0f), 2);
    AddMeshInstance(Crate, YardB + FVector(-650, 850, 0), -32.0f);

    AddMeshInstance(ShoppingCart, YardC + FVector(900, -600, 0), 75.0f);
    AddMeshInstance(MetalBarrel, YardC + FVector(-700, 700, 0), 12.0f);
}
