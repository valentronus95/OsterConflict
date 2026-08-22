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
        // Runtime acceptance: BASE means the museum test hub, not the old map-edge base. Keep both
        // teams outside the building footprint but close enough that the museum is immediately visible.
        const FVector Museum = AOCWorldSectorOster::MuseumAnchor();
        if (Team == EOCTeam::TeamTwo)
        {
            return Museum + (bSecondary
                ? FVector(2200.0f, 1500.0f, 120.0f)
                : FVector(1450.0f, 900.0f, 120.0f));
        }
        return Museum + (bSecondary
            ? FVector(-2200.0f, -1500.0f, 120.0f)
            : FVector(-1450.0f, -900.0f, 120.0f));
    }

    float ResolveCanonicalBaseYaw(EOCTeam Team)
    {
        return Team == EOCTeam::TeamTwo ? -148.0f : 32.0f;
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
        for (int32 Index = 0; Index < UE_ARRAY_COUNT(WeaponClasses); ++Index)
        {
            const int32 Row = Index / 6;
            const int32 Column = Index % 6;
            const float Along = 420.0f + Column * 150.0f;
            const float Side = 430.0f + Row * 230.0f;
            FVector Desired = Base + Forward * Along + Right * Side;
            Desired = SnapLocationToWalkableSurface(World, Desired, 72.0f);

            AOCWeaponBase* Weapon = World->SpawnActor<AOCWeaponBase>(WeaponClasses[Index], Desired, FacingRotation, SpawnParams);
            if (!Weapon) continue;

            Weapon->Tags.Add(RuntimeBaseRackTag);
            Weapon->DropToWorldServer(Desired, FacingRotation);
            ++Spawned;
        }

        UE_LOG(LogTemp, Display,
            TEXT("Runtime BASE weapon rack rebuilt beside museum: team=%s removed_partial=%d weapons=%d location=%s"),
            *OCTeamToString(Team), ExistingCount, Spawned, *Base.ToCompactString());
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

    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();
    if (FVector::DistSquared2D(GetActorLocation(), Museum) > FMath::Square(3500.0f))
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
        const FVector LegacyLocation = GetActorLocation();
        const bool bSecondary = TeamId == EOCTeam::TeamTwo
            ? LegacyLocation.Y > 92000.0f
            : LegacyLocation.Y < -92000.0f;

        FVector NewLocation = ResolveCanonicalBaseLocation(TeamId, bSecondary);
        NewLocation = SnapLocationToWalkableSurface(GetWorld(), NewLocation, 95.0f);
        const FRotator NewRotation(0.0f, ResolveCanonicalBaseYaw(TeamId), 0.0f);
        SetActorLocationAndRotation(NewLocation, NewRotation, false, nullptr, ETeleportType::TeleportPhysics);

        UE_LOG(LogTemp, Display,
            TEXT("BASE spawn relocated beside museum: team=%s secondary=%d old=%s new=%s museum=%s"),
            *OCTeamToString(TeamId), bSecondary ? 1 : 0,
            *LegacyLocation.ToCompactString(), *NewLocation.ToCompactString(),
            *AOCWorldSectorOster::MuseumAnchor().ToCompactString());

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
