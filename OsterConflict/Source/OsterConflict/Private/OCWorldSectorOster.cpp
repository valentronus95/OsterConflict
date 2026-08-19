#include "OCWorldSectorOster.h"
#include "OCGeoReference.h"
#include "OCLocationSectorPlan.h"
#include "OCLocationSectorS01Data.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
    constexpr float MapWidthCm = 240000.0f;
    constexpr float MapHeightCm = 240000.0f;
    constexpr float RoadZ = 8.0f;
    constexpr float GroundTopZ = 0.0f;

    FVector Rotate2D(const FVector& Local, float YawDegrees)
    {
        return FRotator(0.0f, YawDegrees, 0.0f).RotateVector(Local);
    }
}

AOCWorldSectorOster::AOCWorldSectorOster()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetReplicateMovement(false);

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    Ground = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ground"));
    Ground->SetupAttachment(SceneRoot);
    Ground->SetCollisionProfileName(TEXT("BlockAll"));

    auto MakeISM = [this](const TCHAR* Name, const TCHAR* CollisionProfile)
    {
        UInstancedStaticMeshComponent* Result = CreateDefaultSubobject<UInstancedStaticMeshComponent>(FName(Name));
        Result->SetupAttachment(SceneRoot);
        Result->SetCollisionProfileName(FName(CollisionProfile));
        return Result;
    };

    Roads = MakeISM(TEXT("Roads"), TEXT("BlockAll"));
    Sidewalks = MakeISM(TEXT("Sidewalks"), TEXT("BlockAll"));
    Buildings = MakeISM(TEXT("Buildings"), TEXT("BlockAll"));
    ResidentialRoofs = MakeISM(TEXT("ResidentialRoofs"), TEXT("BlockAll"));
    ResidentialDetails = MakeISM(TEXT("ResidentialDetails"), TEXT("NoCollision"));
    LandmarkBlocks = MakeISM(TEXT("LandmarkBlocks"), TEXT("BlockAll"));
    LandmarkRoofs = MakeISM(TEXT("LandmarkRoofs"), TEXT("BlockAll"));
    LandmarkWindows = MakeISM(TEXT("LandmarkWindows"), TEXT("NoCollision"));
    LandmarkDetails = MakeISM(TEXT("LandmarkDetails"), TEXT("BlockAll"));
    Fences = MakeISM(TEXT("Fences"), TEXT("BlockAll"));
    WoodFences = MakeISM(TEXT("WoodFences"), TEXT("BlockAll"));
    MetalFences = MakeISM(TEXT("MetalFences"), TEXT("BlockAll"));
    LightSheetFences = MakeISM(TEXT("LightSheetFences"), TEXT("BlockAll"));
    TreeTrunks = MakeISM(TEXT("TreeTrunks"), TEXT("BlockAll"));
    TreeCrowns = MakeISM(TEXT("TreeCrowns"), TEXT("NoCollision"));
    SovietPoplarTrunks = MakeISM(TEXT("SovietPoplarTrunks"), TEXT("BlockAll"));
    SovietPoplarCrowns = MakeISM(TEXT("SovietPoplarCrowns"), TEXT("NoCollision"));
    BirchTrunks = MakeISM(TEXT("BirchTrunks"), TEXT("BlockAll"));
    BirchCrowns = MakeISM(TEXT("BirchCrowns"), TEXT("NoCollision"));
    PineTrunks = MakeISM(TEXT("PineTrunks"), TEXT("BlockAll"));
    PineCrowns = MakeISM(TEXT("PineCrowns"), TEXT("NoCollision"));
    GrassMown = MakeISM(TEXT("GrassMown"), TEXT("NoCollision"));
    GrassRough = MakeISM(TEXT("GrassRough"), TEXT("NoCollision"));
    GrassWetland = MakeISM(TEXT("GrassWetland"), TEXT("NoCollision"));
    StadiumGeometry = MakeISM(TEXT("StadiumGeometry"), TEXT("BlockAll"));
    StadiumDetails = MakeISM(TEXT("StadiumDetails"), TEXT("BlockAll"));
    ParkGeometry = MakeISM(TEXT("ParkGeometry"), TEXT("BlockAll"));
    ParkDetails = MakeISM(TEXT("ParkDetails"), TEXT("BlockAll"));
    Waterways = MakeISM(TEXT("Waterways"), TEXT("NoCollision"));
    Bridges = MakeISM(TEXT("Bridges"), TEXT("BlockAll"));
    ReferenceMarkers = MakeISM(TEXT("ReferenceMarkers"), TEXT("NoCollision"));

    MuseumLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("MuseumLabel"));
    MuseumLabel->SetupAttachment(SceneRoot);
    StadiumLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("StadiumLabel"));
    StadiumLabel->SetupAttachment(SceneRoot);
    ParkLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ParkLabel"));
    ParkLabel->SetupAttachment(SceneRoot);
    CollegeLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("CollegeLabel"));
    CollegeLabel->SetupAttachment(SceneRoot);
    KrushelnytskaStreetLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("KrushelnytskaStreetLabel"));
    KrushelnytskaStreetLabel->SetupAttachment(SceneRoot);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));

    if (CubeMesh.Succeeded())
    {
        Ground->SetStaticMesh(CubeMesh.Object);
        UInstancedStaticMeshComponent* CubeComponents[] =
        {
            Roads, Sidewalks, Buildings, ResidentialRoofs, ResidentialDetails,
            LandmarkBlocks, LandmarkRoofs, LandmarkWindows, LandmarkDetails,
            Fences, WoodFences, MetalFences, LightSheetFences, StadiumGeometry, StadiumDetails, ParkGeometry, ParkDetails,
            GrassMown, GrassRough, GrassWetland,
            Waterways, Bridges, ReferenceMarkers
        };
        for (UInstancedStaticMeshComponent* Component : CubeComponents)
        {
            Component->SetStaticMesh(CubeMesh.Object);
        }
    }
    if (CylinderMesh.Succeeded())
    {
        TreeTrunks->SetStaticMesh(CylinderMesh.Object);
        SovietPoplarTrunks->SetStaticMesh(CylinderMesh.Object);
        BirchTrunks->SetStaticMesh(CylinderMesh.Object);
        PineTrunks->SetStaticMesh(CylinderMesh.Object);
    }
    if (SphereMesh.Succeeded())
    {
        TreeCrowns->SetStaticMesh(SphereMesh.Object);
        SovietPoplarCrowns->SetStaticMesh(SphereMesh.Object);
        BirchCrowns->SetStaticMesh(SphereMesh.Object);
        PineCrowns->SetStaticMesh(SphereMesh.Object);
    }

    Ground->SetRelativeLocation(FVector(0.0f, 0.0f, -100.0f));
    Ground->SetRelativeScale3D(FVector(MapWidthCm / 100.0f, MapHeightCm / 100.0f, 2.0f));

    BuildRoadNetwork();
    BuildHydrography();
    BuildVerifiedReferenceMarkers();
    BuildMuseumAndStadium();
    BuildCentralPark();
    BuildCollegeSector();
    BuildSolomiiKrushelnytskoiStreet();
    BuildResidentialBlocks();
    BuildVegetation();
    BuildGameplayBases();

    // S07 regression marker: OSTER LOCAL HISTORY MUSEUM / TATARIVSKA 30
    // S07 regression marker: OSTER COLLEGE / SOLOMII KRUSHELNYTSKOI 7A
    // S08 regression marker: SOLOMII KRUSHELNYTSKOI STREET / S08
    ConfigureLabel(MuseumLabel, TEXT("OSTER LOCAL HISTORY MUSEUM / SOLONYNA HOUSE / S16A VERIFIED ANCHOR"),
        MuseumAnchor() + FVector(0, 0, 1050));
    ConfigureLabel(StadiumLabel, TEXT("OSTER CENTRAL STADIUM / CANONICAL GEO ANCHOR"),
        StadiumAnchor() + FVector(0, 0, 700));
    ConfigureLabel(ParkLabel, TEXT("OSTER CENTRAL CITY PARK / S16A VERIFIED ANCHOR"), ParkAnchor() + FVector(0, 0, 800));
    ConfigureLabel(CollegeLabel, TEXT("OSTER COLLEGE / SOLOMII KRUSHELNYTSKOI 7A / S16A VERIFIED ANCHOR"),
        CollegeAnchor() + FVector(0, 0, 1700));
    ConfigureLabel(KrushelnytskaStreetLabel, TEXT("SOLOMII KRUSHELNYTSKOI STREET / OSTER"),
        FVector(-33500.0f, 32000.0f, 720.0f));
}

