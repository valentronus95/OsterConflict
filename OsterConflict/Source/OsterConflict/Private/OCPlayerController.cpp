#include "OCPlayerController.h"
#include "OCBuildVersion.h"
#include "OCGameInstance.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerInput.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "OCGameState.h"
#include "OCGameUIRootWidget.h"
#include "OCGameMode.h"
#include "OCPlayerState.h"
#include "OCPlayerUserSettings.h"
#include "OCAudioUserSettings.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine/Engine.h"
#include "OCTeamTypes.h"
#include "OCCharacter.h"
#include "OCHealthComponent.h"
#include "OCWeaponBase.h"
#include "OCWeaponVariants.h"
#include "OCAntiArmorLauncher.h"
#include "OCLocalInboxRuntimeSubsystem.h"
#include "OCAIController.h"
#include "OCAmmoBox.h"
#include "OCCivilianVehicle.h"
#include "OCPickupGunTruck.h"
#include "OCBTR.h"
#include "OCInteractableDoor.h"
#include "OCInteractableGate.h"
#include "OCInteractableLight.h"
#include "OCWorldSectorOster.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

AOCPlayerController::AOCPlayerController()
{
    ControllerMappingContext = CreateDefaultSubobject<UInputMappingContext>(TEXT("IMC_ControllerRuntime"));
    ScoreboardAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Scoreboard"));
    ScoreboardAction->ValueType = EInputActionValueType::Boolean;
    AdminToggleAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_AdminToggle"));
    AdminUpAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_AdminUp"));
    AdminDownAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_AdminDown"));
    AdminExecuteAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_AdminExecute"));
    DeploymentToggleAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_DeploymentToggle"));
    RoleCycleAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_RoleCycle"));
    SquadCycleAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_SquadCycle"));
    ReadyAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_LobbyReady"));
    MenuToggleAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_MenuToggle"));
    ChatToggleAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_ChatToggle"));

    if (ControllerMappingContext && ScoreboardAction)
    {
        ControllerMappingContext->MapKey(ScoreboardAction, EKeys::Tab);
        ControllerMappingContext->MapKey(AdminToggleAction, EKeys::F10);
        ControllerMappingContext->MapKey(AdminUpAction, EKeys::Up);
        ControllerMappingContext->MapKey(AdminDownAction, EKeys::Down);
        ControllerMappingContext->MapKey(AdminExecuteAction, EKeys::Enter);
        ControllerMappingContext->MapKey(DeploymentToggleAction, EKeys::F8);
        ControllerMappingContext->MapKey(RoleCycleAction, EKeys::F2);
        ControllerMappingContext->MapKey(SquadCycleAction, EKeys::F3);
        ControllerMappingContext->MapKey(ReadyAction, EKeys::F4);
        ControllerMappingContext->MapKey(MenuToggleAction, EKeys::Escape);
        ControllerMappingContext->MapKey(MenuToggleAction, EKeys::Gamepad_Special_Right); // Menu/Start opens the same top-level UI flow.
        ControllerMappingContext->MapKey(ChatToggleAction, EKeys::T);
    }
}

void AOCPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (IsLocalController())
    {
        ConfigureControllerInput();
        const bool bForceFrontend = FParse::Param(FCommandLine::Get(), TEXT("Frontend"));
        const bool bNoFrontend = FParse::Param(FCommandLine::Get(), TEXT("NoFrontend"));
        bFrontendMenuVisible = !bNoFrontend && (bForceFrontend || GetNetMode() == NM_Standalone);
        CreateRichUI();
        ApplyUIInputMode();
        if (GetNetMode() == NM_Client)
        {
            if (UOCGameInstance* GI = Cast<UOCGameInstance>(GetGameInstance())) GI->MarkConnected();
        }
    }
}

void AOCPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
    {
        EnhancedInput->BindAction(ScoreboardAction, ETriggerEvent::Started, this, &AOCPlayerController::ShowScoreboard);
        EnhancedInput->BindAction(ScoreboardAction, ETriggerEvent::Completed, this, &AOCPlayerController::HideScoreboard);
        EnhancedInput->BindAction(ScoreboardAction, ETriggerEvent::Canceled, this, &AOCPlayerController::HideScoreboard);
        EnhancedInput->BindAction(AdminToggleAction, ETriggerEvent::Started, this, &AOCPlayerController::ToggleAdminPanel);
        EnhancedInput->BindAction(AdminUpAction, ETriggerEvent::Started, this, &AOCPlayerController::AdminSelectUp);
        EnhancedInput->BindAction(AdminDownAction, ETriggerEvent::Started, this, &AOCPlayerController::AdminSelectDown);
        EnhancedInput->BindAction(AdminExecuteAction, ETriggerEvent::Started, this, &AOCPlayerController::AdminExecute);
        EnhancedInput->BindAction(DeploymentToggleAction, ETriggerEvent::Started, this, &AOCPlayerController::ToggleDeploymentPanel);
        EnhancedInput->BindAction(RoleCycleAction, ETriggerEvent::Started, this, &AOCPlayerController::CycleRole);
        EnhancedInput->BindAction(SquadCycleAction, ETriggerEvent::Started, this, &AOCPlayerController::CycleSquad);
        EnhancedInput->BindAction(ReadyAction, ETriggerEvent::Started, this, &AOCPlayerController::ToggleReady);
        EnhancedInput->BindAction(MenuToggleAction, ETriggerEvent::Started, this, &AOCPlayerController::ToggleFrontendMenu);
        EnhancedInput->BindAction(ChatToggleAction, ETriggerEvent::Started, this, &AOCPlayerController::ToggleChatInput);
    }
}

void AOCPlayerController::ConfigureControllerInput()
{
    if (!ControllerMappingContext)
    {
        return;
    }

    RefreshControllerUserKeyMappings();
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->RemoveMappingContext(ControllerMappingContext);
        Subsystem->AddMappingContext(ControllerMappingContext, 50);
    }
}

void AOCPlayerController::RefreshControllerUserKeyMappings()
{
    if (!ControllerMappingContext) return;
    const UOCPlayerUserSettings* Settings = UOCPlayerUserSettings::Get();
    if (ScoreboardAction)
    {
        ControllerMappingContext->UnmapAllKeysFromAction(ScoreboardAction);
        ControllerMappingContext->MapKey(ScoreboardAction, Settings->GetKey(TEXT("Scoreboard")));
    }
    if (ChatToggleAction)
    {
        ControllerMappingContext->UnmapAllKeysFromAction(ChatToggleAction);
        ControllerMappingContext->MapKey(ChatToggleAction, Settings->GetKey(TEXT("Chat")));
    }
}

void AOCPlayerController::ShowScoreboard()
{
    bScoreboardVisible = true;
}

void AOCPlayerController::HideScoreboard()
{
    bScoreboardVisible = false;
}


void AOCPlayerController::CreateRichUI()
{
    if (!IsLocalController() || RichUIRoot) return;
    RichUIRoot = CreateWidget<UOCGameUIRootWidget>(this, UOCGameUIRootWidget::StaticClass());
    if (RichUIRoot) RichUIRoot->AddToViewport(500);
}

void AOCPlayerController::ApplyUIInputMode()
{
    if (!IsLocalController()) return;

    const bool bNeedsUI = bFrontendMenuVisible || bDeploymentPanelVisible || bAdminPanelVisible || bChatInputActive || bSettingsVisible;

    // SetIgnoreMoveInput / SetIgnoreLookInput are stack based. ApplyUIInputMode can be
    // called repeatedly while a menu is open, so always clear the accumulated stack
    // before applying the single state that is actually required now.
    ResetIgnoreMoveInput();
    ResetIgnoreLookInput();

    if (bNeedsUI)
    {
        SetIgnoreMoveInput(true);
        SetIgnoreLookInput(true);
        bShowMouseCursor = true;

        FInputModeGameAndUI Mode;
        Mode.SetHideCursorDuringCapture(false);
        SetInputMode(Mode);
        return;
    }

    // Returning from Escape/settings/chat must restore a clean gameplay input state.
    bShowMouseCursor = false;
    if (PlayerInput)
    {
        PlayerInput->FlushPressedKeys();
    }
    SetInputMode(FInputModeGameOnly());
}

void AOCPlayerController::ApplyDeploymentInputLock()
{
    ApplyUIInputMode();
}

void AOCPlayerController::ToggleFrontendMenu()
{
    if (!IsLocalController()) return;
    if (bSettingsVisible)
    {
        UOCPlayerUserSettings::Get()->ReloadConfig();
        UOCAudioUserSettings::Get()->ReloadConfig();
        if (UGameUserSettings* GU = GEngine ? GEngine->GetGameUserSettings() : nullptr) GU->LoadSettings(true);
        bSettingsVisible = false;
        UIApplyLocalPreferences();
        ApplyUIInputMode();
        return;
    }
    // R6: a normal standalone packaged launch is a Frontend shell, not a hidden local match.
    // There is deliberately no gameplay pawn/world behind this menu, so Esc/Close must not
    // strand the player on an empty screen. -NoFrontend remains the explicit dev/gameplay path.
    if (bFrontendMenuVisible && GetNetMode() == NM_Standalone &&
        !FParse::Param(FCommandLine::Get(), TEXT("NoFrontend")))
    {
        return;
    }

    bFrontendMenuVisible = !bFrontendMenuVisible;
    if (bFrontendMenuVisible) bAdminPanelVisible = false;
    ApplyUIInputMode();
}

