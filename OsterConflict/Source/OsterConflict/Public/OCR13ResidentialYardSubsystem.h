#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13ResidentialYardSubsystem.generated.h"

/** Adds restrained rural-yard props around non-landmark residential proxies without changing gameplay collision. */
UCLASS()
class OSTERCONFLICT_API UOCR13ResidentialYardSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void BuildResidentialYards(UWorld& World);
};