void AOCWorldSectorOster::BeginPlay()
{
    Super::BeginPlay();

    UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

    auto Tint = [BaseMaterial](UPrimitiveComponent* Component, const FLinearColor& Color)
    {
        if (!BaseMaterial || !Component) return;
        UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMaterial, Component);
        if (!MID) return;
        MID->SetVectorParameterValue(TEXT("Color"), Color);
        Component->SetMaterial(0, MID);
    };

    Tint(Ground,              FLinearColor(0.16f, 0.25f, 0.10f));
    Tint(Roads,               FLinearColor(0.055f, 0.060f, 0.065f));
    Tint(Sidewalks,           FLinearColor(0.42f, 0.43f, 0.41f));
    Tint(Buildings,           FLinearColor(0.58f, 0.49f, 0.34f));
    Tint(ResidentialRoofs,    FLinearColor(0.28f, 0.075f, 0.045f));
    Tint(ResidentialDetails,  FLinearColor(0.72f, 0.77f, 0.74f));
    Tint(LandmarkBlocks,      FLinearColor(0.56f, 0.26f, 0.13f));
    Tint(LandmarkRoofs,       FLinearColor(0.25f, 0.065f, 0.045f));
    Tint(LandmarkWindows,     FLinearColor(0.12f, 0.29f, 0.38f));
    Tint(LandmarkDetails,     FLinearColor(0.54f, 0.50f, 0.42f));
    Tint(Fences,              FLinearColor(0.26f, 0.28f, 0.25f));
    Tint(WoodFences,          FLinearColor(0.30f, 0.16f, 0.075f));
    Tint(MetalFences,         FLinearColor(0.18f, 0.21f, 0.22f));
    Tint(LightSheetFences,    FLinearColor(0.46f, 0.49f, 0.45f));
    Tint(TreeTrunks,          FLinearColor(0.19f, 0.095f, 0.035f));
    Tint(TreeCrowns,          FLinearColor(0.09f, 0.27f, 0.055f));
    Tint(SovietPoplarTrunks,  FLinearColor(0.20f, 0.11f, 0.045f));
    Tint(SovietPoplarCrowns,  FLinearColor(0.075f, 0.30f, 0.065f));
    Tint(BirchTrunks,         FLinearColor(0.63f, 0.62f, 0.54f));
    Tint(BirchCrowns,         FLinearColor(0.15f, 0.38f, 0.075f));
    Tint(PineTrunks,          FLinearColor(0.17f, 0.085f, 0.035f));
    Tint(PineCrowns,          FLinearColor(0.035f, 0.18f, 0.055f));
    Tint(GrassMown,           FLinearColor(0.18f, 0.34f, 0.095f));
    Tint(GrassRough,          FLinearColor(0.24f, 0.38f, 0.10f));
    Tint(GrassWetland,        FLinearColor(0.13f, 0.28f, 0.12f));
    Tint(StadiumGeometry,     FLinearColor(0.055f, 0.31f, 0.12f));
    Tint(StadiumDetails,      FLinearColor(0.82f, 0.82f, 0.76f));
    Tint(ParkGeometry,        FLinearColor(0.12f, 0.31f, 0.075f));
    Tint(ParkDetails,         FLinearColor(0.40f, 0.34f, 0.25f));
    Tint(Waterways,           FLinearColor(0.055f, 0.22f, 0.36f));
    Tint(Bridges,             FLinearColor(0.32f, 0.31f, 0.29f));

    if (ReferenceMarkers) ReferenceMarkers->SetVisibility(false, true);
    UTextRenderComponent* Labels[] = { MuseumLabel, StadiumLabel, ParkLabel, CollegeLabel, KrushelnytskaStreetLabel };
    for (UTextRenderComponent* Label : Labels)
    {
        if (Label) Label->SetVisibility(false, true);
    }

    if (GrassMown) GrassMown->SetCastShadow(false);
    if (GrassRough) GrassRough->SetCastShadow(false);
    if (GrassWetland) GrassWetland->SetCastShadow(false);
    if (Waterways) Waterways->SetCastShadow(false);
}

FVector AOCWorldSectorOster::MuseumAnchor()
{
    const FOCGeoReferencePoint Ref = FOCGeoReference::Museum();
    return FOCGeoReference::ToLocalCm(Ref.Latitude, Ref.Longitude, GroundTopZ);
}

FVector AOCWorldSectorOster::CollegeAnchor()
{
    const FOCGeoReferencePoint Ref = FOCGeoReference::College();
    return FOCGeoReference::ToLocalCm(Ref.Latitude, Ref.Longitude, GroundTopZ);
}