void AOCPlayerController::ToggleChatInput()
{
    if (!IsLocalController() || bFrontendMenuVisible || bDeploymentPanelVisible || bAdminPanelVisible || bSettingsVisible) return;
    bChatInputActive = !bChatInputActive;
    ApplyUIInputMode();
}

void AOCPlayerController::ToggleDeploymentPanel()
{
    bDeploymentPanelVisible = !bDeploymentPanelVisible;
    if (bDeploymentPanelVisible) bFrontendMenuVisible = false;
    ApplyUIInputMode();
}

void AOCPlayerController::CycleRole()
{
    if (!bDeploymentPanelVisible) return;
    if (HasAuthority()) ServerCycleRole_Implementation(); else ServerCycleRole();
}

void AOCPlayerController::CycleSquad()
{
    if (!bDeploymentPanelVisible) return;
    const AOCPlayerState* State = GetPlayerState<AOCPlayerState>();
    const int32 NextSquad = State ? (State->GetSquadId() + 1) % 4 : 0;
    if (HasAuthority()) ServerRequestSquad_Implementation(NextSquad); else ServerRequestSquad(NextSquad);
}

void AOCPlayerController::ToggleReady()
{
    if (!bDeploymentPanelVisible) return;
    const AOCPlayerState* State = GetPlayerState<AOCPlayerState>();
    const bool bNextReady = !(State && State->IsLobbyReady());
    if (HasAuthority()) ServerSetLobbyReady_Implementation(bNextReady); else ServerSetLobbyReady(bNextReady);
    if (bNextReady)
    {
        bDeploymentPanelVisible = false;
        ApplyUIInputMode();
    }
}

void AOCPlayerController::ServerCycleRole_Implementation()
{
    if (AOCPlayerState* State = GetPlayerState<AOCPlayerState>())
    {
        EOCPlayerRole Next = EOCPlayerRole::Rifleman;
        switch (State->GetPlayerRole())
        {
        case EOCPlayerRole::Rifleman: Next = EOCPlayerRole::Medic; break;
        case EOCPlayerRole::Medic: Next = EOCPlayerRole::Engineer; break;
        case EOCPlayerRole::Engineer: Next = EOCPlayerRole::Support; break;
        default: Next = EOCPlayerRole::Rifleman; break;
        }
        State->SetRoleServer(Next);
    }
}

void AOCPlayerController::ServerRequestSquad_Implementation(int32 SquadId)
{
    if (AOCGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AOCGameMode>() : nullptr)
        if (AOCPlayerState* State = GetPlayerState<AOCPlayerState>()) GM->RequestSquadChange(State, SquadId);
}

void AOCPlayerController::ServerSetLobbyReady_Implementation(bool bReady)
{
    if (AOCPlayerState* State = GetPlayerState<AOCPlayerState>())
    {
        State->SetLobbyReadyServer(bReady);
        if (bReady && !GetPawn())
        {
            if (AOCGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AOCGameMode>() : nullptr) GM->RestartPlayer(this);
        }
    }
}

void AOCPlayerController::ServerRequestTeam_Implementation(EOCTeam RequestedTeam)
{
    if (AOCGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AOCGameMode>() : nullptr)
        if (AOCPlayerState* State = GetPlayerState<AOCPlayerState>()) GM->RequestTeamChange(State, RequestedTeam);
}

void AOCPlayerController::ServerSetDeploymentSpawn_Implementation(FName SpawnId)
{
    const FString Clean = SpawnId.ToString().ToUpper();
    RequestedDeploymentSpawn = (Clean == TEXT("A") || Clean == TEXT("B") || Clean == TEXT("C")) ? FName(*Clean) : FName(TEXT("BASE"));
}

bool AOCPlayerController::IsSandboxAdmin() const
{
    if (HasAuthority())
    {
        const AOCGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AOCGameMode>() : nullptr;
        return GM && GM->CanUseSandboxAdmin(this);
    }
    return bSandboxAdminAllowed;
}

