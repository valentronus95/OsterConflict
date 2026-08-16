#include "OCAIController.h"
#include "OCLobbyTypes.h"

#include "OCBotCharacter.h"
#include "OCCapturePoint.h"
#include "OCGameState.h"
#include "OCGameMode.h"
#include "OCHealthComponent.h"
#include "OCInteractableDoor.h"
#include "OCInteractableGate.h"
#include "OCPlayerState.h"
#include "OCSmokeCloud.h"
#include "OCVehicleBase.h"
#include "OCWeaponBase.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"

AOCAIController::AOCAIController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f;
    bWantsPlayerState = true;

    BotPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
    SetPerceptionComponent(*BotPerception);
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 6500.0f;
    SightConfig->LoseSightRadius = 7800.0f;
    SightConfig->PeripheralVisionAngleDegrees = 80.0f;
    SightConfig->SetMaxAge(4.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    BotPerception->ConfigureSense(*SightConfig);
    BotPerception->SetDominantSense(SightConfig->GetSenseImplementation());
    BotPerception->OnTargetPerceptionUpdated.AddDynamic(this, &AOCAIController::HandleTargetPerceptionUpdated);
}

void AOCAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    NextThinkTime = 0.0;
    if (AOCBotCharacter* Bot = Cast<AOCBotCharacter>(InPawn))
    {
        Bot->SetActorTickEnabled(true);
    }
}

void AOCAIController::AssignBotIdentityServer(EOCTeam Team, EOCPlayerRole BotRole, EOCBotDifficulty Difficulty, int32 BotIndex)
{
    if (!HasAuthority()) return;
    if (!PlayerState) InitPlayerState();
    if (AOCPlayerState* State = GetPlayerState<AOCPlayerState>())
    {
        State->SetTeamServer(Team);
        State->SetRoleServer(BotRole);
        State->SetPlayerName(FString::Printf(TEXT("BOT-%02d-%s"), BotIndex, *OCRoleToString(BotRole)));
    }
    SetDifficultyServer(Difficulty);
}

void AOCAIController::SetDifficultyServer(EOCBotDifficulty Difficulty)
{
    if (!HasAuthority()) return;
    BotDifficulty = Difficulty;
    Tuning = MakeTuning(Difficulty);
    if (const AOCGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AOCGameMode>() : nullptr)
    {
        Tuning.ThinkInterval = FMath::Clamp(Tuning.ThinkInterval * GM->GetAIThinkIntervalScale(), 0.08f, 0.60f);
    }
    if (SightConfig && BotPerception)
    {
        SightConfig->SightRadius = Tuning.SightRadius;
        SightConfig->LoseSightRadius = Tuning.LoseSightRadius;
        SightConfig->PeripheralVisionAngleDegrees = Tuning.PeripheralVisionDegrees;
        BotPerception->ConfigureSense(*SightConfig);
        BotPerception->RequestStimuliListenerUpdate();
    }
}

FOCBotRuntimeTuning AOCAIController::MakeTuning(EOCBotDifficulty Difficulty)
{
    FOCBotRuntimeTuning T;
    switch (Difficulty)
    {
    case EOCBotDifficulty::Easy:
        T.ThinkInterval=0.34f; T.SightRadius=4700.0f; T.LoseSightRadius=5800.0f; T.PeripheralVisionDegrees=68.0f;
        T.ReactionSeconds=0.70f; T.AimErrorDegrees=3.1f; T.PreferredCombatRange=2100.0f; T.MaxCombatRange=5200.0f;
        T.CoverSearchRadius=650.0f; T.ReviveSearchRadius=1150.0f; T.VehicleUseObjectiveDistance=6500.0f; break;
    case EOCBotDifficulty::Hard:
        T.ThinkInterval=0.14f; T.SightRadius=7600.0f; T.LoseSightRadius=9000.0f; T.PeripheralVisionDegrees=92.0f;
        T.ReactionSeconds=0.16f; T.AimErrorDegrees=0.72f; T.PreferredCombatRange=3000.0f; T.MaxCombatRange=8200.0f;
        T.CoverSearchRadius=1050.0f; T.ReviveSearchRadius=1850.0f; T.VehicleUseObjectiveDistance=4300.0f; break;
    case EOCBotDifficulty::Veteran:
        T.ThinkInterval=0.10f; T.SightRadius=9000.0f; T.LoseSightRadius=10500.0f; T.PeripheralVisionDegrees=105.0f;
        T.ReactionSeconds=0.09f; T.AimErrorDegrees=0.38f; T.PreferredCombatRange=3400.0f; T.MaxCombatRange=9500.0f;
        T.CoverSearchRadius=1250.0f; T.ReviveSearchRadius=2100.0f; T.VehicleUseObjectiveDistance=3800.0f; break;
    default: break;
    }
    return T;
}

