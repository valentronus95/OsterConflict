#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13TacticalMapSubsystem.generated.h"

class UOCR13TacticalMapWidget;

/** Local M-key tactical-map owner. Keeps the overlay separate from persistent frontend/deployment ownership. */
UCLASS()
class OSTERCONFLICT_API UOCR13TacticalMapSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

private:
    void SetMapVisible(bool bVisible);

    TWeakObjectPtr<UOCR13TacticalMapWidget> TacticalMapWidget;
    bool bMapVisible = false;
    bool bMapKeyWasDown = false;
};
