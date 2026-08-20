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
        if (!Character.IsLocallyControlled()) continue;

        const TWeakObjectPtr<AOCCharacter> CharacterKey(&Character);
        if (!Character.IsInVehicle())
        {
            UpdateLocalCharacter(Character, DeltaTime);
        }
        else if (FOCFirstPersonWeaponState* ExistingState = StateByCharacter.Find(CharacterKey))
        {
            // Do not leave camera-space offsets or single-node animations parked on the character
            // while vehicle presentation owns the local view.
            RestorePresentationState(Character, *ExistingState);
            StateByCharacter.Remove(CharacterKey);
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

void UOCFirstPersonWeaponPresentationSubsystem::RestorePresentationState(AOCCharacter& Character,
    FOCFirstPersonWeaponState& State)
{
    if (AOCWeaponBase* PreviousWeapon = State.Weapon.Get())
    {
        if (State.bWeaponAnimationActive)
        {
            if (USkeletalMeshComponent* Visual = FindProductionWeaponVisual(*PreviousWeapon))
            {
                // We only start single-node sequences when no AnimBlueprint is authoritative.
                // If another system has since installed an AnimBlueprint, leave it alone.
                if (Visual->GetAnimationMode() != EAnimationMode::AnimationBlueprint)
                {
                    Visual->SetAnimation(nullptr);
                }
            }
        }

        // A dropped weapon has already been detached and positioned in world space by gameplay code.
        // Never reinterpret the old camera-relative base transform as a world transform.
        if (!PreviousWeapon->IsWorldPickup())
        {
            PreviousWeapon->SetActorRelativeLocation(State.BaseWeaponLocation);
            PreviousWeapon->SetActorRelativeRotation(State.BaseWeaponRotation);
        }
    }

    if (USkeletalMeshComponent* Arms = Character.GetFirstPersonArms())
    {
        Arms->SetRelativeLocation(State.BaseArmsLocation);
        Arms->SetRelativeRotation(State.BaseArmsRotation);
        if (State.bRiflePoseApplied && Arms->GetAnimationMode() != EAnimationMode::AnimationBlueprint)
        {
            Arms->SetAnimation(nullptr);
        }
    }

    State.bWeaponAnimationActive = false;
    State.bRiflePoseApplied = false;
    State.RecoilAlpha = 0.0f;
}

void UOCFirstPersonWeaponPresentationSubsystem::PlayWeaponAnimation(AOCWeaponBase& Weapon, UAnimSequence* Sequence,
    FOCFirstPersonWeaponState& State, double ResetDelaySeconds)
{
    if (!Sequence) return;
    USkeletalMeshComponent* Visual = FindProductionWeaponVisual(Weapon);
    USkeletalMesh* Mesh = Visual ? Visual->GetSkeletalMeshAsset() : nullptr;
    if (!Visual || !Mesh || !Sequence->GetSkeleton() || Sequence->GetSkeleton() != Mesh->GetSkeleton()) return;

    // Production weapons may later gain their own animation blueprint. Do not replace an authored
    // AnimInstance with this lightweight fallback sequence layer.
    if (Visual->GetAnimationMode() == EAnimationMode::AnimationBlueprint && Visual->GetAnimClass()) return;

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

    // CharacterVisualComponent may install a real first-person AnimBlueprint from the active profile.
    // SampleAnimationPack is only a fallback and must never knock that AnimInstance out of blueprint mode.
    if (Arms->GetAnimationMode() == EAnimationMode::AnimationBlueprint && Arms->GetAnimClass()) return;

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
    const TWeakObjectPtr<AOCCharacter> CharacterKey(&Character);

    if (!Weapon || Weapon->IsWorldPickup() || !Arms)
    {
        if (FOCFirstPersonWeaponState* ExistingState = StateByCharacter.Find(CharacterKey))
        {
            RestorePresentationState(Character, *ExistingState);
            StateByCharacter.Remove(CharacterKey);
        }
        return;
    }

    FOCFirstPersonWeaponState* ExistingState = StateByCharacter.Find(CharacterKey);
    if (ExistingState && ExistingState->Weapon.Get() != Weapon)
    {
        // Weapon switches can happen while ADS/recoil/reload offsets are non-zero. Restore the old
        // presentation before capturing the new weapon's base transform so offsets never accumulate.
        RestorePresentationState(Character, *ExistingState);
        StateByCharacter.Remove(CharacterKey);
        ExistingState = nullptr;
    }

    FOCFirstPersonWeaponState& State = StateByCharacter.FindOrAdd(CharacterKey);
    const double Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;

    if (!ExistingState)
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
    if (bLongGun)
    {
        ApplyArmsPose(Character, State, bADS);
    }
    else if (State.bRiflePoseApplied && Arms->GetAnimationMode() != EAnimationMode::AnimationBlueprint)
    {
        // Covers a same-weapon-class/presentation mutation without waiting for a full state reset.
        Arms->SetAnimation(nullptr);
        State.bRiflePoseApplied = false;
    }

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
            if (Visual->GetAnimationMode() != EAnimationMode::AnimationBlueprint)
            {
                Visual->SetAnimation(nullptr);
            }
        }
        State.bWeaponAnimationActive = false;
    }
    State.bWasReloading = bReloading;

    if (State.bWeaponAnimationActive && !bReloading && Now >= State.WeaponAnimationResetTime)
    {
        if (USkeletalMeshComponent* Visual = FindProductionWeaponVisual(*Weapon))
        {
            if (Visual->GetAnimationMode() != EAnimationMode::AnimationBlueprint)
            {
                Visual->SetAnimation(nullptr);
            }
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
        const float Duration = FMath::Max(0.05f, Weapon->GetReloadDuration());
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