FVector AOCWorldSectorOster::ParkAnchor()
{
    const FOCGeoReferencePoint Ref = FOCGeoReference::CentralPark();
    return FOCGeoReference::ToLocalCm(Ref.Latitude, Ref.Longitude, GroundTopZ);
}

FVector AOCWorldSectorOster::CultureParkNorthAnchor()
{
    const FOCGeoReferencePoint Ref = FOCGeoReference::CultureParkNorth();
    return FOCGeoReference::ToLocalCm(Ref.Latitude, Ref.Longitude, GroundTopZ);
}

FVector AOCWorldSectorOster::FormerCityAdministrationAnchor()
{
    const FOCGeoReferencePoint Ref = FOCGeoReference::FormerCityAdministration();
    return FOCGeoReference::ToLocalCm(Ref.Latitude, Ref.Longitude, GroundTopZ);
}

FVector AOCWorldSectorOster::HistoricCourtAnchor()
{
    const FOCGeoReferencePoint Ref = FOCGeoReference::HistoricCourtBuilding();
    return FOCGeoReference::ToLocalCm(Ref.Latitude, Ref.Longitude, GroundTopZ);
}

FVector AOCWorldSectorOster::ResurrectionChurchAnchor()
{
    const FOCGeoReferencePoint Ref = FOCGeoReference::ResurrectionChurch();
    return FOCGeoReference::ToLocalCm(Ref.Latitude, Ref.Longitude, GroundTopZ);
}

FVector AOCWorldSectorOster::StadiumAnchor()
{
    const FOCGeoReferencePoint Ref = FOCGeoReference::Stadium();
    return FOCGeoReference::ToLocalCm(Ref.Latitude, Ref.Longitude, GroundTopZ);
}

FVector AOCWorldSectorOster::KrushelnytskaEnterableHouseAnchor()
{
    return FVector(-27000.0f, 28500.0f, GroundTopZ);
}

float AOCWorldSectorOster::KrushelnytskaEnterableHouseYaw() { return -88.0f; }

void AOCWorldSectorOster::AddBox(UInstancedStaticMeshComponent* Component, const FVector& Center,
    const FVector& SizeCm, float YawDegrees)
{
    AddBoxRotated(Component, Center, SizeCm, FRotator(0.0f, YawDegrees, 0.0f));
}

void AOCWorldSectorOster::AddBoxRotated(UInstancedStaticMeshComponent* Component, const FVector& Center,
    const FVector& SizeCm, const FRotator& Rotation)
{
    if (!Component) return;
    FTransform Transform;
    Transform.SetLocation(Center);
    Transform.SetRotation(FQuat(Rotation));
    Transform.SetScale3D(SizeCm / 100.0f);
    Component->AddInstance(Transform);
}

void AOCWorldSectorOster::AddCylinder(UInstancedStaticMeshComponent* Component, const FVector& Center,
    float RadiusCm, float HeightCm)
{
    if (!Component) return;
    FTransform Transform;
    Transform.SetLocation(Center);
    Transform.SetScale3D(FVector(RadiusCm / 50.0f, RadiusCm / 50.0f, HeightCm / 100.0f));
    Component->AddInstance(Transform);
}

void AOCWorldSectorOster::AddGableRoof(UInstancedStaticMeshComponent* Component, const FVector& Center,
    float WidthCm, float DepthCm, float RidgeZCm, float YawDegrees, float SlopeDegrees)
{
    if (!Component) return;

    const float HalfDepth = DepthCm * 0.5f;
    const float PlaneDepth = DepthCm * 0.58f;
    const float Rise = FMath::Tan(FMath::DegreesToRadians(SlopeDegrees)) * HalfDepth;
    const float PlaneZ = RidgeZCm - Rise * 0.48f;

    const FVector LeftOffset = Rotate2D(FVector(0.0f, -DepthCm * 0.255f, 0.0f), YawDegrees);
    const FVector RightOffset = Rotate2D(FVector(0.0f, DepthCm * 0.255f, 0.0f), YawDegrees);

    AddBoxRotated(Component, FVector(Center.X + LeftOffset.X, Center.Y + LeftOffset.Y, PlaneZ),
        FVector(WidthCm + 120.0f, PlaneDepth, 20.0f), FRotator(0.0f, YawDegrees, -SlopeDegrees));
    AddBoxRotated(Component, FVector(Center.X + RightOffset.X, Center.Y + RightOffset.Y, PlaneZ),
        FVector(WidthCm + 120.0f, PlaneDepth, 20.0f), FRotator(0.0f, YawDegrees, SlopeDegrees));
}

void AOCWorldSectorOster::AddFacadeWindow(UInstancedStaticMeshComponent* Component, const FVector& BuildingCenter,
    const FVector& LocalOffset, const FVector& SizeCm, float BuildingYawDegrees, bool bFrontFacade)
{
    const FVector WorldOffset = Rotate2D(LocalOffset, BuildingYawDegrees);
    const float WindowYaw = BuildingYawDegrees + (bFrontFacade ? 0.0f : 90.0f);
    AddBox(Component, BuildingCenter + WorldOffset, SizeCm, WindowYaw);
}

void AOCWorldSectorOster::BuildGameplayBases()
{
    struct FBaseSeed { FVector Center; float Yaw; };
    const FBaseSeed Bases[] =
    {
        { FVector(-104000.0f, -92000.0f, 0.0f), 35.0f },
        { FVector(104000.0f, 92000.0f, 0.0f), 215.0f }
    };

    for (const FBaseSeed& Base : Bases)
    {
        AddBox(Sidewalks, Base.Center + FVector(0,0,10), FVector(8200, 6200, 20), Base.Yaw);
        AddBox(Roads, Base.Center + Rotate2D(FVector(6500,0,8), Base.Yaw), FVector(7600, 780, 16), Base.Yaw);

        AddBox(Buildings, Base.Center + Rotate2D(FVector(-1500,-1450,230), Base.Yaw), FVector(2200,1200,460), Base.Yaw);
        AddGableRoof(ResidentialRoofs, Base.Center + Rotate2D(FVector(-1500,-1450,0), Base.Yaw),
            2350, 1350, 610, Base.Yaw, 20.0f);
        AddBox(Buildings, Base.Center + Rotate2D(FVector(-1500,1450,230), Base.Yaw), FVector(2200,1200,460), Base.Yaw);
        AddGableRoof(ResidentialRoofs, Base.Center + Rotate2D(FVector(-1500,1450,0), Base.Yaw),
            2350, 1350, 610, Base.Yaw, 20.0f);
        AddBox(Buildings, Base.Center + Rotate2D(FVector(1700,1800,170), Base.Yaw), FVector(1200,900,340), Base.Yaw);

        for (int32 I=-2; I<=2; ++I)
        {
            AddBox(ParkDetails, Base.Center + Rotate2D(FVector(1650, I*720.0f, 55), Base.Yaw),
                FVector(280,560,110), Base.Yaw + 12.0f * I);
        }

        AddBox(MetalFences, Base.Center + Rotate2D(FVector(-3400,0,125), Base.Yaw), FVector(45,6000,250), Base.Yaw);
        AddBox(MetalFences, Base.Center + Rotate2D(FVector(0,-3000,125), Base.Yaw), FVector(6800,45,250), Base.Yaw);
        AddBox(MetalFences, Base.Center + Rotate2D(FVector(0,3000,125), Base.Yaw), FVector(6800,45,250), Base.Yaw);
        AddBox(MetalFences, Base.Center + Rotate2D(FVector(3400,-2100,125), Base.Yaw), FVector(45,1800,250), Base.Yaw);
        AddBox(MetalFences, Base.Center + Rotate2D(FVector(3400,2100,125), Base.Yaw), FVector(45,1800,250), Base.Yaw);
    }
}

