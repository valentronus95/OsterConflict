#include "OCAntiArmorProjectile.h"

#include "OCDamageTypes.h"
#include "OCCharacter.h"
#include "OCCombatVisualComponent.h"
#include "OCHealthComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AOCAntiArmorProjectile::AOCAntiArmorProjectile()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetReplicateMovement(true);

    Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    Collision->InitSphereRadius(5.5f);
    Collision->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    Collision->SetNotifyRigidBodyCollision(true);
    SetRootComponent(Collision);
    Collision->OnComponentHit.AddDynamic(this, &AOCAntiArmorProjectile::HandleImpact);

    ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
    ProjectileMesh->SetupAttachment(Collision);
    ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (CylinderMesh.Succeeded()) ProjectileMesh->SetStaticMesh(CylinderMesh.Object);
    ProjectileMesh->SetRelativeRotation(FRotator(0,90,0));
    ProjectileMesh->SetRelativeScale3D(FVector(0.07f,0.07f,0.30f));

    Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
    Movement->UpdatedComponent = Collision;
    Movement->InitialSpeed = 5200.0f;
    Movement->MaxSpeed = 5200.0f;
    Movement->ProjectileGravityScale = 0.12f;
    Movement->bRotationFollowsVelocity = true;
    InitialLifeSpan = 8.0f;
}

void AOCAntiArmorProjectile::HandleImpact(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, FVector, const FHitResult& Hit)
{
    if (!HasAuthority()) return;
    if (OtherActor && OtherActor != GetInstigator())
    {
        const float AppliedDamage = UGameplayStatics::ApplyPointDamage(OtherActor, DirectDamage, GetActorForwardVector(), Hit,
            GetInstigatorController(), this, UOCAntiArmorDamageType::StaticClass());
        if (AppliedDamage > 0.0f)
        {
            if (AOCCharacter* TargetCharacter = Cast<AOCCharacter>(OtherActor))
            {
                if (UOCCombatVisualComponent* Trauma = TargetCharacter->GetCombatVisualComponent())
                {
                    const UOCHealthComponent* Health = TargetCharacter->GetHealthComponent();
                    Trauma->RecordPointTraumaServer(AppliedDamage, Hit.ImpactPoint, GetActorForwardVector(), Hit.BoneName,
                        EOCWeaponClass::Launcher, UOCAntiArmorDamageType::StaticClass(), Health && Health->IsDead());
                }
            }
        }
    }
    UGameplayStatics::ApplyRadialDamageWithFalloff(this, DirectDamage * 0.28f, 18.0f, GetActorLocation(), 80.0f,
        BlastRadius, 1.0f, UOCAntiArmorDamageType::StaticClass(), TArray<AActor*>(), this, GetInstigatorController(), ECC_Visibility);
    Destroy();
}
