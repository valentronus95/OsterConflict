#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13StadiumRuntimeValidationSubsystem.generated.h"

/**
 * Runtime evidence gate for the current hard-georeferenced Stadion Oster site.
 *
 * Source contracts are not enough here: this validator waits for the actual gameplay world,
 * checks the authoritative stadium actor/components against terrain and legacy ownership, then
 * emits one PASS9_* marker consumed by the local Windows UE acceptance launcher.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13StadiumRuntimeValidationSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

private:
    void FailValidation(const FString& Reason);

    float ElapsedSeconds = 0.0f;
    bool bFinished = false;
};
