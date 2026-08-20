#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13OsterPropArtSubsystem.generated.h"

/** Bridges semantic Oster greybox prop families to art already bundled with the project. */
UCLASS()
class OSTERCONFLICT_API UOCR13OsterPropArtSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ApplyPropBridge(UWorld& World);
};
