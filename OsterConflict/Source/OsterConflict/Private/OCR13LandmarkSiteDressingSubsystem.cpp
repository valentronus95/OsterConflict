#include "OCR13LandmarkSiteDressingSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
    constexpr float DressingDelaySeconds = 2.05f;

    UStaticMesh* LoadSiteMesh(const TCHAR* Path)
    {
        return LoadObject<UStaticMesh>(nullptr, Path);
    }

    UInstancedStaticMeshComponent* MakeVisualISM(AActor* Owner, USceneComponent* Root, UStaticMesh* Mesh,
        const FName Name, const bool bCastShadow, const int32 CullEndCm)
    {
        if (!Owner || !Root || !Mesh) return nullptr;

        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;

        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        Component->SetCastShadow(bCastShadow);
        if (CullEndCm > 0) Component->SetCullDistances(0, CullEndCm);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void AddFittedInstance(UInstancedStaticMeshComponent* Target, const FVector& Center,
        const FVector& DesiredLocalSize, const float YawDegrees)
    {
        if (!Target || !Target->GetStaticMesh()) return;

        const FBoxSphereBounds Bounds = Target->GetStaticMesh()->GetBounds();
        const FVector MeshSize = Bounds.BoxExtent * 2.0f;
        if (MeshSize.X <= 1.0f || MeshSize.Y <= 1.0f || MeshSize.Z <= 1.0f) return;

        const FVector Scale(
            DesiredLocalSize.X / MeshSize.X,
            DesiredLocalSize.Y / MeshSize.Y,
            DesiredLocalSize.Z / MeshSize.Z);
        const FQuat Rotation = FRotator(0.0f, YawDegrees, 0.0f).Quaternion();
        const FVector Location = Center - Rotation.RotateVector(Bounds.Origin * Scale);
        Target->AddInstance(FTransform(Rotation, Location, Scale), true);
    }

    void AddHeightFittedInstance(UInstancedStaticMeshComponent* Target, const FVector& GroundLocation,
        const float DesiredHeightCm, const float YawDegrees, const float ScaleVariation = 1.0f)
    {
        if (!Target || !Target->GetStaticMesh()) return;

        const FBoxSphereBounds Bounds = Target->GetStaticMesh()->GetBounds();
        const FVector MeshSize = Bounds.BoxExtent * 2.0f;
        if (MeshSize.Z <= 1.0f) return;

        const float UniformScale = FMath::Clamp((DesiredHeightCm / MeshSize.Z) * ScaleVariation, 0.18f, 4.0f);
        const FQuat Rotation = FRotator(0.0f, YawDegrees, 0.0f).Quaternion();
        const FVector Scale(UniformScale);
        FVector Center = GroundLocation;
        Center.Z += Bounds.BoxExtent.Z * UniformScale;
        const FVector Location = Center - Rotation.RotateVector(Bounds.Origin * Scale);
        Target->AddInstance(FTransform(Rotation, Location, Scale), true);
    }

    UInstancedStaticMeshComponent* FirstAvailable(const TArray<UInstancedStaticMeshComponent*>& Families, const int32 Index)
    {
        if (Families.Num() == 0) return nullptr;
        for (int32 Offset = 0; Offset < Families.Num(); ++Offset)
        {
            if (UInstancedStaticMeshComponent* Candidate = Families[(Index + Offset) % Families.Num()]) return Candidate;
        }
        return nullptr;
    }

    void DressMuseum(const FVector& Museum,
        UInstancedStaticMeshComponent* Path,
        const TArray<UInstancedStaticMeshComponent*>& Shrubs,
        UInstancedStaticMeshComponent* Bin)
    {
        // The photographed facade is intentionally left unobstructed. The approach stays centered on the existing
        // entrance projection; the front boundary/gate remains owned by the base landmark until photo refinement.
        if (Path)
        {
            for (int32 Segment = 0; Segment < 6; ++Segment)
            {
                AddFittedInstance(Path,
                    Museum + FVector(1180.0f, -3500.0f - Segment * 1550.0f, 2.0f),
                    FVector(720.0f, 1680.0f, 12.0f), 0.0f);
            }
        }

        const FVector ShrubOffsets[] =
        {
            FVector(-3650, -650, 0), FVector(-3850, 900, 0), FVector(-3500, 2100, 0),
            FVector(3600, -500, 0), FVector(3900, 1050, 0), FVector(3550, 2200, 0),
            FVector(-2100, 3100, 0), FVector(2300, 3200, 0)
        };
        for (int32 Index = 0; Index < UE_ARRAY_COUNT(ShrubOffsets); ++Index)
        {
            if (UInstancedStaticMeshComponent* Family = FirstAvailable(Shrubs, Index))
            {
                AddHeightFittedInstance(Family, Museum + ShrubOffsets[Index],
                    95.0f + static_cast<float>(Index % 3) * 18.0f,
                    static_cast<float>((Index * 47) % 360));
            }
        }

        AddHeightFittedInstance(Bin, Museum + FVector(2400, -3050, 0), 95.0f, 14.0f);
    }

    void DressStadium(const FVector& Stadium,
        const TArray<UInstancedStaticMeshComponent*>& Shrubs,
        UInstancedStaticMeshComponent* Bin,
        UInstancedStaticMeshComponent* UtilityBox)
    {
        // Keep the pitch and running apron completely clear. Furniture lives only on the spectator/service edge.
        const FVector BinOffsets[] =
        {
            FVector(-5000, -5000, 0), FVector(-1550, -5350, 0),
            FVector(1450, -5350, 0), FVector(5050, -5000, 0)
        };
        for (int32 Index = 0; Index < UE_ARRAY_COUNT(BinOffsets); ++Index)
        {
            AddHeightFittedInstance(Bin, Stadium + BinOffsets[Index], 105.0f, 90.0f + Index * 11.0f);
        }
        AddHeightFittedInstance(UtilityBox, Stadium + FVector(3900, -5900, 0), 125.0f, 0.0f);

        for (int32 Index = 0; Index < 8; ++Index)
        {
            if (UInstancedStaticMeshComponent* Family = FirstAvailable(Shrubs, Index + 2))
            {
                const float X = -5250.0f + static_cast<float>(Index) * 1500.0f;
                const float Y = 5450.0f + static_cast<float>(Index % 2) * 240.0f;
                AddHeightFittedInstance(Family, Stadium + FVector(X, Y, 0),
                    105.0f + static_cast<float>(Index % 3) * 16.0f,
                    static_cast<float>((Index * 53) % 360));
            }
        }
    }

    void DressCollege(const FVector& College,
        const TArray<UInstancedStaticMeshComponent*>& Shrubs,
        UInstancedStaticMeshComponent* Bin,
        UInstancedStaticMeshComponent* UtilityBox,
        UInstancedStaticMeshComponent* PowerPole,
        UInstancedStaticMeshComponent* Pallet,
        UInstancedStaticMeshComponent* Crate)
    {
        // Front entrance and broad steps stay open. Utility clutter is restricted to side/rear service areas.
        AddHeightFittedInstance(PowerPole, College + FVector(-5050, -3300, 0), 760.0f, 2.0f);
        AddHeightFittedInstance(PowerPole, College + FVector(5050, -3300, 0), 760.0f, 1.0f);

        AddHeightFittedInstance(Bin, College + FVector(-2550, -2180, 0), 105.0f, 8.0f);
        AddHeightFittedInstance(Bin, College + FVector(3500, -2180, 0), 105.0f, -8.0f);
        AddHeightFittedInstance(UtilityBox, College + FVector(4200, 3650, 0), 135.0f, 90.0f);
        AddHeightFittedInstance(Pallet, College + FVector(4050, 4550, 0), 18.0f, 7.0f);
        AddHeightFittedInstance(Crate, College + FVector(4400, 4450, 0), 95.0f, -12.0f);

        const FVector ShrubOffsets[] =
        {
            FVector(-4700, -1750, 0), FVector(-3900, -1750, 0),
            FVector(4300, -1700, 0), FVector(5050, -1500, 0),
            FVector(-5200, 2850, 0), FVector(5350, 3350, 0),
            FVector(-5000, 7900, 0), FVector(4750, 8350, 0)
        };
        for (int32 Index = 0; Index < UE_ARRAY_COUNT(ShrubOffsets); ++Index)
        {
            if (UInstancedStaticMeshComponent* Family = FirstAvailable(Shrubs, Index + 4))
            {
                AddHeightFittedInstance(Family, College + ShrubOffsets[Index],
                    110.0f + static_cast<float>(Index % 4) * 14.0f,
                    static_cast<float>((Index * 41 + 17) % 360));
            }
        }
    }
}