void AOCAIController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!HasAuthority() || !GetWorld()) return;
    const double Now = GetWorld()->GetTimeSeconds();
    if (Now < NextThinkTime) return;
    NextThinkTime = Now + Tuning.ThinkInterval;
    RunDecisionLoop();
}

void AOCAIController::HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    AOCCharacter* SensedCharacter = Cast<AOCCharacter>(Actor);
    if (!SensedCharacter || SensedCharacter == GetPawn()) return;
    const AOCPlayerState* MyState = GetPlayerState<AOCPlayerState>();
    const AOCPlayerState* OtherState = SensedCharacter->GetPlayerState<AOCPlayerState>();
    if (!MyState || !OtherState || MyState->GetTeamId() == OtherState->GetTeamId()) return;
    if (Stimulus.WasSuccessfullySensed())
    {
        CombatTarget = SensedCharacter;
        TargetAcquiredTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
    }
    else if (CombatTarget == SensedCharacter)
    {
        CombatTarget = nullptr;
    }
}

void AOCAIController::RunDecisionLoop()
{
    if (AOCVehicleBase* Vehicle = Cast<AOCVehicleBase>(GetPawn()))
    {
        BrainState = EOCBotBrainState::Vehicle;
        UpdateVehicleBrain(Vehicle);
        return;
    }
    if (AOCBotCharacter* Bot = Cast<AOCBotCharacter>(GetPawn())) UpdateInfantryBrain(Bot);
}

EOCTeam AOCAIController::GetBotTeam() const
{
    const AOCPlayerState* State = GetPlayerState<AOCPlayerState>();
    return State ? State->GetTeamId() : EOCTeam::None;
}

AOCCharacter* AOCAIController::FindBestEnemy(const AOCBotCharacter* Bot) const
{
    if (!Bot || !GetWorld()) return nullptr;
    TArray<AActor*> Perceived;
    if (BotPerception) BotPerception->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), Perceived);
    AOCCharacter* Best = nullptr;
    double BestSq = TNumericLimits<double>::Max();
    const EOCTeam MyTeam = GetBotTeam();
    auto Consider = [&](AOCCharacter* Candidate)
    {
        if (!Candidate || Candidate==Bot || !Candidate->GetHealthComponent() || !Candidate->GetHealthComponent()->IsAlive()) return;
        const AOCPlayerState* State = Candidate->GetPlayerState<AOCPlayerState>();
        if (!State || State->GetTeamId()==EOCTeam::None || State->GetTeamId()==MyTeam) return;
        const double D = FVector::DistSquared(Bot->GetActorLocation(), Candidate->GetActorLocation());
        if (D < BestSq && D <= FMath::Square(Tuning.MaxCombatRange)) { BestSq=D; Best=Candidate; }
    };
    for (AActor* Actor : Perceived)
    {
        AOCCharacter* Candidate = Cast<AOCCharacter>(Actor);
        if (Candidate && HasClearLineOfSightTo(Bot, Candidate)) Consider(Candidate);
    }
    if (!Best)
    {
        // Conservative fallback for source-only maps: keep the configured sight range/FOV even if
        // a Pawn was not registered in Perception yet. This prevents 360-degree omniscience.
        const FVector Forward = Bot->GetActorForwardVector().GetSafeNormal2D();
        const float MinDot = FMath::Cos(FMath::DegreesToRadians(Tuning.PeripheralVisionDegrees));
        for (TActorIterator<AOCCharacter> It(GetWorld()); It; ++It)
        {
            AOCCharacter* Candidate=*It;
            if (!Candidate || Candidate==Bot) continue;
            const FVector ToCandidate = (Candidate->GetActorLocation()-Bot->GetActorLocation()).GetSafeNormal2D();
            if (FVector::DotProduct(Forward,ToCandidate) < MinDot) continue;
            if (FVector::DistSquared(Bot->GetActorLocation(),Candidate->GetActorLocation()) > FMath::Square(Tuning.SightRadius)) continue;
            if (HasClearLineOfSightTo(Bot,Candidate)) Consider(Candidate);
        }
    }
    return Best;
}

