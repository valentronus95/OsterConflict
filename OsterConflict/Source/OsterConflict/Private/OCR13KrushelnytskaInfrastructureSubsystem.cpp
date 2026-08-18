#include "OCR13KrushelnytskaInfrastructureSubsystem.h"

#include "OCGameMode.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
    constexpr float SliceDelaySeconds = 1.30f;
    constexpr float StreetCenterX = -3400.0f;
    constexpr float StreetStartY = -12000.0f;
    constexpr float StreetEndY = 15000.0f;
    constexpr float PoleSpacingCm = 5900.0f;
    constexpr float RoadsideOffsetCm = 2150.0f;

    UInstancedStaticMeshComponent* MakeVisualISM(AActor* Owner, USceneComponent* Root,
        UStaticMesh* Mesh, const FName Name, const bool bCastShadow)
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
        Component->SetCullDistances(0, 65000);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void AddGroundedPole(UInstancedStaticMeshComponent* Target, FVector Location,
        const float DesiredHeightCm, const float YawDegrees)
    {
        if (!Target || !Target->GetStaticMesh()) return;
        const FBoxSphereBounds Bounds = Target->GetStaticMesh()->GetBounds();
        const FVector Size = Bounds.BoxExtent * 2.0f;
        if (Size.Z <= 1.0f) return;
        const float Scale = FMath::Clamp(DesiredHeightCm / Size.Z, 0.25f, 3.5f);
        const float LocalBottom = Bounds.Origin.Z - Bounds.BoxExtent.Z;
        Location.Z = -LocalBottom * Scale;
        Target->AddInstance(FTransform(FRotator(0.0f, YawDegrees, 0.0f), Location, FVector(Scale)), true);
    }
}

bool UOCR13KrushelnytskaInfrastructureSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13KrushelnytskaInfrastructureSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) ApplyKrushelnytskaInfrastructure(*World);
        }), SliceDelaySeconds, false);
}

void UOCR13KrushelnytskaInfrastructureSubsystem::ApplyKrushelnytskaInfrastructure(UWorld& World)
{
    if (bApplied) return;

    UStaticMesh* PoleMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Power_Pole_1.Power_Pole_1"));
    UStaticMesh* AddonMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Power_Pole_Addons.Power_Pole_Addons"));
    UStaticMesh* LightMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Power_Pole_Light.Power_Pole_Light"));
    if (!PoleMesh) return;

    AActor* ArtRoot = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!ArtRoot) return;
    ArtRoot->SetReplicates(false);
    ArtRoot->SetActorEnableCollision(false);

    USceneComponent* Root = NewObject<USceneComponent>(ArtRoot, TEXT("R13_KrushelnytskaInfrastructureRoot"));
    if (!Root)
    {
        ArtRoot->Destroy();
        return;
    }
    Root->SetMobility(EComponentMobility::Static);
    ArtRoot->SetRootComponent(Root);
    ArtRoot->AddInstanceComponent(Root);
    Root->RegisterComponent();

    UInstancedStaticMeshComponent* Poles = MakeVisualISM(
        ArtRoot, Root, PoleMesh, TEXT("R13_KrushelnytskaUtilityPoles"), true);
    UInstancedStaticMeshComponent* Addons = MakeVisualISM(
        ArtRoot, Root, AddonMesh, TEXT("R13_KrushelnytskaPoleAddons"), true);
    UInstancedStaticMeshComponent* Lights = MakeVisualISM(
        ArtRoot, Root, LightMesh, TEXT("R13_KrushelnytskaPoleLights"), false);
    if (!Poles)
    {
        ArtRoot->Destroy();
        return;
    }

    int32 PoleCount = 0;
    int32 AddonCount = 0;
    int32 LightCount = 0;
    int32 Slot = 0;
    for (float Y = StreetStartY + 1200.0f; Y <= StreetEndY; Y += PoleSpacingCm, ++Slot)
    {
        const float SideSign = (Slot % 2 == 0) ? 1.0f : -1.0f;
        const FVector Location(StreetCenterX + SideSign * RoadsideOffsetCm, Y, 0.0f);
        const float Yaw = SideSign > 0.0f ? 180.0f : 0.0f;
        const float Height = 760.0f + 22.0f * static_cast<float>(Slot % 4);

        AddGroundedPole(Poles, Location, Height, Yaw);
        ++PoleCount;
        if (Addons)
        {
            AddGroundedPole(Addons, Location, Height, Yaw);
            ++AddonCount;
        }
        if (Lights && (Slot % 3) == 1)
        {
            AddGroundedPole(Lights, Location, Height, Yaw);
            ++LightCount;
        }
    }

    bApplied = PoleCount > 0;
    UE_LOG(LogTemp, Display,
        TEXT("R13.4 Krushelnytska infrastructure: real utility poles=%d addons=%d sparse lights=%d; fantasy R12 streetlights remain suppressed."),
        PoleCount, AddonCount, LightCount);
}
