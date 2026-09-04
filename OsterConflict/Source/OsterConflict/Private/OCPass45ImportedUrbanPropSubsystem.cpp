#include "OCPass45ImportedUrbanPropSubsystem.h"

#include "OCGameMode.h"
#include "OCPass45LocalAssetResolver.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"

namespace
{
    UInstancedStaticMeshComponent* FindISM(AOCWorldSectorOster* Sector, const FName Name)
    {
        if (!Sector) return nullptr;
        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Sector->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (Component && Component->GetFName() == Name) return Component;
        }
        return nullptr;
    }

    UStaticMesh* ResolveStreetBin()
    {
        const TArray<FString> BinTokens =
        {
            TEXT("trash"), TEXT("garbage"), TEXT("waste"), TEXT("bin"), TEXT("litter")
        };
        if (UStaticMesh* Mesh = OCPass45FindLocalStaticMeshStrict(
            { FName(TEXT("/Game/Mega_Street_Props_Pack")) }, BinTokens))
        {
            return Mesh;
        }
        return OCPass45FindLocalStaticMeshStrict(
            { FName(TEXT("/Game/Street_Props_Pack_V1")) }, BinTokens);
    }

    UStaticMesh* ResolveSardineCan()
    {
        return OCPass45FindLocalStaticMeshStrict(
            { FName(TEXT("/Game/konserva-sardines")) },
            { TEXT("sardine"), TEXT("konserv"), TEXT("can") });
    }

    UStaticMesh* ResolveCherryJuice()
    {
        return OCPass45FindLocalStaticMeshStrict(
            { FName(TEXT("/Game/ukrainian-cherry-juice-nash-sik")) },
            { TEXT("juice"), TEXT("cherry"), TEXT("nash"), TEXT("sik") });
    }

    int32 AddAnchoredPropLayer(
        AOCWorldSectorOster* Sector,
        UInstancedStaticMeshComponent* Anchor,
        UStaticMesh* PropMesh,
        const FName ComponentBaseName,
        const float DesiredHeightCm,
        const int32 Stride,
        const FVector LocalOffset,
        const float YawOffsetDegrees,
        const bool bCollision)
    {
        if (!Sector || !Anchor || !Anchor->GetStaticMesh() || !PropMesh || Stride <= 0) return 0;

        const FBoxSphereBounds PropBounds = PropMesh->GetBounds();
        const FVector PropSize = PropBounds.BoxExtent * 2.0f;
        if (PropSize.Z <= 1.0f) return 0;
        const float PropScale = FMath::Clamp(DesiredHeightCm / PropSize.Z, 0.05f, 6.0f);

        UInstancedStaticMeshComponent* Layer = NewObject<UInstancedStaticMeshComponent>(
            Sector,
            MakeUniqueObjectName(Sector, UInstancedStaticMeshComponent::StaticClass(), ComponentBaseName));
        if (!Layer) return 0;

        Layer->SetupAttachment(Sector->GetRootComponent());
        Layer->SetStaticMesh(PropMesh);
        Layer->SetCollisionProfileName(bCollision ? TEXT("BlockAll") : TEXT("NoCollision"));
        Layer->SetCanEverAffectNavigation(false);
        Layer->SetCastShadow(true);
        Sector->AddInstanceComponent(Layer);
        Layer->RegisterComponent();

        const FBoxSphereBounds AnchorBounds = Anchor->GetStaticMesh()->GetBounds();
        int32 Added = 0;
        for (int32 Index = 0; Index < Anchor->GetInstanceCount(); Index += Stride)
        {
            FTransform AnchorTransform;
            if (!Anchor->GetInstanceTransform(Index, AnchorTransform, false)) continue;

            const FVector AnchorScale = AnchorTransform.GetScale3D().GetAbs();
            const float AnchorBottomZ = AnchorTransform.GetLocation().Z +
                (AnchorBounds.Origin.Z - AnchorBounds.BoxExtent.Z) * AnchorScale.Z;

            FVector Location = AnchorTransform.GetLocation() +
                AnchorTransform.GetRotation().RotateVector(LocalOffset);
            Location.Z = AnchorBottomZ - (PropBounds.Origin.Z - PropBounds.BoxExtent.Z) * PropScale;

            const FQuat Rotation = AnchorTransform.GetRotation() * FQuat(FRotator(0.0f, YawOffsetDegrees, 0.0f));
            const FTransform PropTransform(Rotation, Location, FVector(PropScale));
            if (Layer->AddInstance(PropTransform, false) != INDEX_NONE) ++Added;
        }

        if (Added <= 0)
        {
            Layer->DestroyComponent();
        }
        return Added;
    }

    int32 AddBenchTopPropLayer(
        AOCWorldSectorOster* Sector,
        UInstancedStaticMeshComponent* Benches,
        UStaticMesh* PropMesh,
        const FName ComponentBaseName,
        const float DesiredHeightCm,
        const int32 Stride,
        const FVector LocalOffset,
        const float SurfaceHeightCm,
        const float YawOffsetDegrees)
    {
        if (!Sector || !Benches || !Benches->GetStaticMesh() || !PropMesh || Stride <= 0) return 0;

        const FBoxSphereBounds PropBounds = PropMesh->GetBounds();
        const FVector PropSize = PropBounds.BoxExtent * 2.0f;
        if (PropSize.Z <= 1.0f) return 0;
        const float PropScale = FMath::Clamp(DesiredHeightCm / PropSize.Z, 0.03f, 4.0f);

        UInstancedStaticMeshComponent* Layer = NewObject<UInstancedStaticMeshComponent>(
            Sector,
            MakeUniqueObjectName(Sector, UInstancedStaticMeshComponent::StaticClass(), ComponentBaseName));
        if (!Layer) return 0;

        Layer->SetupAttachment(Sector->GetRootComponent());
        Layer->SetStaticMesh(PropMesh);
        Layer->SetCollisionProfileName(TEXT("NoCollision"));
        Layer->SetCanEverAffectNavigation(false);
        Layer->SetCastShadow(true);
        Sector->AddInstanceComponent(Layer);
        Layer->RegisterComponent();

        const FBoxSphereBounds BenchBounds = Benches->GetStaticMesh()->GetBounds();
        int32 Added = 0;
        for (int32 Index = 0; Index < Benches->GetInstanceCount(); Index += Stride)
        {
            FTransform BenchTransform;
            if (!Benches->GetInstanceTransform(Index, BenchTransform, false)) continue;

            const FVector BenchScale = BenchTransform.GetScale3D().GetAbs();
            const float BenchBottomZ = BenchTransform.GetLocation().Z +
                (BenchBounds.Origin.Z - BenchBounds.BoxExtent.Z) * BenchScale.Z;
            FVector Location = BenchTransform.GetLocation() +
                BenchTransform.GetRotation().RotateVector(LocalOffset);
            Location.Z = BenchBottomZ + SurfaceHeightCm -
                (PropBounds.Origin.Z - PropBounds.BoxExtent.Z) * PropScale;

            const FQuat Rotation = BenchTransform.GetRotation() * FQuat(FRotator(0.0f, YawOffsetDegrees, 0.0f));
            if (Layer->AddInstance(FTransform(Rotation, Location, FVector(PropScale)), false) != INDEX_NONE) ++Added;
        }

        if (Added <= 0) Layer->DestroyComponent();
        return Added;
    }
}

