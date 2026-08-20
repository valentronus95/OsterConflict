#include "OCR13FoliageDiversitySubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
    constexpr float FoliageDiversityDelaySeconds = 1.75f;
    constexpr int32 MaxWetlandReedsPerZone = 180;

    UStaticMesh* LoadFoliageMesh(const TCHAR* Path)
    {
        return LoadObject<UStaticMesh>(nullptr, Path);
    }

    UInstancedStaticMeshComponent* FindISM(AActor* Actor, const FName Name)
    {
        if (!Actor) return nullptr;
        TArray<UInstancedStaticMeshComponent*> Components;
        Actor->GetComponents<UInstancedStaticMeshComponent>(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (Component && Component->GetFName() == Name) return Component;
        }
        return nullptr;
    }

    UInstancedStaticMeshComponent* MakeFoliageISM(AActor* Owner, USceneComponent* Root, UStaticMesh* Mesh,
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

    uint32 StableFoliageSeed(const FVector& Location, const int32 Index, const int32 Salt)
    {
        uint32 Hash = 2166136261u;
        Hash = (Hash ^ static_cast<uint32>(FMath::RoundToInt(Location.X * 0.01f))) * 16777619u;
        Hash = (Hash ^ static_cast<uint32>(FMath::RoundToInt(Location.Y * 0.01f))) * 16777619u;
        Hash = (Hash ^ static_cast<uint32>(Index + 1)) * 16777619u;
        Hash = (Hash ^ static_cast<uint32>(Salt + 97)) * 16777619u;
        return Hash;
    }

    float SeedUnit(const uint32 Seed)
    {
        return static_cast<float>(Seed & 0x00ffffffu) / static_cast<float>(0x00ffffffu);
    }

    bool IsInsideKrushelnytskaSlice(const FVector& Location)
    {
        return FMath::Abs(Location.X + 3400.0f) < 7000.0f &&
            Location.Y > -14500.0f && Location.Y < 17500.0f;
    }

    void AddCompanionPines(UInstancedStaticMeshComponent* Source,
        const TArray<UInstancedStaticMeshComponent*>& PineTargets, int32& OutCount)
    {
        if (!Source || PineTargets.IsEmpty()) return;

        for (int32 Index = 0; Index < Source->GetInstanceCount(); ++Index)
        {
            FTransform SourceTransform;
            if (!Source->GetInstanceTransform(Index, SourceTransform, true)) continue;
            const FVector SourceLocation = SourceTransform.GetLocation();
            if (IsInsideKrushelnytskaSlice(SourceLocation)) continue;

            const uint32 Seed = StableFoliageSeed(SourceLocation, Index, 1201);
            const int32 CompanionCount = (Seed % 3u == 0u) ? 2 : 1;
            for (int32 Companion = 0; Companion < CompanionCount; ++Companion)
            {
                const float BaseAngle = static_cast<float>((Seed >> (Companion * 5)) % 360u);
                const float Angle = FMath::DegreesToRadians(BaseAngle + Companion * 137.0f);
                const float Radius = 520.0f + SeedUnit(Seed ^ (0x6d2b79f5u + Companion * 131u)) * 420.0f;
                FVector Location = SourceLocation + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.0f);
                Location.Z = FMath::Max(0.0f, SourceLocation.Z);

                UInstancedStaticMeshComponent* Target = PineTargets[(Seed + Companion) % PineTargets.Num()];
                if (!Target) continue;
                const float Scale = 0.62f + SeedUnit(Seed ^ (0x9e3779b9u + Companion * 17u)) * 0.30f;
                Target->AddInstance(FTransform(
                    FRotator(0.0f, static_cast<float>((Seed >> 7) % 360u) + Companion * 41.0f, 0.0f),
                    Location, FVector(Scale)), true);
                ++OutCount;
            }
        }
    }

    void AddShrubCompanions(UInstancedStaticMeshComponent* Source,
        const TArray<UInstancedStaticMeshComponent*>& ShrubTargets, const int32 Salt, int32& OutCount)
    {
        if (!Source || ShrubTargets.IsEmpty()) return;

        for (int32 Index = 0; Index < Source->GetInstanceCount(); ++Index)
        {
            if (Index % 4 != 0) continue;
            FTransform SourceTransform;
            if (!Source->GetInstanceTransform(Index, SourceTransform, true)) continue;
            const FVector SourceLocation = SourceTransform.GetLocation();
            if (IsInsideKrushelnytskaSlice(SourceLocation)) continue;

            const uint32 Seed = StableFoliageSeed(SourceLocation, Index, Salt);
            const float Angle = FMath::DegreesToRadians(static_cast<float>(Seed % 360u));
            const float Radius = 260.0f + SeedUnit(Seed ^ 0x85ebca6bu) * 300.0f;
            FVector Location = SourceLocation + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.0f);
            Location.Z = FMath::Max(0.0f, SourceLocation.Z);

            UInstancedStaticMeshComponent* Target = ShrubTargets[(Seed >> 11) % ShrubTargets.Num()];
            if (!Target) continue;
            const float Scale = 0.68f + SeedUnit(Seed ^ 0xc2b2ae35u) * 0.38f;
            Target->AddInstance(FTransform(
                FRotator(0.0f, static_cast<float>((Seed >> 3) % 360u), 0.0f),
                Location, FVector(Scale)), true);
            ++OutCount;
        }
    }

    void AddWetlandReeds(UInstancedStaticMeshComponent* WetlandProxy,
        const TArray<UInstancedStaticMeshComponent*>& ReedTargets, int32& OutCount)
    {
        if (!WetlandProxy || ReedTargets.IsEmpty()) return;

        for (int32 ZoneIndex = 0; ZoneIndex < WetlandProxy->GetInstanceCount(); ++ZoneIndex)
        {
            FTransform ZoneTransform;
            if (!WetlandProxy->GetInstanceTransform(ZoneIndex, ZoneTransform, true)) continue;
            const FVector Scale = ZoneTransform.GetScale3D().GetAbs();
            const float WidthCm = FMath::Max(1000.0f, Scale.X * 100.0f);
            const float DepthCm = FMath::Max(1000.0f, Scale.Y * 100.0f);

            float SpacingCm = 1350.0f;
            int32 CellsX = FMath::Max(1, FMath::CeilToInt(WidthCm / SpacingCm));
            int32 CellsY = FMath::Max(1, FMath::CeilToInt(DepthCm / SpacingCm));
            int32 Requested = CellsX * CellsY;
            if (Requested > MaxWetlandReedsPerZone)
            {
                SpacingCm *= FMath::Sqrt(static_cast<float>(Requested) /
                    static_cast<float>(MaxWetlandReedsPerZone));
                CellsX = FMath::Max(1, FMath::CeilToInt(WidthCm / SpacingCm));
                CellsY = FMath::Max(1, FMath::CeilToInt(DepthCm / SpacingCm));
            }

            const float StepX = WidthCm / static_cast<float>(CellsX);
            const float StepY = DepthCm / static_cast<float>(CellsY);
            const FVector Center = ZoneTransform.GetLocation();
            const FQuat Rotation = ZoneTransform.GetRotation();

            for (int32 X = 0; X < CellsX; ++X)
            {
                for (int32 Y = 0; Y < CellsY; ++Y)
                {
                    const int32 CellIndex = X * CellsY + Y;
                    const uint32 Seed = StableFoliageSeed(Center, CellIndex, 1601 + ZoneIndex);
                    if (Seed % 3u == 0u) continue;

                    const float JitterX = (SeedUnit(Seed ^ 0x27d4eb2fu) - 0.5f) * StepX * 0.70f;
                    const float JitterY = (SeedUnit(Seed ^ 0x165667b1u) - 0.5f) * StepY * 0.70f;
                    const FVector Local(
                        -WidthCm * 0.5f + (static_cast<float>(X) + 0.5f) * StepX + JitterX,
                        -DepthCm * 0.5f + (static_cast<float>(Y) + 0.5f) * StepY + JitterY,
                        0.0f);
                    FVector Location = Center + Rotation.RotateVector(Local);
                    Location.Z = FMath::Max(2.0f, Center.Z + 2.0f);
                    if (IsInsideKrushelnytskaSlice(Location)) continue;

                    UInstancedStaticMeshComponent* Target = ReedTargets[(Seed >> 9) % ReedTargets.Num()];
                    if (!Target) continue;
                    const float ReedScale = 0.78f + SeedUnit(Seed ^ 0xd3a2646cu) * 0.36f;
                    Target->AddInstance(FTransform(
                        FRotator(0.0f, static_cast<float>((Seed >> 4) % 360u), 0.0f),
                        Location, FVector(ReedScale)), true);
                    ++OutCount;
                }
            }
        }
    }
}

