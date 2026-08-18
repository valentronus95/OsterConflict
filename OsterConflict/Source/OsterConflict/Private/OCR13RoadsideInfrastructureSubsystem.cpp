#include "OCR13RoadsideInfrastructureSubsystem.h"

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
    constexpr float InfrastructureDelaySeconds = 2.15f;
    constexpr float MinPoleSpacingCm = 5200.0f;
    constexpr float MaxPoleSpacingCm = 6400.0f;
    constexpr float PoleRoadsideOffsetCm = 430.0f;
    constexpr int32 MaxPolesPerRoad = 28;

    UStaticMesh* LoadInfrastructureMesh(const TCHAR* Path)
    {
        return LoadObject<UStaticMesh>(nullptr, Path);
    }

    bool IsInsideKrushelnytskaSlice(const FVector& Location)
    {
        // The dedicated R12 Krushelnytska slice remains photo-driven and owns its own streetscape.
        return FMath::Abs(Location.X + 3400.0f) < 7000.0f &&
            Location.Y > -14500.0f && Location.Y < 17500.0f;
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

    UInstancedStaticMeshComponent* MakeInfrastructureISM(AActor* Owner, USceneComponent* Root,
        UStaticMesh* Mesh, const FName Name, const bool bCastShadow, const int32 CullEndCm)
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

    float StableFraction(const int32 RoadIndex, const int32 Salt)
    {
        uint32 Value = static_cast<uint32>((RoadIndex + 17) * 2654435761u) ^
            static_cast<uint32>((Salt + 31) * 2246822519u);
        Value ^= Value >> 15;
        return static_cast<float>(Value & 0x0000ffffu) / 65535.0f;
    }

    void AddRoadsidePoles(UInstancedStaticMeshComponent* Roads,
        UInstancedStaticMeshComponent* Poles,
        UInstancedStaticMeshComponent* Addons,
        UInstancedStaticMeshComponent* Lights,
        int32& OutPoleCount, int32& OutAddonCount, int32& OutLightCount)
    {
        if (!Roads || !Poles) return;

        for (int32 RoadIndex = 0; RoadIndex < Roads->GetInstanceCount(); ++RoadIndex)
        {
            FTransform RoadTransform;
            if (!Roads->GetInstanceTransform(RoadIndex, RoadTransform, true)) continue;

            const FVector Scale = RoadTransform.GetScale3D().GetAbs();
            const float SizeX = FMath::Max(100.0f, Scale.X * 100.0f);
            const float SizeY = FMath::Max(100.0f, Scale.Y * 100.0f);
            const bool bLongAxisX = SizeX >= SizeY;
            const float LengthCm = bLongAxisX ? SizeX : SizeY;
            const float WidthCm = bLongAxisX ? SizeY : SizeX;

            // Skip tiny hardstands/connector pieces. This pass is meant for recognizable street runs.
            if (LengthCm < 12000.0f) continue;

            const float SpacingCm = FMath::Lerp(MinPoleSpacingCm, MaxPoleSpacingCm,
                StableFraction(RoadIndex, 101));
            int32 PoleSlots = FMath::Clamp(FMath::FloorToInt(LengthCm / SpacingCm), 2, MaxPolesPerRoad);
            if (PoleSlots <= 0) continue;

            const float UsableLengthCm = LengthCm * 0.88f;
            const float StepCm = UsableLengthCm / static_cast<float>(PoleSlots);
            const float SideSign = StableFraction(RoadIndex, 211) > 0.5f ? 1.0f : -1.0f;
            const float SideOffsetCm = WidthCm * 0.5f + PoleRoadsideOffsetCm;
            const FQuat RoadRotation = RoadTransform.GetRotation();
            const FVector Center = RoadTransform.GetLocation();

            for (int32 Slot = 0; Slot < PoleSlots; ++Slot)
            {
                const float Along = -UsableLengthCm * 0.5f +
                    (static_cast<float>(Slot) + 0.5f) * StepCm;
                FVector Local = bLongAxisX
                    ? FVector(Along, SideSign * SideOffsetCm, 0.0f)
                    : FVector(SideSign * SideOffsetCm, Along, 0.0f);
                FVector Location = Center + RoadRotation.RotateVector(Local);
                Location.Z = FMath::Max(3.0f, Center.Z + 3.0f);

                if (IsInsideKrushelnytskaSlice(Location)) continue;

                const float NaturalYaw = RoadTransform.Rotator().Yaw + (bLongAxisX ? 0.0f : 90.0f);
                const float PoleYaw = NaturalYaw + (SideSign > 0.0f ? 180.0f : 0.0f);
                const float ScaleVariation = 0.92f + StableFraction(RoadIndex * 31 + Slot, 307) * 0.16f;
                const FTransform PoleTransform(FRotator(0.0f, PoleYaw, 0.0f), Location, FVector(ScaleVariation));
                Poles->AddInstance(PoleTransform, true);
                ++OutPoleCount;

                if (Addons && ((RoadIndex + Slot) % 2 == 0))
                {
                    Addons->AddInstance(PoleTransform, true);
                    ++OutAddonCount;
                }

                // Street lamps are deliberately sparse. Location photos can later decide which runs actually have them.
                if (Lights && ((RoadIndex * 3 + Slot) % 4 == 0))
                {
                    Lights->AddInstance(PoleTransform, true);
                    ++OutLightCount;
                }
            }
        }
    }
}