bool UOCPass45ImportedUrbanPropSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCPass45ImportedUrbanPropSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    AOCWorldSectorOster* Sector = nullptr;
    int32 SectorCount = 0;
    for (TActorIterator<AOCWorldSectorOster> It(&InWorld); It; ++It)
    {
        Sector = *It;
        ++SectorCount;
    }
    if (SectorCount != 1 || !Sector) return;

    UInstancedStaticMeshComponent* Benches = FindISM(Sector, TEXT("ParkBenches"));
    UInstancedStaticMeshComponent* Sidewalks = FindISM(Sector, TEXT("Sidewalks"));
    if ((!Benches || Benches->GetInstanceCount() < 2 || !Benches->GetStaticMesh()) &&
        (!Sidewalks || Sidewalks->GetInstanceCount() <= 0 || !Sidewalks->GetStaticMesh()))
    {
        return;
    }

    UStaticMesh* BinMesh = ResolveStreetBin();
    UStaticMesh* LampMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Meshes/SM_Lamp_1.SM_Lamp_1"));
    UStaticMesh* BicycleStandMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Meshes/SM_Bicycle_Stand_1.SM_Bicycle_Stand_1"));
    UStaticMesh* FlowerPotMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Mega_Street_Props_Pack/Street_Props_Pack_V1/Mesh/SM_Flower_Pot.SM_Flower_Pot"));
    UStaticMesh* BusStopMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Mega_Street_Props_Pack/Street_Props_Pack_V1/Mesh/SM_Bus_stop.SM_Bus_stop"));
    UStaticMesh* RoadSignMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Mega_Street_Props_Pack/Street_Props_Pack_V1/Mesh/SM_Sign_1.SM_Sign_1"));
    UStaticMesh* StreetBarrierMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Mega_Street_Props_Pack/Street_Props_Pack_V1/Mesh/SM_Barrier.SM_Barrier"));
    UStaticMesh* PylonMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Mega_Street_Props_Pack/Street_Props_Pack_V1/Mesh/SM_Pylons.SM_Pylons"));

    UStaticMesh* CementBagMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Scene_RoadsideConstruction/Assets/MS/3D/Ind_Con_Bag_Cement_Closed_01/SM_Ind_Con_Bag_Cement_Closed_01.SM_Ind_Con_Bag_Cement_Closed_01"));
    UStaticMesh* DebrisBucketMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Scene_RoadsideConstruction/Assets/MS/3D/Ind_Con_Bucket_Debris_White_01/SM_Ind_Con_Bucket_Debris_White_01.SM_Ind_Con_Bucket_Debris_White_01"));
    UStaticMesh* CableWheelMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Scene_RoadsideConstruction/Assets/MS/3D/Ind_Con_CableWheel_Wood_L_01/SM_Ind_Con_CableWheel_Wood_L_01.SM_Ind_Con_CableWheel_Wood_L_01"));

    UStaticMesh* SardineCanMesh = ResolveSardineCan();
    UStaticMesh* CherryJuiceMesh = ResolveCherryJuice();

    int32 BinsAdded = 0;
    int32 LampsAdded = 0;
    int32 BicycleStandsAdded = 0;
    int32 FlowerPotsAdded = 0;
    int32 SardineCansAdded = 0;
    int32 CherryJuiceAdded = 0;
    if (Benches && Benches->GetInstanceCount() >= 2 && Benches->GetStaticMesh())
    {
        BinsAdded = AddAnchoredPropLayer(
            Sector, Benches, BinMesh,
            FName(TEXT("Pass45ImportedStreetBins")),
            90.0f, 2, FVector(0.0f, 175.0f, 0.0f), 0.0f, true);

        LampsAdded = AddAnchoredPropLayer(
            Sector, Benches, LampMesh,
            FName(TEXT("Pass45ImportedParkLamps")),
            430.0f, 3, FVector(-60.0f, -260.0f, 0.0f), 0.0f, true);

        BicycleStandsAdded = AddAnchoredPropLayer(
            Sector, Benches, BicycleStandMesh,
            FName(TEXT("Pass45ImportedBicycleStands")),
            105.0f, 5, FVector(120.0f, -285.0f, 0.0f), 90.0f, true);

        FlowerPotsAdded = AddAnchoredPropLayer(
            Sector, Benches, FlowerPotMesh,
            FName(TEXT("Pass45ImportedFlowerPots")),
            70.0f, 4, FVector(-130.0f, 190.0f, 0.0f), 0.0f, true);

        // Tiny user-imported props are intentionally used as sparse bench dressing instead of becoming
        // meaningless standalone world owners. Missing local ignored payload simply leaves the layer absent.
        SardineCansAdded = AddBenchTopPropLayer(
            Sector, Benches, SardineCanMesh,
            FName(TEXT("Pass45ImportedSardineCans")),
            8.0f, 7, FVector(-25.0f, 0.0f, 0.0f), 55.0f, 18.0f);

        CherryJuiceAdded = AddBenchTopPropLayer(
            Sector, Benches, CherryJuiceMesh,
            FName(TEXT("Pass45ImportedCherryJuice")),
            20.0f, 9, FVector(35.0f, 0.0f, 0.0f), 55.0f, -12.0f);
    }

    int32 BusStopsAdded = 0;
    int32 RoadSignsAdded = 0;
    int32 StreetBarriersAdded = 0;
    int32 PylonsAdded = 0;
    int32 CementBagsAdded = 0;
    int32 DebrisBucketsAdded = 0;
    int32 CableWheelsAdded = 0;
    if (Sidewalks && Sidewalks->GetInstanceCount() > 0 && Sidewalks->GetStaticMesh())
    {
        // Sidewalk segments are already the authoritative road-edge topology. Sparse props derive from those
        // transforms, so they follow the current compact Oster road network without inventing a second map layout.
        BusStopsAdded = AddAnchoredPropLayer(
            Sector, Sidewalks, BusStopMesh,
            FName(TEXT("Pass45ImportedBusStops")),
            285.0f, 5, FVector(0.0f, 90.0f, 0.0f), 90.0f, true);

        RoadSignsAdded = AddAnchoredPropLayer(
            Sector, Sidewalks, RoadSignMesh,
            FName(TEXT("Pass45ImportedRoadSigns")),
            250.0f, 3, FVector(0.0f, -95.0f, 0.0f), 90.0f, false);

        StreetBarriersAdded = AddAnchoredPropLayer(
            Sector, Sidewalks, StreetBarrierMesh,
            FName(TEXT("Pass45ImportedStreetBarriers")),
            115.0f, 7, FVector(160.0f, -135.0f, 0.0f), 90.0f, true);

        PylonsAdded = AddAnchoredPropLayer(
            Sector, Sidewalks, PylonMesh,
            FName(TEXT("Pass45ImportedPylons")),
            72.0f, 6, FVector(-145.0f, 120.0f, 0.0f), 90.0f, true);

        // Roadside Construction is now more than a surface donor: a sparse construction-dressing layer uses
        // exact tracked meshes while preserving the current Sidewalks topology as the only placement authority.
        CementBagsAdded = AddAnchoredPropLayer(
            Sector, Sidewalks, CementBagMesh,
            FName(TEXT("Pass45ImportedCementBags")),
            34.0f, 8, FVector(220.0f, 150.0f, 0.0f), 25.0f, false);

        DebrisBucketsAdded = AddAnchoredPropLayer(
            Sector, Sidewalks, DebrisBucketMesh,
            FName(TEXT("Pass45ImportedDebrisBuckets")),
            46.0f, 10, FVector(-210.0f, 145.0f, 0.0f), -20.0f, false);

        CableWheelsAdded = AddAnchoredPropLayer(
            Sector, Sidewalks, CableWheelMesh,
            FName(TEXT("Pass45ImportedCableWheels")),
            125.0f, 12, FVector(260.0f, -160.0f, 0.0f), 90.0f, true);
    }

    if (!BinMesh)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_IMPORTED_URBAN_PROP_CONTENT_GAP type=street_bin strict_token_match=1 wrong_prop_substitution=0"));
    }
    if (!LampMesh) UE_LOG(LogTemp, Warning, TEXT("PASS45_IMPORTED_URBAN_PROP_CONTENT_GAP type=park_lamp exact_asset=SM_Lamp_1 wrong_prop_substitution=0"));
    if (!BicycleStandMesh) UE_LOG(LogTemp, Warning, TEXT("PASS45_IMPORTED_URBAN_PROP_CONTENT_GAP type=bicycle_stand exact_asset=SM_Bicycle_Stand_1 wrong_prop_substitution=0"));
    if (!FlowerPotMesh) UE_LOG(LogTemp, Warning, TEXT("PASS45_IMPORTED_URBAN_PROP_CONTENT_GAP type=flower_pot exact_asset=SM_Flower_Pot wrong_prop_substitution=0"));
    if (!BusStopMesh) UE_LOG(LogTemp, Warning, TEXT("PASS45_IMPORTED_URBAN_PROP_CONTENT_GAP type=bus_stop exact_asset=SM_Bus_stop wrong_prop_substitution=0"));
    if (!RoadSignMesh) UE_LOG(LogTemp, Warning, TEXT("PASS45_IMPORTED_URBAN_PROP_CONTENT_GAP type=road_sign exact_asset=SM_Sign_1 wrong_prop_substitution=0"));
    if (!StreetBarrierMesh) UE_LOG(LogTemp, Warning, TEXT("PASS45_IMPORTED_URBAN_PROP_CONTENT_GAP type=street_barrier exact_asset=SM_Barrier wrong_prop_substitution=0"));
    if (!PylonMesh) UE_LOG(LogTemp, Warning, TEXT("PASS45_IMPORTED_URBAN_PROP_CONTENT_GAP type=pylon exact_asset=SM_Pylons wrong_prop_substitution=0"));
    if (!CementBagMesh || !DebrisBucketMesh || !CableWheelMesh)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("PASS45_ROADSIDE_CONSTRUCTION_PROP_GAP cement_bag=%d debris_bucket=%d cable_wheel=%d wrong_prop_substitution=0"),
            CementBagMesh ? 1 : 0, DebrisBucketMesh ? 1 : 0, CableWheelMesh ? 1 : 0);
    }
    if (!SardineCanMesh) UE_LOG(LogTemp, Display, TEXT("PASS45_SMALL_PROP_CONTENT_GAP type=sardine_can local_ignored_payload=1 wrong_prop_substitution=0"));
    if (!CherryJuiceMesh) UE_LOG(LogTemp, Display, TEXT("PASS45_SMALL_PROP_CONTENT_GAP type=cherry_juice local_ignored_payload=1 wrong_prop_substitution=0"));

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_IMPORTED_URBAN_PROP_LAYER_READY bins=%d lamps=%d bicycle_stands=%d flower_pots=%d bus_stops=%d road_signs=%d barriers=%d pylons=%d cement_bags=%d debris_buckets=%d cable_wheels=%d sardine_cans=%d cherry_juice=%d park_owner=ParkBenches road_owner=Sidewalks duplicate_world_owner=0 runtime_acceptance=0"),
        BinsAdded, LampsAdded, BicycleStandsAdded, FlowerPotsAdded, BusStopsAdded, RoadSignsAdded,
        StreetBarriersAdded, PylonsAdded, CementBagsAdded, DebrisBucketsAdded, CableWheelsAdded,
        SardineCansAdded, CherryJuiceAdded);
}
