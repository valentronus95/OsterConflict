#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "OCHUD.generated.h"

UCLASS()
class OSTERCONFLICT_API AOCHUD : public AHUD
{
    GENERATED_BODY()

public:
    virtual void DrawHUD() override;

private:
    void DrawMatchStatus();
    void DrawScoreboard();
    void DrawVehicleHUD();
    void DrawGunnerHUD(const class AOCCharacter* Character);
    void DrawSandboxAdminPanel(const class AOCPlayerController* PC);
    void DrawDeploymentPanel(const class AOCPlayerController* PC);
    void DrawChat(const class AOCPlayerController* PC);
    void DrawSquadOrder(const class AOCPlayerController* PC);
};