bool UOCR13FoliageDiversitySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13FoliageDiversitySubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) ApplyFoliageDiversity(*World);
        }), FoliageDiversityDelaySeconds, false);
}

void UOCR13FoliageDiversitySubsystem::ApplyFoliageDiversity(UWorld& World)
{
    if (bApplied) return;

    AOCWorldSectorOster* Sector = nullptr;
    for (TActorIterator<AOCWorldSectorOster> It(&World); It; ++It)
    {
        Sector = *It;
        if (Sector) break;
    }
    if (!Sector) return;

    UStaticMesh* PineMeshes[] = {
        LoadFoliageMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_01.SM_Pine_Tree_01")),
        LoadFoliageMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_02.SM_Pine_Tree_02")),
        LoadFoliageMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_03.SM_Pine_Tree_03")),
        LoadFoliageMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_04.SM_Pine_Tree_04")),
        LoadFoliageMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_05.SM_Pine_Tree_05")),
    };
    UStaticMesh* ShrubMeshes[] = {
        LoadFoliageMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/Shrubs_1.Shrubs_1")),
        LoadFoliageMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/Shrubs_1_Single.Shrubs_1_Single")),
        LoadFoliageMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/Bush_1.Bush_1")),
    };
    UStaticMesh* ReedMeshes[] = {
        LoadFoliageMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/Cat_Tail.Cat_Tail")),
        LoadFoliageMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/Cat_Tail_2.Cat_Tail_2")),
    };

    bool bAnyMesh = false;
    for (UStaticMesh* Mesh : PineMeshes) bAnyMesh |= Mesh != nullptr;
    for (UStaticMesh* Mesh : ShrubMeshes) bAnyMesh |= Mesh != nullptr;
    for (UStaticMesh* Mesh : ReedMeshes) bAnyMesh |= Mesh != nullptr;
    if (!bAnyMesh) return;

    AActor* FoliageRoot = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!FoliageRoot) return;
    FoliageRoot->SetReplicates(false);
    FoliageRoot->SetActorEnableCollision(false);

    USceneComponent* Root = NewObject<USceneComponent>(FoliageRoot, TEXT("R13_FoliageDiversityRoot"));
    if (!Root) return;
    FoliageRoot->SetRootComponent(Root);
    FoliageRoot->AddInstanceComponent(Root);
    Root->RegisterComponent();

    TArray<UInstancedStaticMeshComponent*> PineTargets;
    for (int32 Index = 0; Index < 5; ++Index)
    {
        if (UInstancedStaticMeshComponent* Component = MakeFoliageISM(FoliageRoot, Root, PineMeshes[Index],
            FName(*FString::Printf(TEXT("R13_ExplicitPine%02d"), Index + 1)), true, 90000))
        {
            PineTargets.Add(Component);
        }
    }

    TArray<UInstancedStaticMeshComponent*> ShrubTargets;
    for (int32 Index = 0; Index < 3; ++Index)
    {
        if (UInstancedStaticMeshComponent* Component = MakeFoliageISM(FoliageRoot, Root, ShrubMeshes[Index],
            FName(*FString::Printf(TEXT("R13_Shrub%02d"), Index + 1)), false, 38000))
        {
            ShrubTargets.Add(Component);
        }
    }

    TArray<UInstancedStaticMeshComponent*> ReedTargets;
    for (int32 Index = 0; Index < 2; ++Index)
    {
        if (UInstancedStaticMeshComponent* Component = MakeFoliageISM(FoliageRoot, Root, ReedMeshes[Index],
            FName(*FString::Printf(TEXT("R13_WetlandReed%02d"), Index + 1)), false, 26000))
        {
            ReedTargets.Add(Component);
        }
    }

    int32 PineCount = 0;
    int32 ShrubCount = 0;
    int32 ReedCount = 0;
    AddCompanionPines(FindISM(Sector, TEXT("PineTrunks")), PineTargets, PineCount);
    AddShrubCompanions(FindISM(Sector, TEXT("TreeTrunks")), ShrubTargets, 2001, ShrubCount);
    AddShrubCompanions(FindISM(Sector, TEXT("SovietPoplarTrunks")), ShrubTargets, 2101, ShrubCount);
    AddShrubCompanions(FindISM(Sector, TEXT("BirchTrunks")), ShrubTargets, 2201, ShrubCount);
    AddWetlandReeds(FindISM(Sector, TEXT("GrassWetland")), ReedTargets, ReedCount);

    bApplied = true;
    UE_LOG(LogTemp, Display,
        TEXT("R13 foliage diversity: companion pines=%d shrubs=%d wetland reeds=%d."),
        PineCount, ShrubCount, ReedCount);
}
