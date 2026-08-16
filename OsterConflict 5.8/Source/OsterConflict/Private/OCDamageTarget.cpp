#include "OCDamageTarget.h"

#include "OCHealthComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AOCDamageTarget::AOCDamageTarget()
{
    bReplicates = true;
    SetReplicateMovement(false);

    TargetMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TargetMesh"));
    SetRootComponent(TargetMesh);
    TargetMesh->SetRelativeScale3D(FVector(0.65f, 0.65f, 1.80f));
    TargetMesh->SetCollisionProfileName(TEXT("BlockAll"));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        TargetMesh->SetStaticMesh(CubeMesh.Object);
    }

    HealthComponent = CreateDefaultSubobject<UOCHealthComponent>(TEXT("HealthComponent"));
}

void AOCDamageTarget::BeginPlay()
{
    Super::BeginPlay();

    if (HealthComponent)
    {
        HealthComponent->OnDeath.AddDynamic(this, &AOCDamageTarget::HandleDeath);
    }
}

void AOCDamageTarget::HandleDeath()
{
    if (HasAuthority())
    {
        SetActorEnableCollision(false);
        SetLifeSpan(0.25f);
    }
}