void AOCPlayerController::ClientSetSandboxAdminAllowed_Implementation(bool bAllowed)
{
    bSandboxAdminAllowed = bAllowed;
    if (!bAllowed && bAdminPanelVisible)
    {
        bAdminPanelVisible = false;
        ApplyUIInputMode();
    }
}

FString AOCPlayerController::GetAdminActionLabel(int32 Index) const
{
    static const TCHAR* Labels[] = {TEXT("Spawn all weapons"),TEXT("Refill ammo"),TEXT("Restore player"),TEXT("Spawn civilian car"),
        TEXT("Spawn gun truck"),TEXT("Spawn BTR"),TEXT("Toggle god mode"),TEXT("Reset doors/gates/lights"),
        TEXT("Teleport: Museum"),TEXT("Teleport: Stadium"),TEXT("Teleport: Park"),TEXT("Teleport: College"),TEXT("Spawn 4 AI bots"),TEXT("Clear AI bots")};
    return (Index>=0 && Index<UE_ARRAY_COUNT(Labels)) ? FString(Labels[Index]) : FString(TEXT("Unknown"));
}

void AOCPlayerController::ToggleAdminPanel(){if(!IsSandboxAdmin()){bAdminPanelVisible=false;ApplyUIInputMode();return;}bAdminPanelVisible=!bAdminPanelVisible;if(bAdminPanelVisible)bFrontendMenuVisible=false;ApplyUIInputMode();}
void AOCPlayerController::AdminSelectUp(){if(bAdminPanelVisible)SelectedAdminActionIndex=(SelectedAdminActionIndex+13)%14;}
void AOCPlayerController::AdminSelectDown(){if(bAdminPanelVisible)SelectedAdminActionIndex=(SelectedAdminActionIndex+1)%14;}
void AOCPlayerController::AdminExecute(){if(!bAdminPanelVisible||!IsSandboxAdmin())return;if(HasAuthority())ExecuteSandboxAdminActionServer(static_cast<EOCSandboxAdminAction>(SelectedAdminActionIndex));else ServerExecuteSandboxAdminAction(SelectedAdminActionIndex);}
void AOCPlayerController::ServerExecuteSandboxAdminAction_Implementation(int32 ActionIndex){if(!IsSandboxAdmin()||ActionIndex<0||ActionIndex>=14)return;ExecuteSandboxAdminActionServer(static_cast<EOCSandboxAdminAction>(ActionIndex));}