bool AOCAIController::HasClearLineOfSightTo(const AOCCharacter* Bot, const AOCCharacter* Target) const
{
    if (!Bot || !Target || !GetWorld()) return false;
    const FVector Start = Bot->GetActorLocation() + FVector(0,0,62);
    const FVector End = Target->GetActorLocation() + FVector(0,0,55);

    // Source smoke is non-colliding presentation geometry, so AI LOS must explicitly honor its gameplay radius.
    const FVector Segment = End - Start;
    const float SegmentLengthSq = Segment.SizeSquared();
    for (TActorIterator<AOCSmokeCloud> It(GetWorld()); It; ++It)
    {
        const AOCSmokeCloud* Smoke = *It;
        if (!Smoke) continue;
        const FVector ToSmoke = Smoke->GetActorLocation() - Start;
        const float Alpha = SegmentLengthSq > KINDA_SMALL_NUMBER
            ? FMath::Clamp(FVector::DotProduct(ToSmoke, Segment) / SegmentLengthSq, 0.0f, 1.0f)
            : 0.0f;
        const FVector Closest = Start + Segment * Alpha;
        if (FVector::DistSquared(Closest, Smoke->GetActorLocation()) <= FMath::Square(Smoke->GetSmokeRadiusCm()))
        {
            return false;
        }
    }

    FCollisionQueryParams Params(SCENE_QUERY_STAT(OCBotLOS), true, Bot);
    FHitResult Hit;
    const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
    return !bHit || Hit.GetActor() == Target;
}

AOCCharacter* AOCAIController::FindReviveCandidate(const AOCBotCharacter* Bot) const
{
    if (!Bot || !GetWorld()) return nullptr;
    const AOCPlayerState* MyState = GetPlayerState<AOCPlayerState>();
    if (!MyState || !MyState->IsMedic()) return nullptr;
    AOCCharacter* Best=nullptr; double BestSq=FMath::Square(Tuning.ReviveSearchRadius);
    for (TActorIterator<AOCCharacter> It(GetWorld()); It; ++It)
    {
        AOCCharacter* Candidate=*It;
        if (!Candidate || Candidate==Bot || !Candidate->IsDowned()) continue;
        const AOCPlayerState* Other=Candidate->GetPlayerState<AOCPlayerState>();
        if (!Other || Other->GetTeamId()!=MyState->GetTeamId()) continue;
        const double D=FVector::DistSquared(Bot->GetActorLocation(),Candidate->GetActorLocation());
        if (D<BestSq){BestSq=D;Best=Candidate;}
    }
    return Best;
}

AOCCapturePoint* AOCAIController::ChooseObjective(const AOCBotCharacter* Bot) const
{
    if (!Bot || !GetWorld()) return nullptr;
    const EOCTeam Team=GetBotTeam();
    const AOCPlayerState* MyState = GetPlayerState<AOCPlayerState>();
    FOCSquadOrder SquadOrder;
    if (MyState)
    {
        if (const AOCGameMode* GM = GetWorld()->GetAuthGameMode<AOCGameMode>())
        {
            if (GM->GetSquadOrderFor(Team, MyState->GetSquadId(), SquadOrder) &&
                (SquadOrder.Type == EOCSquadOrderType::AttackObjective || SquadOrder.Type == EOCSquadOrderType::DefendObjective))
            {
                for (TActorIterator<AOCCapturePoint> It(GetWorld()); It; ++It)
                    if (It->GetPointId() == SquadOrder.ObjectiveId) return *It;
            }
        }
    }
    AOCCapturePoint* Best=nullptr; double BestScore=TNumericLimits<double>::Max();
    for (TActorIterator<AOCCapturePoint> It(GetWorld()); It; ++It)
    {
        AOCCapturePoint* Point=*It; if(!Point)continue;
        const double DistSq=FVector::DistSquared(Bot->GetActorLocation(),Point->GetActorLocation());
        double Score=DistSq;
        if(Point->GetOwnerTeam()!=Team) Score*=0.48;
        if(Point->IsContested()) Score*=0.42;
        if(Score<BestScore){BestScore=Score;Best=Point;}
    }
    return Best;
}

