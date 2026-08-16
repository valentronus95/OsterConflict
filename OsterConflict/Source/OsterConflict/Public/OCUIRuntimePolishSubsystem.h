#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCUIRuntimePolishSubsystem.generated.h"

/**
 * R12.1 runtime UI polish for the source-built test interface.
 *
 * Keeps the old S17 backend/API intact while correcting two test-blocking presentation problems:
 * - the inactive chat panel must not look like a permanent 430x330 menu;
 * - Escape during an active match should show a compact pause menu, not the direct-connect frontend.
 *
 * This is intentionally isolated so the later final-art UI can replace it without touching gameplay/network code.
 */
UCLASS()
class OSTERCONFLICT_API UOCUIRuntimePolishSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
};
