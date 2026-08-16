#include "OCTeamSpawnPoint.h"

#include "OCCapturePoint.h"
#include "EngineUtils.h"
#include "Engine/World.h"

AOCTeamSpawnPoint::AOCTeamSpawnPoint(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
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