bool AOCAIController::FindSimpleCoverPoint(const AOCBotCharacter* Bot, const AOCCharacter* Enemy, FVector& OutPoint) const
{
    if(!Bot||!Enemy||!GetWorld())return false;
    UNavigationSystemV1* Nav=UNavigationSystemV1::GetCurrent(GetWorld()); if(!Nav)return false;
    const FVector EnemyEye=Enemy->GetActorLocation()+FVector(0,0,65);
    const FVector Origin=Bot->GetActorLocation();
    for(int32 I=0;I<12;++I)
    {
        const float Angle=(360.0f/12.0f)*I+FMath::FRandRange(-10.0f,10.0f);
        const float Radius=FMath::FRandRange(Tuning.CoverSearchRadius*0.55f,Tuning.CoverSearchRadius);
        const FVector Candidate=Origin+FRotator(0,Angle,0).Vector()*Radius;
        FNavLocation Projected;
        if(!Nav->ProjectPointToNavigation(Candidate,Projected,FVector(220,220,300)))continue;
        FCollisionQueryParams Params(SCENE_QUERY_STAT(OCBotCover),true,Enemy);
        FHitResult Hit;
        const bool bBlocked=GetWorld()->LineTraceSingleByChannel(Hit,EnemyEye,Projected.Location+FVector(0,0,55),ECC_Visibility,Params);
        if(bBlocked && Hit.GetActor()!=Bot){OutPoint=Projected.Location;return true;}
    }
    return false;
}

void AOCAIController::EngageEnemy(AOCBotCharacter* Bot, AOCCharacter* Enemy)
{
    if(!Bot||!Enemy)return;
    BrainState=EOCBotBrainState::Combat;
    SetFocus(Enemy);
    const float Distance=FVector::Dist(Bot->GetActorLocation(),Enemy->GetActorLocation());
    const bool bVisible=HasClearLineOfSightTo(Bot,Enemy);
    if(!bVisible){MoveBotTo(Bot,Enemy->GetActorLocation(),450.0f);return;}
    if(Distance>Tuning.PreferredCombatRange*1.25f) MoveBotTo(Bot,Enemy->GetActorLocation(),Tuning.PreferredCombatRange);
    else if(GetWorld()->GetTimeSeconds()>=NextCoverSearchTime)
    {
        NextCoverSearchTime=GetWorld()->GetTimeSeconds()+FMath::FRandRange(1.8f,3.8f);
        FVector Cover;
        if(FindSimpleCoverPoint(Bot,Enemy,Cover)) MoveBotTo(Bot,Cover,130.0f);
        else StopMovement();
    }
    AOCWeaponBase* Weapon=Bot->GetCurrentWeapon(); if(!Weapon)return;
    if(Weapon->GetAmmoInMagazine()<=0){Weapon->BeginReloadServer();return;}
    if(GetWorld()->GetTimeSeconds()-TargetAcquiredTime<Tuning.ReactionSeconds)return;
    const FVector Origin=Bot->GetActorLocation()+FVector(0,0,62);
    FVector Dir=(Enemy->GetActorLocation()+FVector(0,0,55)-Origin).GetSafeNormal();
    Dir=FMath::VRandCone(Dir,FMath::DegreesToRadians(Tuning.AimErrorDegrees));
    SetControlRotation(Dir.Rotation());
    FHitResult Hit; bool bDamaged=false,bFatal=false;
    Weapon->TryFireServer(Bot,Origin,Dir,true,Bot->GetVelocity().SizeSquared()>FMath::Square(80.0f),Hit,bDamaged,bFatal);
}

