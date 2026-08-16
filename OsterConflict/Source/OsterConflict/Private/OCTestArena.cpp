#include "OCTestArena.h"

#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AOCTestArena::AOCTestArena()
{
    bReplicates = true;
    SetReplicateMovement(false);

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));

    Floor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Floor"));
    Floor->SetupAttachment(SceneRoot);
    Floor->SetRelativeLocation(FVector(0.0f, 0.0f, -50.0f));
    Floor->SetRelativeScale3D(FVector(40.0f, 40.0f, 0.50f));
    Floor->SetCollisionProfileName(TEXT("BlockAll"));

    WallNorth = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallNorth"));
    WallNorth->SetupAttachment(SceneRoot);
    WallNorth->SetRelativeLocation(FVector(0.0f, 4000.0f, 200.0f));
    WallNorth->SetRelativeScale3D(FVector(40.0f, 0.50f, 2.50f));
    WallNorth->SetCollisionProfileName(TEXT("BlockAll"));

    WallSouth = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallSouth"));
    WallSouth->SetupAttachment(SceneRoot);
    WallSouth->SetRelativeLocation(FVector(0.0f, -4000.0f, 200.0f));
    WallSouth->SetRelativeScale3D(FVector(40.0f, 0.50f, 2.50f));
    WallSouth->SetCollisionProfileName(TEXT("BlockAll"));

    WallEast = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallEast"));
    WallEast->SetupAttachment(SceneRoot);
    WallEast->SetRelativeLocation(FVector(4000.0f, 0.0f, 200.0f));
    WallEast->SetRelativeScale3D(FVector(0.50f, 40.0f, 2.50f));
    WallEast->SetCollisionProfileName(TEXT("BlockAll"));

    WallWest = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallWest"));
    WallWest->SetupAttachment(SceneRoot);
    WallWest->SetRelativeLocation(FVector(-4000.0f, 0.0f, 200.0f));
    WallWest->SetRelativeScale3D(FVector(0.50f, 40.0f, 2.50f));
    WallWest->SetCollisionProfileName(TEXT("BlockAll"));

    if (CubeMesh.Succeeded())
    {
        Floor->SetStaticMesh(CubeMesh.Object);
        WallNorth->SetStaticMesh(CubeMesh.Object);
        WallSouth->SetStaticMesh(CubeMesh.Object);
        WallEast->SetStaticMesh(CubeMesh.Object);
        WallWest->SetStaticMesh(CubeMesh.Object);
    }

    ArenaLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ArenaLight"));
    ArenaLight->SetupAttachment(SceneRoot);
    ArenaLight->SetRelativeLocation(FVector(0.0f, 0.0f, 1800.0f));
    ArenaLight->SetIntensity(250000.0f);
    ArenaLight->SetAttenuationRadius(9000.0f);
    ArenaLight->SetCastShadows(true);
}
