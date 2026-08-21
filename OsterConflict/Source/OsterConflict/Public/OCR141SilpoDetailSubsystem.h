#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR141SilpoDetailSubsystem.generated.h"

class UWorld;

/**
 * R14.1 photo-detail pass for the Oster Silpo location.
 *
 * R14.0 owns the shell, collision, entrance, basic shelving/checkouts and lighting.
 * This subsystem adds non-destructive photo-derived detail: suspended-ceiling grid,
 * checkout lane markers, entrance/facade trim, cart bay, utility pole and the immediate
 * side-market edge. It is intentionally source-only and does not require a Content pack.
 */
UCLASS()
class OSTERCONFLICT_API UOCR141SilpoDetailSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    void RunAuthoritativeDetailNow(UWorld& World) { BuildDetails(World); }

private:
    void BuildDetails(UWorld& World);
};