#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR143SilpoFacadeIdentitySubsystem.generated.h"

/**
 * R14.3 visual identity pass for the Oster Silpo facade.
 * Replaces the temporary rectangular sign read with a source-only layered oval logo approximation,
 * adds the photographed dark parapet caps and the parking-direction sign in front of the facade.
 */
UCLASS()
class OSTERCONFLICT_API UOCR143SilpoFacadeIdentitySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void BuildFacadeIdentity(UWorld& World);
};
