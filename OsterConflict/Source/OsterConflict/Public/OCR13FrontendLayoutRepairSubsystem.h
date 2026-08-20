#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13FrontendLayoutRepairSubsystem.generated.h"

/** Repairs the initial Slate/viewport race that could leave the R13 menu background tiled/misaligned until alt-tab. */
UCLASS()
class OSTERCONFLICT_API UOCR13FrontendLayoutRepairSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

private:
    bool RepairFrontendGeometry();

    float FrontendVisibleAge = 0.0f;
    int32 RepairPass = 0;
    FVector2D LastRootSize = FVector2D::ZeroVector;
};