void AOCWorldSectorOster::ConfigureLabel(UTextRenderComponent* Label, const FString& Text, const FVector& Location)
{
    if (!Label) return;
    Label->SetText(FText::FromString(Text));
    Label->SetRelativeLocation(Location);
    Label->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
    Label->SetHorizontalAlignment(EHTA_Center);
    Label->SetWorldSize(120.0f);
    Label->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AOCWorldSectorOster::BuildRoadNetwork()
{
    auto AddRoadWithWalks = [this](const FVector& Center, const FVector& Size, float Yaw, bool bTwoWalks = true)
    {
        AddBox(Roads, Center, Size, Yaw);
        const float HalfWidth = Size.Y * 0.5f;
        const FVector LateralA = Rotate2D(FVector(0.0f, HalfWidth + 260.0f, 10.0f), Yaw);
        AddBox(Sidewalks, Center + LateralA, FVector(Size.X, 260.0f, 18.0f), Yaw);
        if (bTwoWalks)
        {
            const FVector LateralB = Rotate2D(FVector(0.0f, -HalfWidth - 260.0f, 10.0f), Yaw);
            AddBox(Sidewalks, Center + LateralB, FVector(Size.X, 260.0f, 18.0f), Yaw);
        }
    };

    AddRoadWithWalks(FVector(-5000, -9000, RoadZ), FVector(138000, 1050, 16), 0.0f);
    AddRoadWithWalks(FVector(-18000, 17000, RoadZ), FVector(61000, 820, 16), 0.0f);
    AddRoadWithWalks(FVector(-33500, 25000, RoadZ), FVector(112000, 920, 16), 91.5f);
    AddRoadWithWalks(FVector(-23500, 40500, RoadZ), FVector(51000, 760, 16), 18.0f);
    AddRoadWithWalks(FVector(-48000, 51000, RoadZ), FVector(52000, 720, 16), 63.0f, false);
    AddRoadWithWalks(FVector(-5000, 33500, RoadZ), FVector(49000, 760, 16), -34.0f);
    AddRoadWithWalks(FVector(20500, 20500, RoadZ), FVector(52000, 780, 16), 73.0f, false);
    AddRoadWithWalks(FVector(31500, -23000, RoadZ), FVector(63000, 850, 16), 4.0f, false);
    AddRoadWithWalks(FVector(-33000, -25500, RoadZ), FVector(56000, 760, 16), -7.0f, false);

    const FVector Park = ParkAnchor();
    AddRoadWithWalks(Park + FVector(0, -8500, RoadZ), FVector(43000, 720, 16), 2.0f);
    AddRoadWithWalks(Park + FVector(-9000, 13500, RoadZ), FVector(37000, 700, 16), 79.0f, false);

    const FVector College = CollegeAnchor();
    AddRoadWithWalks(College + FVector(-13500, 0, RoadZ), FVector(30000, 660, 14), 0.0f);

    AddBox(Roads, FVector(-86500, 15000, RoadZ), FVector(780, 138000, 16), 7.0f);
    AddBox(Roads, FVector(65500, 15000, RoadZ), FVector(780, 120000, 16), -5.0f);
    AddBox(Roads, FVector(2500, -67500, RoadZ), FVector(130000, 820, 16), -3.0f);
}

void AOCWorldSectorOster::BuildHydrography()
{
    struct FWaterSeed { FVector Center; FVector Size; float Yaw; };
    const FWaterSeed DesnaSeeds[] =
    {
        { FVector(-112000, -25000, -18), FVector(17000, 72000, 20), 3.0f },
        { FVector(-104000, 38000, -18), FVector(19000, 72000, 20), 13.0f },
        { FVector(-83000, 92000, -18), FVector(22000, 60000, 20), 36.0f }
    };
    for (const FWaterSeed& Seed : DesnaSeeds) AddBox(Waterways, Seed.Center, Seed.Size, Seed.Yaw);

    const FWaterSeed OsterSeeds[] =
    {
        { FVector(-52000, -98000, -16), FVector(62000, 9000, 18), -7.0f },
        { FVector(3000, -101000, -16), FVector(57000, 8200, 18), 3.0f },
        { FVector(52000, -87000, -16), FVector(56000, 8000, 18), 26.0f },
        { FVector(82000, -52000, -16), FVector(51000, 7600, 18), 67.0f }
    };
    for (const FWaterSeed& Seed : OsterSeeds) AddBox(Waterways, Seed.Center, Seed.Size, Seed.Yaw);

    AddBox(Bridges, FVector(-17000, -100000, 75), FVector(1200, 11800, 150), 87.0f);
    AddBox(Bridges, FVector(76000, -65000, 75), FVector(1200, 10500, 150), 28.0f);
}

void AOCWorldSectorOster::BuildVerifiedReferenceMarkers()
{
    const FVector Points[] =
    {
        MuseumAnchor(), StadiumAnchor(), CollegeAnchor(), ParkAnchor(), CultureParkNorthAnchor(),
        FormerCityAdministrationAnchor(), HistoricCourtAnchor(), ResurrectionChurchAnchor()
    };
    for (const FVector& P : Points)
    {
        AddBox(ReferenceMarkers, P + FVector(0,0,220), FVector(70,70,440));
    }
}

void AOCWorldSectorOster::BuildMuseumAndStadium()
{
    const FVector Museum = MuseumAnchor();
    const float MuseumYaw = 0.0f;

    AddBox(LandmarkBlocks, Museum + FVector(0, 0, 270), FVector(3400, 1750, 540), MuseumYaw);
    AddBox(LandmarkBlocks, Museum + FVector(-2050, 120, 245), FVector(1100, 1500, 490), MuseumYaw);
    AddBox(LandmarkBlocks, Museum + FVector(2050, 80, 245), FVector(1100, 1500, 490), MuseumYaw);
    AddBox(LandmarkBlocks, Museum + FVector(50, 80, 720), FVector(1550, 1280, 420), MuseumYaw);

    AddBox(LandmarkDetails, Museum + FVector(1180, -1120, 250), FVector(1050, 620, 500), MuseumYaw);
    AddBox(LandmarkDetails, Museum + FVector(1180, -1510, 55), FVector(1450, 760, 110), MuseumYaw);
    for (int32 Step = 0; Step < 4; ++Step)
    {
        AddBox(LandmarkDetails, Museum + FVector(1180, -1880 - Step * 120.0f, 22.0f + Step * 20.0f),
            FVector(1550 - Step * 90.0f, 220, 35), MuseumYaw);
    }

    AddGableRoof(LandmarkRoofs, Museum + FVector(0, 0, 0), 3550, 1900, 1160, MuseumYaw, 30.0f);
    AddGableRoof(LandmarkRoofs, Museum + FVector(-2050, 120, 0), 1250, 1650, 720, MuseumYaw, 24.0f);
    AddGableRoof(LandmarkRoofs, Museum + FVector(2050, 80, 0), 1250, 1650, 720, MuseumYaw, 24.0f);
    AddGableRoof(LandmarkRoofs, Museum + FVector(1180, -1120, 0), 1200, 760, 650, MuseumYaw, 28.0f);

    AddBox(LandmarkDetails, Museum + FVector(0, -40, 1080), FVector(1500, 90, 95), MuseumYaw);
    AddBox(LandmarkDetails, Museum + FVector(-1520, 250, 970), FVector(190, 190, 520), MuseumYaw);
    AddBox(LandmarkDetails, Museum + FVector(1580, 180, 940), FVector(180, 180, 460), MuseumYaw);
    AddBox(LandmarkDetails, Museum + FVector(0, -910, 525), FVector(3350, 55, 90), MuseumYaw);

    const float MuseumWindowX[] = { -2650.0f, -1850.0f, -650.0f, 100.0f, 750.0f, 2250.0f };
    for (float X : MuseumWindowX)
    {
        AddFacadeWindow(LandmarkWindows, Museum, FVector(X, -885, 300), FVector(420, 24, 250), MuseumYaw, true);
    }
    AddFacadeWindow(LandmarkWindows, Museum, FVector(-420, -655, 770), FVector(330, 24, 260), MuseumYaw, true);
    AddFacadeWindow(LandmarkWindows, Museum, FVector(70, -655, 770), FVector(330, 24, 260), MuseumYaw, true);
    AddFacadeWindow(LandmarkWindows, Museum, FVector(560, -655, 770), FVector(330, 24, 260), MuseumYaw, true);

    AddBox(Fences, Museum + FVector(0, -2500, 90), FVector(6000, 40, 180));
    AddBox(Fences, Museum + FVector(0, 2450, 90), FVector(6000, 40, 180));
    AddBox(Fences, Museum + FVector(-3000, 0, 90), FVector(40, 4900, 180));

    const FVector Stadium = StadiumAnchor();
    AddBox(StadiumGeometry, Stadium + FVector(0, 0, 4), FVector(11900, 8200, 8));
    AddBox(StadiumGeometry, Stadium + FVector(0, 0, 12), FVector(10500, 6800, 12));

    AddBox(StadiumGeometry, Stadium + FVector(0, -3740, 18), FVector(11200, 520, 14));
    AddBox(StadiumGeometry, Stadium + FVector(0, 3740, 18), FVector(11200, 520, 14));
    AddBox(StadiumGeometry, Stadium + FVector(-5600, 0, 18), FVector(520, 7400, 14));
    AddBox(StadiumGeometry, Stadium + FVector(5600, 0, 18), FVector(520, 7400, 14));

    AddBox(StadiumDetails, Stadium + FVector(0, 0, 25), FVector(10400, 18, 8));
    AddBox(StadiumDetails, Stadium + FVector(-5200, 0, 25), FVector(18, 6750, 8));
    AddBox(StadiumDetails, Stadium + FVector(5200, 0, 25), FVector(18, 6750, 8));

    AddBox(StadiumDetails, Stadium + FVector(0, -4750, 150), FVector(5200, 720, 300));
    AddBox(StadiumDetails, Stadium + FVector(2600, -5250, 300), FVector(2400, 850, 600));
    AddBox(StadiumDetails, Stadium + FVector(-3750, -5050, 210), FVector(1300, 800, 420));

    AddBox(Fences, Stadium + FVector(0, -4300, 125), FVector(12400, 35, 250));
    AddBox(Fences, Stadium + FVector(0, 4300, 125), FVector(12400, 35, 250));
    AddBox(Fences, Stadium + FVector(-6200, 0, 125), FVector(35, 8600, 250));
    AddBox(Fences, Stadium + FVector(6200, 0, 125), FVector(35, 8600, 250));

    for (float GoalX : { -5200.0f, 5200.0f })
    {
        AddBox(StadiumDetails, Stadium + FVector(GoalX, -365, 125), FVector(30, 30, 250));
        AddBox(StadiumDetails, Stadium + FVector(GoalX, 365, 125), FVector(30, 30, 250));
        AddBox(StadiumDetails, Stadium + FVector(GoalX, 0, 250), FVector(30, 760, 30));
    }
}

void AOCWorldSectorOster::BuildCentralPark()
{
    const FVector Park = ParkAnchor();

    AddBox(ParkGeometry, Park + FVector(0, 0, 3), FVector(20500, 16000, 6));
    AddBox(Sidewalks, Park + FVector(0, 0, 14), FVector(17800, 360, 18));
    AddBox(Sidewalks, Park + FVector(0, -300, 14), FVector(360, 13200, 18));
    AddBox(Sidewalks, Park + FVector(1800, 900, 14), FVector(11800, 260, 18), 31.0f);
    AddBox(Sidewalks, Park + FVector(-2300, 1300, 14), FVector(9300, 240, 18), -28.0f);

    AddBox(ParkDetails, Park + FVector(-600, 200, 28), FVector(3100, 2500, 56));
    AddBox(ParkDetails, Park + FVector(-600, 200, 230), FVector(260, 260, 400));
    for (int32 Step = 0; Step < 4; ++Step)
    {
        AddBox(ParkDetails, Park + FVector(-6100 + Step * 150.0f, -4900, 18 + Step * 14.0f),
            FVector(1900 - Step * 120.0f, 260, 28), 0.0f);
    }

    AddBox(ParkDetails, Park + FVector(6100, -4100, 18), FVector(4300, 2600, 36));
    AddBoxRotated(ParkDetails, Park + FVector(6100, -4100, 120), FVector(1200, 600, 35), FRotator(0, 0, 16));
    AddBoxRotated(ParkDetails, Park + FVector(7400, -3500, 95), FVector(950, 500, 30), FRotator(0, 90, -13));

    const FVector NorthCivic = CultureParkNorthAnchor();
    const FVector Mid = (Park + NorthCivic) * 0.5f;
    const FVector Delta = NorthCivic - Park;
    const float LinkYaw = FMath::RadiansToDegrees(FMath::Atan2(Delta.Y, Delta.X));
    AddBox(ParkGeometry, NorthCivic + FVector(0,0,4), FVector(8500, 7200, 8));
    AddBox(Sidewalks, Mid + FVector(0,0,15), FVector(Delta.Size2D(), 260, 18), LinkYaw);

    for (int32 I = -3; I <= 3; ++I)
    {
        AddBox(ParkDetails, Park + FVector(I * 1900.0f, -850.0f, 60.0f), FVector(180, 55, 120));
        AddBox(ParkDetails, Park + FVector(I * 1900.0f, 850.0f, 60.0f), FVector(180, 55, 120));
    }
}

void AOCWorldSectorOster::BuildCollegeSector()
{
    const FVector College = CollegeAnchor();
    const float Yaw = 1.0f;

    const FVector MainCenter = College + FVector(0, 0, 720);
    AddBox(LandmarkBlocks, MainCenter, FVector(6500, 1900, 1440), Yaw);
    AddBox(LandmarkRoofs, College + FVector(0, 0, 1460), FVector(6650, 2020, 70), Yaw);

    AddBox(LandmarkDetails, College + FVector(900, -1230, 230), FVector(2450, 600, 460), Yaw);
    AddBox(LandmarkDetails, College + FVector(900, -1590, 505), FVector(2650, 920, 70), Yaw);
    for (int32 Step = 0; Step < 5; ++Step)
    {
        AddBox(LandmarkDetails, College + FVector(900, -1940 - Step * 115.0f, 22 + Step * 22.0f),
            FVector(2750 - Step * 100.0f, 220, 40), Yaw);
    }

    constexpr int32 Columns = 9;
    constexpr int32 Rows = 4;
    for (int32 Row = 0; Row < Rows; ++Row)
    {
        for (int32 Col = 0; Col < Columns; ++Col)
        {
            const float X = -2800.0f + Col * 700.0f;
            const float Z = 255.0f + Row * 340.0f;
            if (Row == 0 && (Col == 5 || Col == 6)) continue;
            AddFacadeWindow(LandmarkWindows, College, FVector(X, -965, Z), FVector(430, 24, 220), Yaw, true);
        }
    }

    AddBox(LandmarkBlocks, College + FVector(-2450, 2500, 520), FVector(2200, 4200, 1040), Yaw);
    AddBox(LandmarkBlocks, College + FVector(3100, 2650, 320), FVector(2500, 1900, 640), Yaw);
    AddBox(LandmarkRoofs, College + FVector(-2450, 2500, 1060), FVector(2320, 4320, 60), Yaw);
    AddBox(LandmarkRoofs, College + FVector(3100, 2650, 660), FVector(2620, 2020, 55), Yaw);

    AddBox(LandmarkBlocks, College + FVector(-5200, 4700, 430), FVector(1900, 3600, 860), Yaw + 2.0f);
    AddBox(LandmarkRoofs, College + FVector(-5200, 4700, 885), FVector(2020, 3720, 55), Yaw + 2.0f);
    AddBox(LandmarkBlocks, College + FVector(4800, 6000, 510), FVector(2100, 4300, 1020), Yaw - 1.0f);
    AddBox(LandmarkRoofs, College + FVector(4800, 6000, 1045), FVector(2220, 4420, 55), Yaw - 1.0f);
    AddBox(LandmarkBlocks, College + FVector(9000, 2600, 340), FVector(2600, 1500, 680), Yaw);

    AddBox(Sidewalks, College + FVector(900, 5200, 12), FVector(8000, 5900, 18), Yaw);
    AddBox(ParkGeometry, College + FVector(-4900, 7000, 10), FVector(6100, 3300, 12), Yaw);
    AddBox(Fences, College + FVector(0, -2450, 110), FVector(10400, 45, 220), Yaw);
    AddBox(Fences, College + FVector(0, 9300, 110), FVector(11200, 45, 220), Yaw);
    AddBox(Fences, College + FVector(-5600, 3400, 110), FVector(45, 11700, 220), Yaw);
}

void AOCWorldSectorOster::BuildSolomiiKrushelnytskoiStreet()
{
    auto AddHouseArchetype = [this](const FOCS01ResidentialPlotSeed& Plot)
    {
        if (!Plot.bHasPrimaryHouse) return;

        const FVector& Center = Plot.HouseCenter;
        const FVector& Size = Plot.HouseSizeCm;
        AddBox(Buildings, Center, Size, Plot.HouseYaw);
        AddGableRoof(ResidentialRoofs, Center, Size.X + 120.0f, Size.Y + 160.0f,
            Center.Z + Size.Z * 0.5f + 245.0f, Plot.HouseYaw,
            24.0f + static_cast<float>((Plot.VisualVariant % 3) * 3));

        const int32 WindowCount = Plot.VisualVariant % 2 == 0 ? 3 : 2;
        for (int32 W = 0; W < WindowCount; ++W)
        {
            const float X = (static_cast<float>(W) - (WindowCount - 1) * 0.5f) *
                (Size.X / (WindowCount + 0.8f));
            AddFacadeWindow(ResidentialDetails, Center, FVector(X, -Size.Y * 0.505f, 40.0f),
                FVector(280, 18, 190), Plot.HouseYaw, true);
        }
        AddFacadeWindow(ResidentialDetails, Center, FVector(Size.X * 0.34f, -Size.Y * 0.51f, -25.0f),
            FVector(220, 22, 310), Plot.HouseYaw, true);
    };

    for (const FOCS01ResidentialPlotSeed& Plot : FOCLocationSectorS01Data::ProvisionalResidentialPlots())
    {
        AddHouseArchetype(Plot);

        if (!Plot.bHasOutbuilding) continue;
        AddBox(Buildings, Plot.OutbuildingCenter, Plot.OutbuildingSizeCm, Plot.OutbuildingYaw);
        if (Plot.bOutbuildingHasRoof)
        {
            AddGableRoof(ResidentialRoofs, Plot.OutbuildingCenter,
                Plot.OutbuildingSizeCm.X + 80.0f, Plot.OutbuildingSizeCm.Y + 100.0f,
                Plot.OutbuildingCenter.Z + Plot.OutbuildingSizeCm.Z * 0.5f + 90.0f,
                Plot.OutbuildingYaw, 24.0f);
        }
    }

    for (const FOCS01FrontageSeed& Frontage : FOCLocationSectorS01Data::ProvisionalFrontages())
    {
        AddBox(Fences, Frontage.WestFenceCenter, Frontage.FenceSizeCm, Frontage.FenceYaw);
        AddBox(Fences, Frontage.EastFenceCenter, Frontage.FenceSizeCm, Frontage.FenceYaw);
        AddBox(Sidewalks, Frontage.WestWalkCenter, Frontage.WalkSizeCm, Frontage.WalkYaw);
        AddBox(Sidewalks, Frontage.EastWalkCenter, Frontage.WalkSizeCm, Frontage.WalkYaw);
    }

    for (const FOCS01RoadSeed& Road : FOCLocationSectorS01Data::ProvisionalServiceRoads())
    {
        AddBox(Roads, Road.Center, Road.SizeCm, Road.Yaw);
    }
}

void AOCWorldSectorOster::BuildResidentialBlocks()
{
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
        if (FOCLocationSectorPlan::IsInsideKrushelnytskaCollegePark(Block.Origin)) continue;

        for (int32 Row = 0; Row < Block.Rows; ++Row)
        {
            for (int32 Col = 0; Col < Block.Columns; ++Col)
            {
                const float OffsetJitter = static_cast<float>((HouseCounter % 3) - 1) * 170.0f;
                const FVector Center = Block.Origin + FVector(Col * Block.Spacing.X + OffsetJitter,
                    Row * Block.Spacing.Y - OffsetJitter, 260.0f);
                const float Width = 1600.0f + static_cast<float>((HouseCounter % 4) * 180);
                const float Depth = 1050.0f + static_cast<float>((HouseCounter % 3) * 130);
                const float Height = 480.0f + static_cast<float>((HouseCounter % 2) * 120);
                const float HouseYaw = Block.Yaw + (HouseCounter % 2 == 0 ? -5.0f : 5.0f);

                AddBox(Buildings, Center, FVector(Width, Depth, Height), HouseYaw);
                AddGableRoof(ResidentialRoofs, Center, Width + 120.0f, Depth + 140.0f,
                    Center.Z + Height * 0.5f + 230.0f, HouseYaw, 24.0f + (HouseCounter % 3) * 2.0f);

                const FVector ShedOffset = Rotate2D(FVector(-Width * 0.32f, Depth * 1.35f, -120.0f), HouseYaw);
                AddBox(Buildings, Center + ShedOffset, FVector(620, 900, 280), HouseYaw + 90.0f);
                AddGableRoof(ResidentialRoofs, Center + ShedOffset, 700, 980,
                    Center.Z + ShedOffset.Z + 260.0f, HouseYaw + 90.0f, 22.0f);

                const int32 WindowCount = (HouseCounter % 3 == 0) ? 3 : 2;
                for (int32 W = 0; W < WindowCount; ++W)
                {
                    const float X = (static_cast<float>(W) - (WindowCount - 1) * 0.5f) * (Width / (WindowCount + 0.8f));
                    AddFacadeWindow(ResidentialDetails, Center, FVector(X, -Depth * 0.505f, 30.0f),
                        FVector(260, 18, 180), HouseYaw, true);
                }

                if ((HouseCounter % 4) == 0)
                {
                    const FVector PorchOffset = Rotate2D(FVector(Width * 0.25f, -Depth * 0.70f, -65.0f), HouseYaw);
                    AddBox(Buildings, Center + PorchOffset, FVector(520, 430, 250), HouseYaw);
                }
                if ((HouseCounter % 5) == 0)
                {
                    const FVector AnnexOffset = Rotate2D(FVector(-Width * 0.58f, Depth * 0.25f, -70.0f), HouseYaw);
                    AddBox(Buildings, Center + AnnexOffset, FVector(650, 780, 320), HouseYaw + 90.0f);
                }
                if ((HouseCounter % 11) != 0)
                {
                    const float FenceLength = (HouseCounter % 3 == 0) ? 2100.0f : 2850.0f;
                    const float FenceHeight = 190.0f + static_cast<float>((HouseCounter % 4) * 15);
                    UInstancedStaticMeshComponent* FenceFamily = WoodFences;
                    const int32 FenceRoll = HouseCounter % 20;
                    if (FenceRoll >= 12 && FenceRoll < 17) FenceFamily = MetalFences;
                    else if (FenceRoll >= 17) FenceFamily = LightSheetFences;

                    AddBox(FenceFamily, Center + Rotate2D(FVector(0, -1500, -120), HouseYaw),
                        FVector(FenceLength, 45, FenceHeight), HouseYaw);

                    if ((HouseCounter % 4) != 1)
                    {
                        AddBox(FenceFamily, Center + Rotate2D(FVector(-FenceLength * 0.48f, 150.0f, -135.0f), HouseYaw),
                            FVector(38, 3000.0f, FMath::Max(160.0f, FenceHeight - 25.0f)), HouseYaw);
                    }
                }
                ++HouseCounter;
            }
        }
    }

    AddBox(Buildings, FVector(8500, 9000, 520), FVector(4200, 2600, 1040), 3.0f);
    AddBox(Buildings, FVector(-10000, 11500, 460), FVector(3600, 2200, 920), -2.0f);
    AddBox(Buildings, FVector(9500, -15000, 420), FVector(5000, 2100, 840), 0.0f);
    AddBox(Buildings, FVector(-19000, -16500, 380), FVector(3900, 2400, 760), 1.0f);
}

