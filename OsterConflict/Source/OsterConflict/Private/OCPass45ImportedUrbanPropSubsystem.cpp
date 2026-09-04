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

    int32 BinsAdded = 0;
    int32 LampsAdded = 0;
    int32 BicycleStandsAdded = 0;
    int32 FlowerPotsAdded = 0;
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
    }

    int32 BusStopsAdded = 0;
    int32 RoadSignsAdded = 0;
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
    }

    if (!BinMesh)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_IMPORTED_URBAN_PROP_CONTENT_GAP type=street_bin strict_token_match=1 wrong_prop_substitution=0"));
    }
    if (!LampMesh)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("PASS45_IMPORTED_URBAN_PROP_CONTENT_GAP type=park_lamp exact_asset=SM_Lamp_1 wrong_prop_substitution=0"));
    }
    if (!BicycleStandMesh)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("PASS45_IMPORTED_URBAN_PROP_CONTENT_GAP type=bicycle_stand exact_asset=SM_Bicycle_Stand_1 wrong_prop_substitution=0"));
    }
    if (!FlowerPotMesh)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("PASS45_IMPORTED_URBAN_PROP_CONTENT_GAP type=flower_pot exact_asset=SM_Flower_Pot wrong_prop_substitution=0"));
    }
    if (!BusStopMesh)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("PASS45_IMPORTED_URBAN_PROP_CONTENT_GAP type=bus_stop exact_asset=SM_Bus_stop wrong_prop_substitution=0"));
    }
    if (!RoadSignMesh)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("PASS45_IMPORTED_URBAN_PROP_CONTENT_GAP type=road_sign exact_asset=SM_Sign_1 wrong_prop_substitution=0"));
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_IMPORTED_URBAN_PROP_LAYER_READY bins=%d lamps=%d bicycle_stands=%d flower_pots=%d bus_stops=%d road_signs=%d park_owner=ParkBenches road_owner=Sidewalks duplicate_world_owner=0 runtime_acceptance=0"),
        BinsAdded, LampsAdded, BicycleStandsAdded, FlowerPotsAdded, BusStopsAdded, RoadSignsAdded);
}
