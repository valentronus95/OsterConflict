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

    void AddGroundedTree(UInstancedStaticMeshComponent* Component, const FVector& GroundLocation,
        const float DesiredHeightCm, const float YawDegrees, const float WidthScale)
    {
        if (!Component) return;
        UStaticMesh* Mesh = Component->GetStaticMesh();
        if (!Mesh) return;

        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        if (NativeSize.Z <= 10.0f) return;

        const float HeightScale = FMath::Clamp(DesiredHeightCm / NativeSize.Z, 0.25f, 4.0f);
        const FVector Scale(HeightScale * WidthScale, HeightScale * WidthScale, HeightScale);
        const float LocalBottom = Bounds.Origin.Z - Bounds.BoxExtent.Z;
        FVector Location = GroundLocation;
        Location.Z += -LocalBottom * HeightScale;
        Component->AddInstance(FTransform(FRotator(0.0f, YawDegrees, 0.0f), Location, Scale), true);
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
    ParkPaths = MakeISM(TEXT("ParkPaths"), TEXT("BlockAll"));
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
    AuthoredDeciduousTrees = MakeISM(TEXT("AuthoredDeciduousTrees"), TEXT("BlockAll"));
    AuthoredPine01Trees = MakeISM(TEXT("AuthoredPine01Trees"), TEXT("BlockAll"));
    AuthoredPine03Trees = MakeISM(TEXT("AuthoredPine03Trees"), TEXT("BlockAll"));
    GrassMown = MakeISM(TEXT("GrassMown"), TEXT("NoCollision"));
    GrassRough = MakeISM(TEXT("GrassRough"), TEXT("NoCollision"));
    GrassWetland = MakeISM(TEXT("GrassWetland"), TEXT("NoCollision"));
    StadiumGeometry = MakeISM(TEXT("StadiumGeometry"), TEXT("BlockAll"));
    StadiumDetails = MakeISM(TEXT("StadiumDetails"), TEXT("BlockAll"));

    // PASS45 Gate K: legacy mixed/shared names remain zero-instance quarantine. Exact semantic owners are primary
    // actor subobjects and receive all park/college instances directly during construction.
    ParkGeometry = MakeISM(TEXT("ParkGeometry"), TEXT("BlockAll"));
    ParkCentralGround = MakeISM(TEXT("ParkCentralGround"), TEXT("BlockAll"));
    ParkNorthCivicGround = MakeISM(TEXT("ParkNorthCivicGround"), TEXT("BlockAll"));
    CollegeRecreationGround = MakeISM(TEXT("CollegeRecreationGround"), TEXT("BlockAll"));
    ParkDetails = MakeISM(TEXT("ParkDetails"), TEXT("BlockAll"));
    ParkMemorialPlaza = MakeISM(TEXT("ParkMemorialPlaza"), TEXT("BlockAll"));
    ParkMemorialSurface = MakeISM(TEXT("ParkMemorialSurface"), TEXT("BlockAll"));
    ParkMemorialMonument = MakeISM(TEXT("ParkMemorialMonument"), TEXT("BlockAll"));
    ParkMemorialApproach = MakeISM(TEXT("ParkMemorialApproach"), TEXT("BlockAll"));
    ParkSkateFitness = MakeISM(TEXT("ParkSkateFitness"), TEXT("BlockAll"));
    ParkSkateSurface = MakeISM(TEXT("ParkSkateSurface"), TEXT("BlockAll"));
    ParkSkateRamps = MakeISM(TEXT("ParkSkateRamps"), TEXT("BlockAll"));
    ParkBenches = MakeISM(TEXT("ParkBenches"), TEXT("BlockAll"));

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
    static ConstructorHelpers::FObjectFinder<UStaticMesh> DeciduousTreeMesh(
        TEXT("/Game/KiteDemo/Environments/Trees/HillTree_02/HillTree_02.HillTree_02"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Pine01Mesh(
        TEXT("/Game/KiteDemo/Environments/Trees/ScotsPine_01/ScotsPine_01.ScotsPine_01"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Pine03Mesh(
        TEXT("/Game/KiteDemo/Environments/Trees/ScotsPineTall_01/ScotsPineTall_01.ScotsPineTall_01"));

    if (CubeMesh.Succeeded())
    {
        Ground->SetStaticMesh(CubeMesh.Object);
        UInstancedStaticMeshComponent* CubeComponents[] =
        {
            Roads, Sidewalks, ParkPaths, Buildings, ResidentialRoofs, ResidentialDetails,
            LandmarkBlocks, LandmarkRoofs, LandmarkWindows, LandmarkDetails,
            Fences, WoodFences, MetalFences, LightSheetFences, StadiumGeometry, StadiumDetails,
            ParkGeometry, ParkCentralGround, ParkNorthCivicGround, CollegeRecreationGround,
            ParkDetails, ParkMemorialPlaza, ParkMemorialSurface, ParkMemorialMonument,
            ParkMemorialApproach, ParkSkateFitness, ParkSkateSurface, ParkSkateRamps, ParkBenches,
            GrassMown, GrassRough, GrassWetland,
            Waterways, Bridges, ReferenceMarkers
        };
        for (UInstancedStaticMeshComponent* Component : CubeComponents)
        {
            Component->SetStaticMesh(CubeMesh.Object);
        }
    }

    if (DeciduousTreeMesh.Succeeded()) AuthoredDeciduousTrees->SetStaticMesh(DeciduousTreeMesh.Object);
    if (Pine01Mesh.Succeeded()) AuthoredPine01Trees->SetStaticMesh(Pine01Mesh.Object);
    if (Pine03Mesh.Succeeded()) AuthoredPine03Trees->SetStaticMesh(Pine03Mesh.Object);

    UInstancedStaticMeshComponent* AuthoredTrees[] =
    {
        AuthoredDeciduousTrees, AuthoredPine01Trees, AuthoredPine03Trees
    };
    for (UInstancedStaticMeshComponent* Component : AuthoredTrees)
    {
        Component->SetCanEverAffectNavigation(false);
        Component->SetCullDistances(0, 90000);
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
    // Pass45: generic private-residence/fence generators are physically retired from primary authoring.
    // Road topology remains owned by BuildRoadNetwork; private structures return only from location-specific references.
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

    // R11 visual foundation: source-only blockout geometry still uses a readable outdoor palette.
    // PASS45 authored trees keep their packaged materials and are never overwritten by BasicShapeMaterial.
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

    // Block 0 ordering guard: UWorldSubsystem::OnWorldBeginPlay may already have replaced Ground with the exact
    // authored mesh/material. Never let this legacy source-palette step reclaim that authored state.
    const bool bGroundStillSourceCube = Ground && Ground->GetStaticMesh() &&
        Ground->GetStaticMesh()->GetPathName().Contains(TEXT("/Engine/BasicShapes/Cube"), ESearchCase::IgnoreCase);
    if (bGroundStillSourceCube)
    {
        Tint(Ground, FLinearColor(0.16f, 0.25f, 0.10f));
    }
    else
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_BLOCK0_SOURCE_GROUND_TINT_SKIPPED authored_ground_preserved=1 basicshape_material_reclaim=0 runtime_acceptance=0"));
    }

    Tint(Roads,               FLinearColor(0.055f, 0.060f, 0.065f));
    Tint(Sidewalks,           FLinearColor(0.42f, 0.43f, 0.41f));
    Tint(ParkPaths,           FLinearColor(0.40f, 0.39f, 0.34f));
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
    Tint(GrassMown,           FLinearColor(0.18f, 0.34f, 0.095f));
    Tint(GrassRough,          FLinearColor(0.24f, 0.38f, 0.10f));
    Tint(GrassWetland,        FLinearColor(0.13f, 0.28f, 0.12f));
    Tint(StadiumGeometry,     FLinearColor(0.055f, 0.31f, 0.12f));
    Tint(StadiumDetails,      FLinearColor(0.82f, 0.82f, 0.76f));

    // Zero-instance quarantine keeps legacy names stable without creating visible mixed/shared geometry.
    Tint(ParkGeometry,        FLinearColor(0.12f, 0.31f, 0.075f));
    Tint(ParkDetails,         FLinearColor(0.40f, 0.34f, 0.25f));
    Tint(ParkMemorialPlaza,   FLinearColor(0.40f, 0.34f, 0.25f));
    Tint(ParkSkateFitness,    FLinearColor(0.40f, 0.34f, 0.25f));

    Tint(ParkCentralGround,   FLinearColor(0.12f, 0.31f, 0.075f));
    Tint(ParkNorthCivicGround,FLinearColor(0.12f, 0.31f, 0.075f));
    Tint(CollegeRecreationGround, FLinearColor(0.12f, 0.31f, 0.075f));
    Tint(ParkMemorialSurface, FLinearColor(0.40f, 0.34f, 0.25f));
    Tint(ParkMemorialMonument,FLinearColor(0.40f, 0.34f, 0.25f));
    Tint(ParkMemorialApproach,FLinearColor(0.40f, 0.34f, 0.25f));
    Tint(ParkSkateSurface,    FLinearColor(0.40f, 0.34f, 0.25f));
    Tint(ParkSkateRamps,      FLinearColor(0.40f, 0.34f, 0.25f));
    Tint(ParkBenches,         FLinearColor(0.40f, 0.34f, 0.25f));
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
        TEXT("PASS45_WORLD_GENERIC_RESIDENTIAL_RETIRED procedural_residential_grids=0 generic_private_fences=0 reference_specific_private_structures_required=1"));

    // PASS45 item 27: the final player-facing tree family is selected during primary sector construction.
    // There is no late world-subsystem remap, transform rewrite or second mutating tree owner after BeginPlay.
    const int32 TreeInstances =
        (AuthoredDeciduousTrees ? AuthoredDeciduousTrees->GetInstanceCount() : 0) +
        (AuthoredPine01Trees ? AuthoredPine01Trees->GetInstanceCount() : 0) +
        (AuthoredPine03Trees ? AuthoredPine03Trees->GetInstanceCount() : 0);
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_REGIONAL_TREE_INTAKE_WIRED deciduous=HillTree_02 pine=ScotsPine_01 tall_pine=ScotsPineTall_01 families=3 instances=%d primary_authoring=1 late_mutation=0 imported_materials=1 runtime_acceptance=0"),
        TreeInstances);

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
    constexpr int32 ExpectedParkPaths = 5;
    constexpr int32 ExpectedMemorialSurface = 1;
    constexpr int32 ExpectedMemorialMonument = 1;
    constexpr int32 ExpectedMemorialApproach = 4;
    constexpr int32 ExpectedSkateSurface = 1;
    constexpr int32 ExpectedSkateRamps = 2;
    constexpr int32 ExpectedBenches = 14;
    constexpr int32 ExpectedSemanticDetails =
        ExpectedMemorialSurface + ExpectedMemorialMonument + ExpectedMemorialApproach +
        ExpectedSkateSurface + ExpectedSkateRamps + ExpectedBenches;
    static_assert(ExpectedParkPaths == 5, "Central Park must retain exactly five canonical ParkPaths proxies");
    static_assert(ExpectedSemanticDetails == 23, "Central Park semantic detail contract must remain exactly 23 proxies");

    const FVector Park = ParkAnchor();

    // Gate K primary semantic ground ownership: the old shared ParkGeometry bucket stays empty.
    AddBox(ParkCentralGround, Park + FVector(0, 0, 3), FVector(20500, 16000, 6));

    // Gate K: these five pedestrian paths are source-owned by ParkPaths, never mixed into Sidewalks.
    AddBox(ParkPaths, Park + FVector(0, 0, 14), FVector(17800, 360, 18));
    AddBox(ParkPaths, Park + FVector(0, -300, 14), FVector(360, 13200, 18));
    AddBox(ParkPaths, Park + FVector(1800, 900, 14), FVector(11800, 260, 18), 31.0f);
    AddBox(ParkPaths, Park + FVector(-2300, 1300, 14), FVector(9300, 240, 18), -28.0f);

    // Gate K primary semantic owners. Legacy ParkDetails/ParkMemorialPlaza/ParkSkateFitness remain empty quarantine.
    AddBox(ParkMemorialSurface, Park + FVector(-600, 200, 28), FVector(3100, 2500, 56));
    AddBox(ParkMemorialMonument, Park + FVector(-600, 200, 230), FVector(260, 260, 400));
    for (int32 Step = 0; Step < ExpectedMemorialApproach; ++Step)
    {
        AddBox(ParkMemorialApproach, Park + FVector(-6100 + Step * 150.0f, -4900, 18 + Step * 14.0f),
            FVector(1900 - Step * 120.0f, 260, 28), 0.0f);
    }

    // Small skate/active-recreation pad is present in recent public park coverage; placement is approximate.
    AddBox(ParkSkateSurface, Park + FVector(6100, -4100, 18), FVector(4300, 2600, 36));
    AddBoxRotated(ParkSkateRamps, Park + FVector(6100, -4100, 120), FVector(1200, 600, 35), FRotator(0, 0, 16));
    AddBoxRotated(ParkSkateRamps, Park + FVector(7400, -3500, 95), FVector(950, 500, 30), FRotator(0, 90, -13));

    // The separate published "city park near culture house" point lies farther north. Keep it as a secondary
    // civic grove/reference instead of incorrectly using it as the whole central-park centroid (S09 behavior).
    const FVector NorthCivic = CultureParkNorthAnchor();
    const FVector Mid = (Park + NorthCivic) * 0.5f;
    const FVector Delta = NorthCivic - Park;
    const float LinkYaw = FMath::RadiansToDegrees(FMath::Atan2(Delta.Y, Delta.X));
    AddBox(ParkNorthCivicGround, NorthCivic + FVector(0,0,4), FVector(8500, 7200, 8));
    AddBox(ParkPaths, Mid + FVector(0,0,15), FVector(Delta.Size2D(), 260, 18), LinkYaw);

    // Benches along main alleys. Their dedicated authored replacement remains separately guarded.
    for (int32 I = -3; I <= 3; ++I)
    {
        AddBox(ParkBenches, Park + FVector(I * 1900.0f, -850.0f, 60.0f), FVector(180, 55, 120));
        AddBox(ParkBenches, Park + FVector(I * 1900.0f, 850.0f, 60.0f), FVector(180, 55, 120));
    }

    const int32 ParkPathCount = ParkPaths ? ParkPaths->GetInstanceCount() : -1;
    const int32 LegacyDetailsCount = ParkDetails ? ParkDetails->GetInstanceCount() : -1;
    const int32 LegacyGeometryCount = ParkGeometry ? ParkGeometry->GetInstanceCount() : -1;
    const int32 LegacyMemorialCount = ParkMemorialPlaza ? ParkMemorialPlaza->GetInstanceCount() : -1;
    const int32 LegacySkateCount = ParkSkateFitness ? ParkSkateFitness->GetInstanceCount() : -1;
    const int32 CentralGroundCount = ParkCentralGround ? ParkCentralGround->GetInstanceCount() : -1;
    const int32 NorthGroundCount = ParkNorthCivicGround ? ParkNorthCivicGround->GetInstanceCount() : -1;
    const int32 MemorialSurfaceCount = ParkMemorialSurface ? ParkMemorialSurface->GetInstanceCount() : -1;
    const int32 MemorialMonumentCount = ParkMemorialMonument ? ParkMemorialMonument->GetInstanceCount() : -1;
    const int32 MemorialApproachCount = ParkMemorialApproach ? ParkMemorialApproach->GetInstanceCount() : -1;
    const int32 SkateSurfaceCount = ParkSkateSurface ? ParkSkateSurface->GetInstanceCount() : -1;
    const int32 SkateRampsCount = ParkSkateRamps ? ParkSkateRamps->GetInstanceCount() : -1;
    const int32 BenchCount = ParkBenches ? ParkBenches->GetInstanceCount() : -1;
    const int32 SemanticDetailCount = MemorialSurfaceCount + MemorialMonumentCount + MemorialApproachCount +
        SkateSurfaceCount + SkateRampsCount + BenchCount;

    const bool bSemanticSplitValid =
        ParkPathCount == ExpectedParkPaths &&
        LegacyDetailsCount == 0 && LegacyGeometryCount == 0 && LegacyMemorialCount == 0 && LegacySkateCount == 0 &&
        CentralGroundCount == 1 && NorthGroundCount == 1 &&
        MemorialSurfaceCount == ExpectedMemorialSurface &&
        MemorialMonumentCount == ExpectedMemorialMonument &&
        MemorialApproachCount == ExpectedMemorialApproach &&
        SkateSurfaceCount == ExpectedSkateSurface &&
        SkateRampsCount == ExpectedSkateRamps &&
        BenchCount == ExpectedBenches &&
        SemanticDetailCount == ExpectedSemanticDetails;

    if (!bSemanticSplitValid)
    {
        if (ParkPaths) ParkPaths->ClearInstances();
        if (ParkGeometry) ParkGeometry->ClearInstances();
        if (ParkCentralGround) ParkCentralGround->ClearInstances();
        if (ParkNorthCivicGround) ParkNorthCivicGround->ClearInstances();
        if (ParkDetails) ParkDetails->ClearInstances();
        if (ParkMemorialPlaza) ParkMemorialPlaza->ClearInstances();
        if (ParkMemorialSurface) ParkMemorialSurface->ClearInstances();
        if (ParkMemorialMonument) ParkMemorialMonument->ClearInstances();
        if (ParkMemorialApproach) ParkMemorialApproach->ClearInstances();
        if (ParkSkateFitness) ParkSkateFitness->ClearInstances();
        if (ParkSkateSurface) ParkSkateSurface->ClearInstances();
        if (ParkSkateRamps) ParkSkateRamps->ClearInstances();
        if (ParkBenches) ParkBenches->ClearInstances();
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_GATE_K_PARK_SEMANTIC_SPLIT_REJECTED park_paths=%d legacy_details=%d legacy_geometry=%d legacy_memorial=%d legacy_skate=%d central_ground=%d north_ground=%d memorial_surface=%d memorial_monument=%d memorial_approach=%d skate_surface=%d skate_ramps=%d benches=%d total=%d expected=5/0/0/0/0/1/1/1/1/4/1/2/14/23 primary_authoring=1 normalization_bridge=0"),
            ParkPathCount, LegacyDetailsCount, LegacyGeometryCount, LegacyMemorialCount, LegacySkateCount,
            CentralGroundCount, NorthGroundCount, MemorialSurfaceCount, MemorialMonumentCount, MemorialApproachCount,
            SkateSurfaceCount, SkateRampsCount, BenchCount, SemanticDetailCount);
        return;
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_SOURCE_PARK_PATH_OWNERSHIP_READY component=ParkPaths park_path_instances=5 authored_in_sidewalks=0 canonical_source_owner=1 runtime_migration_required=0"));
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_GATE_K_PARK_SEMANTIC_SPLIT_READY park_paths=5 legacy_details=0 legacy_geometry=0 legacy_memorial_plaza=0 legacy_skate_fitness=0 memorial_surface=1 memorial_monument=1 memorial_approach=4 skate_surface=1 skate_ramps=2 benches=14 total=23 primary_authoring=1 normalization_bridge=0 authored_detail_replacements=0 gate_k_complete=0 runtime_acceptance=0"));
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_PARK_PRIMARY_SEMANTIC_OWNERS_READY park_central_ground=1 park_north_civic_ground=1 memorial_surface=1 memorial_monument=1 memorial_approach=4 skate_surface=1 skate_ramps=2 benches=14 legacy_geometry=0 legacy_memorial_plaza=0 legacy_skate_fitness=0 primary_authoring=1 normalization_bridge=0 remaining_content_gap_instances=3 gate_k_complete=0 runtime_acceptance=0"));
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

    // Courtyard, sport/recreation strip and perimeter. The recreation ground has a dedicated primary owner.
    AddBox(Sidewalks, College + FVector(900, 5200, 12), FVector(8000, 5900, 18), Yaw);
    AddBox(CollegeRecreationGround, College + FVector(-4900, 7000, 10), FVector(6100, 3300, 12), Yaw);
    AddBox(Fences, College + FVector(0, -2450, 110), FVector(10400, 45, 220), Yaw);
    AddBox(Fences, College + FVector(0, 9300, 110), FVector(11200, 45, 220), Yaw);
    AddBox(Fences, College + FVector(-5600, 3400, 110), FVector(45, 11700, 220), Yaw);

    const int32 LegacyGeometryCount = ParkGeometry ? ParkGeometry->GetInstanceCount() : -1;
    const int32 CentralGroundCount = ParkCentralGround ? ParkCentralGround->GetInstanceCount() : -1;
    const int32 NorthGroundCount = ParkNorthCivicGround ? ParkNorthCivicGround->GetInstanceCount() : -1;
    const int32 CollegeGroundCount = CollegeRecreationGround ? CollegeRecreationGround->GetInstanceCount() : -1;
    const bool bGroundOwnersValid = LegacyGeometryCount == 0 && CentralGroundCount == 1 &&
        NorthGroundCount == 1 && CollegeGroundCount == 1;
    if (!bGroundOwnersValid)
    {
        if (ParkCentralGround) ParkCentralGround->ClearInstances();
        if (ParkNorthCivicGround) ParkNorthCivicGround->ClearInstances();
        if (CollegeRecreationGround) CollegeRecreationGround->ClearInstances();
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_PARK_GROUND_PRIMARY_OWNERS_REJECTED legacy_geometry=%d park_central_ground=%d park_north_civic_ground=%d college_recreation_ground=%d expected=0/1/1/1 primary_authoring=1 normalization_bridge=0 gate_k_complete=0 runtime_acceptance=0"),
            LegacyGeometryCount, CentralGroundCount, NorthGroundCount, CollegeGroundCount);
        return;
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_PARK_GROUND_PRIMARY_OWNERS_READY legacy_geometry=0 park_central_ground=1 park_north_civic_ground=1 college_recreation_ground=1 primary_authoring=1 normalization_bridge=0 authored_surface_upgrade_pending=1 gate_k_complete=0 runtime_acceptance=0"));
}

void AOCWorldSectorOster::BuildVegetation()
{
    enum class ETreeFamily : uint8 { Deciduous, Pine };

    auto AddAuthoredTree = [this](const FVector& Base, const float Scale, const ETreeFamily Family, const int32 Salt)
    {
        if (!IsPointInsidePlayableAuthoringBounds(Base, 600.0f)) return;

        UInstancedStaticMeshComponent* Component = AuthoredDeciduousTrees;
        float BaseHeightCm = 1650.0f;
        if (Family == ETreeFamily::Pine)
        {
            Component = (Salt & 1) == 0 ? AuthoredPine01Trees : AuthoredPine03Trees;
            BaseHeightCm = 2150.0f;
        }

        const float Yaw = FMath::Fmod(
            FMath::Abs(Base.X * 0.013f + Base.Y * 0.019f + static_cast<float>(Salt) * 47.0f), 360.0f);
        const float WidthScale = 0.94f + 0.035f * static_cast<float>(FMath::Abs(Salt) % 5);
        AddGroundedTree(Component, Base, BaseHeightCm * Scale, Yaw, WidthScale);
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

    // PASS45 item 26: Museum vegetation has a dedicated R145 authored owner. Do not create a second generic
    // tree grid here and do not fabricate an oak family that is not present in tracked content.

    // Stadium perimeter: preserve the designed edge, but stop making unsupported poplar/birch species claims.
    for (int32 I = -6; I <= 6; ++I)
    {
        AddAuthoredTree(Stadium + FVector(I * 1500.0f, 5700.0f + (I % 2) * 350.0f, 0),
            0.90f + 0.03f * static_cast<float>(FMath::Abs(I) % 3), ETreeFamily::Deciduous, I + 30);
    }

    // Central park: authored deciduous planting with occasional verified pine/conifer assets. Exact deciduous
    // species remain intentionally unspecified until a species-specific asset is actually imported.
    for (int32 Row = -3; Row <= 3; ++Row)
    {
        for (int32 Col = -4; Col <= 4; ++Col)
        {
            if (FMath::Abs(Row) <= 1 && FMath::Abs(Col) <= 1) continue;
            const float JitterX = static_cast<float>(((Row * 7 + Col * 3) % 5) - 2) * 180.0f;
            const float JitterY = static_cast<float>(((Row * 5 + Col * 11) % 5) - 2) * 160.0f;
            const int32 Roll = FMath::Abs(Row * 9 + Col * 5) % 12;
            const ETreeFamily Family = Roll == 5 ? ETreeFamily::Pine : ETreeFamily::Deciduous;
            const int32 Salt = (Row + 4) * 17 + (Col + 5);
            AddAuthoredTree(Park + FVector(Col * 1850.0f + JitterX, Row * 1700.0f + JitterY, 0),
                0.85f + 0.05f * static_cast<float>((Row + Col + 8) % 4), Family, Salt);
        }
    }

    // College: official photos support tall conifers framing the facade. Alternate two tracked authored pine
    // meshes so the family does not read as one repeated primitive silhouette.
    AddAuthoredTree(College + FVector(-3800, -1100, 0), 1.20f, ETreeFamily::Pine, 100);
    AddAuthoredTree(College + FVector(3900, -950, 0), 1.15f, ETreeFamily::Pine, 101);
    AddAuthoredTree(College + FVector(-4600, 1500, 0), 1.00f, ETreeFamily::Pine, 102);
    AddAuthoredTree(College + FVector(4700, 2100, 0), 0.90f, ETreeFamily::Deciduous, 103);

    // Selected street corridors retain mature authored deciduous rows. No poplar/birch/oak label is asserted
    // because no verified species-specific asset exists in the tracked packs used by this source owner.
    for (int32 Index = -7; Index <= 7; ++Index)
    {
        AddAuthoredTree(FVector(Index * 7000.0f, -11500.0f, 0), 0.75f, ETreeFamily::Deciduous, 200 + Index);
        if ((Index % 2) == 0)
            AddAuthoredTree(FVector(28500.0f, Index * 6500.0f, 0), 0.80f, ETreeFamily::Deciduous, 300 + Index);
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_SOURCE_AUTHORED_VEGETATION_READY deciduous_asset=HillTree_02 pine_assets=ScotsPine_01,ScotsPineTall_01 primitive_tree_components=0 cylinder_sphere_trees=0 oak_asset_verified=0"));
}