void AOCPlayerController::ExecuteSandboxAdminActionServer(EOCSandboxAdminAction Action)
{
    if (!HasAuthority() || !IsSandboxAdmin()) return;
    AOCCharacter* ControlledCharacter = Cast<AOCCharacter>(GetPawn());
    FVector Anchor = GetPawn() ? GetPawn()->GetActorLocation() : AOCWorldSectorOster::MuseumAnchor();
    FRotator Facing = GetControlRotation(); Facing.Pitch=0; Facing.Roll=0;
    FActorSpawnParameters Params; Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    if(Action==EOCSandboxAdminAction::SpawnWeaponRack)
    {
        struct FWeaponRackBinding
        {
            TSubclassOf<AOCWeaponBase> Class;
            const TCHAR* Category;
        };

        const FWeaponRackBinding BoundCategories[] =
        {
            { AOCWeapon_AssaultRifle::StaticClass(), TEXT("M16_M4") },
            { AOCWeapon_AssaultRifle::StaticClass(), TEXT("AR15") },
            { AOCWeapon_AssaultRifle::StaticClass(), TEXT("AK74") },
            { AOCWeapon_AssaultRifle::StaticClass(), TEXT("AK47") },
            { AOCWeapon_AssaultRifle::StaticClass(), TEXT("ASSAULT_GENERIC") },
            { AOCWeapon_AssaultRifle::StaticClass(), TEXT("RIFLE_GENERIC") },
            { AOCWeapon_SMG::StaticClass(), TEXT("MP5") },
            { AOCWeapon_SMG::StaticClass(), TEXT("SMG_GENERIC") },
            { AOCWeapon_Pistol::StaticClass(), TEXT("M1911") },
            { AOCWeapon_Pistol::StaticClass(), TEXT("MAKAROV") },
            { AOCWeapon_Pistol::StaticClass(), TEXT("PISTOL_GENERIC") },
            { AOCWeapon_Sniper::StaticClass(), TEXT("M700") },
            { AOCWeapon_Sniper::StaticClass(), TEXT("BALLISTA") },
            { AOCWeapon_Sniper::StaticClass(), TEXT("KAR98") },
            { AOCWeapon_Sniper::StaticClass(), TEXT("SNIPER_GENERIC") },
            { AOCWeapon_Shotgun::StaticClass(), TEXT("REMINGTON870") },
            { AOCWeapon_Shotgun::StaticClass(), TEXT("SHOTGUN_GENERIC") },
            { AOCWeapon_LMG::StaticClass(), TEXT("M249") },
            { AOCWeapon_LMG::StaticClass(), TEXT("LMG_GENERIC") },
            { AOCWeapon_M14::StaticClass(), TEXT("M14") },
            { AOCWeapon_Mac10::StaticClass(), TEXT("MAC10") },
            { AOCWeapon_Tec9::StaticClass(), TEXT("TEC9") },
            { AOCWeapon_LeverAction::StaticClass(), TEXT("LEVER_ACTION") },
            { AOCAntiArmorLauncher::StaticClass(), TEXT("M72") },
            { AOCAntiArmorLauncher::StaticClass(), TEXT("LAUNCHER") },
            { AOCAntiArmorLauncher::StaticClass(), TEXT("LAUNCHER_GENERIC") }
        };

        const TSubclassOf<AOCWeaponBase> GameplayClasses[] =
        {
            AOCWeapon_AssaultRifle::StaticClass(), AOCWeapon_SMG::StaticClass(), AOCWeapon_Pistol::StaticClass(),
            AOCWeapon_Sniper::StaticClass(), AOCWeapon_Shotgun::StaticClass(), AOCWeapon_LMG::StaticClass(),
            AOCWeapon_M14::StaticClass(), AOCWeapon_Mac10::StaticClass(), AOCWeapon_Tec9::StaticClass(),
            AOCWeapon_LeverAction::StaticClass(), AOCAntiArmorLauncher::StaticClass()
        };

        TSet<UClass*> RepresentedClasses;
        int32 SpawnedBoundModels = 0;
        int32 SpawnedFallbackClasses = 0;
        int32 RackIndex = 0;

        auto SpawnRackWeapon = [&](const TSubclassOf<AOCWeaponBase> WeaponClass, const FString& ForcedCategory, const int32 PathIndex)
        {
            if (!WeaponClass) return;
            const int32 Column = RackIndex % 5;
            const int32 Row = RackIndex / 5;
            const FVector Pos = Anchor + Facing.RotateVector(FVector(240.0f + Row * 125.0f, -240.0f + Column * 120.0f, 40.0f));
            if (AOCWeaponBase* Weapon = GetWorld()->SpawnActor<AOCWeaponBase>(WeaponClass, Pos, Facing, Params))
            {
                if (!ForcedCategory.IsEmpty())
                {
                    Weapon->Tags.Add(FName(*FString::Printf(TEXT("OC_FORCE_WEAPON_CATEGORY_%s"), *ForcedCategory)));
                    Weapon->Tags.Add(FName(*FString::Printf(TEXT("OC_FORCE_WEAPON_PATH_INDEX_%d"), PathIndex)));
                }
                Weapon->DropToWorldServer(Pos, Facing);
                ++RackIndex;
            }
        };

        for (const FWeaponRackBinding& Binding : BoundCategories)
        {
            TArray<FString> Paths;
            UOCLocalInboxRuntimeSubsystem::GetAssetObjectPathsForCategory(Binding.Category, Paths);
            if (Paths.IsEmpty()) continue;

            RepresentedClasses.Add(Binding.Class.Get());
            for (int32 PathIndex = 0; PathIndex < Paths.Num(); ++PathIndex)
            {
                SpawnRackWeapon(Binding.Class, Binding.Category, PathIndex);
                ++SpawnedBoundModels;
            }
        }

        for (const TSubclassOf<AOCWeaponBase>& GameplayClass : GameplayClasses)
        {
            if (!GameplayClass || RepresentedClasses.Contains(GameplayClass.Get())) continue;
            SpawnRackWeapon(GameplayClass, FString(), INDEX_NONE);
            ++SpawnedFallbackClasses;
        }

        GetWorld()->SpawnActor<AOCAmmoBox>(AOCAmmoBox::StaticClass(),
            Anchor + Facing.RotateVector(FVector(220.0f, 360.0f, 30.0f)), Facing, Params);
        UE_LOG(LogTemp, Display,
            TEXT("OC_SANDBOX_ALL_WEAPONS_SPAWNED bound_models=%d fallback_gameplay_classes=%d total=%d"),
            SpawnedBoundModels, SpawnedFallbackClasses, RackIndex);
        return;
    }
    if(Action==EOCSandboxAdminAction::RefillAmmo&&ControlledCharacter){ControlledCharacter->AddAmmoFromBoxServer(EOCAmmoType::Any,9999);return;}
    if(Action==EOCSandboxAdminAction::RestorePlayer&&ControlledCharacter&&ControlledCharacter->GetHealthComponent()){ControlledCharacter->GetHealthComponent()->RestoreFullServer();return;}
    if(Action==EOCSandboxAdminAction::SpawnCivilianVehicle){GetWorld()->SpawnActor<AOCCivilianVehicle>(AOCCivilianVehicle::StaticClass(),Anchor+Facing.RotateVector(FVector(520,0,90)),Facing,Params);return;}
    if(Action==EOCSandboxAdminAction::SpawnGunTruck){GetWorld()->SpawnActor<AOCPickupGunTruck>(AOCPickupGunTruck::StaticClass(),Anchor+Facing.RotateVector(FVector(650,0,120)),Facing,Params);return;}
    if(Action==EOCSandboxAdminAction::SpawnBTR){GetWorld()->SpawnActor<AOCBTR>(AOCBTR::StaticClass(),Anchor+Facing.RotateVector(FVector(800,0,160)),Facing,Params);return;}
    if(Action==EOCSandboxAdminAction::ToggleGodMode){bSandboxGodMode=!bSandboxGodMode;UE_LOG(LogTemp,Log,TEXT("Sandbox god mode for %s: %s"),*GetName(),bSandboxGodMode?TEXT("ON"):TEXT("OFF"));return;}
    if(Action==EOCSandboxAdminAction::ResetInteractables){for(TActorIterator<AOCInteractableDoor>It(GetWorld());It;++It)It->ResetServer();for(TActorIterator<AOCInteractableGate>It(GetWorld());It;++It)It->ResetServer();for(TActorIterator<AOCInteractableLight>It(GetWorld());It;++It)It->ResetServer();return;}
    if(Action==EOCSandboxAdminAction::SpawnFourBots){if(AOCGameMode* GM=GetWorld()->GetAuthGameMode<AOCGameMode>())GM->SpawnDebugBots(4);return;}
    if(Action==EOCSandboxAdminAction::ClearBots){if(AOCGameMode* GM=GetWorld()->GetAuthGameMode<AOCGameMode>())GM->RemoveAllBots();return;}
    if(!ControlledCharacter)return;FVector D=Anchor;
    if(Action==EOCSandboxAdminAction::TeleportMuseum)D=AOCWorldSectorOster::MuseumAnchor();else if(Action==EOCSandboxAdminAction::TeleportStadium)D=AOCWorldSectorOster::StadiumAnchor();else if(Action==EOCSandboxAdminAction::TeleportPark)D=AOCWorldSectorOster::ParkAnchor();else if(Action==EOCSandboxAdminAction::TeleportCollege)D=AOCWorldSectorOster::CollegeAnchor();
    ControlledCharacter->SetActorLocation(D+FVector(0,0,140),false,nullptr,ETeleportType::TeleportPhysics);
}