void AOCAIController::MoveTowardObjective(AOCBotCharacter* Bot, AOCCapturePoint* Objective)
{
    if(!Bot||!Objective)return;
    BrainState=EOCBotBrainState::Objective;
    ClearFocus(EAIFocusPriority::Gameplay);
    CachedVehicleObjective=Objective->GetActorLocation();
    MoveBotTo(Bot,Objective->GetActorLocation(),360.0f);
}

void AOCAIController::MoveBotTo(AOCBotCharacter* Bot, const FVector& Destination, float AcceptanceRadius)
{
    if (!Bot || !GetWorld()) return;
    if (UNavigationSystemV1* Nav = UNavigationSystemV1::GetCurrent(GetWorld()))
    {
        FNavLocation Projected;
        if (Nav->ProjectPointToNavigation(Destination, Projected, FVector(500.0f, 500.0f, 450.0f)))
        {
            MoveToLocation(Projected.Location, AcceptanceRadius, true, true);
            return;
        }
    }
    // Source-only fallback when the runtime map has not produced NavData yet.
    // CharacterMovement collision still applies; production map replaces this fallback with baked/dynamic NavMesh.
    const FVector Direction = (Destination - Bot->GetActorLocation()).GetSafeNormal2D();
    if (!Direction.IsNearlyZero())
    {
        SetControlRotation(Direction.Rotation());
        Bot->AddMovementInput(Direction, 1.0f, true);
    }
}

void AOCAIController::TryUseDoorAhead(AOCBotCharacter* Bot)
{
    if(!Bot||!GetWorld()||GetWorld()->GetTimeSeconds()<NextDoorCheckTime)return;
    NextDoorCheckTime=GetWorld()->GetTimeSeconds()+0.65f;
    const FVector Start=Bot->GetActorLocation()+FVector(0,0,55);
    FVector Forward=Bot->GetVelocity().GetSafeNormal2D(); if(Forward.IsNearlyZero())Forward=Bot->GetActorForwardVector();
    FHitResult Hit; FCollisionQueryParams Params(SCENE_QUERY_STAT(OCBotDoor),false,Bot);
    if(GetWorld()->LineTraceSingleByChannel(Hit,Start,Start+Forward*260.0f,ECC_Visibility,Params))
    {
        if(AOCInteractableDoor* Door=Cast<AOCInteractableDoor>(Hit.GetActor())){if(Door->CanInteractServer(Bot))Door->InteractServer(Bot);}
        else if(AOCInteractableGate* Gate=Cast<AOCInteractableGate>(Hit.GetActor())){if(Gate->CanInteractServer(Bot))Gate->InteractServer(Bot);}
    }
}

AOCVehicleBase* AOCAIController::FindUsefulVehicle(const AOCBotCharacter* Bot, const FVector& ObjectiveLocation) const
{
    if(!Bot||!GetWorld())return nullptr;
    if(FVector::DistSquared(Bot->GetActorLocation(),ObjectiveLocation)<FMath::Square(Tuning.VehicleUseObjectiveDistance))return nullptr;
    AOCVehicleBase* Best=nullptr; double BestSq=FMath::Square(Tuning.VehicleSeekDistance);
    for(TActorIterator<AOCVehicleBase> It(GetWorld());It;++It)
    {
        AOCVehicleBase* V=*It;
        if(!V||V->HasDriver()||V->IsVehicleDestroyed())continue;
        const double D=FVector::DistSquared(Bot->GetActorLocation(),V->GetActorLocation());
        if(D<BestSq){BestSq=D;Best=V;}
    }
    return Best;
}

