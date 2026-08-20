#include "OCFirstPersonWeaponPresentationSubsystem.h"

#include "OCCharacter.h"
#include "OCGameMode.h"
#include "OCWeaponBase.h"

#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"

bool UOCFirstPersonWeaponPresentationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCFirstPersonWeaponPresentationSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (InWorld.GetNetMode() == NM_DedicatedServer) return;

    RifleIdleAnimation = LoadObject<UAnimSequence>(nullptr,
        TEXT("/Game/SampleAnimationPack/Animations/Rifle/AS_F_Rifle_Idle_01.AS_F_Rifle_Idle_01"));
    RifleADSIdleAnimation = LoadObject<UAnimSequence>(nullptr,
        TEXT("/Game/SampleAnimationPack/Animations/Rifle/AS_F_ADS_Rifle_Idle_01.AS_F_ADS_Rifle_Idle_01"));
    AKFireAnimation = LoadObject<UAnimSequence>(nullptr,
        TEXT("/Game/AK-47/Animations/AK-47_Fire_W.AK-47_Fire_W"));
    AKReloadAnimation = LoadObject<UAnimSequence>(nullptr,
        TEXT("/Game/AK-47/Animations/AK-47_Reload_W.AK-47_Reload_W"));
}

TStatId UOCFirstPersonWeaponPresentationSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCFirstPersonWeaponPresentationSubsystem, STATGROUP_Tickables);
}

void UOCFirstPersonWeaponPresentationSubsystem::Tick(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World || World->GetNetMode() == NM_DedicatedServer) return;

    if (const AOCGameMode* GameMode = World->GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    for (TActorIterator<AOCCharacter> It(World); It; ++It)
    {
        AOCCharacter& Character = **It;
        if (Character.IsLocallyControlled() && !Character.IsInVehicle())
        {
            UpdateLocalCharacter(Character, DeltaTime);
        }
    }

    for (auto It = StateByCharacter.CreateIterator(); It; ++It)
    {
        if (!It.Key().IsValid()) It.RemoveCurrent();
    }
}

USkeletalMeshComponent* UOCFirstPersonWeaponPresentationSubsystem::FindProductionWeaponVisual(AOCWeaponBase& Weapon) const
{
    TArray<USkeletalMeshComponent*> Components;
    Weapon.GetComponents<USkeletalMeshComponent>(Components);
    for (USkeletalMeshComponent* Component : Components)
    {
        if (Component && Component->ComponentHasTag(FName(TEXT("OC_ProductionWeaponVisual"))))
        {
            return Component;
        }
    }
    return nullptr;
}

void UOCFirstPersonWeaponPresentationSubsystem::PlayWeaponAnimation(AOCWeaponBase& Weapon, UAnimSequence* Sequence,
    FOCFirstPersonWeaponState& State, double ResetDelaySeconds)
{
    if (!Sequence) return;
    USkeletalMeshComponent* Visual = FindProductionWeaponVisual(Weapon);
    USkeletalMesh* Mesh = Visual ? Visual->GetSkeletalMeshAsset() : nullptr;
    if (!Visual || !Mesh || !Sequence->GetSkeleton() || Sequence->GetSkeleton() != Mesh->GetSkeleton()) return;

    Visual->PlayAnimation(Sequence, false);
    State.bWeaponAnimationActive = true;
    State.WeaponAnimationResetTime = GetWorld() ? GetWorld()->GetTimeSeconds() + FMath::Max(0.05, ResetDelaySeconds) : 0.0;
}

void UOCFirstPersonWeaponPresentationSubsystem::ApplyArmsPose(AOCCharacter& Character,
    FOCFirstPersonWeaponState& State, bool bADS)
{
    USkeletalMeshComponent* Arms = Character.GetFirstPersonArms();
    USkeletalMesh* ArmsMesh = Arms ? Arms->GetSkeletalMeshAsset() : nullptr;
    if (!Arms || !ArmsMesh) return;

    UAnimSequence* Desired = bADS ? RifleADSIdleAnimation.Get() : RifleIdleAnimation.Get();
    if (!Desired || !Desired->GetSkeleton() || Desired->GetSkeleton() != ArmsMesh->GetSkeleton())
    {
        return;
    }

    if (!State.bRiflePoseApplied || State.bADSArmsPose != bADS)
    {
        Arms->PlayAnimation(Desired, true);
        State.bRiflePoseApplied = true;
        State.bADSArmsPose = bADS;
    }
}