void AOCPlayerController::UIConnect(const FString& Address, const FString& Username)
{
    FString CleanAddress = Address; CleanAddress.TrimStartAndEndInline();
    const FString CleanName = SanitizeNickname(Username);
    if (CleanAddress.IsEmpty()) CleanAddress = TEXT("127.0.0.1:7777");
    if (!CleanAddress.Contains(TEXT(" ")) && CleanAddress.Len() <= 128)
    {
        UOCPlayerUserSettings::Get()->SetFrontendIdentity(CleanName, CleanAddress);
    }
    const FString Travel = FString::Printf(TEXT("%s?Name=%s"), *CleanAddress, *CleanName.Replace(TEXT(" "), TEXT("_")));
    ConnectToServer(Travel);
}

void AOCPlayerController::UIToggleFrontend(){ToggleFrontendMenu();}
void AOCPlayerController::UIRequestTeam(EOCTeam Team){if(Team==EOCTeam::None)return;if(HasAuthority())ServerRequestTeam_Implementation(Team);else ServerRequestTeam(Team);}
void AOCPlayerController::UICycleRole(){CycleRole();}
void AOCPlayerController::UICycleSquad(){CycleSquad();}
void AOCPlayerController::UISelectSpawn(FName SpawnId){if(HasAuthority())ServerSetDeploymentSpawn_Implementation(SpawnId);else ServerSetDeploymentSpawn(SpawnId);}
void AOCPlayerController::UIReadyDeploy()
{
    if (HasAuthority()) ServerSetLobbyReady_Implementation(true); else ServerSetLobbyReady(true);
    bDeploymentPanelVisible = false;
    ApplyUIInputMode();
}
void AOCPlayerController::UISendChat(EOCChatChannel Channel,const FString& Message){SendChat(Channel,Message);}
void AOCPlayerController::UIEndChatInput(){bChatInputActive=false;ApplyUIInputMode();}
void AOCPlayerController::UIAdminPrevious(){AdminSelectUp();}
void AOCPlayerController::UIAdminNext(){AdminSelectDown();}
void AOCPlayerController::UIAdminExecute(){AdminExecute();}
void AOCPlayerController::UIOpenSettings(){if(!IsLocalController())return;bSettingsVisible=true;bAdminPanelVisible=false;bChatInputActive=false;ApplyUIInputMode();}
void AOCPlayerController::UICloseSettings(){if(!IsLocalController())return;bSettingsVisible=false;ApplyUIInputMode();}
void AOCPlayerController::UICloseDeployment(){if(!IsLocalController())return;bDeploymentPanelVisible=false;ApplyUIInputMode();}
void AOCPlayerController::UICloseAdmin(){if(!IsLocalController())return;bAdminPanelVisible=false;ApplyUIInputMode();}
void AOCPlayerController::UIApplyLocalPreferences()
{
    if (!IsLocalController()) return;
    UOCPlayerUserSettings::Get()->ApplyPresentationCVars();
    RefreshControllerUserKeyMappings();
    ConfigureControllerInput();
    if (AOCCharacter* ControlledCharacter = Cast<AOCCharacter>(GetPawn())) ControlledCharacter->ApplyLocalUserPreferences();
}

