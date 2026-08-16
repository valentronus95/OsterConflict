#include "OCBreakableWindow.h"
#include "OCWorldAudioComponent.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AOCBreakableWindow::AOCBreakableWindow()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetReplicateMovement(false);
    SetNetUpdateFrequency(2.0f);
    SetMinNetUpdateFrequency(0.5f);
    SetNetCullDistanceSquared(FMath::Square(20000.0f));
    WorldAudioComponent = CreateDefaultSubobject<UOCWorldAudioComponent>(TEXT("WorldAudioComponent"));

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    GlassPane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GlassPane"));
    GlassPane->SetupAttachment(SceneRoot);
    GlassPane->SetCollisionProfileName(TEXT("BlockAll"));

    FrameLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrameLeft"));
    FrameLeft->SetupAttachment(SceneRoot);
    FrameRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrameRight"));
    FrameRight->SetupAttachment(SceneRoot);
    FrameTop = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrameTop"));
    FrameTop->SetupAttachment(SceneRoot);
    FrameBottom = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrameBottom"));
    FrameBottom->SetupAttachment(SceneRoot);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        GlassPane->SetStaticMesh(CubeMesh.Object);
        FrameLeft->SetStaticMesh(CubeMesh.Object);
        FrameRight->SetStaticMesh(CubeMesh.Object);
        FrameTop->SetStaticMesh(CubeMesh.Object);
        FrameBottom->SetStaticMesh(CubeMesh.Object);
    }

    constexpr float Width = 200.0f;
    constexpr float Height = 155.0f;
    constexpr float Frame = 10.0f;

    GlassPane->SetRelativeScale3D(FVector(Width / 100.0f, 2.0f / 100.0f, Height / 100.0f));

    FrameLeft->SetRelativeLocation(FVector(-Width * 0.5f - Frame * 0.5f, 0.0f, 0.0f));
    FrameLeft->SetRelativeScale3D(FVector(Frame / 100.0f, 10.0f / 100.0f, (Height + Frame * 2.0f) / 100.0f));
    FrameRight->SetRelativeLocation(FVector(Width * 0.5f + Frame * 0.5f, 0.0f, 0.0f));
    FrameRight->SetRelativeScale3D(FVector(Frame / 100.0f, 10.0f / 100.0f, (Height + Frame * 2.0f) / 100.0f));
    FrameTop->SetRelativeLocation(FVector(0.0f, 0.0f, Height * 0.5f + Frame * 0.5f));
    FrameTop->SetRelativeScale3D(FVector(Width / 100.0f, 10.0f / 100.0f, Frame / 100.0f));
    FrameBottom->SetRelativeLocation(FVector(0.0f, 0.0f, -Height * 0.5f - Frame * 0.5f));
    FrameBottom->SetRelativeScale3D(FVector(Width / 100.0f, 10.0f / 100.0f, Frame / 100.0f));

    const FVector ShardOffsets[] =
    {
        FVector(-55.0f, 0.0f, 42.0f), FVector(5.0f, 0.0f, 50.0f), FVector(58.0f, 0.0f, 32.0f),
        FVector(-62.0f, 0.0f, -35.0f), FVector(0.0f, 0.0f, -40.0f), FVector(62.0f, 0.0f, -28.0f)
    };

    for (int32 Index = 0; Index < UE_ARRAY_COUNT(ShardOffsets); ++Index)
    {
        UStaticMeshComponent* Shard = CreateDefaultSubobject<UStaticMeshComponent>(
            *FString::Printf(TEXT("Shard_%02d"), Index));
        Shard->SetupAttachment(SceneRoot);
        Shard->SetMobility(EComponentMobility::Movable);
        if (CubeMesh.Succeeded())
        {
            Shard->SetStaticMesh(CubeMesh.Object);
        }
        Shard->SetRelativeLocation(ShardOffsets[Index]);
        Shard->SetRelativeScale3D(FVector(0.48f, 0.015f, 0.48f));
        Shard->SetHiddenInGame(true);
        Shard->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
        Shard->SetCollisionResponseToAllChannels(ECR_Ignore);
        Shard->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
        Shard->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
        Shard->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
        Shard->SetSimulatePhysics(false);
        DebrisPieces.Add(Shard);
    }
}

