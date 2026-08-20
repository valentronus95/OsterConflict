#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13AccessibilitySubsystem.generated.h"

/**
 * Small runtime repair layer for source-built R13 landmark collision/layout issues.
 * Keeps fixes isolated from the large reference-driven world generator until final authored meshes replace proxies.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13AccessibilitySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void RepairMuseumEntranceSteps(UWorld& World);
};
