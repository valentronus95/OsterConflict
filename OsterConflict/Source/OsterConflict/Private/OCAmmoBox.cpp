#include "OCAmmoBox.h"

#include "OCCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AOCAmmoBox::AOCAmmoBox()
{
    bReplicates = true;
    SetReplicateMovement(false);

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    SetRootComponent(Mesh);
    Mesh->SetRelativeScale3D(FVector(0.38f, 0.26f, 0.18f));
    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        Mesh->SetStaticMesh(CubeMesh.Object);
    }
}

bool AOCAmmoBox::TryGiveAmmoServer(AOCCharacter* Character)
{
    if (!HasAuthority() || !Character)
    {
        return false;
    }

    const int32 Granted = Character->AddAmmoFromBoxServer(AmmoType, AmmoAmount);
    if (Granted > 0)
    {
        Destroy();
        return true;
    }
    return false;
}

FString AOCAmmoBox::GetPromptText() const
{
    return FString::Printf(TEXT("E  AMMO BOX  +%d"), AmmoAmount);
}