float AOCBreakableWindow::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    const float Applied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    if (HasAuthority() && !bBroken && DamageAmount >= BreakDamageThreshold)
    {
        const FVector Direction = DamageCauser ? DamageCauser->GetActorForwardVector() : FVector::ForwardVector;
        BreakServer(Direction);
    }
    return FMath::Max(Applied, DamageAmount);
}

void AOCBreakableWindow::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AOCBreakableWindow, bBroken);
}

void AOCBreakableWindow::BreakServer(const FVector& ImpulseDirection)
{
    if (!HasAuthority() || bBroken)
    {
        return;
    }

    bBroken = true;
    ApplyBrokenPresentation();
    ForceNetUpdate();
    MulticastBreakFX(ImpulseDirection.GetSafeNormal());
    if (WorldAudioComponent) WorldAudioComponent->PlayEventServer(EOCWorldAudioEvent::WindowBreak, GetActorLocation());
}

void AOCBreakableWindow::OnRep_Broken()
{
    if (bBroken) ApplyBrokenPresentation();
    else ApplyIntactPresentation();
}

void AOCBreakableWindow::ApplyBrokenPresentation()
{
    if (!bBroken || !GlassPane)
    {
        return;
    }

    GlassPane->SetHiddenInGame(true);
    GlassPane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AOCBreakableWindow::ApplyIntactPresentation()
{
    HideDebris();
    if (GlassPane)
    {
        GlassPane->SetHiddenInGame(false);
        GlassPane->SetCollisionProfileName(TEXT("BlockAll"));
        GlassPane->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
}

void AOCBreakableWindow::ResetServer()
{
    if (!HasAuthority()) return;
    bBroken = false;
    ApplyIntactPresentation();
    ForceNetUpdate();
}

void AOCBreakableWindow::MulticastBreakFX_Implementation(FVector_NetQuantizeNormal ImpulseDirection)
{
    if (GlassPane)
    {
        GlassPane->SetHiddenInGame(true);
        GlassPane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // Cosmetic fragments are client-side only; gameplay collision is the replicated bBroken state.
    if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer)
    {
        return;
    }

    const FVector BaseImpulse = FVector(ImpulseDirection) * 180.0f + FVector(0.0f, 0.0f, 110.0f);
    for (int32 Index = 0; Index < DebrisPieces.Num(); ++Index)
    {
        UStaticMeshComponent* Shard = DebrisPieces[Index];
        if (!Shard)
        {
            continue;
        }

        Shard->SetHiddenInGame(false);
        Shard->SetSimulatePhysics(true);
        const FVector Jitter(
            static_cast<float>((Index % 3) - 1) * 45.0f,
            static_cast<float>((Index % 2) == 0 ? 35 : -35),
            static_cast<float>(Index % 3) * 25.0f);
        Shard->AddImpulse(BaseImpulse + Jitter, NAME_None, true);
    }

    GetWorldTimerManager().ClearTimer(HideDebrisTimerHandle);
    GetWorldTimerManager().SetTimer(HideDebrisTimerHandle, this, &AOCBreakableWindow::HideDebris,
        DebrisLifetime, false);
}

void AOCBreakableWindow::HideDebris()
{
    for (const TObjectPtr<UStaticMeshComponent>& ShardPtr : DebrisPieces)
    {
        UStaticMeshComponent* Shard = ShardPtr.Get();
        if (!Shard)
        {
            continue;
        }
        Shard->SetSimulatePhysics(false);
        Shard->SetHiddenInGame(true);
    }
}
