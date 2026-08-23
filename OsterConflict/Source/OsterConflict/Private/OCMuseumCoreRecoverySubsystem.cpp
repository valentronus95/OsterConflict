#include "OCMuseumCoreRecoverySubsystem.h"

#include "OCGameMode.h"
#include "OCR138MuseumInteractiveArchitectureSubsystem.h"
#include "OCR139MuseumMainDoorReplacementSubsystem.h"
#include "OCR140MuseumFacadeDetailSubsystem.h"
#include "OCR141MuseumWindowReplacementSubsystem.h"
#include "OCR142MuseumEntranceDetailSubsystem.h"
#include "OCR143MuseumSiteVegetationSubsystem.h"
#include "OCR144MuseumRearExteriorDetailSubsystem.h"
#include "OCR145MuseumTreeLayoutSubsystem.h"
#include "OCTeamSpawnPoint.h"
#include "OCWorldSectorOster.h"

#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    constexpr float RecoveryDelaySeconds = 0.65f;
    constexpr float MuseumNoSpawnRadiusCm = 3000.0f;
    constexpr float MuseumNearbyBaseRadiusCm = 6000.0f;

    const FName MuseumPrototypeTag(TEXT("R137_MuseumPhotoModel"));
    const FName MuseumArchitectureTag(TEXT("R138_MuseumHighFidelityArchitecture"));
    const FName RecoveryCarrierTag(TEXT("PASS35_MuseumRecoveryCarrier"));

    int32 CountActorsWithTag(UWorld& World, const FName Tag)
    {
        int32 Count = 0;
        for (TActorIterator<AActor> It(&World); It; ++It)
        {
            const AActor* Actor = *It;
            if (Actor && !Actor->IsActorBeingDestroyed() && Actor->ActorHasTag(Tag)) ++Count;
        }
        return Count;
    }

    AActor* FindActorWithTag(UWorld& World, const FName Tag)
    {
        for (TActorIterator<AActor> It(&World); It; ++It)
        {
            AActor* Actor = *It;
            if (Actor && !Actor->IsActorBeingDestroyed() && Actor->ActorHasTag(Tag)) return Actor;
        }
        return nullptr;
    }

    AActor* EnsurePrototypeCarrier(UWorld& World)
    {
        if (AActor* Existing = FindActorWithTag(World, MuseumPrototypeTag)) return Existing;

        FActorSpawnParameters Params;
        Params.Name = MakeUniqueObjectName(&World, AActor::StaticClass(), FName(TEXT("R137_MuseumRecoveryCarrier")));
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AActor* Carrier = World.SpawnActor<AActor>(
            AActor::StaticClass(),
            FTransform(FRotator::ZeroRotator, AOCWorldSectorOster::MuseumAnchor()),
            Params);
        if (!Carrier) return nullptr;

        Carrier->SetReplicates(false);
        Carrier->Tags.Add(MuseumPrototypeTag);
        Carrier->Tags.Add(RecoveryCarrierTag);

        USceneComponent* Root = NewObject<USceneComponent>(
            Carrier,
            MakeUniqueObjectName(Carrier, USceneComponent::StaticClass(), FName(TEXT("Pass35MuseumCarrierRoot"))));
        if (!Root)
        {
            Carrier->Destroy();
            return nullptr;
        }
        Root->SetMobility(EComponentMobility::Static);
        Carrier->SetRootComponent(Root);
        Carrier->AddInstanceComponent(Root);
        Root->RegisterComponent();

        UE_LOG(LogTemp, Warning,
            TEXT("PASS35_MUSEUM_OWNER_CARRIER_RECOVERED location=%s reason=R137_owner_missing"),
            *AOCWorldSectorOster::MuseumAnchor().ToCompactString());
        return Carrier;
    }

    void ReplayMuseumDetails(UWorld& World)
    {
        const bool bAuthority = World.GetNetMode() != NM_Client;

        if (bAuthority)
        {
            if (UOCR139MuseumMainDoorReplacementSubsystem* Stage =
                World.GetSubsystem<UOCR139MuseumMainDoorReplacementSubsystem>())
            {
                Stage->RunAuthoritativeDetailNow(World);
            }
        }
        if (UOCR140MuseumFacadeDetailSubsystem* Stage = World.GetSubsystem<UOCR140MuseumFacadeDetailSubsystem>())
            Stage->RunAuthoritativeDetailNow(World);
        if (bAuthority)
        {
            if (UOCR141MuseumWindowReplacementSubsystem* Stage =
                World.GetSubsystem<UOCR141MuseumWindowReplacementSubsystem>())
            {
                Stage->RunAuthoritativeDetailNow(World);
            }
        }
        if (UOCR142MuseumEntranceDetailSubsystem* Stage = World.GetSubsystem<UOCR142MuseumEntranceDetailSubsystem>())
            Stage->RunAuthoritativeDetailNow(World);
        if (UOCR143MuseumSiteVegetationSubsystem* Stage = World.GetSubsystem<UOCR143MuseumSiteVegetationSubsystem>())
            Stage->RunAuthoritativeDetailNow(World);
        if (UOCR144MuseumRearExteriorDetailSubsystem* Stage = World.GetSubsystem<UOCR144MuseumRearExteriorDetailSubsystem>())
            Stage->RunAuthoritativeDetailNow(World);
        if (UOCR145MuseumTreeLayoutSubsystem* Stage = World.GetSubsystem<UOCR145MuseumTreeLayoutSubsystem>())
            Stage->RunAuthoritativeDetailNow(World);

        UE_LOG(LogTemp, Display, TEXT("PASS35_MUSEUM_DETAIL_REPLAY_COMPLETE"));
    }

    void LogBaseDistances(UWorld& World)
    {
        const FVector Museum = AOCWorldSectorOster::MuseumAnchor();
        float BestTeamOneCm = TNumericLimits<float>::Max();
        float BestTeamTwoCm = TNumericLimits<float>::Max();

        for (TActorIterator<AOCTeamSpawnPoint> It(&World); It; ++It)
        {
            const AOCTeamSpawnPoint* Point = *It;
            if (!Point || !Point->IsBaseSpawn()) continue;
            const float DistanceCm = FVector::Dist2D(Point->GetActorLocation(), Museum);
            if (Point->GetTeamId() == EOCTeam::TeamOne) BestTeamOneCm = FMath::Min(BestTeamOneCm, DistanceCm);
            if (Point->GetTeamId() == EOCTeam::TeamTwo) BestTeamTwoCm = FMath::Min(BestTeamTwoCm, DistanceCm);
        }

        const bool bTeamOneReady = BestTeamOneCm >= MuseumNoSpawnRadiusCm && BestTeamOneCm <= MuseumNearbyBaseRadiusCm;
        const bool bTeamTwoReady = BestTeamTwoCm >= MuseumNoSpawnRadiusCm && BestTeamTwoCm <= MuseumNearbyBaseRadiusCm;
        if (bTeamOneReady && bTeamTwoReady)
        {
            UE_LOG(LogTemp, Display,
                TEXT("PASS35_MUSEUM_BASE_DISTANCE_READY team1_m=%.1f team2_m=%.1f min_m=%.1f max_m=%.1f"),
                BestTeamOneCm / 100.0f,
                BestTeamTwoCm / 100.0f,
                MuseumNoSpawnRadiusCm / 100.0f,
                MuseumNearbyBaseRadiusCm / 100.0f);
        }
        else
        {
            UE_LOG(LogTemp, Error,
                TEXT("PASS35_MUSEUM_BASE_DISTANCE_FAIL team1_m=%.1f team2_m=%.1f"),
                BestTeamOneCm == TNumericLimits<float>::Max() ? -1.0f : BestTeamOneCm / 100.0f,
                BestTeamTwoCm == TNumericLimits<float>::Max() ? -1.0f : BestTeamTwoCm / 100.0f);
        }
    }
}

bool UOCMuseumCoreRecoverySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCMuseumCoreRecoverySubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    InWorld.GetTimerManager().SetTimer(
        RecoveryTimer,
        this,
        &UOCMuseumCoreRecoverySubsystem::EnsureMuseumCore,
        RecoveryDelaySeconds,
        false);
}

void UOCMuseumCoreRecoverySubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(RecoveryTimer);
    Super::Deinitialize();
}

void UOCMuseumCoreRecoverySubsystem::EnsureMuseumCore()
{
    UWorld* World = GetWorld();
    if (!World) return;

    int32 PrototypeOwners = CountActorsWithTag(*World, MuseumPrototypeTag);
    int32 ArchitectureOwners = CountActorsWithTag(*World, MuseumArchitectureTag);

    if (ArchitectureOwners == 0)
    {
        if (PrototypeOwners == 0 && !EnsurePrototypeCarrier(*World))
        {
            UE_LOG(LogTemp, Error, TEXT("PASS35_MUSEUM_CORE_FAIL reason=carrier_spawn_failed"));
            return;
        }

        if (UOCR138MuseumInteractiveArchitectureSubsystem* Architecture =
            World->GetSubsystem<UOCR138MuseumInteractiveArchitectureSubsystem>())
        {
            Architecture->RunAuthoritativeUpgradeNow(*World);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("PASS35_MUSEUM_CORE_FAIL reason=R138_subsystem_missing"));
            return;
        }

        ReplayMuseumDetails(*World);
        PrototypeOwners = CountActorsWithTag(*World, MuseumPrototypeTag);
        ArchitectureOwners = CountActorsWithTag(*World, MuseumArchitectureTag);
    }

    LogBaseDistances(*World);

    if (PrototypeOwners == 1 && ArchitectureOwners == 1)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS35_MUSEUM_CORE_READY prototypeOwners=%d architectureOwners=%d anchor=%s recoveryCarrier=%d"),
            PrototypeOwners,
            ArchitectureOwners,
            *AOCWorldSectorOster::MuseumAnchor().ToCompactString(),
            FindActorWithTag(*World, RecoveryCarrierTag) ? 1 : 0);
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS35_MUSEUM_CORE_FAIL prototypeOwners=%d architectureOwners=%d"),
            PrototypeOwners,
            ArchitectureOwners);
    }
}
