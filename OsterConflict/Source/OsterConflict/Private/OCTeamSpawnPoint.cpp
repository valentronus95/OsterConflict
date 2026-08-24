#include "OCTeamSpawnPoint.h"

#include "OCAntiArmorLauncher.h"
#include "OCCapturePoint.h"
#include "OCWeaponBase.h"
#include "OCWeaponVariants.h"
#include "OCWorldSectorOster.h"
#include "EngineUtils.h"
#include "Engine/World.h"

namespace
{
    const FName RuntimeBaseRackTag(TEXT("OC_RuntimeBaseWeaponRack"));
    constexpr int32 RequiredRackWeaponCount = 11;
    constexpr float RackRecoveryRadiusCm = 1800.0f;
    constexpr float RackGroundClearanceCm = 12.0f;

    FVector SnapLocationToWalkableSurface(UWorld* World, const FVector& DesiredLocation, float HeightAboveSurfaceCm)
    {
        if (!World) return DesiredLocation;
        FHitResult Hit;
        FCollisionQueryParams Params(SCENE_QUERY_STAT(OCBaseSpawnGround), false);
        const FVector Start(DesiredLocation.X, DesiredLocation.Y, 8000.0f);
        const FVector End(DesiredLocation.X, DesiredLocation.Y, -3000.0f);
        if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
        {
            return FVector(DesiredLocation.X, DesiredLocation.Y, Hit.ImpactPoint.Z + HeightAboveSurfaceCm);
        }
        return DesiredLocation;
    }

    FVector ResolveCanonicalBaseLocation(EOCTeam Team, bool bSecondary)
    {
        // Pass 37: the 41 m Pass 30 BASE was collision-safe but the user runtime still read as an
        // empty distant field. Put the primary deployment on the open front approach at ~27.8 m from
        // MuseumAnchor: outside the authored shell/vestibule, but close enough that the museum must
        // dominate the initial view. Secondary remains farther out as a fallback, not the preferred BASE.
        const FVector Museum = AOCWorldSectorOster::MuseumAnchor();
        if (Team == EOCTeam::TeamTwo)
        {
            return Museum + (bSecondary
                ? FVector(2300.0f, -3100.0f, 120.0f)
                : FVector(1400.0f, -2400.0f, 120.0f));
        }
        return Museum + (bSecondary
            ? FVector(-2300.0f, -3100.0f, 120.0f)
            : FVector(-1400.0f, -2400.0f, 120.0f));
    }

    float ResolveCanonicalBaseYaw(EOCTeam Team)
    {
        // Face the museum directly from the front-side exterior BASE.
        return Team == EOCTeam::TeamTwo ? 120.0f : 60.0f;
    }

    int32 CollectRackWeaponsNear(UWorld* World, const FVector& BaseLocation, TArray<AOCWeaponBase*>* OutWeapons = nullptr)
    {
        if (!World) return 0;
        int32 Count = 0;
        for (TActorIterator<AOCWeaponBase> It(World); It; ++It)
        {
            AOCWeaponBase* Weapon = *It;
            if (!Weapon || !Weapon->ActorHasTag(RuntimeBaseRackTag)) continue;
            if (FVector::DistSquared2D(Weapon->GetActorLocation(), BaseLocation) > FMath::Square(RackRecoveryRadiusCm)) continue;
            ++Count;
            if (OutWeapons) OutWeapons->Add(Weapon);
        }
        return Count;
    }

