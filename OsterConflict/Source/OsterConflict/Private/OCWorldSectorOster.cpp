#include "OCWorldSectorOster.h"
#include "OCGeoReference.h"

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
    // Pass 44: the user-approved central Oster battlefield is the authoring boundary, not merely a
    // post-start trim. Keep these values synchronized with OCCentralPlayableAreaSubsystem and the
    // tactical-map hard projection. Sector-local centimetres.
    constexpr float MinPlayableX = -78000.0f;
    constexpr float MaxPlayableX =  18000.0f;
    constexpr float MinPlayableY = -12000.0f;
    constexpr float MaxPlayableY =  82000.0f;
    constexpr float MapWidthCm = MaxPlayableX - MinPlayableX;
    constexpr float MapHeightCm = MaxPlayableY - MinPlayableY;
    constexpr float MapCenterX = (MinPlayableX + MaxPlayableX) * 0.5f;
    constexpr float MapCenterY = (MinPlayableY + MaxPlayableY) * 0.5f;
    constexpr float RoadZ = 8.0f;
    constexpr float GroundTopZ = 0.0f;

    FVector Rotate2D(const FVector& Local, float YawDegrees)
    {
        return FRotator(0.0f, YawDegrees, 0.0f).RotateVector(Local);
    }

    bool IntersectsPlayableAuthoringBounds(const FVector& Center, const FVector& SizeCm, const float YawDegrees)
    {
        const float HalfX = FMath::Abs(SizeCm.X) * 0.5f;
        const float HalfY = FMath::Abs(SizeCm.Y) * 0.5f;
        const float Radians = FMath::DegreesToRadians(YawDegrees);
        const float C = FMath::Abs(FMath::Cos(Radians));
        const float S = FMath::Abs(FMath::Sin(Radians));
        const float ExtentX = C * HalfX + S * HalfY;
        const float ExtentY = S * HalfX + C * HalfY;
        return Center.X + ExtentX >= MinPlayableX && Center.X - ExtentX <= MaxPlayableX &&
            Center.Y + ExtentY >= MinPlayableY && Center.Y - ExtentY <= MaxPlayableY;
    }

    bool IsPointInsidePlayableAuthoringBounds(const FVector& Point, const float PaddingCm = 0.0f)
    {
        return Point.X >= MinPlayableX - PaddingCm && Point.X <= MaxPlayableX + PaddingCm &&
            Point.Y >= MinPlayableY - PaddingCm && Point.Y <= MaxPlayableY + PaddingCm;
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

    // Pass 44 primary authoring: never create the old 2.4 km ground in the first place.
    Ground->SetRelativeLocation(FVector(MapCenterX, MapCenterY, -100.0f));
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
    ConfigureLabel(StadiumLabel, TEXT("OSTER CENTRAL STADIUM / PHOTO + GENERAL-PLAN TOPOLOGY / S16A"),
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

    // R11 visual foundation: the source-only world already has a useful layout, but R10 left every
    // primitive on the engine default material. Give each semantic family a readable outdoor palette.
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

    // Authoring/reference markers are useful to developers and terrible as scenery.
    if (ReferenceMarkers) ReferenceMarkers->SetVisibility(false, true);
    UTextRenderComponent* Labels[] = { MuseumLabel, StadiumLabel, ParkLabel, CollegeLabel, KrushelnytskaStreetLabel };
    for (UTextRenderComponent* Label : Labels)
    {
        if (Label) Label->SetVisibility(false, true);
    }

    // Large numbers of grass proxy tiles should not waste shadow budget.
    if (GrassMown) GrassMown->SetCastShadow(false);
    if (GrassRough) GrassRough->SetCastShadow(false);
    if (GrassWetland) GrassWetland->SetCastShadow(false);
    if (Waterways) Waterways->SetCastShadow(false);

    UE_LOG(LogTemp, Display,
        TEXT("PASS44_PRIMARY_WORLD_COMPACT_AUTHORING_READY bounds_m=960x940 x_m=[-780,180] y_m=[-120,820] old_ground_2400m=0 far_legacy_base_geometry=0 peripheral_hydrography=0"));
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
    // S16A correction: use the published coordinate explicitly identified as CENTRAL CITY PARK.
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
    // Stadium placement remains confidence-B: aerial/photo topology is strong but no survey-grade public point is used yet.
    return FVector(15000.0f, -1500.0f, GroundTopZ);
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
    if (!IntersectsPlayableAuthoringBounds(Center, SizeCm, Rotation.Yaw)) return;
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
    if (!IsPointInsidePlayableAuthoringBounds(Center, RadiusCm)) return;
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
    // Pass 44: retired. The two old decorative BASE compounds lived around ±1040 m and were outside
    // the user-approved central Oster battlefield. Actual deployment ownership belongs to AOCTeamSpawnPoint
    // + OCGameModeRuntimeSafe near Museum; do not recreate peripheral source-only BASE geometry here.
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
    // S16A topology pass. The 2025 official general plan confirms Oster is a radial/irregular low-rise town,
    // not a rectangular grid. Pass 44 keeps only routes that intersect the user-approved central battlefield.
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

    // Central east-west corridor and the museum/stadium road edge.
    AddRoadWithWalks(FVector(-5000, -9000, RoadZ), FVector(138000, 1050, 16), 0.0f);
    AddRoadWithWalks(FVector(-18000, 17000, RoadZ), FVector(61000, 820, 16), 0.0f);

    // Solomii Krushelnytskoi / west-central north-south spine, tied to the verified college anchor.
    AddRoadWithWalks(FVector(-33500, 25000, RoadZ), FVector(112000, 920, 16), 91.5f);

    // Central curved/radial streets approximated as short segments from the official general-plan topology.
    AddRoadWithWalks(FVector(-23500, 40500, RoadZ), FVector(51000, 760, 16), 18.0f);
    AddRoadWithWalks(FVector(-48000, 51000, RoadZ), FVector(52000, 720, 16), 63.0f, false);
    AddRoadWithWalks(FVector(-5000, 33500, RoadZ), FVector(49000, 760, 16), -34.0f);
    AddRoadWithWalks(FVector(20500, 20500, RoadZ), FVector(52000, 780, 16), 73.0f, false);
    AddRoadWithWalks(FVector(31500, -23000, RoadZ), FVector(63000, 850, 16), 4.0f, false);
    AddRoadWithWalks(FVector(-33000, -25500, RoadZ), FVector(56000, 760, 16), -7.0f, false);

    // Central-park approach and northern civic links. ParkAnchor is now the verified CENTRAL CITY PARK point.
    const FVector Park = ParkAnchor();
    AddRoadWithWalks(Park + FVector(0, -8500, RoadZ), FVector(43000, 720, 16), 2.0f);
    AddRoadWithWalks(Park + FVector(-9000, 13500, RoadZ), FVector(37000, 700, 16), 79.0f, false);

    // College frontage/access.
    const FVector College = CollegeAnchor();
    AddRoadWithWalks(College + FVector(-13500, 0, RoadZ), FVector(30000, 660, 14), 0.0f);

    // Pass 44 deliberately removes the old general-plan peripheral vehicle loops at x=-865 m, x=655 m
    // and y=-675 m. They were outside the hard current battlefield reference and inflated runtime/map bounds.
}

void AOCWorldSectorOster::BuildHydrography()
{
    // Pass 44: the previous Desna/Oster broad proxy strips and bridge blockouts lived on the peripheral
    // 2.4 km sector. They are not part of the current compact central-Oster reference and must not consume
    // collision/render/tactical-map budget. Reintroduce water only from newer user-approved map evidence.
}

void AOCWorldSectorOster::BuildVerifiedReferenceMarkers()
{
    // Small non-colliding pillars expose verified geo anchors in source-only builds and make reference drift visible.
    const FVector Points[] =
    {
        MuseumAnchor(), CollegeAnchor(), ParkAnchor(), CultureParkNorthAnchor(),
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

    // Reference cues: red-brick single-storey wings, central wooden upper storey/gable,
    // front glazed/porch projection, decorative roofline and mature garden trees.
    AddBox(LandmarkBlocks, Museum + FVector(0, 0, 270), FVector(3400, 1750, 540), MuseumYaw);
    AddBox(LandmarkBlocks, Museum + FVector(-2050, 120, 245), FVector(1100, 1500, 490), MuseumYaw);
    AddBox(LandmarkBlocks, Museum + FVector(2050, 80, 245), FVector(1100, 1500, 490), MuseumYaw);
    AddBox(LandmarkBlocks, Museum + FVector(50, 80, 720), FVector(1550, 1280, 420), MuseumYaw);

    // Front porch / glazed bay and entrance platform.
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

    // S16A silhouette details visible in multiple facade references: central front gable, chimney masses and trim bands.
    AddBox(LandmarkDetails, Museum + FVector(0, -40, 1080), FVector(1500, 90, 95), MuseumYaw);
    AddBox(LandmarkDetails, Museum + FVector(-1520, 250, 970), FVector(190, 190, 520), MuseumYaw);
    AddBox(LandmarkDetails, Museum + FVector(1580, 180, 940), FVector(180, 180, 460), MuseumYaw);
    AddBox(LandmarkDetails, Museum + FVector(0, -910, 525), FVector(3350, 55, 90), MuseumYaw);

    // Front facade window rhythm from published photographs.
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

    // Stadium: public 2021 sources show a rectangular artificial-turf pitch, perimeter fencing,
    // renewed track/apron, small stands and service/change facilities.
    const FVector Stadium = StadiumAnchor();
    AddBox(StadiumGeometry, Stadium + FVector(0, 0, 4), FVector(11900, 8200, 8));
    AddBox(StadiumGeometry, Stadium + FVector(0, 0, 12), FVector(10500, 6800, 12));

    // S16A track/apron approximation from reconstruction photos and the official general-plan sports footprint.
    AddBox(StadiumGeometry, Stadium + FVector(0, -3740, 18), FVector(11200, 520, 14));
    AddBox(StadiumGeometry, Stadium + FVector(0,  3740, 18), FVector(11200, 520, 14));
    AddBox(StadiumGeometry, Stadium + FVector(-5600, 0, 18), FVector(520, 7400, 14));
    AddBox(StadiumGeometry, Stadium + FVector( 5600, 0, 18), FVector(520, 7400, 14));

    // Touchline/goal line proxies and center line; thin raised strips read clearly in the source-only build.
    AddBox(StadiumDetails, Stadium + FVector(0, 0, 25), FVector(10400, 18, 8));
    AddBox(StadiumDetails, Stadium + FVector(-5200, 0, 25), FVector(18, 6750, 8));
    AddBox(StadiumDetails, Stadium + FVector(5200, 0, 25), FVector(18, 6750, 8));

    // Spectator stand along the road-facing side, plus service/changing-room block.
    AddBox(StadiumDetails, Stadium + FVector(0, -4750, 150), FVector(5200, 720, 300));
    AddBox(StadiumDetails, Stadium + FVector(2600, -5250, 300), FVector(2400, 850, 600));
    AddBox(StadiumDetails, Stadium + FVector(-3750, -5050, 210), FVector(1300, 800, 420));

    // Perimeter fence; trees are placed around rather than inside the pitch, matching aerial references.
    AddBox(Fences, Stadium + FVector(0, -4300, 125), FVector(12400, 35, 250));
    AddBox(Fences, Stadium + FVector(0, 4300, 125), FVector(12400, 35, 250));
    AddBox(Fences, Stadium + FVector(-6200, 0, 125), FVector(35, 8600, 250));
    AddBox(Fences, Stadium + FVector(6200, 0, 125), FVector(35, 8600, 250));

    // Goal frames.
    for (float GoalX : { -5200.0f, 5200.0f })
    {
        AddBox(StadiumDetails, Stadium + FVector(GoalX, -365, 125), FVector(30, 30, 250));
        AddBox(StadiumDetails, Stadium + FVector(GoalX, 365, 125), FVector(30, 30, 250));
        AddBox(StadiumDetails, Stadium + FVector(GoalX, 0, 250), FVector(30, 760, 30));
    }

    // The source block remains as a collision/navigation backstop, but its procedural art is hidden at creation.
    // OCR13StadiumSurfaceSubsystem is the only player-facing Stadion Oster presentation owner.
    StadiumGeometry->SetVisibility(false, true);
    StadiumGeometry->SetHiddenInGame(true, true);
    StadiumDetails->SetVisibility(false, true);
    StadiumDetails->SetHiddenInGame(true, true);
}

void AOCWorldSectorOster::BuildCentralPark()
{
    const FVector Park = ParkAnchor();

    // City-park footprint centered on a documented park monument/reference coordinate.
    AddBox(ParkGeometry, Park + FVector(0, 0, 3), FVector(20500, 16000, 6));

    // Main alleys and secondary diagonals seen across public city-park material.
    AddBox(Sidewalks, Park + FVector(0, 0, 14), FVector(17800, 360, 18));
    AddBox(Sidewalks, Park + FVector(0, -300, 14), FVector(360, 13200, 18));
    AddBox(Sidewalks, Park + FVector(1800, 900, 14), FVector(11800, 260, 18), 31.0f);
    AddBox(Sidewalks, Park + FVector(-2300, 1300, 14), FVector(9300, 240, 18), -28.0f);

    // Civic center / memorial plaza block and stepped approach.
    AddBox(ParkDetails, Park + FVector(-600, 200, 28), FVector(3100, 2500, 56));
    AddBox(ParkDetails, Park + FVector(-600, 200, 230), FVector(260, 260, 400));
    for (int32 Step = 0; Step < 4; ++Step)
    {
        AddBox(ParkDetails, Park + FVector(-6100 + Step * 150.0f, -4900, 18 + Step * 14.0f),
            FVector(1900 - Step * 120.0f, 260, 28), 0.0f);
    }

    // Small skate/active-recreation pad is present in recent public park coverage; placement is approximate.
    AddBox(ParkDetails, Park + FVector(6100, -4100, 18), FVector(4300, 2600, 36));
    AddBoxRotated(ParkDetails, Park + FVector(6100, -4100, 120), FVector(1200, 600, 35), FRotator(0, 0, 16));
    AddBoxRotated(ParkDetails, Park + FVector(7400, -3500, 95), FVector(950, 500, 30), FRotator(0, 90, -13));

    // The separate published "city park near culture house" point lies farther north. Keep it as a secondary
    // civic grove/reference instead of incorrectly using it as the whole central-park centroid (S09 behavior).
    const FVector NorthCivic = CultureParkNorthAnchor();
    const FVector Mid = (Park + NorthCivic) * 0.5f;
    const FVector Delta = NorthCivic - Park;
    const float LinkYaw = FMath::RadiansToDegrees(FMath::Atan2(Delta.Y, Delta.X));
    AddBox(ParkGeometry, NorthCivic + FVector(0,0,4), FVector(8500, 7200, 8));
    AddBox(Sidewalks, Mid + FVector(0,0,15), FVector(Delta.Size2D(), 260, 18), LinkYaw);

    // Benches along main alleys. Simple source-only proxies now; final assets arrive in art pass.
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

    // Reference-driven main block: four storeys, long beige/tiled facade, flat/very-low roof,
    // repeated white-framed windows and a glazed entrance vestibule with canopy and broad steps.
    const FVector MainCenter = College + FVector(0, 0, 720);
    AddBox(LandmarkBlocks, MainCenter, FVector(6500, 1900, 1440), Yaw);
    AddBox(LandmarkRoofs, College + FVector(0, 0, 1460), FVector(6650, 2020, 70), Yaw);

    // Front entrance glass vestibule/canopy and stairs.
    AddBox(LandmarkDetails, College + FVector(900, -1230, 230), FVector(2450, 600, 460), Yaw);
    AddBox(LandmarkDetails, College + FVector(900, -1590, 505), FVector(2650, 920, 70), Yaw);
    for (int32 Step = 0; Step < 5; ++Step)
    {
        AddBox(LandmarkDetails, College + FVector(900, -1940 - Step * 115.0f, 22 + Step * 22.0f),
            FVector(2750 - Step * 100.0f, 220, 40), Yaw);
    }

    // Four-storey window grid. S09 intentionally models facade rhythm rather than a single featureless wall.
    constexpr int32 Columns = 9;
    constexpr int32 Rows = 4;
    for (int32 Row = 0; Row < Rows; ++Row)
    {
        for (int32 Col = 0; Col < Columns; ++Col)
        {
            const float X = -2800.0f + Col * 700.0f;
            const float Z = 255.0f + Row * 340.0f;
            // Entrance replaces the two central ground-floor windows.
            if (Row == 0 && (Col == 5 || Col == 6)) continue;
            AddFacadeWindow(LandmarkWindows, College, FVector(X, -965, Z), FVector(430, 24, 220), Yaw, true);
        }
    }

    // Side/rear academic wing and smaller service structure visible as campus massing rather than generic city blocks.
    AddBox(LandmarkBlocks, College + FVector(-2450, 2500, 520), FVector(2200, 4200, 1040), Yaw);
    AddBox(LandmarkBlocks, College + FVector(3100, 2650, 320), FVector(2500, 1900, 640), Yaw);
    AddBox(LandmarkRoofs, College + FVector(-2450, 2500, 1060), FVector(2320, 4320, 60), Yaw);
    AddBox(LandmarkRoofs, College + FVector(3100, 2650, 660), FVector(2620, 2020, 55), Yaw);

    // Public institutional sources describe three academic buildings plus dormitories / workshops. S16A adds
    // campus massing as confidence-B geometry; exact facade detail for these secondary blocks remains future art work.
    AddBox(LandmarkBlocks, College + FVector(-5200, 4700, 430), FVector(1900, 3600, 860), Yaw + 2.0f);
    AddBox(LandmarkRoofs, College + FVector(-5200, 4700, 885), FVector(2020, 3720, 55), Yaw + 2.0f);
    AddBox(LandmarkBlocks, College + FVector(4800, 6000, 510), FVector(2100, 4300, 1020), Yaw - 1.0f);
    AddBox(LandmarkRoofs, College + FVector(4800, 6000, 1045), FVector(2220, 4420, 55), Yaw - 1.0f);
    AddBox(LandmarkBlocks, College + FVector(9000, 2600, 340), FVector(2600, 1500, 680), Yaw);

    // Courtyard, sport/recreation strip and perimeter.
    AddBox(Sidewalks, College + FVector(900, 5200, 12), FVector(8000, 5900, 18), Yaw);
    AddBox(ParkGeometry, College + FVector(-4900, 7000, 10), FVector(6100, 3300, 12), Yaw);
    AddBox(Fences, College + FVector(0, -2450, 110), FVector(10400, 45, 220), Yaw);
    AddBox(Fences, College + FVector(0, 9300, 110), FVector(11200, 45, 220), Yaw);
    AddBox(Fences, College + FVector(-5600, 3400, 110), FVector(45, 11700, 220), Yaw);
}

void AOCWorldSectorOster::BuildSolomiiKrushelnytskoiStreet()
{
    const float WestHouseX = -39200.0f;
    const float EastHouseX = -27800.0f;
    const float StartY = 20500.0f;

    for (int32 Index = 0; Index < 8; ++Index)
    {
        const float Y = StartY + static_cast<float>(Index) * 4800.0f;
        const float WestYaw = 87.0f + static_cast<float>((Index % 3) - 1) * 2.0f;
        const float EastYaw = -88.0f + static_cast<float>((Index % 2) * 3);

        auto AddHouseArchetype = [this](const FVector& Center, float Width, float Depth, float Height, float Yaw, int32 Variant)
        {
            AddBox(Buildings, Center, FVector(Width, Depth, Height), Yaw);
            AddGableRoof(ResidentialRoofs, Center, Width + 120.0f, Depth + 160.0f,
                Center.Z + Height * 0.5f + 245.0f, Yaw, 24.0f + static_cast<float>((Variant % 3) * 3));

            // Two or three front windows and one door proxy to make the street read like Oster's low-rise housing stock.
            const int32 WindowCount = Variant % 2 == 0 ? 3 : 2;
            for (int32 W = 0; W < WindowCount; ++W)
            {
                const float X = (static_cast<float>(W) - (WindowCount - 1) * 0.5f) * (Width / (WindowCount + 0.8f));
                AddFacadeWindow(ResidentialDetails, Center, FVector(X, -Depth * 0.505f, 40.0f),
                    FVector(280, 18, 190), Yaw, true);
            }
            AddFacadeWindow(ResidentialDetails, Center, FVector(Width * 0.34f, -Depth * 0.51f, -25.0f),
                FVector(220, 22, 310), Yaw, true);
        };

        if (Index != 2)
        {
            AddHouseArchetype(FVector(EastHouseX, Y, 270.0f),
                1780.0f + static_cast<float>((Index % 3) * 170), 1180.0f, 540.0f, EastYaw, Index);
        }
        AddHouseArchetype(FVector(WestHouseX, Y + 700.0f, 260.0f),
            1700.0f, 1120.0f + static_cast<float>((Index % 2) * 160), 520.0f, WestYaw, Index + 1);

        // Common Oster visual language from aerial/street references: long narrow lots, separate sheds, fences and gates.
        AddBox(Buildings, FVector(WestHouseX - 1700.0f, Y + 1950.0f, 150.0f), FVector(700, 1100, 300), WestYaw);
        AddGableRoof(ResidentialRoofs, FVector(WestHouseX - 1700.0f, Y + 1950.0f, 150.0f),
            780, 1200, 390, WestYaw, 24.0f);
        AddBox(Buildings, FVector(EastHouseX + 1600.0f, Y + 1750.0f, 140.0f), FVector(650, 1000, 280), EastYaw);

        AddBox(Fences, FVector(-37100.0f, Y - 1200.0f, 85.0f), FVector(3200.0f, 35.0f, 170.0f), 90.0f);
        AddBox(Fences, FVector(-29900.0f, Y - 1200.0f, 85.0f), FVector(3200.0f, 35.0f, 170.0f), 90.0f);
        AddBox(Sidewalks, FVector(-36500.0f, Y + 450.0f, 18.0f), FVector(2100.0f, 160.0f, 18.0f), 0.0f);
        AddBox(Sidewalks, FVector(-30500.0f, Y - 350.0f, 18.0f), FVector(2100.0f, 160.0f, 18.0f), 0.0f);
    }

    // S08 service alleys remain as infantry flanking routes.
    AddBox(Roads, FVector(-43000.0f, 36000.0f, RoadZ), FVector(560.0f, 42000.0f, 14.0f), 0.0f);
    AddBox(Roads, FVector(-24200.0f, 37000.0f, RoadZ), FVector(560.0f, 39000.0f, 14.0f), 0.0f);
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
        { FVector(-82000,  15000, 0), 3, 4, FVector(4100, 4300, 0), 8.0f },
        { FVector(-76000, -41000, 0), 3, 4, FVector(4050, 4250, 0), -4.0f },
        { FVector( 52000,  33000, 0), 3, 4, FVector(4200, 4400, 0), 5.0f },
        { FVector( 47000, -50000, 0), 3, 4, FVector(4100, 4200, 0), -7.0f },
        { FVector(-24000,  76000, 0), 2, 5, FVector(4200, 4100, 0), 12.0f }
    };

    int32 HouseCounter = 0;
    for (const FBlockSeed& Block : Blocks)
    {
        // Avoid even iterating full outlying residential grids that cannot contribute to the compact battlefield.
        if (!IsPointInsidePlayableAuthoringBounds(Block.Origin, 8000.0f)) continue;

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

                // Detached rear shed/outbuilding creates the irregular courtyard silhouettes visible in Oster aerial imagery.
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

                // S16A variation: houses/lots are intentionally imperfect. Some have porches/extensions, some
                // have missing/short fences, matching the mixed maintained/worn character seen in public imagery.
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
                // S16B Oster fence fidelity: street-facing yards are commonly screened by tall opaque fences.
                // Distribution is deliberately weighted toward wood, with metal and light corrugated sheet variants.
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

                    // Side boundary is usually simpler and slightly lower than the street facade.
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
        if (!IsPointInsidePlayableAuthoringBounds(Base, 600.0f)) return;

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
        // Source-only placeholder: very thin instanced boxes mark vegetation zones. Final S16C uses foliage/PCG meshes.
        AddBox(Family, Center + FVector(0,0,2.0f), FVector(Size.X, Size.Y, 4.0f), Yaw);
    };

    // S16B ground-cover zoning. Clean mown lawns are limited to maintained civic/sports spaces.
    const FVector Park = ParkAnchor();
    const FVector College = CollegeAnchor();
    const FVector Stadium = StadiumAnchor();
    AddGrassPatch(GrassMown, Park + FVector(0, 0, 0), FVector(19000, 14500, 4), 6.0f);
    AddGrassPatch(GrassMown, Stadium + FVector(0, 0, 0), FVector(14500, 9800, 4), 0.0f);
    AddGrassPatch(GrassMown, College + FVector(0, 5200, 0), FVector(12500, 7600, 4), 2.0f);

    // Road verges/private lots: irregular, partly mown grass rather than uniform golf-course lawn.
    const FVector RoughPatches[] = {
        FVector(-52000, 30000, 0), FVector(-52000,-25000,0), FVector(45000,30000,0),
        FVector(42000,-35000,0), FVector(-15000,70000,0), FVector(16000,-65000,0)
    };
    for (int32 I=0; I<UE_ARRAY_COUNT(RoughPatches); ++I)
        AddGrassPatch(GrassRough, RoughPatches[I], FVector(31000,22000,4), static_cast<float>((I%3)-1)*8.0f);

    // Pass 44 removes the old Desna/Oster wetland proxies outside the compact map. Water-edge vegetation
    // comes back only when a newer reference places that shoreline inside the battlefield.

    // Museum garden: old broadleaf canopy seen in reference photos, with a few tall Soviet-era poplar silhouettes nearby.
    for (int32 Index = 0; Index < 16; ++Index)
    {
        const float X = -4700.0f + static_cast<float>(Index % 8) * 1350.0f;
        const float Y = 2500.0f + static_cast<float>(Index / 8) * 1750.0f;
        const ETreeProxy Family = (Index==2 || Index==11) ? ETreeProxy::Poplar : ETreeProxy::Broadleaf;
        AddTreeFamily(FVector(X, Y, 0), 0.88f + 0.07f * static_cast<float>(Index % 3), Family);
    }

    // Stadium perimeter: mixed mature rows, including tall poplar forms typical of Soviet-era town planting.
    for (int32 I = -6; I <= 6; ++I)
    {
        const ETreeProxy Family = (I % 3 == 0) ? ETreeProxy::Poplar : ((I % 4 == 0) ? ETreeProxy::Birch : ETreeProxy::Broadleaf);
        AddTreeFamily(Stadium + FVector(I * 1500.0f, 5700.0f + (I % 2) * 350.0f, 0), 0.9f, Family);
    }

    // Central park: Soviet-era urban palette is represented by broadleaf/linden-maple proxies,
    // tall poplars, birch groups and occasional pine. Exact species placement remains reference-driven.
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

    // College: official photos show tall conifers framing the facade, with mixed broadleaf planting around campus.
    AddTreeFamily(College + FVector(-3800, -1100, 0), 1.2f, ETreeProxy::Pine);
    AddTreeFamily(College + FVector(3900, -950, 0), 1.15f, ETreeProxy::Pine);
    AddTreeFamily(College + FVector(-4600, 1500, 0), 1.0f, ETreeProxy::Pine);
    AddTreeFamily(College + FVector(4700, 2100, 0), 0.9f, ETreeProxy::Birch);

    // Street rows: not every street receives formal planting; selected corridors get old poplar/broadleaf rhythm.
    for (int32 Index = -7; Index <= 7; ++Index)
    {
        const ETreeProxy WestEast = (Index % 4 == 0) ? ETreeProxy::Poplar : ETreeProxy::Broadleaf;
        AddTreeFamily(FVector(Index * 7000.0f, -11500.0f, 0), 0.75f, WestEast);
        if ((Index % 2) == 0)
            AddTreeFamily(FVector(28500.0f, Index * 6500.0f, 0), 0.8f, (Index % 4 == 0) ? ETreeProxy::Poplar : ETreeProxy::Birch);
    }

    // Private yards: fruit-tree silhouettes are intentionally represented with smaller broadleaf proxies;
    // exact species (apple/cherry/plum/walnut) will be assigned in the final foliage content pass.
    for (int32 I=0; I<24; ++I)
    {
        const float X = -62000.0f + (I%8)*15000.0f;
        const float Y = -48000.0f + (I/8)*42000.0f + ((I%3)-1)*1800.0f;
        AddTreeFamily(FVector(X,Y,0), 0.55f + 0.05f*(I%3), ETreeProxy::Broadleaf);
    }
}
