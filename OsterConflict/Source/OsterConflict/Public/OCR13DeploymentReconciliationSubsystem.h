#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13DeploymentReconciliationSubsystem.generated.h"

/** Small watcher that lets the staged deployment UI reconcile optimistic choices with replicated authority. */
UCLASS()
class OSTERCONFLICT_API UOCR13DeploymentReconciliationSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
};
