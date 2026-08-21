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
        // RUNTIME 2026-08-22: deployment is intentionally staged around the museum test hub.
        // Keep the player outside the museum footprint while making the verified museum landmark,
        // the test weapon rack and nearby vehicle checks immediately reachable after deployment.
        const FVector Museum = AOCWorldSectorOster::MuseumAnchor();
        if (Team == EOCTeam::TeamTwo)
        {
            return Museum + (bSecondary
                ? FVector(3200.0f, 1000.0f, 120.0f)
                : FVector(2200.0f, 1700.0f, 120.0f));
        }
        return Museum + (bSecondary
            ? FVector(-3200.0f, -1000.0f, 120.0f)
            : FVector(-2200.0f, -1700.0f, 120.0f));
    }

    float ResolveCanonicalBaseYaw(EOCTeam Team)
    {
        // Spawn groups face back toward the museum/test hub rather than toward the map edge.
        return Team == EOCTeam::TeamTwo ? -142.0f : 38.0f;
    }

    bool HasRackForTeam(UWorld* World, EOCTeam Team)
    {
        if (!World) return true;
        for (TActorIterator<AOCTeamSpawnPoint> It(World); It; ++It)
        {
            const AOCTeamSpawnPoint* Point = *It;
            if (Point && Point->GetTeamId() == Team && Point->ActorHasTag(RuntimeBaseRackTag))
            {
                return true;
            }
        }
        return false;
    }

    void SpawnRuntimeBaseWeaponRack(AOCTeamSpawnPoint& OwnerPoint, EOCTeam Team)
    {
        UWorld* World = OwnerPoint.GetWorld();
        if (!World || HasRackForTeam(World, Team)) return;

        OwnerPoint.Tags.Add(RuntimeBaseRackTag);

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
        for (int32 Index = 0; Index < UE_ARRAY_COUNT(WeaponClasses); ++Index)
        {
            const int32 Row = Index / 6;
            const int32 Column = Index % 6;
            const float Along = 650.0f + Column * 185.0f;
            const float AwayFromRoad = 360.0f + Row * 230.0f;
            FVector Desired = Base + Forward * Along + Right * AwayFromRoad;
            Desired = SnapLocationToWalkableSurface(World, Desired, 72.0f);

            AOCWeaponBase* Weapon = World->SpawnActor<AOCWeaponBase>(WeaponClasses[Index], Desired, FacingRotation, SpawnParams);
            if (!Weapon) continue;

            Weapon->Tags.Add(RuntimeBaseRackTag);
            Weapon->DropToWorldServer(Desired, FacingRotation);
            ++Spawned;
        }

        UE_LOG(LogTemp, Display,
            TEXT("Runtime BASE spawn rack created: team=%s weapons=%d location=%s"),
            *OCTeamToString(Team), Spawned, *Base.ToCompactString());
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

    // Serialized PlayerStarts can enter play without being rebuilt through ConfigureServer. That was
    // the reason an old far-away BASE could survive source-side relocation code. Runtime now treats
    // the museum anchor as authoritative too, not merely the source builder that created the actor.
    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();
    if (FVector::DistSquared2D(GetActorLocation(), Museum) > FMath::Square(6000.0f))
    {
        ConfigureServer(TeamId, true, NAME_None);
    }
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
        // Preserve the old pair ordering only to choose primary/secondary. The actual legacy coordinates
        // are no longer authoritative after this point.
        const FVector LegacyLocation = GetActorLocation();
        const bool bSecondary = TeamId == EOCTeam::TeamTwo
            ? LegacyLocation.Y > 92000.0f
            : LegacyLocation.Y < -92000.0f;

        FVector NewLocation = ResolveCanonicalBaseLocation(TeamId, bSecondary);
        NewLocation = SnapLocationToWalkableSurface(GetWorld(), NewLocation, 95.0f);
        const FRotator NewRotation(0.0f, ResolveCanonicalBaseYaw(TeamId), 0.0f);
        SetActorLocationAndRotation(NewLocation, NewRotation, false, nullptr, ETeleportType::TeleportPhysics);

        UE_LOG(LogTemp, Display,
            TEXT("BASE spawn relocated to museum test hub: team=%s secondary=%d old=%s new=%s"),
            *OCTeamToString(TeamId), bSecondary ? 1 : 0,
            *LegacyLocation.ToCompactString(), *NewLocation.ToCompactString());

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
