#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR142SilpoInteriorDetailSubsystem.generated.h"

class UWorld;

/**
 * R14.2 non-destructive interior detail pass for the Oster Silpo.
 * Adds photo-derived visual structure on top of the gameplay-ready R14.0 interior:
 * floor tile joints, shelf end trims, cooler door divisions, checkout belt surfaces,
 * produce-bin dividers and the entrance mat. No product inventory is authored here.
 */
UCLASS()
class OSTERCONFLICT_API UOCR142SilpoInteriorDetailSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    void RunAuthoritativeDetailNow(UWorld& World) { BuildInteriorDetails(World); }

private:
    void BuildInteriorDetails(UWorld& World);
};