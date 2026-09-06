#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCSettingsRecoverySubsystem.generated.h"

/**
 * GAME RECOVERY guard for the source-built settings screen.
 * R13 frontend intentionally disables hidden legacy panels, but SettingsPanel must be re-enabled when it
 * becomes the active screen. This subsystem also applies the shared dark production treatment at runtime.
 */
UCLASS()
class OSTERCONFLICT_API UOCSettingsRecoverySubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickableWhenPaused() const override { return true; }

private:
    void RestoreSettingsScreen();
    void ApplyProductionStyle(class UWidget* Widget);

    bool bWasSettingsVisible = false;
};
