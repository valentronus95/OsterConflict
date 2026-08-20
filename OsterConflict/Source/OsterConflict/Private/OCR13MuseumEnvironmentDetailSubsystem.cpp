#include "OCR13MuseumEnvironmentDetailSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"

namespace
{
    constexpr float EnvironmentDelaySeconds = 5.38f;
    const FName EnvironmentTag(TEXT("R13_MuseumEnvironmentDetail"));

    UMaterialInstanceDynamic* MakeColor(AActor* Owner, UMaterialInterface* Base,
        const FName Name, const FLinearColor& Color)
    {
        if (!Owner || !Base) return nullptr;
        UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Base, Owner, Name);
        if (Material) Material->SetVectorParameterValue(TEXT("Color"), Color);
        return Material;
    }

    UInstancedStaticMeshComponent* MakeISM(AActor* Owner, USceneComponent* Root, UStaticMesh* Mesh,
        UMaterialInterface* Material, const FName Name, const bool bCollision,
        const bool bShadow = true, const int32 CullEndCm = 100000)
    {
        if (!Owner || !Root || !Mesh) return nullptr;
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        if (Material)
        {
            const int32 Slots = FMath::Max(1, Mesh->GetStaticMaterials().Num());
            for (int32 Slot = 0; Slot < Slots; ++Slot) Component->SetMaterial(Slot, Material);
        }
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionProfileName(FName(bCollision ? TEXT("BlockAll") : TEXT("NoCollision")));
        Component->SetCollisionEnabled(bCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(bCollision);
        Component->SetCastShadow(bShadow);
        if (CullEndCm > 0) Component->SetCullDistances(0, CullEndCm);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void AddBox(UInstancedStaticMeshComponent* Component, const FVector& Center,
        const FVector& SizeCm, const FRotator& Rotation = FRotator::ZeroRotator)
    {
        if (!Component) return;
        Component->AddInstance(FTransform(Rotation, Center, SizeCm / 100.0f), true);
    }

    void AddGroundedMesh(UInstancedStaticMeshComponent* Component, UStaticMesh* Mesh,
        const FVector& GroundLocation, const float DesiredHeightCm, const FRotator& Rotation,
        const float UniformScaleBias = 1.0f)
    {
        if (!Component || !Mesh) return;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        if (NativeSize.Z <= 1.0f) return;

        const float Scale = FMath::Clamp((DesiredHeightCm / NativeSize.Z) * UniformScaleBias, 0.15f, 6.0f);
        const FVector LocalBottom(Bounds.Origin.X, Bounds.Origin.Y, Bounds.Origin.Z - Bounds.BoxExtent.Z);
        const FVector RotatedBottom = Rotation.RotateVector(LocalBottom * Scale);
        const FVector Location = GroundLocation - RotatedBottom;
        Component->AddInstance(FTransform(Rotation, Location, FVector(Scale)), true);
    }

    void SuppressUnsupportedMuseumConiferRows(UWorld& World)
    {
        int32 HiddenComponents = 0;
        for (TActorIterator<AActor> It(&World); It; ++It)
        {
            AActor* Actor = *It;
            if (!Actor) continue;

            TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
            Actor->GetComponents(Components);
            for (UInstancedStaticMeshComponent* Component : Components)
            {
                if (!Component) continue;
                const FName Name = Component->GetFName();
                if (Name != TEXT("R137Museum_Pine01") && Name != TEXT("R137Museum_Pine03")) continue;

                Component->SetVisibility(false, true);
                Component->SetHiddenInGame(true, true);
                Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                Component->SetCanEverAffectNavigation(false);
                ++HiddenComponents;
            }
        }

        UE_LOG(LogTemp, Display,
            TEXT("R13 museum environment: suppressed unsupported symmetric conifer-row components=%d."),
            HiddenComponents);
    }
}

bool UOCR13MuseumEnvironmentDetailSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13MuseumEnvironmentDetailSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) BuildMuseumEnvironment(*World);
        }), EnvironmentDelaySeconds, false);
}

