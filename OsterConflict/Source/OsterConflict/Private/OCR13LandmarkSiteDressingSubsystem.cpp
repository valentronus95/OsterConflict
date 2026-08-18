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
    constexpr float DressingDelaySeconds = 2.35f;

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
        const float DesiredHeightCm, const float YawDegrees)
    {
        if (!Target || !Target->GetStaticMesh()) return;
        const FBoxSphereBounds Bounds = Target->GetStaticMesh()->GetBounds();
        const FVector MeshSize = Bounds.BoxExtent * 2.0f;
        if (MeshSize.Z <= 1.0f) return;

        const float UniformScale = FMath::Clamp(DesiredHeightCm / MeshSize.Z, 0.18f, 4.0f);
        const FQuat Rotation = FRotator(0.0f, YawDegrees, 0.0f).Quaternion();
        const FVector Scale(UniformScale);
        FVector Center = GroundLocation;
        Center.Z += Bounds.BoxExtent.Z * UniformScale;
        const FVector Location = Center - Rotation.RotateVector(Bounds.Origin * Scale);
        Target->AddInstance(FTransform(Rotation, Location, Scale), true);
    }

    void DressMuseum(const FVector& Museum,
        UInstancedStaticMeshComponent* Path,
        UInstancedStaticMeshComponent* Bin)
    {
        // Photos show the historic facade framed by mature planting with a strong, readable approach axis.
        // The path is deliberately visual-only; the existing front boundary remains untouched until photo refinement.
        if (Path)
        {
            for (int32 Segment = 0; Segment < 6; ++Segment)
            {
                AddFittedInstance(Path,
                    Museum + FVector(1180.0f, -3500.0f - Segment * 1550.0f, 2.0f),
                    FVector(720.0f, 1680.0f, 12.0f), 0.0f);
            }
        }
        AddHeightFittedInstance(Bin, Museum + FVector(2400, -3050, 0), 95.0f, 14.0f);
    }

    void DressStadium(const FVector& Stadium,
        UInstancedStaticMeshComponent* Bin,
        UInstancedStaticMeshComponent* UtilityBox)
    {
        // Pitch, track and goal area remain completely clear. Details sit on the spectator/service edge only.
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
    }

    void DressCollege(const FVector& College,
        UInstancedStaticMeshComponent* Bin,
        UInstancedStaticMeshComponent* UtilityBox,
        UInstancedStaticMeshComponent* Pallet,
        UInstancedStaticMeshComponent* Crate)
    {
        // Existing R13.5 civic planting owns the green campus edge. This pass only adds restrained service furniture.
        // Main entrance and broad stairs around X=900 stay unobstructed.
        AddHeightFittedInstance(Bin, College + FVector(-2550, -2180, 0), 105.0f, 8.0f);
        AddHeightFittedInstance(Bin, College + FVector(3500, -2180, 0), 105.0f, -8.0f);
        AddHeightFittedInstance(UtilityBox, College + FVector(4200, 3650, 0), 135.0f, 90.0f);
        AddHeightFittedInstance(Pallet, College + FVector(4050, 4550, 0), 18.0f, 7.0f);
        AddHeightFittedInstance(Crate, College + FVector(4400, 4450, 0), 95.0f, -12.0f);
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
    UStaticMesh* BinMesh = LoadSiteMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Plastic_Trash_Bin_Bin.Plastic_Trash_Bin_Bin"));
    UStaticMesh* UtilityMesh = LoadSiteMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Utility_Box_1a.Utility_Box_1a"));
    UStaticMesh* PalletMesh = LoadSiteMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Pallet.Pallet"));
    UStaticMesh* CrateMesh = LoadSiteMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Wooden_Crate_1.Wooden_Crate_1"));

    if (!PathMesh && !BinMesh && !UtilityMesh && !PalletMesh && !CrateMesh)
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
    UInstancedStaticMeshComponent* Bin = MakeVisualISM(ArtRoot, Root, BinMesh,
        TEXT("R13_LandmarkTrashBins"), true, 42000);
    UInstancedStaticMeshComponent* Utility = MakeVisualISM(ArtRoot, Root, UtilityMesh,
        TEXT("R13_LandmarkUtilityBoxes"), true, 48000);
    UInstancedStaticMeshComponent* Pallet = MakeVisualISM(ArtRoot, Root, PalletMesh,
        TEXT("R13_CollegeServicePallets"), true, 32000);
    UInstancedStaticMeshComponent* Crate = MakeVisualISM(ArtRoot, Root, CrateMesh,
        TEXT("R13_CollegeServiceCrates"), true, 32000);

    DressMuseum(AOCWorldSectorOster::MuseumAnchor(), Path, Bin);
    DressStadium(AOCWorldSectorOster::StadiumAnchor(), Bin, Utility);
    DressCollege(AOCWorldSectorOster::CollegeAnchor(), Bin, Utility, Pallet, Crate);

    bApplied = true;
    UE_LOG(LogTemp, Display,
        TEXT("R13 landmark site dressing: museum approach + stadium/college furniture applied; civic planting and roadside poles remain separately owned."));
}
