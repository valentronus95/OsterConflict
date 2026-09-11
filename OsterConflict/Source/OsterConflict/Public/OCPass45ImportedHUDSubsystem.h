#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCPass45ImportedHUDSubsystem.generated.h"

class UUserWidget;

/**
 * Owns the safe UUserWidget-backed imported/local HUD for normal gameplay.
 * Raw Slate texture overlays stay retired after the previous GC crash.
 */
UCLASS()
class OSTERCONFLICT_API UOCPass45ImportedHUDSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    void BindGameplayHUD();

    UPROPERTY(Transient)
    TObjectPtr<UUserWidget> ImportedHUDWidget = nullptr;

    FTimerHandle BindTimer;
    int32 BindAttempts = 0;
};
