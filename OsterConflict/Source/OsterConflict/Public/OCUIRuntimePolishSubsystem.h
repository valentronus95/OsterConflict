#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCUIRuntimePolishSubsystem.generated.h"

class APawn;

/**
 * R12 runtime presentation/QA polish for the source-built interface.
 *
 * Keeps the old S17 backend/API intact while making the current test build usable:
 * - inactive chat is not a permanent giant panel;
 * - Escape inside a match shows a compact game menu;
 * - deployment is a compact functional panel instead of a full-screen debug roster;
 * - stale vehicle input mapping is removed when control returns to the character.
 *
 * Final-art UI can replace this subsystem later without changing gameplay/network APIs.
 */
UCLASS()
class OSTERCONFLICT_API UOCUIRuntimePolishSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

private:
    TWeakObjectPtr<APawn> LastLocalPawn;
};
