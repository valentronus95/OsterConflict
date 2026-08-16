#include "OCDeployableTrap.h"

#include "OCCharacter.h"
#include "OCGameMode.h"
#include "OCPlayerState.h"
#include "OCDamageTypes.h"
#include "OCSmokeCloud.h"
#include "OCVehicleBase.h"
#include "OCArmedVehicleBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AOCDeployableTrap::AOCDeployableTrap()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetReplicateMovement(false);
    MaxInteractionDistance = 240.0f;

    Trigger = CreateDefaultSubobject<USphereComponent>(TEXT("Trigger"));
    Trigger->InitSphereRadius(92.0f);
    Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    SetRootComponent(Trigger);
    Trigger->OnComponentBeginOverlap.AddDynamic(this, &AOCDeployableTrap::HandleOverlap);

    TrapMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TrapMesh"));
    TrapMesh->SetupAttachment(Trigger);
    TrapMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (CylinderMesh.Succeeded()) TrapMesh->SetStaticMesh(CylinderMesh.Object);
    TrapMesh->SetRelativeScale3D(FVector(0.24f,0.24f,0.05f));
}

void AOCDeployableTrap::BeginPlay()
{
    Super::BeginPlay();
    if (HasAuthority()) GetWorldTimerManager().SetTimer(ArmTimerHandle, this, &AOCDeployableTrap::ArmServer, 1.25f, false);
}

void AOCDeployableTrap::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AOCDeployableTrap, TrapPreset);
    DOREPLIFETIME(AOCDeployableTrap, OwningTeam);
    DOREPLIFETIME(AOCDeployableTrap, bArmed);
}

void AOCDeployableTrap::ConfigureTrapServer(EOCTrapPreset NewPreset, EOCTeam NewTeam)
{
    if (!HasAuthority()) return;
    TrapPreset = NewPreset;
    OwningTeam = NewTeam;
    ForceNetUpdate();
}

void AOCDeployableTrap::ArmServer()
{
    if (!HasAuthority()) return;
    bArmed = true;
    ForceNetUpdate();
}

FString AOCDeployableTrap::GetInteractionPrompt(const AOCCharacter*) const
{
    return FString::Printf(TEXT("E  DISARM  %s"), *OCTrapPresetToString(TrapPreset));
}

bool AOCDeployableTrap::CanInteractServer(const AOCCharacter* InteractingCharacter) const
{
    if (!Super::CanInteractServer(InteractingCharacter)) return false;
    const AOCPlayerState* State = InteractingCharacter ? InteractingCharacter->GetPlayerState<AOCPlayerState>() : nullptr;
    return State && State->IsEngineer();
}

void AOCDeployableTrap::InteractServer(AOCCharacter* InteractingCharacter)
{
    if (CanInteractServer(InteractingCharacter)) Destroy();
}

void AOCDeployableTrap::HandleOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    if (!HasAuthority() || !bArmed || !OtherActor || OtherActor == GetOwner()) return;

    EOCTeam OtherTeam = EOCTeam::None;
    if (const AOCCharacter* Character = Cast<AOCCharacter>(OtherActor))
    {
        if (const AOCPlayerState* State = Character->GetPlayerState<AOCPlayerState>()) OtherTeam = State->GetTeamId();
    }
    else if (const AOCArmedVehicleBase* Armed = Cast<AOCArmedVehicleBase>(OtherActor))
    {
        OtherTeam = Armed->GetOccupantTeam();
    }
    if (OwningTeam != EOCTeam::None && OtherTeam == OwningTeam) return;

    TriggerTrapServer(OtherActor);
}

void AOCDeployableTrap::TriggerTrapServer(AActor* TriggeringActor)
{
    if (!HasAuthority() || !bArmed) return;
    bArmed = false;

    const FVector Origin = GetActorLocation();
    if (TrapPreset == EOCTrapPreset::SmokeTrap)
    {
        GetWorld()->SpawnActor<AOCSmokeCloud>(AOCSmokeCloud::StaticClass(), Origin, FRotator::ZeroRotator);
    }
    else if (TrapPreset == EOCTrapPreset::FlashTrap || TrapPreset == EOCTrapPreset::ElectronicDisruptor || TrapPreset == EOCTrapPreset::DecoyTrap)
    {
        if (AOCCharacter* Character = Cast<AOCCharacter>(TriggeringActor)) Character->ApplyFlashEffectServer(0.55f, 2.2f);
    }
    else if (TrapPreset == EOCTrapPreset::SignalMine || TrapPreset == EOCTrapPreset::RepairJammer || TrapPreset == EOCTrapPreset::ObjectiveTrap)
    {
        // Non-lethal S12 gameplay placeholders. AI/objective systems will consume these presets later.
    }
    else
    {
        const bool bAntiArmor = TrapPreset == EOCTrapPreset::AntiArmorGame;
        const bool bAntiVehicle = TrapPreset == EOCTrapPreset::AntiVehicle;
        const float BaseDamage = bAntiArmor ? 520.0f : (bAntiVehicle ? 230.0f : 115.0f);
        const float Radius = bAntiArmor ? 245.0f : 310.0f;
        UGameplayStatics::ApplyRadialDamageWithFalloff(this, BaseDamage, 8.0f, Origin, 75.0f, Radius, 1.0f,
            bAntiArmor ? UOCAntiArmorDamageType::StaticClass() : UOCExplosiveDamageType::StaticClass(),
            TArray<AActor*>(), this, GetInstigatorController(), ECC_Visibility);
    }
    SetLifeSpan(0.15f);
    ForceNetUpdate();
}