bool UOCR13RoadsideInfrastructureSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13RoadsideInfrastructureSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) ApplyRoadsideInfrastructure(*World);
        }), InfrastructureDelaySeconds, false);
}

void UOCR13RoadsideInfrastructureSubsystem::ApplyRoadsideInfrastructure(UWorld& World)
{
    if (bApplied) return;

    AOCWorldSectorOster* Sector = nullptr;
    for (TActorIterator<AOCWorldSectorOster> It(&World); It; ++It)
    {
        Sector = *It;
        if (Sector) break;
    }
    if (!Sector) return;

    UInstancedStaticMeshComponent* Roads = FindISM(Sector, TEXT("Roads"));
    if (!Roads) return;

    UStaticMesh* PoleMesh = LoadInfrastructureMesh(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Power_Pole_1.Power_Pole_1"));
    UStaticMesh* AddonMesh = LoadInfrastructureMesh(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Power_Pole_Addons.Power_Pole_Addons"));
    UStaticMesh* LightMesh = LoadInfrastructureMesh(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Power_Pole_Light.Power_Pole_Light"));
    if (!PoleMesh) return;

    AActor* InfrastructureRoot = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!InfrastructureRoot) return;
    InfrastructureRoot->SetReplicates(false);
    InfrastructureRoot->SetActorEnableCollision(false);

    USceneComponent* Root = NewObject<USceneComponent>(InfrastructureRoot, TEXT("R13_RoadsideInfrastructureRoot"));
    if (!Root) return;
    InfrastructureRoot->SetRootComponent(Root);
    InfrastructureRoot->AddInstanceComponent(Root);
    Root->SetMobility(EComponentMobility::Static);
    Root->RegisterComponent();

    UInstancedStaticMeshComponent* Poles = MakeInfrastructureISM(
        InfrastructureRoot, Root, PoleMesh, TEXT("R13_UtilityPoles"), true, 90000);
    UInstancedStaticMeshComponent* Addons = MakeInfrastructureISM(
        InfrastructureRoot, Root, AddonMesh, TEXT("R13_UtilityPoleAddons"), true, 70000);
    UInstancedStaticMeshComponent* Lights = MakeInfrastructureISM(
        InfrastructureRoot, Root, LightMesh, TEXT("R13_UtilityPoleLights"), false, 60000);
    if (!Poles)
    {
        InfrastructureRoot->Destroy();
        return;
    }

    int32 PoleCount = 0;
    int32 AddonCount = 0;
    int32 LightCount = 0;
    AddRoadsidePoles(Roads, Poles, Addons, Lights, PoleCount, AddonCount, LightCount);

    if (PoleCount <= 0)
    {
        InfrastructureRoot->Destroy();
        return;
    }

    bApplied = true;
    UE_LOG(LogTemp, Display,
        TEXT("R13 roadside infrastructure: poles=%d addons=%d sparse lights=%d; roads/navigation unchanged."),
        PoleCount, AddonCount, LightCount);
}
