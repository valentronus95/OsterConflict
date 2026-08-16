#include "OCCapturePoint.h"

#include "OCCharacter.h"
#include "OCGameMode.h"
#include "OCGameState.h"
#include "OCPlayerState.h"
#include "OCHealthComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

AOCCapturePoint::AOCCapturePoint()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.10f;
    bReplicates = true;
    SetReplicateMovement(false);
    SetNetUpdateFrequency(10.0f);
    SetMinNetUpdateFrequency(2.0f);

    CaptureSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CaptureSphere"));
    SetRootComponent(CaptureSphere);
    CaptureSphere->SetSphereRadius(CaptureRadius);
    CaptureSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CaptureSphere->SetGenerateOverlapEvents(true);
    CaptureSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    CaptureSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    MarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MarkerMesh"));
    MarkerMesh->SetupAttachment(CaptureSphere);
    MarkerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MarkerMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));
    MarkerMesh->SetRelativeScale3D(FVector(0.35f, 0.35f, 2.5f));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (CylinderMesh.Succeeded())
    {
        MarkerMesh->SetStaticMesh(CylinderMesh.Object);
    }
}

void AOCCapturePoint::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AOCCapturePoint, PointId);
    DOREPLIFETIME(AOCCapturePoint, CaptureProgress);
    DOREPLIFETIME(AOCCapturePoint, OwnerTeam);
    DOREPLIFETIME(AOCCapturePoint, bContested);
}

void AOCCapturePoint::ConfigureServer(FName InPointId, float InCaptureRadius, float InCaptureSeconds)
{
    if (!HasAuthority())
    {
        return;
    }

    PointId = InPointId;
    CaptureRadius = FMath::Max(100.0f, InCaptureRadius);
    CaptureSeconds = FMath::Max(1.0f, InCaptureSeconds);
    CaptureSphere->SetSphereRadius(CaptureRadius);
    ForceNetUpdate();
}


void AOCCapturePoint::ResetPointServer()
{
    if (!HasAuthority())
    {
        return;
    }

    CaptureProgress = 0.0f;
    OwnerTeam = EOCTeam::None;
    bContested = false;
    ForceNetUpdate();
}

void AOCCapturePoint::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (HasAuthority())
    {
        UpdateCaptureServer(DeltaSeconds);
    }
}

void AOCCapturePoint::UpdateCaptureServer(float DeltaSeconds)
{
    const AOCGameState* MatchState = GetWorld() ? GetWorld()->GetGameState<AOCGameState>() : nullptr;
    if (MatchState && MatchState->GetOCMatchPhase() != EOCMatchPhase::InProgress)
    {
        if (bContested)
        {
            bContested = false;
            ForceNetUpdate();
        }
        return;
    }

    TArray<AActor*> OverlappingActors;
    CaptureSphere->GetOverlappingActors(OverlappingActors, AOCCharacter::StaticClass());

    int32 TeamOneCount = 0;
    int32 TeamTwoCount = 0;
    for (AActor* Actor : OverlappingActors)
    {
        const AOCCharacter* Character = Cast<AOCCharacter>(Actor);
        if (!Character || !Character->GetHealthComponent() || !Character->GetHealthComponent()->IsAlive())
        {
            continue;
        }

        const AOCPlayerState* State = Character->GetPlayerState<AOCPlayerState>();
        if (!State)
        {
            continue;
        }

        if (State->GetTeamId() == EOCTeam::TeamOne)
        {
            ++TeamOneCount;
        }
        else if (State->GetTeamId() == EOCTeam::TeamTwo)
        {
            ++TeamTwoCount;
        }
    }

    const bool bWasContested = bContested;
    bContested = TeamOneCount > 0 && TeamTwoCount > 0;
    if (bContested || (TeamOneCount == 0 && TeamTwoCount == 0))
    {
        if (bWasContested != bContested)
        {
            ForceNetUpdate();
        }
        return;
    }

    const int32 Advantage = FMath::Abs(TeamOneCount - TeamTwoCount);
    const float Direction = TeamOneCount > TeamTwoCount ? 1.0f : -1.0f;
    const float SpeedMultiplier = FMath::Clamp(static_cast<float>(Advantage), 1.0f, 3.0f);
    const float OldProgress = CaptureProgress;
    CaptureProgress = FMath::Clamp(CaptureProgress + Direction * (DeltaSeconds / CaptureSeconds) * SpeedMultiplier, -1.0f, 1.0f);

    if (OwnerTeam == EOCTeam::TeamOne && CaptureProgress <= 0.0f)
    {
        SetOwnerServer(EOCTeam::None);
    }
    else if (OwnerTeam == EOCTeam::TeamTwo && CaptureProgress >= 0.0f)
    {
        SetOwnerServer(EOCTeam::None);
    }

    if (CaptureProgress >= 1.0f - KINDA_SMALL_NUMBER)
    {
        CaptureProgress = 1.0f;
        SetOwnerServer(EOCTeam::TeamOne);
    }
    else if (CaptureProgress <= -1.0f + KINDA_SMALL_NUMBER)
    {
        CaptureProgress = -1.0f;
        SetOwnerServer(EOCTeam::TeamTwo);
    }

    if (!FMath::IsNearlyEqual(OldProgress, CaptureProgress, 0.001f) || bWasContested != bContested)
    {
        ForceNetUpdate();
    }
}

void AOCCapturePoint::SetOwnerServer(EOCTeam NewOwner)
{
    if (!HasAuthority() || OwnerTeam == NewOwner)
    {
        return;
    }

    const EOCTeam PreviousOwner = OwnerTeam;
    OwnerTeam = NewOwner;
    ForceNetUpdate();

    if (AOCGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AOCGameMode>() : nullptr)
    {
        GameMode->HandleCapturePointOwnerChanged(this, PreviousOwner, NewOwner);
    }
}
