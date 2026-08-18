#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13EnvironmentDressingSubsystem.generated.h"

/**
 * R13.4 additive environment-density pass.
 *
 * This subsystem deliberately does not rewrite the authored Oster topology or gameplay collision.
 * It waits until the compact layout and the existing R12/R13 art bridges have finished, then adds
 * visual-only grass, ground plants, house companion pieces, yard props and secondary vegetation.
 * The result is a denser contemporary small-town environment while the existing collidable source
 * proxies / primary art replacements remain the gameplay authority.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13EnvironmentDressingSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ApplyEnvironmentDressing(UWorld& World);

    bool bApplied = false;
};
