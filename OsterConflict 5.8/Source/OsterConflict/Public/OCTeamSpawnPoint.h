#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "OCTeamTypes.h"
#include "OCTeamSpawnPoint.generated.h"

UCLASS()
class OSTERCONFLICT_API AOCTeamSpawnPoint : public APlayerStart
{
    GENERATED_BODY()

public:
    AOCTeamSpawnPoint(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintPure, Category="Spawn")
    EOCTeam GetTeamId() const { return TeamId; }

    UFUNCTION(BlueprintPure, Category="Spawn")
    FName GetLinkedCapturePointId() const { return LinkedCapturePointId; }

    UFUNCTION(BlueprintPure, Category="Spawn")
    bool IsBaseSpawn() const { return bBaseSpawn; }

    /** Server-only helper for source-only prototype setup. */
    void ConfigureServer(EOCTeam InTeam, bool bInBaseSpawn, FName InLinkedCapturePointId = NAME_None);

    /** A base spawn is always available to its team; a forward spawn requires ownership of its linked point. */
    bool IsAvailableForTeam(EOCTeam RequestedTeam) const;

protected:
    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Spawn")
    EOCTeam TeamId = EOCTeam::None;

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Spawn")
    bool bBaseSpawn = true;

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Spawn", meta=(EditCondition="!bBaseSpawn"))
    FName LinkedCapturePointId = NAME_None;
};
