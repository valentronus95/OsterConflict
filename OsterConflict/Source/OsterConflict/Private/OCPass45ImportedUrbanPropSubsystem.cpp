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

    int32 AddBenchAnchoredPropLayer(
        AOCWorldSectorOster* Sector,
        UInstancedStaticMeshComponent* Benches,
        UStaticMesh* PropMesh,
        const FName ComponentBaseName,
        const float DesiredHeightCm,
        const int32 Stride,
        const FVector LocalOffset,
        const float YawOffsetDegrees)
    {
        if (!Sector || !Benches || !Benches->GetStaticMesh() || !PropMesh || Stride <= 0) return 0;

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
        Layer->SetCollisionProfileName(TEXT("BlockAll"));
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
            Location.Z = BenchBottomZ - (PropBounds.Origin.Z - PropBounds.BoxExtent.Z) * PropScale;

            const FQuat Rotation = BenchTransform.GetRotation() * FQuat(FRotator(0.0f, YawOffsetDegrees, 0.0f));
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
    if (!Benches || Benches->GetInstanceCount() < 2 || !Benches->GetStaticMesh()) return;

    UStaticMesh* BinMesh = ResolveStreetBin();
    UStaticMesh* LampMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Meshes/SM_Lamp_1.SM_Lamp_1"));
    UStaticMesh* BicycleStandMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Meshes/SM_Bicycle_Stand_1.SM_Bicycle_Stand_1"));
    UStaticMesh* FlowerPotMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Mega_Street_Props_Pack/Street_Props_Pack_V1/Mesh/SM_Flower_Pot.SM_Flower_Pot"));

    const int32 BinsAdded = AddBenchAnchoredPropLayer(
        Sector, Benches, BinMesh,
        FName(TEXT("Pass45ImportedStreetBins")),
        90.0f, 2, FVector(0.0f, 175.0f, 0.0f), 0.0f);

    const int32 LampsAdded = AddBenchAnchoredPropLayer(
        Sector, Benches, LampMesh,
        FName(TEXT("Pass45ImportedParkLamps")),
        430.0f, 3, FVector(-60.0f, -260.0f, 0.0f), 0.0f);

    const int32 BicycleStandsAdded = AddBenchAnchoredPropLayer(
        Sector, Benches, BicycleStandMesh,
        FName(TEXT("Pass45ImportedBicycleStands")),
        105.0f, 5, FVector(120.0f, -285.0f, 0.0f), 90.0f);

    const int32 FlowerPotsAdded = AddBenchAnchoredPropLayer(
        Sector, Benches, FlowerPotMesh,
        FName(TEXT("Pass45ImportedFlowerPots")),
        70.0f, 4, FVector(-130.0f, 190.0f, 0.0f), 0.0f);

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

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_IMPORTED_URBAN_PROP_LAYER_READY bins=%d lamps=%d bicycle_stands=%d flower_pots=%d placement_owner=ParkBenches duplicate_world_owner=0 runtime_acceptance=0"),
        BinsAdded, LampsAdded, BicycleStandsAdded, FlowerPotsAdded);
}
