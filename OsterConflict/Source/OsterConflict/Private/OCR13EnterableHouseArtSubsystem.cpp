#include "OCR13EnterableHouseArtSubsystem.h"

#include "OCEnterableHouse.h"
#include "OCGameMode.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"

namespace
{
    constexpr float FirstArtPassDelaySeconds = 2.05f;
    constexpr float ArtRetryIntervalSeconds = 0.75f;
    constexpr int32 MaxArtRetryPasses = 8;

    UMaterialInterface* LoadMaterial(const TCHAR* Path)
    {
        return LoadObject<UMaterialInterface>(nullptr, Path);
    }

    UInstancedStaticMeshComponent* FindISM(AActor* Actor, const FName Name)
    {
        return Actor ? FindObjectFast<UInstancedStaticMeshComponent>(Actor, Name) : nullptr;
    }

    void ApplyIfAvailable(UPrimitiveComponent* Component, UMaterialInterface* Material)
    {
        if (Component && Material) Component->SetMaterial(0, Material);
    }

    void AddRoof(AOCEnterableHouse* House, UStaticMesh* RoofMesh, UMaterialInterface* RoofMaterial)
    {
        if (!House || !House->GetRootComponent() || !RoofMesh) return;
        if (FindObjectFast<UInstancedStaticMeshComponent>(House, TEXT("R13_EnterableRoof"))) return;

        UInstancedStaticMeshComponent* Roof = NewObject<UInstancedStaticMeshComponent>(
            House, TEXT("R13_EnterableRoof"));
        if (!Roof) return;

        Roof->SetupAttachment(House->GetRootComponent());
        Roof->SetStaticMesh(RoofMesh);
        Roof->SetMobility(EComponentMobility::Static);
        Roof->SetCollisionProfileName(TEXT("NoCollision"));
        Roof->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Roof->SetGenerateOverlapEvents(false);
        Roof->SetCanEverAffectNavigation(false);
        Roof->SetCastShadow(true);
        if (RoofMaterial) Roof->SetMaterial(0, RoofMaterial);
        House->AddInstanceComponent(Roof);
        Roof->RegisterComponent();

        const FBoxSphereBounds Bounds = RoofMesh->GetBounds();
        const FVector Size = Bounds.BoxExtent * 2.0f;
        if (Size.X <= 10.0f || Size.Y <= 10.0f || Size.Z <= 10.0f)
        {
            Roof->DestroyComponent();
            return;
        }

        // Functional shell footprint is 16 x 11 m. Slight overhang keeps the flat prototype ceiling hidden.
        const float ScaleX = 1780.0f / Size.X;
        const float ScaleY = 1280.0f / Size.Y;
        const float HorizontalScale = FMath::Min(ScaleX, ScaleY);
        const FVector Scale(ScaleX, ScaleY, FMath::Clamp(HorizontalScale, 0.45f, 4.0f));

        // BuildShell's flat prototype ceiling top is ~344 cm. Ground the authored pitched roof on that plane.
        const float LocalBottom = Bounds.Origin.Z - Bounds.BoxExtent.Z;
        const FVector Location(-Bounds.Origin.X * Scale.X, -Bounds.Origin.Y * Scale.Y,
            344.0f - LocalBottom * Scale.Z);

        Roof->AddInstance(FTransform(FRotator::ZeroRotator, Location, Scale), false);
    }
}

bool UOCR13EnterableHouseArtSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13EnterableHouseArtSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    InWorld.GetTimerManager().SetTimer(ArtRetryTimer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) ApplyEnterableHouseArt(*World);
        }), ArtRetryIntervalSeconds, true, FirstArtPassDelaySeconds);
}

void UOCR13EnterableHouseArtSubsystem::ApplyEnterableHouseArt(UWorld& World)
{
    UStaticMesh* RoofMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Roof_Both_Ends_4m.Roof_Both_Ends_4m"));
    UMaterialInterface* RoofMaterial = LoadMaterial(
        TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Metal_Roof.Metal_Roof"));

    UMaterialInterface* WallMaterials[] = {
        LoadMaterial(TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Wood_Planks_Painted_Blue.Wood_Planks_Painted_Blue")),
        LoadMaterial(TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Wood_Planks_Painted_Green.Wood_Planks_Painted_Green")),
        LoadMaterial(TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Wood_Planks_Painted_Yellow.Wood_Planks_Painted_Yellow")),
        LoadMaterial(TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Wood_Planks_Painted_Red.Wood_Planks_Painted_Red")),
        LoadMaterial(TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Wood_Old.Wood_Old")),
    };
    UMaterialInterface* FenceMaterial = LoadMaterial(
        TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Wood_Old.Wood_Old"));
    UMaterialInterface* YardMaterial = LoadMaterial(
        TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Diorama_Ground.Diorama_Ground"));

    int32 NewlyStyledHouseCount = 0;
    int32 RoofCount = 0;
    for (TActorIterator<AOCEnterableHouse> It(&World); It; ++It)
    {
        AOCEnterableHouse* House = *It;
        if (!House) continue;

        const TWeakObjectPtr<AOCEnterableHouse> HouseKey(House);
        if (StyledHouses.Contains(HouseKey)) continue;

        const uint32 StableHash = HashCombine(GetTypeHash(House->GetActorLocation().X),
            HashCombine(GetTypeHash(House->GetActorLocation().Y), GetTypeHash(House->GetActorRotation().Yaw)));
        UMaterialInterface* WallMaterial = WallMaterials[StableHash % UE_ARRAY_COUNT(WallMaterials)];
        if (!WallMaterial)
        {
            for (UMaterialInterface* Candidate : WallMaterials)
            {
                if (Candidate)
                {
                    WallMaterial = Candidate;
                    break;
                }
            }
        }

        // Preserve all shell instances and therefore every authored opening/collision face. Only their surface changes.
        ApplyIfAvailable(FindISM(House, TEXT("Shell")), WallMaterial);
        ApplyIfAvailable(FindISM(House, TEXT("YardFences")), FenceMaterial);
        ApplyIfAvailable(FindISM(House, TEXT("YardPaths")), YardMaterial);

        if (UTextRenderComponent* Label = FindObjectFast<UTextRenderComponent>(House, TEXT("DebugLabel")))
        {
            Label->SetVisibility(false);
        }

        if (RoofMesh)
        {
            AddRoof(House, RoofMesh, RoofMaterial);
            ++RoofCount;
        }

        StyledHouses.Add(HouseKey);
        ++NewlyStyledHouseCount;
    }

    for (auto It = StyledHouses.CreateIterator(); It; ++It)
    {
        if (!It->IsValid()) It.RemoveCurrent();
    }

    ++ArtRetryPass;
    if (ArtRetryPass >= MaxArtRetryPasses)
    {
        World.GetTimerManager().ClearTimer(ArtRetryTimer);
    }

    if (NewlyStyledHouseCount > 0)
    {
        UE_LOG(LogTemp, Display,
            TEXT("R13.4 enterable-house art: newly styled=%d roofs=%d pass=%d/%d; authored doors/windows/interiors preserved."),
            NewlyStyledHouseCount, RoofCount, ArtRetryPass, MaxArtRetryPasses);
    }
}