void AOCPlayerController::ConnectToServer(const FString& Address)
{
    if (!IsLocalController())
    {
        return;
    }

    FString CleanAddress = Address;
    CleanAddress.TrimStartAndEndInline();

    if (CleanAddress.IsEmpty() || CleanAddress.Contains(TEXT(" ")) || CleanAddress.Len() > 128)
    {
        UE_LOG(LogTemp, Warning, TEXT("ConnectToServer rejected invalid address: '%s'"), *CleanAddress);
        return;
    }

    FString TravelAddress = CleanAddress;
    if (!TravelAddress.Contains(TEXT("?Protocol="), ESearchCase::IgnoreCase))
    {
        TravelAddress += FString::Printf(TEXT("?Protocol=%d"), OCBuildVersion::NetworkProtocol);
    }
    if (UOCGameInstance* GI = Cast<UOCGameInstance>(GetGameInstance())) GI->BeginDirectConnect(CleanAddress);
    ClientTravel(TravelAddress, TRAVEL_Absolute);
}

void AOCPlayerController::SetNickname(const FString& NewNickname)
{
    const FString CleanName = SanitizeNickname(NewNickname);
    if (CleanName.IsEmpty())
    {
        return;
    }

    if (HasAuthority())
    {
        if (AOCPlayerState* State = GetPlayerState<AOCPlayerState>())
        {
            if (AOCGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AOCGameMode>() : nullptr)
                State->SetPlayerName(GM->MakeUniquePlayerName(CleanName, State));
            else State->SetPlayerName(CleanName);
        }
    }
    else
    {
        ServerSetNickname(CleanName);
    }
}

void AOCPlayerController::ServerSetNickname_Implementation(const FString& NewNickname)
{
    const FString CleanName = SanitizeNickname(NewNickname);
    if (AOCPlayerState* State = GetPlayerState<AOCPlayerState>(); State && !CleanName.IsEmpty())
    {
        if (AOCGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AOCGameMode>() : nullptr)
            State->SetPlayerName(GM->MakeUniquePlayerName(CleanName, State));
        else State->SetPlayerName(CleanName);
    }
}


void AOCPlayerController::SendChat(EOCChatChannel Channel, const FString& Message)
{
    const FString Clean = SanitizeChat(Message);
    if (Clean.IsEmpty()) return;
    if (HasAuthority()) ServerSendChat_Implementation(Channel, Clean); else ServerSendChat(Channel, Clean);
}

void AOCPlayerController::SayGlobal(const FString& Message) { SendChat(EOCChatChannel::Global, Message); }
void AOCPlayerController::SayTeam(const FString& Message) { SendChat(EOCChatChannel::Team, Message); }
void AOCPlayerController::SaySquad(const FString& Message) { SendChat(EOCChatChannel::Squad, Message); }