void UOCR13MuseumEnvironmentDetailSubsystem::BuildMuseumEnvironment(UWorld& World)
{
    // The older museum pass planted eight conifers in an almost symmetric corridor. Available facade references do not
    // support that arrangement, so remove its visuals and collision before authoring the evidence-backed grounds.
    SuppressUnsupportedMuseumConiferRows(World);

    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Existing = *It;
        if (Existing && Existing->ActorHasTag(EnvironmentTag)) return;
    }

    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UStaticMesh* Tree01 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var01.SM_Tree_Var01"));
    UStaticMesh* Tree02 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var02.SM_Tree_Var02"));
    UStaticMesh* Tree04 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var04.SM_Tree_Var04"));
    UStaticMesh* Grass = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_GrassPatch_Var01.SM_GrassPatch_Var01"));
    UMaterialInterface* Basic = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!Cube || !Basic) return;

    FActorSpawnParameters Params;
    Params.Name = TEXT("R13_OsterMuseumEnvironmentDetail");
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AActor* Detail = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, Params);
    if (!Detail) return;
    Detail->SetReplicates(false);
    Detail->SetActorEnableCollision(true);
    Detail->Tags.Add(EnvironmentTag);

    USceneComponent* Root = NewObject<USceneComponent>(Detail, TEXT("R13_MuseumEnvironmentRoot"));
    if (!Root)
    {
        Detail->Destroy();
        return;
    }
    Root->SetMobility(EComponentMobility::Static);
    Detail->SetRootComponent(Root);
    Detail->AddInstanceComponent(Root);
    Root->RegisterComponent();

    UMaterialInstanceDynamic* LawnMat = MakeColor(Detail, Basic, TEXT("R13_MuseumLawn"),
        FLinearColor(0.10f, 0.24f, 0.07f, 1.0f));

    UInstancedStaticMeshComponent* Lawn = MakeISM(Detail, Root, Cube, LawnMat,
        TEXT("R13Museum_LawnGround"), false, false, 45000);
    UInstancedStaticMeshComponent* Trees01 = MakeISM(Detail, Root, Tree01, nullptr,
        TEXT("R13Museum_MatureTree01"), true, true, 100000);
    UInstancedStaticMeshComponent* Trees02 = MakeISM(Detail, Root, Tree02, nullptr,
        TEXT("R13Museum_MatureTree02"), true, true, 100000);
    UInstancedStaticMeshComponent* Trees04 = MakeISM(Detail, Root, Tree04, nullptr,
        TEXT("R13Museum_MatureTree04"), true, true, 100000);
    UInstancedStaticMeshComponent* GrassISM = MakeISM(Detail, Root, Grass, nullptr,
        TEXT("R13Museum_GrassTufts"), false, true, 30000);

    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();

    // Both modern public facade views show mown lawn on either side of the straight concrete-slab approach.
    AddBox(Lawn, Museum + FVector(-1040.0f, -1830.0f, -2.0f), FVector(1520.0f, 3060.0f, 4.0f));
    AddBox(Lawn, Museum + FVector(1040.0f, -1830.0f, -2.0f), FVector(1520.0f, 3060.0f, 4.0f));

    struct FTreeSeed
    {
        FVector Offset;
        float Height;
        FRotator Rotation;
        int32 Family;
    };

    // Photo 3/4 view clearly shows a mature trunk immediately left of the house leaning inward toward the raised timber
    // center. Other visible crowns frame the facade irregularly; no mirrored avenue is authored.
    const FTreeSeed TreeSeeds[] =
    {
        { FVector(-980.0f, -1040.0f, 0.0f), 1880.0f, FRotator(10.0f, 14.0f, -7.0f), 0 },
        { FVector(1320.0f, -1280.0f, 0.0f), 1900.0f, FRotator(0.0f, 71.0f, 2.0f), 1 },
        { FVector(-1680.0f, -2470.0f, 0.0f), 2080.0f, FRotator(-2.0f, 141.0f, 1.0f), 2 },
        { FVector(1760.0f, -2820.0f, 0.0f), 2140.0f, FRotator(1.0f, 219.0f, -1.0f), 0 }
    };

    UInstancedStaticMeshComponent* TreeFamilies[] = { Trees01, Trees02, Trees04 };
    UStaticMesh* TreeMeshes[] = { Tree01, Tree02, Tree04 };
    int32 MatureTreeCount = 0;
    for (const FTreeSeed& Seed : TreeSeeds)
    {
        if (Seed.Family < 0 || Seed.Family >= UE_ARRAY_COUNT(TreeFamilies)) continue;
        if (!TreeFamilies[Seed.Family] || !TreeMeshes[Seed.Family]) continue;
        AddGroundedMesh(TreeFamilies[Seed.Family], TreeMeshes[Seed.Family],
            Museum + Seed.Offset, Seed.Height, Seed.Rotation);
        ++MatureTreeCount;
    }

    // Keep the photographed lawn visually alive without placing invented park furniture or flower beds at unsupported
    // coordinates. Small grass tufts have no gameplay collision and remain away from the slab path and entrance steps.
    int32 GrassCount = 0;
    if (GrassISM && Grass)
    {
        for (int32 Side = -1; Side <= 1; Side += 2)
        {
            for (int32 Row = 0; Row < 5; ++Row)
            {
                for (int32 Col = 0; Col < 3; ++Col)
                {
                    const float X = static_cast<float>(Side) * (600.0f + static_cast<float>(Col) * 320.0f);
                    const float Y = -1120.0f - static_cast<float>(Row) * 590.0f - static_cast<float>((Col + Row) % 2) * 80.0f;
                    const FRotator Rotation(0.0f,
                        static_cast<float>((Row * 59 + Col * 83 + Side * 17 + 360) % 360), 0.0f);
                    AddGroundedMesh(GrassISM, Grass, Museum + FVector(X, Y, 0.0f),
                        32.0f + static_cast<float>((Col + Row) % 3) * 5.0f, Rotation, 0.90f);
                    ++GrassCount;
                }
            }
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("R13 museum environment evidence pass: unsupported conifer corridor suppressed; facade-reference lawn, irregular mature broadleaf framing trees=%d (including photographed leaning left tree) and non-colliding sparse grass=%d authored. Unverified bench/flower-bed/timber-figure placements removed."),
        MatureTreeCount, GrassCount);
}
