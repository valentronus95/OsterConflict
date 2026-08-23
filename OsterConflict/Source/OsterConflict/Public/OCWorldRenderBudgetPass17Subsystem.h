#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCWorldRenderBudgetPass17Subsystem.generated.h"

/**
 * Pass 17 laptop recovery: applies conservative distance culling and shadow budgets to the
 * source-only Oster world ISM families. Gameplay collision remains untouched.
 */
UCLASS()
class OSTERCONFLICT_API UOCWorldRenderBudgetPass17Subsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void TryApplyBudget();

    FTimerHandle RetryHandle;
    int32 Attempts = 0;
    bool bApplied = false;
};