void UOCFirstPersonWeaponPresentationSubsystem::UpdateLocalCharacter(AOCCharacter& Character, float DeltaTime)
{
    AOCWeaponBase* Weapon = Character.GetCurrentWeapon();
    USkeletalMeshComponent* Arms = Character.GetFirstPersonArms();
    if (!Weapon || Weapon->IsWorldPickup() || !Arms) return;

    FOCFirstPersonWeaponState& State = StateByCharacter.FindOrAdd(TWeakObjectPtr<AOCCharacter>(&Character));
    const double Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;

    if (State.Weapon.Get() != Weapon)
    {
        State = FOCFirstPersonWeaponState();
        State.Weapon = Weapon;
        State.LastAmmo = Weapon->GetAmmoInMagazine();
        State.bWasReloading = Weapon->IsReloading();
        State.ReloadStartTime = Now;
        State.BaseWeaponLocation = Weapon->GetActorRelativeLocation();
        State.BaseWeaponRotation = Weapon->GetActorRelativeRotation();
        State.BaseArmsLocation = Arms->GetRelativeLocation();
        State.BaseArmsRotation = Arms->GetRelativeRotation();
    }

    const bool bADS = Character.IsAiming();
    const EOCWeaponClass WeaponClass = Weapon->GetWeaponClass();
    const bool bLongGun = WeaponClass != EOCWeaponClass::Pistol;
    if (bLongGun) ApplyArmsPose(Character, State, bADS);

    const int32 CurrentAmmo = Weapon->GetAmmoInMagazine();
    if (State.LastAmmo != INDEX_NONE && CurrentAmmo < State.LastAmmo && !Weapon->IsReloading())
    {
        State.RecoilAlpha = 1.0f;
        if (Weapon->GetWeaponId() == FName(TEXT("OC_AR1")))
        {
            PlayWeaponAnimation(*Weapon, AKFireAnimation, State, 0.11);
        }
    }
    State.LastAmmo = CurrentAmmo;

    const bool bReloading = Weapon->IsReloading();
    if (bReloading && !State.bWasReloading)
    {
        State.ReloadStartTime = Now;
        if (Weapon->GetWeaponId() == FName(TEXT("OC_AR1")))
        {
            PlayWeaponAnimation(*Weapon, AKReloadAnimation, State, Weapon->GetReloadDuration());
        }
    }

    if (!bReloading && State.bWasReloading && State.bWeaponAnimationActive)
    {
        if (USkeletalMeshComponent* Visual = FindProductionWeaponVisual(*Weapon))
        {
            Visual->SetAnimation(nullptr);
        }
        State.bWeaponAnimationActive = false;
    }
    State.bWasReloading = bReloading;

    if (State.bWeaponAnimationActive && !bReloading && Now >= State.WeaponAnimationResetTime)
    {
        if (USkeletalMeshComponent* Visual = FindProductionWeaponVisual(*Weapon))
        {
            Visual->SetAnimation(nullptr);
        }
        State.bWeaponAnimationActive = false;
    }

    State.RecoilAlpha = FMath::FInterpTo(State.RecoilAlpha, 0.0f, DeltaTime, 19.0f);

    FVector WeaponLocation = State.BaseWeaponLocation;
    FRotator WeaponRotation = State.BaseWeaponRotation;
    FVector ArmsLocation = State.BaseArmsLocation;
    FRotator ArmsRotation = State.BaseArmsRotation;

    // Camera-space ADS convergence. The weapon remains attached to the same authoritative inventory actor.
    if (bADS)
    {
        WeaponLocation += FVector(-5.5f, -9.0f, 4.0f);
        ArmsLocation += FVector(-2.0f, -3.0f, 1.5f);
    }

    // Local recoil moves both the production weapon and hands together instead of letting the gun
    // visually detach from the character during a shot.
    WeaponLocation += FVector(-4.5f * State.RecoilAlpha, 0.0f, 1.4f * State.RecoilAlpha);
    WeaponRotation += FRotator(-4.0f * State.RecoilAlpha, 0.0f, 0.8f * State.RecoilAlpha);
    ArmsLocation += FVector(-2.0f * State.RecoilAlpha, 0.0f, 0.6f * State.RecoilAlpha);
    ArmsRotation += FRotator(-2.0f * State.RecoilAlpha, 0.0f, 0.4f * State.RecoilAlpha);

    if (bReloading)
    {
        const float Duration = Weapon->GetReloadDuration();
        const float Alpha = FMath::Clamp(static_cast<float>((Now - State.ReloadStartTime) / Duration), 0.0f, 1.0f);
        const float Arc = FMath::Sin(Alpha * PI);
        WeaponLocation += FVector(-8.0f, 3.0f, -11.0f) * Arc;
        WeaponRotation += FRotator(-12.0f, 4.0f, 19.0f) * Arc;
        ArmsLocation += FVector(-5.0f, 2.0f, -7.0f) * Arc;
        ArmsRotation += FRotator(-8.0f, 3.0f, 11.0f) * Arc;
    }

    Weapon->SetActorRelativeLocation(WeaponLocation);
    Weapon->SetActorRelativeRotation(WeaponRotation);
    Arms->SetRelativeLocation(ArmsLocation);
    Arms->SetRelativeRotation(ArmsRotation);
}
