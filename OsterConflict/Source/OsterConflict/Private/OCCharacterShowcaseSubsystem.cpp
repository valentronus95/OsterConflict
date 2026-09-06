#include "OCCharacterShowcaseSubsystem.h"

#include "OCGameMode.h"
#include "OCLocalInboxRuntimeSubsystem.h"
#include "OCPlayerController.h"
#include "OCPlayerState.h"
#include "OCTeamSpawnPoint.h"
#include "OCWeaponBase.h"
#include "OCWeaponVariants.h"

#include "Animation/SkeletalMeshActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"

namespace
{
    constexpr int32 RequiredShowcaseCharacters = 5;
    constexpr float ShowcaseSpacingCm = 150.0f;
    constexpr float ShowcaseForwardOffsetCm = 500.0f;
    constexpr float PlayerStartGroundOffsetCm = -88.0f;

    FName ResolveWeaponHandSocket(const USkeletalMeshComponent* Body)
    {
        if (!Body) return NAME_None;
        const FName Candidates[] = {
            FName(TEXT("weapon_r")),
            FName(TEXT("weapon")),
            FName(TEXT("hand_r")),
            FName(TEXT("RightHand"))
        };
        for (const FName Candidate : Candidates)
        {
            if (Body->DoesSocketExist(Candidate)) return Candidate;
        }
        return NAME_None;
    }
}

bool UOCCharacterShowcaseSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCCharacterShowcaseSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (InWorld.GetNetMode() == NM_DedicatedServer) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (FParse::Param(FCommandLine::Get(), TEXT("ValidateLocalInbox"))) return;

    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    InWorld.GetTimerManager().SetTimer(
        RetryTimer, this, &UOCCharacterShowcaseSubsystem::TrySpawnShowcase, 0.50f, true, 0.25f);
}

void UOCCharacterShowcaseSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RetryTimer);
    }
    ClearShowcase();
    Super::Deinitialize();
}

void UOCCharacterShowcaseSubsystem::ClearShowcase()
{
    for (TWeakObjectPtr<AOCWeaponBase>& WeakWeapon : DisplayWeapons)
    {
        if (AOCWeaponBase* Weapon = WeakWeapon.Get()) Weapon->Destroy();
    }
    DisplayWeapons.Reset();

    for (TWeakObjectPtr<ASkeletalMeshActor>& WeakMannequin : Mannequins)
    {
        if (ASkeletalMeshActor* Mannequin = WeakMannequin.Get()) Mannequin->Destroy();
    }
    Mannequins.Reset();
}

void UOCCharacterShowcaseSubsystem::BlockShowcase(const FString& Reason, const int32 CompatibleCount)
{
    if (bShowcaseBlocked) return;
    bShowcaseBlocked = true;
    bShowcaseReady = false;
    if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(RetryTimer);
    ClearShowcase();
    UE_LOG(LogTemp, Error,
        TEXT("CHARACTER_SHOWCASE_BLOCKED reason=%s compatible_models=%d required=5 fallback=0"),
        *Reason, CompatibleCount);
}