    void SpawnRuntimeBaseWeaponRack(AOCTeamSpawnPoint& OwnerPoint, EOCTeam Team)
    {
        UWorld* World = OwnerPoint.GetWorld();
        if (!World) return;

        TArray<AOCWeaponBase*> ExistingRackWeapons;
        const int32 ExistingCount = CollectRackWeaponsNear(World, OwnerPoint.GetActorLocation(), &ExistingRackWeapons);
        if (ExistingCount >= RequiredRackWeaponCount) return;

        // A tag on the spawn point is not evidence that the rack still exists. Remove a partial rack
        // and rebuild all 11 actors so runtime state cannot claim success with missing physical weapons.
        for (AOCWeaponBase* Existing : ExistingRackWeapons)
        {
            if (IsValid(Existing)) Existing->Destroy();
        }
        if (!OwnerPoint.ActorHasTag(RuntimeBaseRackTag)) OwnerPoint.Tags.Add(RuntimeBaseRackTag);

        const TSubclassOf<AOCWeaponBase> WeaponClasses[] =
        {
            AOCWeapon_AssaultRifle::StaticClass(),
            AOCWeapon_SMG::StaticClass(),
            AOCWeapon_Pistol::StaticClass(),
            AOCWeapon_Sniper::StaticClass(),
            AOCWeapon_Shotgun::StaticClass(),
            AOCWeapon_LMG::StaticClass(),
            AOCWeapon_M14::StaticClass(),
            AOCWeapon_Mac10::StaticClass(),
            AOCWeapon_Tec9::StaticClass(),
            AOCWeapon_LeverAction::StaticClass(),
            AOCAntiArmorLauncher::StaticClass()
        };

        const FRotator FacingRotation(0.0f, ResolveCanonicalBaseYaw(Team), 0.0f);
        const FVector Forward = FacingRotation.Vector().GetSafeNormal();
        const FVector Right = FRotationMatrix(FacingRotation).GetScaledAxis(EAxis::Y).GetSafeNormal();
        const FVector Base = OwnerPoint.GetActorLocation();

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        int32 Spawned = 0;
        int32 Grounded = 0;
        float WorstClearanceCm = 0.0f;
        for (int32 Index = 0; Index < UE_ARRAY_COUNT(WeaponClasses); ++Index)
        {
            const int32 Row = Index / 6;
            const int32 Column = Index % 6;
            const float Along = 420.0f + Column * 150.0f;
            const float Side = 430.0f + Row * 230.0f;
            FVector Desired = Base + Forward * Along + Right * Side;
            Desired = SnapLocationToWalkableSurface(World, Desired, RackGroundClearanceCm);

            // Keep every pickup lying horizontally on the walkable surface. Small alternating yaw offsets
            // prevent identical silhouettes from visually clipping while preserving the two-row layout.
            const float RackYaw = ResolveCanonicalBaseYaw(Team) + ((Index % 2 == 0) ? -7.0f : 7.0f);
            const FRotator RackRotation(0.0f, RackYaw, 0.0f);
            AOCWeaponBase* Weapon = World->SpawnActor<AOCWeaponBase>(WeaponClasses[Index], Desired, RackRotation, SpawnParams);
            if (!Weapon) continue;

            Weapon->Tags.Add(RuntimeBaseRackTag);
            Weapon->DropToWorldServer(Desired, RackRotation);
            ++Spawned;

            FHitResult GroundHit;
            FCollisionQueryParams GroundParams(SCENE_QUERY_STAT(OCBaseRackGroundAudit), false, Weapon);
            const FVector TraceStart = Weapon->GetActorLocation() + FVector(0.0f, 0.0f, 80.0f);
            const FVector TraceEnd = Weapon->GetActorLocation() - FVector(0.0f, 0.0f, 160.0f);
            if (World->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_Visibility, GroundParams))
            {
                const float Clearance = FMath::Abs(Weapon->GetActorLocation().Z - GroundHit.ImpactPoint.Z);
                WorstClearanceCm = FMath::Max(WorstClearanceCm, Clearance);
                if (Clearance <= RackGroundClearanceCm + 3.0f) ++Grounded;
            }
        }

        UE_LOG(LogTemp, Display,
            TEXT("PASS37_RUNTIME_BASE_RACK_NEAR_MUSEUM team=%s removed_partial=%d weapons=%d location=%s museum_distance_m=%.1f"),
            *OCTeamToString(Team), ExistingCount, Spawned, *Base.ToCompactString(),
            FVector::Dist2D(Base, AOCWorldSectorOster::MuseumAnchor()) / 100.0f);

        if (Spawned == RequiredRackWeaponCount && Grounded == Spawned)
        {
            UE_LOG(LogTemp, Display,
                TEXT("PASS42_BASE_RACK_GROUNDED_READY team=%s weapons=%d clearance_cm=%.1f worst_cm=%.1f layout=two_rows"),
                *OCTeamToString(Team), Grounded, RackGroundClearanceCm, WorstClearanceCm);
        }
        else
        {
            UE_LOG(LogTemp, Warning,
                TEXT("PASS42_BASE_RACK_GROUNDING_INCOMPLETE team=%s spawned=%d grounded=%d worst_cm=%.1f"),
                *OCTeamToString(Team), Spawned, Grounded, WorstClearanceCm);
        }
    }
}

