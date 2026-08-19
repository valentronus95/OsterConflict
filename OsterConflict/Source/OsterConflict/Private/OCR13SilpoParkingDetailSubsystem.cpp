#include "OCR13SilpoParkingDetailSubsystem.h"

#include "OCGameMode.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
    constexpr float SilpoParkingDetailDelaySeconds = 6.60f;
    constexpr float ParkingSurfaceZ = 10.0f;

    AActor* FindSilpoModel(UWorld& World)
    {
        for (TActorIterator<AActor> It(&World); It; ++It)
        {
            AActor* Actor = *It;
            if (Actor && Actor->ActorHasTag(TEXT("R13_SilpoPhotoModel"))) return Actor;
        }
        return nullptr;
    }

    UInstancedStaticMeshComponent* MakeParkedCarISM(AActor* Owner, USceneComponent* Root,
        UStaticMesh* Mesh, const FName Name)
    {
        if (!Owner || !Root || !Mesh) return nullptr;
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionProfileName(TEXT("NoCollision"));
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        Component->SetCastShadow(true);
        Component->SetCullDistances(0, 80000);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void AddParkedCar(UInstancedStaticMeshComponent* Component, UStaticMesh* Mesh,
        const float X, const float Y, const float Yaw, const float Scale)
    {
        if (!Component || !Mesh) return;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const float MeshBottom = (Bounds.Origin.Z - Bounds.BoxExtent.Z) * Scale;
        const float GroundedZ = ParkingSurfaceZ - MeshBottom;
        const FVector CenterOffset(-Bounds.Origin.X * Scale, -Bounds.Origin.Y * Scale, GroundedZ);
        const FRotator Rotation(0.0f, Yaw, 0.0f);
        const FVector RotatedOffset = Rotation.RotateVector(CenterOffset);
        Component->AddInstance(FTransform(
            Rotation,
            FVector(X, Y, 0.0f) + RotatedOffset,
            FVector(Scale)), false);
    }
}

bool UOCR13SilpoParkingDetailSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13SilpoParkingDetailSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) ApplyParkingDetails(*World);
        }), SilpoParkingDetailDelaySeconds, false);
}

void UOCR13SilpoParkingDetailSubsystem::ApplyParkingDetails(UWorld& World)
{
    AActor* Model = FindSilpoModel(World);
    if (!Model || Model->ActorHasTag(TEXT("R13_SilpoParkingDetailApplied"))) return;

    USceneComponent* Root = Model->GetRootComponent();
    if (!Root) return;

    UStaticMesh* Hatchback = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/VehicleVarietyPack/Meshes/SM_Hatchback.SM_Hatchback"));
    UStaticMesh* Sedan = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/VehicleVarietyPack/Meshes/SM_SportsCar.SM_SportsCar"));
    UStaticMesh* SUV = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/VehicleVarietyPack/Meshes/SM_SUV.SM_SUV"));
    UStaticMesh* Pickup = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/VehicleVarietyPack/Meshes/SM_Pickup.SM_Pickup"));

    int32 LoadedCount = 0;
    for (UStaticMesh* Mesh : { Hatchback, Sedan, SUV, Pickup })
    {
        if (Mesh) ++LoadedCount;
    }
    if (LoadedCount == 0)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("R13 Silpo parking detail: VehicleVarietyPack payload unavailable; parking remains unobstructed."));
        return;
    }

    UInstancedStaticMeshComponent* HatchbackCars = MakeParkedCarISM(Model, Root, Hatchback,
        TEXT("R13SilpoParking_Hatchbacks"));
    UInstancedStaticMeshComponent* SedanCars = MakeParkedCarISM(Model, Root, Sedan,
        TEXT("R13SilpoParking_Sedans"));
    UInstancedStaticMeshComponent* SUVCars = MakeParkedCarISM(Model, Root, SUV,
        TEXT("R13SilpoParking_SUVs"));
    UInstancedStaticMeshComponent* PickupCars = MakeParkedCarISM(Model, Root, Pickup,
        TEXT("R13SilpoParking_Pickups"));

    // Photo references consistently show a dense single row of perpendicular civilian cars in front of the store.
    // Keep the entrance side and the through-road clear, and vary angle/scale slightly to avoid cloned-car rhythm.
    AddParkedCar(HatchbackCars, Hatchback, -880.0f, -1975.0f, 90.0f, 0.94f);
    AddParkedCar(SedanCars, Sedan, -470.0f, -1990.0f, 89.0f, 0.91f);
    AddParkedCar(SUVCars, SUV, -55.0f, -1985.0f, 91.5f, 0.92f);
    AddParkedCar(HatchbackCars, Hatchback, 365.0f, -1980.0f, 88.5f, 0.98f);
    AddParkedCar(SedanCars, Sedan, 790.0f, -1995.0f, 92.0f, 0.95f);
    AddParkedCar(SUVCars, SUV, 1215.0f, -1980.0f, 90.0f, 0.94f);

    // One additional car sits offset on the right edge in several supplied angles, without blocking the road lane.
    AddParkedCar(PickupCars, Pickup, 1490.0f, -2360.0f, 88.0f, 0.90f);

    Model->Tags.Add(TEXT("R13_SilpoParkingDetailApplied"));
    UE_LOG(LogTemp, Display,
        TEXT("R13 Silpo parking detail: static civilian row applied with %d/4 vehicle meshes available; collision/nav disabled."),
        LoadedCount);
}
