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
        const FVector& GroundLocation, const float DesiredHeightCm, const float YawDegrees,
        const float UniformScaleBias = 1.0f)
    {
        if (!Component || !Mesh) return;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        if (NativeSize.Z <= 1.0f) return;
        const float Scale = FMath::Clamp((DesiredHeightCm / NativeSize.Z) * UniformScaleBias, 0.15f, 6.0f);
        const float LocalBottom = Bounds.Origin.Z - Bounds.BoxExtent.Z;
        FVector Location = GroundLocation;
        Location.Z = -LocalBottom * Scale;
        Component->AddInstance(FTransform(FRotator(0.0f, YawDegrees, 0.0f), Location, FVector(Scale)), true);
    }

    void AddBench(UInstancedStaticMeshComponent* Wood, UInstancedStaticMeshComponent* Metal,
        const FVector& Origin, const float YawDegrees)
    {
        const FRotator Rotation(0.0f, YawDegrees, 0.0f);
        auto Rotate = [&Rotation, &Origin](const FVector& Offset)
        {
            return Origin + Rotation.RotateVector(Offset);
        };

        AddBox(Wood, Rotate(FVector(0.0f, 0.0f, 48.0f)), FVector(180.0f, 48.0f, 12.0f), Rotation);
        AddBox(Wood, Rotate(FVector(0.0f, 22.0f, 96.0f)), FVector(180.0f, 10.0f, 72.0f), Rotation);
        for (const float X : { -72.0f, 72.0f })
        {
            AddBox(Metal, Rotate(FVector(X, -10.0f, 25.0f)), FVector(10.0f, 10.0f, 50.0f), Rotation);
            AddBox(Metal, Rotate(FVector(X, 22.0f, 60.0f)), FVector(10.0f, 10.0f, 78.0f), Rotation);
        }
    }

    void AddFlowerBed(UInstancedStaticMeshComponent* Border, UInstancedStaticMeshComponent* Soil,
        UInstancedStaticMeshComponent* Flowers, UStaticMesh* FlowerMesh,
        const FVector& Center, const FVector2D& Size, const float YawDegrees)
    {
        const FRotator Rotation(0.0f, YawDegrees, 0.0f);
        AddBox(Soil, Center + FVector(0.0f, 0.0f, 5.0f), FVector(Size.X, Size.Y, 10.0f), Rotation);
        AddBox(Border, Center + Rotation.RotateVector(FVector(0.0f, Size.Y * 0.5f, 10.0f)),
            FVector(Size.X + 20.0f, 14.0f, 20.0f), Rotation);
        AddBox(Border, Center + Rotation.RotateVector(FVector(0.0f, -Size.Y * 0.5f, 10.0f)),
            FVector(Size.X + 20.0f, 14.0f, 20.0f), Rotation);
        AddBox(Border, Center + Rotation.RotateVector(FVector(Size.X * 0.5f, 0.0f, 10.0f)),
            FVector(14.0f, Size.Y, 20.0f), Rotation);
        AddBox(Border, Center + Rotation.RotateVector(FVector(-Size.X * 0.5f, 0.0f, 10.0f)),
            FVector(14.0f, Size.Y, 20.0f), Rotation);

        if (!Flowers || !FlowerMesh) return;
        for (int32 X = -2; X <= 2; ++X)
        {
            for (int32 Y = -1; Y <= 1; ++Y)
            {
                const FVector Local(
                    static_cast<float>(X) * Size.X / 6.0f,
                    static_cast<float>(Y) * Size.Y / 4.0f,
                    0.0f);
                AddGroundedMesh(Flowers, FlowerMesh, Center + Rotation.RotateVector(Local),
                    34.0f + static_cast<float>((X + Y + 6) % 3) * 4.0f,
                    static_cast<float>((X * 37 + Y * 71 + 360) % 360));
            }
        }
    }

    void AddSimpleWoodFigure(UInstancedStaticMeshComponent* Wood, const FVector& Origin,
        const float HeightCm, const float YawDegrees)
    {
        // Park articles confirm locally carved timber figures. Keep this deliberately abstract rather than inventing
        // facial/garment detail not supported by a close photographic reference.
        const FRotator Rotation(0.0f, YawDegrees, 0.0f);
        AddBox(Wood, Origin + FVector(0.0f, 0.0f, HeightCm * 0.45f),
            FVector(48.0f, 42.0f, HeightCm * 0.70f), Rotation);
        AddBox(Wood, Origin + FVector(0.0f, 0.0f, HeightCm * 0.86f),
            FVector(52.0f, 50.0f, HeightCm * 0.18f), Rotation);
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
    UStaticMesh* Flower = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Flower_Var03.SM_Flower_Var03"));
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
    UMaterialInstanceDynamic* WoodMat = MakeColor(Detail, Basic, TEXT("R13_MuseumParkWood"),
        FLinearColor(0.22f, 0.11f, 0.045f, 1.0f));
    UMaterialInstanceDynamic* MetalMat = MakeColor(Detail, Basic, TEXT("R13_MuseumParkMetal"),
        FLinearColor(0.10f, 0.11f, 0.10f, 1.0f));
    UMaterialInstanceDynamic* BorderMat = MakeColor(Detail, Basic, TEXT("R13_MuseumFlowerBorder"),
        FLinearColor(0.40f, 0.40f, 0.37f, 1.0f));
    UMaterialInstanceDynamic* SoilMat = MakeColor(Detail, Basic, TEXT("R13_MuseumFlowerSoil"),
        FLinearColor(0.12f, 0.055f, 0.025f, 1.0f));

    UInstancedStaticMeshComponent* Lawn = MakeISM(Detail, Root, Cube, LawnMat,
        TEXT("R13Museum_LawnGround"), false, false, 45000);
    UInstancedStaticMeshComponent* BenchWood = MakeISM(Detail, Root, Cube, WoodMat,
        TEXT("R13Museum_ParkBenchWood"), true, true, 45000);
    UInstancedStaticMeshComponent* BenchMetal = MakeISM(Detail, Root, Cube, MetalMat,
        TEXT("R13Museum_ParkBenchMetal"), true, true, 45000);
    UInstancedStaticMeshComponent* BedBorder = MakeISM(Detail, Root, Cube, BorderMat,
        TEXT("R13Museum_FlowerBedBorder"), true, false, 40000);
    UInstancedStaticMeshComponent* BedSoil = MakeISM(Detail, Root, Cube, SoilMat,
        TEXT("R13Museum_FlowerBedSoil"), false, false, 35000);
    UInstancedStaticMeshComponent* WoodFigures = MakeISM(Detail, Root, Cube, WoodMat,
        TEXT("R13Museum_WoodParkFigures"), true, true, 40000);
    UInstancedStaticMeshComponent* Trees01 = MakeISM(Detail, Root, Tree01, nullptr,
        TEXT("R13Museum_MatureTree01"), true, true, 100000);
    UInstancedStaticMeshComponent* Trees02 = MakeISM(Detail, Root, Tree02, nullptr,
        TEXT("R13Museum_MatureTree02"), true, true, 100000);
    UInstancedStaticMeshComponent* Trees04 = MakeISM(Detail, Root, Tree04, nullptr,
        TEXT("R13Museum_MatureTree04"), true, true, 100000);
    UInstancedStaticMeshComponent* GrassISM = MakeISM(Detail, Root, Grass, nullptr,
        TEXT("R13Museum_GrassTufts"), false, true, 30000);
    UInstancedStaticMeshComponent* Flowers = MakeISM(Detail, Root, Flower, nullptr,
        TEXT("R13Museum_ParkFlowers"), false, true, 25000);

    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();

    // Frontal photography consistently shows a clean grass forecourt split by the straight concrete approach.
    AddBox(Lawn, Museum + FVector(-1030.0f, -1800.0f, -2.0f), FVector(1500.0f, 3000.0f, 4.0f));
    AddBox(Lawn, Museum + FVector(1030.0f, -1800.0f, -2.0f), FVector(1500.0f, 3000.0f, 4.0f));

    // Mature broadleaf trees frame the house in public facade views. Do not create a symmetrical plantation.
    struct FTreeSeed { FVector Offset; float Height; float Yaw; int32 Family; };
    const FTreeSeed TreeSeeds[] =
    {
        { FVector(-1180, -1040, 0), 1750, 18, 0 },
        { FVector(1330, -1280, 0), 1850, 72, 1 },
        { FVector(-1650, -2360, 0), 2050, 143, 2 },
        { FVector(1720, -2740, 0), 2150, 215, 0 },
        { FVector(-1950, -3850, 0), 2250, 287, 1 },
        { FVector(2050, -4200, 0), 2300, 331, 2 }
    };
    UInstancedStaticMeshComponent* TreeFamilies[] = { Trees01, Trees02, Trees04 };
    UStaticMesh* TreeMeshes[] = { Tree01, Tree02, Tree04 };
    int32 MatureTreeCount = 0;
    for (const FTreeSeed& Seed : TreeSeeds)
    {
        if (Seed.Family < 0 || Seed.Family >= UE_ARRAY_COUNT(TreeFamilies)) continue;
        if (!TreeFamilies[Seed.Family] || !TreeMeshes[Seed.Family]) continue;
        AddGroundedMesh(TreeFamilies[Seed.Family], TreeMeshes[Seed.Family],
            Museum + Seed.Offset, Seed.Height, Seed.Yaw);
        ++MatureTreeCount;
    }

    // Sparse grass variation keeps the mown lawn from reading as a flat green card.
    if (GrassISM && Grass)
    {
        for (int32 Side = -1; Side <= 1; Side += 2)
        {
            for (int32 Row = 0; Row < 6; ++Row)
            {
                for (int32 Col = 0; Col < 4; ++Col)
                {
                    const float X = static_cast<float>(Side) * (520.0f + static_cast<float>(Col) * 280.0f);
                    const float Y = -1050.0f - static_cast<float>(Row) * 560.0f - static_cast<float>((Col + Row) % 2) * 95.0f;
                    AddGroundedMesh(GrassISM, Grass, Museum + FVector(X, Y, 0.0f),
                        34.0f + static_cast<float>((Col + Row) % 3) * 6.0f,
                        static_cast<float>((Row * 59 + Col * 83 + Side * 17 + 360) % 360), 0.95f);
                }
            }
        }
    }

    // Park reports from 2022 explicitly document benches and flower beds beside the museum.
    AddBench(BenchWood, BenchMetal, Museum + FVector(-1150.0f, -3000.0f, 0.0f), 8.0f);
    AddBench(BenchWood, BenchMetal, Museum + FVector(1180.0f, -3320.0f, 0.0f), 172.0f);
    AddFlowerBed(BedBorder, BedSoil, Flowers, Flower,
        Museum + FVector(-820.0f, -2450.0f, 0.0f), FVector2D(520.0f, 210.0f), 5.0f);
    AddFlowerBed(BedBorder, BedSoil, Flowers, Flower,
        Museum + FVector(840.0f, -2620.0f, 0.0f), FVector2D(520.0f, 210.0f), -7.0f);

    // Carved timber figures are documented in Solonyna park. Their exact faces/poses are intentionally not invented.
    AddSimpleWoodFigure(WoodFigures, Museum + FVector(-1420.0f, -3650.0f, 0.0f), 175.0f, 20.0f);
    AddSimpleWoodFigure(WoodFigures, Museum + FVector(1480.0f, -3920.0f, 0.0f), 155.0f, -18.0f);

    UE_LOG(LogTemp, Display,
        TEXT("R13 museum environment detail: photo/reference-driven lawn, mature broadleaf framing trees=%d, sparse grass variation, benches, flower beds and intentionally abstract documented timber park figures added around the museum forecourt; no invented signage or unsupported micro-detail."),
        MatureTreeCount);
}