AOCTeamSpawnPoint::AOCTeamSpawnPoint(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void AOCTeamSpawnPoint::BeginPlay()
{
    Super::BeginPlay();

    if (!HasAuthority() || !bBaseSpawn || TeamId == EOCTeam::None || !GetWorld()) return;

    // Every serialized/legacy BASE is normalized through the single canonical resolver above.
    ConfigureServer(TeamId, true, NAME_None);
}

void AOCTeamSpawnPoint::ConfigureServer(EOCTeam InTeam, bool bInBaseSpawn, FName InLinkedCapturePointId)
{
    if (!HasAuthority())
    {
        return;
    }

    TeamId = InTeam;
    bBaseSpawn = bInBaseSpawn;
    LinkedCapturePointId = bBaseSpawn ? NAME_None : InLinkedCapturePointId;

    if (bBaseSpawn && TeamId != EOCTeam::None && GetWorld())
    {
        const FVector SeedLocation = GetActorLocation();

        // Pass 44: the old primary/secondary discriminator depended on the retired ±920 m map edges.
        // The first already-configured BASE for a team owns the primary slot; any later BASE is the
        // secondary fallback. Base identity is therefore independent of obsolete world coordinates.
        bool bSecondary = false;
        for (TActorIterator<AOCTeamSpawnPoint> It(GetWorld()); It; ++It)
        {
            const AOCTeamSpawnPoint* Other = *It;
            if (!IsValid(Other) || Other == this) continue;
            if (Other->bBaseSpawn && Other->TeamId == TeamId)
            {
                bSecondary = true;
                break;
            }
        }

        FVector NewLocation = ResolveCanonicalBaseLocation(TeamId, bSecondary);
        NewLocation = SnapLocationToWalkableSurface(GetWorld(), NewLocation, 95.0f);
        const FRotator NewRotation(0.0f, ResolveCanonicalBaseYaw(TeamId), 0.0f);
        SetActorLocationAndRotation(NewLocation, NewRotation, false, nullptr, ETeleportType::TeleportPhysics);

        UE_LOG(LogTemp, Display,
            TEXT("PASS44_BASE_ROLE_COORDINATE_INDEPENDENT_READY team=%s secondary=%d seed=%s new=%s museum=%s distance_m=%.1f legacy_edge_test=0"),
            *OCTeamToString(TeamId), bSecondary ? 1 : 0,
            *SeedLocation.ToCompactString(), *NewLocation.ToCompactString(),
            *AOCWorldSectorOster::MuseumAnchor().ToCompactString(),
            FVector::Dist2D(NewLocation, AOCWorldSectorOster::MuseumAnchor()) / 100.0f);

        UE_LOG(LogTemp, Display,
            TEXT("PASS37_BASE_RELOCATED_VISIBLE_MUSEUM_APPROACH team=%s secondary=%d old=%s new=%s museum=%s distance_m=%.1f"),
            *OCTeamToString(TeamId), bSecondary ? 1 : 0,
            *SeedLocation.ToCompactString(), *NewLocation.ToCompactString(),
            *AOCWorldSectorOster::MuseumAnchor().ToCompactString(),
            FVector::Dist2D(NewLocation, AOCWorldSectorOster::MuseumAnchor()) / 100.0f);

        // Keep the Pass 30 compatibility breadcrumb for older runtime parsers.
        UE_LOG(LogTemp, Display,
            TEXT("PASS30_BASE_RELOCATED_OUTSIDE_MUSEUM team=%s secondary=%d old=%s new=%s museum=%s distance_m=%.1f"),
            *OCTeamToString(TeamId), bSecondary ? 1 : 0,
            *SeedLocation.ToCompactString(), *NewLocation.ToCompactString(),
            *AOCWorldSectorOster::MuseumAnchor().ToCompactString(),
            FVector::Dist2D(NewLocation, AOCWorldSectorOster::MuseumAnchor()) / 100.0f);

        if (!bSecondary)
        {
            SpawnRuntimeBaseWeaponRack(*this, TeamId);
        }
    }
}

bool AOCTeamSpawnPoint::IsAvailableForTeam(EOCTeam RequestedTeam) const
{
    if (RequestedTeam == EOCTeam::None || TeamId != RequestedTeam)
    {
        return false;
    }

    if (bBaseSpawn)
    {
        return true;
    }

    if (LinkedCapturePointId.IsNone() || !GetWorld())
    {
        return false;
    }

    for (TActorIterator<AOCCapturePoint> It(GetWorld()); It; ++It)
    {
        const AOCCapturePoint* Point = *It;
        if (Point && Point->GetPointId() == LinkedCapturePointId)
        {
            return Point->GetOwnerTeam() == RequestedTeam && !Point->IsContested();
        }
    }
    return false;
}