bool UOCR13LandmarkSiteDressingSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13LandmarkSiteDressingSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) ApplyLandmarkSiteDressing(*World);
        }), DressingDelaySeconds, false);
}

void UOCR13LandmarkSiteDressingSubsystem::ApplyLandmarkSiteDressing(UWorld& World)
{
    if (bApplied) return;

    UStaticMesh* PathMesh = LoadSiteMesh(TEXT("/Game/TileableForestRoad/Meshes/SM_Forest_Path.SM_Forest_Path"));
    UStaticMesh* Shrub01 = LoadSiteMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/Shrubs_1.Shrubs_1"));
    UStaticMesh* Shrub02 = LoadSiteMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/Shrubs_1_Single.Shrubs_1_Single"));
    UStaticMesh* Bush01 = LoadSiteMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/Bush_1.Bush_1"));
    UStaticMesh* BinMesh = LoadSiteMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Plastic_Trash_Bin_Bin.Plastic_Trash_Bin_Bin"));
    UStaticMesh* UtilityMesh = LoadSiteMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Utility_Box_1a.Utility_Box_1a"));
    UStaticMesh* PoleMesh = LoadSiteMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Power_Pole_1.Power_Pole_1"));
    UStaticMesh* PalletMesh = LoadSiteMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Pallet.Pallet"));
    UStaticMesh* CrateMesh = LoadSiteMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Wooden_Crate_1.Wooden_Crate_1"));

    if (!PathMesh && !Shrub01 && !Shrub02 && !Bush01 && !BinMesh && !UtilityMesh && !PoleMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("R13 landmark site dressing: bundled site-art meshes unavailable; preserving base landmarks."));
        return;
    }

    AActor* ArtRoot = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!ArtRoot) return;
    ArtRoot->SetReplicates(false);
    ArtRoot->SetActorEnableCollision(false);

    USceneComponent* Root = NewObject<USceneComponent>(ArtRoot, TEXT("R13_LandmarkSiteDressingRoot"));
    if (!Root)
    {
        ArtRoot->Destroy();
        return;
    }
    Root->SetMobility(EComponentMobility::Static);
    ArtRoot->SetRootComponent(Root);
    ArtRoot->AddInstanceComponent(Root);
    Root->RegisterComponent();

    UInstancedStaticMeshComponent* Path = MakeVisualISM(ArtRoot, Root, PathMesh,
        TEXT("R13_MuseumApproachPath"), false, 65000);
    TArray<UInstancedStaticMeshComponent*> Shrubs = {
        MakeVisualISM(ArtRoot, Root, Shrub01, TEXT("R13_LandmarkShrub01"), false, 36000),
        MakeVisualISM(ArtRoot, Root, Shrub02, TEXT("R13_LandmarkShrub02"), false, 36000),
        MakeVisualISM(ArtRoot, Root, Bush01, TEXT("R13_LandmarkShrub03"), false, 36000)
    };
    Shrubs.Remove(nullptr);

    UInstancedStaticMeshComponent* Bin = MakeVisualISM(ArtRoot, Root, BinMesh,
        TEXT("R13_LandmarkTrashBins"), true, 42000);
    UInstancedStaticMeshComponent* Utility = MakeVisualISM(ArtRoot, Root, UtilityMesh,
        TEXT("R13_LandmarkUtilityBoxes"), true, 48000);
    UInstancedStaticMeshComponent* Pole = MakeVisualISM(ArtRoot, Root, PoleMesh,
        TEXT("R13_CollegePowerPoles"), true, 85000);
    UInstancedStaticMeshComponent* Pallet = MakeVisualISM(ArtRoot, Root, PalletMesh,
        TEXT("R13_CollegeServicePallets"), true, 32000);
    UInstancedStaticMeshComponent* Crate = MakeVisualISM(ArtRoot, Root, CrateMesh,
        TEXT("R13_CollegeServiceCrates"), true, 32000);

    DressMuseum(AOCWorldSectorOster::MuseumAnchor(), Path, Shrubs, Bin);
    DressStadium(AOCWorldSectorOster::StadiumAnchor(), Shrubs, Bin, Utility);
    DressCollege(AOCWorldSectorOster::CollegeAnchor(), Shrubs, Bin, Utility, Pole, Pallet, Crate);

    bApplied = true;
    UE_LOG(LogTemp, Display,
        TEXT("R13 landmark site dressing: museum, stadium and college visual-only site layers applied."));
}