void UOCCharacterShowcaseSubsystem::TrySpawnShowcase()
{
    if (bShowcaseReady || bShowcaseBlocked) return;

    UWorld* World = GetWorld();
    AOCPlayerController* PC = World ? Cast<AOCPlayerController>(World->GetFirstPlayerController()) : nullptr;
    if (!World || !PC || !PC->IsLocalController()) return;

    // Do not synchronously load five skeletal meshes while the player is still using frontend/deployment UI.
    // That path was able to seize the game thread immediately after TEAM selection, which also made the native
    // minimize/close buttons and Alt+Tab appear dead. The showcase belongs in the spawned base, not in the menu.
    if (PC->IsFrontendMenuVisible() || PC->IsDeploymentPanelVisible() || PC->IsSettingsVisible() || !PC->GetPawn())
    {
        return;
    }

    AOCPlayerState* PlayerState = PC->GetPlayerState<AOCPlayerState>();
    if (!PlayerState || PlayerState->GetTeamId() == EOCTeam::None) return;

    AOCTeamSpawnPoint* TeamBase = nullptr;
    for (TActorIterator<AOCTeamSpawnPoint> It(World); It; ++It)
    {
        AOCTeamSpawnPoint* Candidate = *It;
        if (Candidate && Candidate->IsBaseSpawn() && Candidate->GetTeamId() == PlayerState->GetTeamId())
        {
            TeamBase = Candidate;
            break;
        }
    }
    if (!TeamBase) return;

    const int32 CompatibleCount = UOCLocalInboxRuntimeSubsystem::GetCompatibleCharacterSkinCount();
    if (CompatibleCount < RequiredShowcaseCharacters)
    {
        BlockShowcase(TEXT("insufficient_compatible_character_models"), CompatibleCount);
        return;
    }

    TArray<USkeletalMesh*> LoadedSkins;
    LoadedSkins.Reserve(RequiredShowcaseCharacters);
    for (int32 Index = 0; Index < RequiredShowcaseCharacters; ++Index)
    {
        USkeletalMesh* Skin = UOCLocalInboxRuntimeSubsystem::LoadCompatibleCharacterSkin(Index);
        if (!Skin)
        {
            BlockShowcase(FString::Printf(TEXT("skin_load_failed_index_%d"), Index), CompatibleCount);
            return;
        }
        if (LoadedSkins.Contains(Skin))
        {
            BlockShowcase(FString::Printf(TEXT("duplicate_character_model_index_%d"), Index), CompatibleCount);
            return;
        }
        LoadedSkins.Add(Skin);
    }

    const TSubclassOf<AOCWeaponBase> WeaponClasses[RequiredShowcaseCharacters] = {
        AOCWeapon_AssaultRifle::StaticClass(),
        AOCWeapon_Pistol::StaticClass(),
        AOCWeapon_Shotgun::StaticClass(),
        AOCWeapon_Sniper::StaticClass(),
        AOCWeapon_LMG::StaticClass()
    };

    const FVector Forward = TeamBase->GetActorForwardVector().GetSafeNormal2D();
    const FVector Right = TeamBase->GetActorRightVector().GetSafeNormal2D();
    const FRotator Facing(0.0f, TeamBase->GetActorRotation().Yaw, 0.0f);
    const FVector BaseOrigin = TeamBase->GetActorLocation()
        + Forward * ShowcaseForwardOffsetCm
        + FVector(0.0f, 0.0f, PlayerStartGroundOffsetCm);

    FActorSpawnParameters MannequinParams;
    MannequinParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    int32 HandSocketFallbacks = 0;
    for (int32 Index = 0; Index < RequiredShowcaseCharacters; ++Index)
    {
        const float LateralOffset = (static_cast<float>(Index) - 2.0f) * ShowcaseSpacingCm;
        const FVector Location = BaseOrigin + Right * LateralOffset;

        ASkeletalMeshActor* Mannequin = World->SpawnActor<ASkeletalMeshActor>(Location, Facing, MannequinParams);
        if (!Mannequin)
        {
            BlockShowcase(FString::Printf(TEXT("mannequin_spawn_failed_index_%d"), Index), CompatibleCount);
            return;
        }

        Mannequin->Tags.Add(FName(TEXT("OC_CharacterShowcase")));
        Mannequin->SetReplicates(false);
        Mannequin->SetActorEnableCollision(false);
        Mannequin->SetActorTickEnabled(false);

        USkeletalMeshComponent* Body = Mannequin->GetSkeletalMeshComponent();
        if (!Body)
        {
            Mannequins.Add(Mannequin);
            BlockShowcase(FString::Printf(TEXT("skeletal_component_missing_index_%d"), Index), CompatibleCount);
            return;
        }

        Body->SetSkeletalMeshAsset(LoadedSkins[Index]);
        Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Body->SetGenerateOverlapEvents(false);
        Body->SetCanEverAffectNavigation(false);
        Body->SetVisibility(true, true);
        Body->SetCastShadow(true);
        Body->SetComponentTickEnabled(false);
        Mannequins.Add(Mannequin);

        FActorSpawnParameters WeaponParams;
        WeaponParams.Owner = Mannequin;
        WeaponParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AOCWeaponBase* Weapon = World->SpawnActor<AOCWeaponBase>(WeaponClasses[Index], Location, Facing, WeaponParams);
        if (!Weapon)
        {
            BlockShowcase(FString::Printf(TEXT("display_weapon_spawn_failed_index_%d"), Index), CompatibleCount);
            return;
        }

        Weapon->SetReplicates(false);
        Weapon->SetActorEnableCollision(false);
        Weapon->SetActorTickEnabled(false);
        Weapon->SetActorHiddenInGame(false);

        const FName HandSocket = ResolveWeaponHandSocket(Body);
        if (!HandSocket.IsNone())
        {
            Weapon->AttachToComponent(Body, FAttachmentTransformRules::SnapToTargetNotIncludingScale, HandSocket);
            Weapon->SetActorRelativeLocation(FVector::ZeroVector);
            Weapon->SetActorRelativeRotation(FRotator::ZeroRotator);
        }
        else
        {
            ++HandSocketFallbacks;
            Weapon->AttachToComponent(Body, FAttachmentTransformRules::KeepWorldTransform);
            Weapon->SetActorLocation(Location + Forward * 28.0f + Right * 18.0f + FVector(0.0f, 0.0f, 112.0f));
            Weapon->SetActorRotation(Facing);
        }
        DisplayWeapons.Add(Weapon);
    }

    bShowcaseReady = true;
    World->GetTimerManager().ClearTimer(RetryTimer);
    UE_LOG(LogTemp, Display,
        TEXT("CHARACTER_SHOWCASE_READY count=5 distinct_models=5 spacing_cm=150 ai=0 base_team=%d weapons=AR,Pistol,Shotgun,Sniper,LMG hand_socket_fallbacks=%d deferred_until_spawned_gameplay=1"),
        static_cast<int32>(PlayerState->GetTeamId()), HandSocketFallbacks);
}
