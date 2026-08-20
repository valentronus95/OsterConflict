#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13FrontendShellGuardSubsystem.generated.h"

/** Keeps the standalone frontend-only shell free of a possessed gameplay pawn. */
UCLASS()
class OSTERCONFLICT_API UOCR13FrontendShellGuardSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

private:
    void EnforceFrontendShell(UWorld& World);
    bool bLoggedPawnLeak = false;
};