void AOCPlayerController::ServerSendChat_Implementation(EOCChatChannel Channel, const FString& Message)
{
    if (!GetWorld()) return;
    const double Now = GetWorld()->GetTimeSeconds();
    if (Now - LastChatServerTime < 0.60) return;
    LastChatServerTime = Now;
    const FString Clean = SanitizeChat(Message);
    if (Clean.IsEmpty()) return;
    if (AOCGameMode* GM = GetWorld()->GetAuthGameMode<AOCGameMode>()) GM->RouteChatMessage(this, Channel, Clean);
}

void AOCPlayerController::ClientReceiveChat_Implementation(const FOCChatMessage& Message)
{
    RecentChatMessages.Add(Message);
    while (RecentChatMessages.Num() > 8) RecentChatMessages.RemoveAt(0);
}

void AOCPlayerController::SubmitSquadOrder(EOCSquadOrderType Type, FName ObjectiveId, const FVector& Location)
{
    if (HasAuthority()) ServerSubmitSquadOrder_Implementation(Type, ObjectiveId, Location);
    else ServerSubmitSquadOrder(Type, ObjectiveId, Location);
}

void AOCPlayerController::SquadAttack(const FString& ObjectiveId) { SubmitSquadOrder(EOCSquadOrderType::AttackObjective, FName(*ObjectiveId.Left(2)), FVector::ZeroVector); }
void AOCPlayerController::SquadDefend(const FString& ObjectiveId) { SubmitSquadOrder(EOCSquadOrderType::DefendObjective, FName(*ObjectiveId.Left(2)), FVector::ZeroVector); }
void AOCPlayerController::SquadRegroup() { SubmitSquadOrder(EOCSquadOrderType::Regroup, NAME_None, GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector); }
void AOCPlayerController::SquadMoveHere()
{
    const FVector Start = GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
    const FVector Target = Start + GetControlRotation().Vector() * 2000.0f;
    SubmitSquadOrder(EOCSquadOrderType::Move, NAME_None, Target);
}

void AOCPlayerController::ServerSubmitSquadOrder_Implementation(EOCSquadOrderType Type, FName ObjectiveId, FVector Location)
{
    if (AOCGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AOCGameMode>() : nullptr)
        GM->SubmitSquadOrder(this, Type, ObjectiveId, Location);
}

void AOCPlayerController::ClientReceiveSquadOrder_Implementation(const FOCSquadOrder& Order)
{
    CurrentSquadOrder = Order;
}

void AOCPlayerController::DisconnectFromServer()
{
    if (!IsLocalController())
    {
        return;
    }

    ConsoleCommand(TEXT("disconnect"), true);
}


void AOCPlayerController::PerfReport()
{
    if (HasAuthority()) ServerRequestPerfReport_Implementation();
    else ServerRequestPerfReport();
}

void AOCPlayerController::ServerRequestPerfReport_Implementation()
{
    const AOCGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AOCGameMode>() : nullptr;
    const FString Report = GM ? GM->BuildPerformanceSnapshot() : TEXT("PERF SNAPSHOT: no authoritative game mode");
    UE_LOG(LogTemp, Log, TEXT("%s"), *Report);
    ClientReceivePerfReport(Report);
}

void AOCPlayerController::ClientReceivePerfReport_Implementation(const FString& Report)
{
    UE_LOG(LogTemp, Log, TEXT("SERVER %s"), *Report);
}

FString AOCPlayerController::SanitizeNickname(const FString& RawName)
{
    FString Result = RawName;
    Result.TrimStartAndEndInline();

    FString Filtered;
    Filtered.Reserve(Result.Len());
    for (const TCHAR Character : Result)
    {
        if (FChar::IsAlnum(Character) || Character == TEXT('_') || Character == TEXT('-') || Character == TEXT(' '))
        {
            Filtered.AppendChar(Character);
        }
    }

    Filtered.TrimStartAndEndInline();
    return Filtered.Left(24);
}


FString AOCPlayerController::SanitizeChat(const FString& RawMessage)
{
    const FString Input = RawMessage.Left(120);
    FString Result;
    Result.Reserve(Input.Len());
    for (const TCHAR C : Input)
    {
        if (C == TEXT('\r') || C == TEXT('\n') || C == TEXT('\t')) Result.AppendChar(TEXT(' '));
        else if (C >= 32) Result.AppendChar(C);
    }
    Result.TrimStartAndEndInline();
    return Result.Left(120);
}