void AOCWorldSectorOster::BuildVegetation()
{
    enum class ETreeProxy : uint8 { Broadleaf, Poplar, Birch, Pine };

    auto AddTreeFamily = [this](const FVector& Base, float Scale, ETreeProxy Family)
    {
        UInstancedStaticMeshComponent* Trunks = TreeTrunks;
        UInstancedStaticMeshComponent* Crowns = TreeCrowns;
        float TrunkRadius = 38.0f;
        float TrunkHeight = 440.0f;
        FVector CrownScale(2.4f, 2.4f, 2.1f);
        float CrownZ = 520.0f;

        switch (Family)
        {
            case ETreeProxy::Poplar:
                Trunks = SovietPoplarTrunks; Crowns = SovietPoplarCrowns;
                TrunkRadius = 34.0f; TrunkHeight = 720.0f; CrownScale = FVector(1.25f, 1.25f, 4.4f); CrownZ = 760.0f;
                break;
            case ETreeProxy::Birch:
                Trunks = BirchTrunks; Crowns = BirchCrowns;
                TrunkRadius = 27.0f; TrunkHeight = 520.0f; CrownScale = FVector(1.8f, 1.8f, 2.6f); CrownZ = 585.0f;
                break;
            case ETreeProxy::Pine:
                Trunks = PineTrunks; Crowns = PineCrowns;
                TrunkRadius = 32.0f; TrunkHeight = 610.0f; CrownScale = FVector(1.65f, 1.65f, 3.4f); CrownZ = 675.0f;
                break;
            default: break;
        }

        AddCylinder(Trunks, Base + FVector(0, 0, (TrunkHeight * 0.5f) * Scale), TrunkRadius * Scale, TrunkHeight * Scale);
        if (Crowns)
        {
            FTransform Crown;
            Crown.SetLocation(Base + FVector(0, 0, CrownZ * Scale));
            Crown.SetScale3D(CrownScale * Scale);
            Crowns->AddInstance(Crown);
        }
    };

    auto AddGrassPatch = [this](UInstancedStaticMeshComponent* Family, const FVector& Center, const FVector& Size, float Yaw)
    {
        AddBox(Family, Center + FVector(0,0,2.0f), FVector(Size.X, Size.Y, 4.0f), Yaw);
    };

    const FVector Park = ParkAnchor();
    const FVector College = CollegeAnchor();
    const FVector Stadium = StadiumAnchor();
    AddGrassPatch(GrassMown, Park + FVector(0, 0, 0), FVector(19000, 14500, 4), 6.0f);
    AddGrassPatch(GrassMown, Stadium + FVector(0, 0, 0), FVector(14500, 9800, 4), 0.0f);
    AddGrassPatch(GrassMown, College + FVector(0, 5200, 0), FVector(12500, 7600, 4), 2.0f);

    const FVector RoughPatches[] = {
        FVector(-52000, 30000, 0), FVector(-52000,-25000,0), FVector(45000,30000,0),
        FVector(42000,-35000,0), FVector(-15000,70000,0), FVector(16000,-65000,0)
    };
    for (int32 I=0; I<UE_ARRAY_COUNT(RoughPatches); ++I)
        AddGrassPatch(GrassRough, RoughPatches[I], FVector(31000,22000,4), static_cast<float>((I%3)-1)*8.0f);

    AddGrassPatch(GrassWetland, FVector(-93000, 35000, 0), FVector(33000, 102000, 4), 12.0f);
    AddGrassPatch(GrassWetland, FVector(43000,-93000, 0), FVector(98000, 21000, 4), 8.0f);

    for (int32 Index = 0; Index < 16; ++Index)
    {
        const float X = -4700.0f + static_cast<float>(Index % 8) * 1350.0f;
        const float Y = 2500.0f + static_cast<float>(Index / 8) * 1750.0f;
        const ETreeProxy Family = (Index==2 || Index==11) ? ETreeProxy::Poplar : ETreeProxy::Broadleaf;
        AddTreeFamily(FVector(X, Y, 0), 0.88f + 0.07f * static_cast<float>(Index % 3), Family);
    }

    for (int32 I = -6; I <= 6; ++I)
    {
        const ETreeProxy Family = (I % 3 == 0) ? ETreeProxy::Poplar : ((I % 4 == 0) ? ETreeProxy::Birch : ETreeProxy::Broadleaf);
        AddTreeFamily(Stadium + FVector(I * 1500.0f, 5700.0f + (I % 2) * 350.0f, 0), 0.9f, Family);
    }

    for (int32 Row = -3; Row <= 3; ++Row)
    {
        for (int32 Col = -4; Col <= 4; ++Col)
        {
            if (FMath::Abs(Row) <= 1 && FMath::Abs(Col) <= 1) continue;
            const float JitterX = static_cast<float>(((Row * 7 + Col * 3) % 5) - 2) * 180.0f;
            const float JitterY = static_cast<float>(((Row * 5 + Col * 11) % 5) - 2) * 160.0f;
            const int32 Roll = FMath::Abs(Row * 9 + Col * 5) % 12;
            ETreeProxy Family = ETreeProxy::Broadleaf;
            if (Roll <= 2) Family = ETreeProxy::Poplar;
            else if (Roll == 3 || Roll == 4) Family = ETreeProxy::Birch;
            else if (Roll == 5) Family = ETreeProxy::Pine;
            AddTreeFamily(Park + FVector(Col * 1850.0f + JitterX, Row * 1700.0f + JitterY, 0),
                0.85f + 0.05f * static_cast<float>((Row + Col + 8) % 4), Family);
        }
    }

    AddTreeFamily(College + FVector(-3800, -1100, 0), 1.2f, ETreeProxy::Pine);
    AddTreeFamily(College + FVector(3900, -950, 0), 1.15f, ETreeProxy::Pine);
    AddTreeFamily(College + FVector(-4600, 1500, 0), 1.0f, ETreeProxy::Pine);
    AddTreeFamily(College + FVector(4700, 2100, 0), 0.9f, ETreeProxy::Birch);

    for (int32 Index = -7; Index <= 7; ++Index)
    {
        const ETreeProxy WestEast = (Index % 4 == 0) ? ETreeProxy::Poplar : ETreeProxy::Broadleaf;
        AddTreeFamily(FVector(Index * 7000.0f, -11500.0f, 0), 0.75f, WestEast);
        if ((Index % 2) == 0)
            AddTreeFamily(FVector(28500.0f, Index * 6500.0f, 0), 0.8f, (Index % 4 == 0) ? ETreeProxy::Poplar : ETreeProxy::Birch);
    }

    for (int32 I=0; I<24; ++I)
    {
        const float X = -62000.0f + (I%8)*15000.0f;
        const float Y = -48000.0f + (I/8)*42000.0f + ((I%3)-1)*1800.0f;
        AddTreeFamily(FVector(X,Y,0), 0.55f + 0.05f*(I%3), ETreeProxy::Broadleaf);
    }
}
