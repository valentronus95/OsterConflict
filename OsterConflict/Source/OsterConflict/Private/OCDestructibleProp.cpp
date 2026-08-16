#include "OCDestructibleProp.h"
#include "OCWorldAudioComponent.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AOCDestructibleProp::AOCDestructibleProp()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetNetUpdateFrequency(2.0f);
    SetMinNetUpdateFrequency(0.5f);
    SetNetCullDistanceSquared(FMath::Square(22000.0f));
    SetReplicateMovement(false);
    WorldAudioComponent = CreateDefaultSubobject<UOCWorldAudioComponent>(TEXT("WorldAudioComponent"));

    IntactMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IntactMesh"));
    SetRootComponent(IntactMesh);
    IntactMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded()) IntactMesh->SetStaticMesh(CubeMesh.Object);
}

void AOCDestructibleProp::BeginPlay()
{
    Super::BeginPlay();
    if (HasAuthority())
    {
        CurrentDurability = MaxDurability;
        OnTakeAnyDamage.AddDynamic(this, &AOCDestructibleProp::HandleAnyDamage);
    }
}

void AOCDestructibleProp::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AOCDestructibleProp, CurrentDurability);
    DOREPLIFETIME(AOCDestructibleProp, bDestroyed);
}

void AOCDestructibleProp::ConfigureRuntime(EOCImpactSurface NewSurface, float NewDurability, const FVector& NewScale)
{
    if (!HasAuthority()) return;
    ImpactSurface = NewSurface;
    MaxDurability = FMath::Max(1.0f, NewDurability);
    CurrentDurability = MaxDurability;
    if (IntactMesh) IntactMesh->SetRelativeScale3D(NewScale);
    ForceNetUpdate();
}

void AOCDestructibleProp::HandleAnyDamage(AActor*, float Damage, const UDamageType*, AController*, AActor* DamageCauser)
{
    if (!HasAuthority() || bDestroyed || Damage <= 0.0f) return;
    CurrentDurability = FMath::Max(0.0f, CurrentDurability - Damage);
    if (CurrentDurability <= 0.0f)
    {
        BreakServer(DamageCauser ? DamageCauser->GetActorLocation() : GetActorLocation() - GetActorForwardVector() * 100.0f);
    }
    ForceNetUpdate();
}

void AOCDestructibleProp::BreakServer(const FVector& ImpulseOrigin)
{
    if (!HasAuthority() || bDestroyed) return;
    bDestroyed = true;
    if (WorldAudioComponent)
    {
        const EOCWorldAudioEvent Event = ImpactSurface == EOCImpactSurface::Metal ? EOCWorldAudioEvent::DestructionMetal
            : (ImpactSurface == EOCImpactSurface::Masonry ? EOCWorldAudioEvent::DestructionMasonry : EOCWorldAudioEvent::DestructionWood);
        WorldAudioComponent->PlayEventServer(Event, GetActorLocation());
    }
    ApplyDestroyedPresentationLocal(ImpulseOrigin);
    ForceNetUpdate();
}

void AOCDestructibleProp::OnRep_Destroyed()
{
    if (bDestroyed) ApplyDestroyedPresentationLocal(GetActorLocation() - GetActorForwardVector() * 100.0f);
    else ApplyIntactPresentationLocal();
}

void AOCDestructibleProp::ResetServer()
{
    if (!HasAuthority()) return;
    CurrentDurability = MaxDurability;
    bDestroyed = false;
    ApplyIntactPresentationLocal();
    ForceNetUpdate();
}

void AOCDestructibleProp::ApplyIntactPresentationLocal()
{
    for (const TObjectPtr<UStaticMeshComponent>& ChunkPtr : TransientChunks)
    {
        if (UStaticMeshComponent* Chunk = ChunkPtr.Get()) Chunk->DestroyComponent();
    }
    TransientChunks.Reset();
    bLocalDestroyedPresentationApplied = false;
    if (IntactMesh)
    {
        IntactMesh->SetVisibility(true, true);
        IntactMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
        IntactMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
}

void AOCDestructibleProp::ApplyDestroyedPresentationLocal(const FVector& ImpulseOrigin)
{
    if (bLocalDestroyedPresentationApplied) return;
    bLocalDestroyedPresentationApplied = true;

    if (IntactMesh)
    {
        IntactMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        IntactMesh->SetVisibility(false, true);
    }

    if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) return;

    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (!Cube || !GetWorld()) return;

    const FVector Away = (GetActorLocation() - ImpulseOrigin).GetSafeNormal();
    for (int32 Index = 0; Index < LocalChunkCount; ++Index)
    {
        UStaticMeshComponent* Chunk = NewObject<UStaticMeshComponent>(this);
        if (!Chunk) continue;
        Chunk->RegisterComponent();
        Chunk->SetStaticMesh(Cube);
        Chunk->SetCollisionProfileName(TEXT("PhysicsActor"));
        Chunk->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
        Chunk->SetWorldLocation(GetActorLocation() + FVector(
            FMath::FRandRange(-55.0f,55.0f), FMath::FRandRange(-55.0f,55.0f), FMath::FRandRange(-20.0f,70.0f)));
        Chunk->SetWorldScale3D(FVector(
            FMath::FRandRange(0.12f,0.32f), FMath::FRandRange(0.10f,0.28f), FMath::FRandRange(0.10f,0.35f)));
        Chunk->SetSimulatePhysics(true);
        Chunk->SetEnableGravity(true);
        TransientChunks.Add(Chunk);
        Chunk->AddImpulse((Away + FVector(FMath::FRandRange(-0.55f,0.55f), FMath::FRandRange(-0.55f,0.55f), 0.45f)).GetSafeNormal()
            * FMath::FRandRange(8000.0f, 18000.0f));

        TWeakObjectPtr<UStaticMeshComponent> WeakChunk = Chunk;
        FTimerHandle Cleanup;
        GetWorldTimerManager().SetTimer(Cleanup, FTimerDelegate::CreateLambda([WeakChunk]()
        {
            if (WeakChunk.IsValid()) WeakChunk->DestroyComponent();
        }), ChunkLifetime, false);
    }
}