void AOCAIController::UpdateInfantryBrain(AOCBotCharacter* Bot)
{
    if(!Bot||!Bot->GetHealthComponent()||!Bot->GetHealthComponent()->IsAlive()){StopMovement();return;}
    TryUseDoorAhead(Bot);
    AOCCharacter* Enemy=FindBestEnemy(Bot);
    AOCCharacter* MedicTarget=FindReviveCandidate(Bot);
    if(MedicTarget && (!Enemy || FVector::DistSquared(Bot->GetActorLocation(),Enemy->GetActorLocation())>FMath::Square(1800.0f)))
    {
        BrainState=EOCBotBrainState::Revive; ReviveTarget=MedicTarget; StopCombat();
        const float Dist=FVector::Dist(Bot->GetActorLocation(),MedicTarget->GetActorLocation());
        if(Dist>190.0f)
        {
            Bot->CancelAIReviveServer();
            MoveBotTo(Bot,MedicTarget->GetActorLocation(),165.0f);
        }
        else
        {
            StopMovement();
            Bot->StartAIReviveServer(MedicTarget);
        }
        return;
    }
    Bot->CancelAIReviveServer();
    ReviveTarget=nullptr;
    if(Enemy){if(CombatTarget!=Enemy){CombatTarget=Enemy;TargetAcquiredTime=GetWorld()->GetTimeSeconds();}EngageEnemy(Bot,Enemy);return;}
    StopCombat();
    if (const AOCPlayerState* MyState = GetPlayerState<AOCPlayerState>())
    {
        FOCSquadOrder Order;
        if (const AOCGameMode* GM = GetWorld()->GetAuthGameMode<AOCGameMode>();
            GM && GM->GetSquadOrderFor(MyState->GetTeamId(), MyState->GetSquadId(), Order) &&
            (Order.Type == EOCSquadOrderType::Move || Order.Type == EOCSquadOrderType::Regroup))
        {
            BrainState = EOCBotBrainState::Objective;
            MoveBotTo(Bot, Order.WorldLocation, Order.Type == EOCSquadOrderType::Regroup ? 260.0f : 180.0f);
            return;
        }
    }
    ObjectiveTarget=ChooseObjective(Bot);
    if(ObjectiveTarget)
    {
        if(GetWorld()->GetTimeSeconds()>=NextVehicleSearchTime)
        {
            NextVehicleSearchTime=GetWorld()->GetTimeSeconds()+3.0;
            if(AOCVehicleBase* Vehicle=FindUsefulVehicle(Bot,ObjectiveTarget->GetActorLocation()))
            {
                if(FVector::DistSquared(Bot->GetActorLocation(),Vehicle->GetActorLocation())<=FMath::Square(360.0f))
                {CachedVehicleObjective=ObjectiveTarget->GetActorLocation();if(Vehicle->TryEnterVehicleServer(Bot))return;}
                else {MoveBotTo(Bot,Vehicle->GetActorLocation(),250.0f);return;}
            }
        }
        MoveTowardObjective(Bot,ObjectiveTarget);
    }
}

void AOCAIController::UpdateVehicleBrain(AOCVehicleBase* Vehicle)
{
    if(!Vehicle||Vehicle->IsVehicleDestroyed())return;
    if(CachedVehicleObjective.IsNearlyZero())
    {
        if(AOCCharacter* Driver=Vehicle->GetDriverCharacter()) if(AOCCapturePoint* P=ChooseObjective(Cast<AOCBotCharacter>(Driver))) CachedVehicleObjective=P->GetActorLocation();
    }
    const FVector FlatTo=(CachedVehicleObjective-Vehicle->GetActorLocation()).GetSafeNormal2D();
    if(FlatTo.IsNearlyZero()){Vehicle->SetAIDriveInputsServer(0,0,true);return;}
    const float TargetYaw=FlatTo.Rotation().Yaw;
    const float DeltaYaw=FMath::FindDeltaAngleDegrees(Vehicle->GetActorRotation().Yaw,TargetYaw);
    const float Steering=FMath::Clamp(DeltaYaw/42.0f,-1.0f,1.0f);
    const float Distance=FVector::Dist2D(Vehicle->GetActorLocation(),CachedVehicleObjective);
    const bool bSharp=FMath::Abs(DeltaYaw)>105.0f;
    const float Throttle=bSharp?0.28f:1.0f;
    Vehicle->SetAIDriveInputsServer(Throttle,Steering,bSharp);
    if(Distance<950.0f && Vehicle->GetSpeedKmh()<18.0f) Vehicle->AIRequestExitServer();
}

void AOCAIController::StopCombat()
{
    CombatTarget=nullptr;
    ClearFocus(EAIFocusPriority::Gameplay);
}
