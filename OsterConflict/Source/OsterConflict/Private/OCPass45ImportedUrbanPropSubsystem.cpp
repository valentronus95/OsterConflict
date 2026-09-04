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
    if (!BinMesh)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_IMPORTED_URBAN_PROP_CONTENT_GAP type=street_bin strict_token_match=1 wrong_prop_substitution=0"));
        return;
    }

    const FBoxSphereBounds BinBounds = BinMesh->GetBounds();
    const FVector BinSize = BinBounds.BoxExtent * 2.0f;
    if (BinSize.Z <= 1.0f) return;
    const float BinScale = FMath::Clamp(90.0f / BinSize.Z, 0.05f, 5.0f);

    UInstancedStaticMeshComponent* Bins = NewObject<UInstancedStaticMeshComponent>(
        Sector,
        MakeUniqueObjectName(Sector, UInstancedStaticMeshComponent::StaticClass(), FName(TEXT("Pass45ImportedStreetBins"))));
    if (!Bins) return;

    Bins->SetupAttachment(Sector->GetRootComponent());
    Bins->SetStaticMesh(BinMesh);
    Bins->SetCollisionProfileName(TEXT("BlockAll"));
    Bins->SetCanEverAffectNavigation(false);
    Bins->SetCastShadow(true);
    Sector->AddInstanceComponent(Bins);
    Bins->RegisterComponent();

    const FBoxSphereBounds BenchBounds = Benches->GetStaticMesh()->GetBounds();
    int32 Added = 0;
    for (int32 Index = 0; Index < Benches->GetInstanceCount(); Index += 2)
    {
        FTransform BenchTransform;
        if (!Benches->GetInstanceTransform(Index, BenchTransform, false)) continue;

        const FVector BenchScale = BenchTransform.GetScale3D().GetAbs();
        const float BenchBottomZ = BenchTransform.GetLocation().Z +
            (BenchBounds.Origin.Z - BenchBounds.BoxExtent.Z) * BenchScale.Z;
        FVector Location = BenchTransform.GetLocation() +
            BenchTransform.GetRotation().RotateVector(FVector(0.0f, 175.0f, 0.0f));
        Location.Z = BenchBottomZ - (BinBounds.Origin.Z - BinBounds.BoxExtent.Z) * BinScale;

        const FTransform BinTransform(
            BenchTransform.GetRotation(),
            Location,
            FVector(BinScale));
        if (Bins->AddInstance(BinTransform, false) != INDEX_NONE) ++Added;
    }

    if (Added <= 0)
    {
        Bins->DestroyComponent();
        return;
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_IMPORTED_URBAN_PROP_READY type=street_bin asset=%s instances=%d placement_owner=ParkBenches strict_identity_match=1 runtime_acceptance=0"),
        *BinMesh->GetPathName(), Added);
}
