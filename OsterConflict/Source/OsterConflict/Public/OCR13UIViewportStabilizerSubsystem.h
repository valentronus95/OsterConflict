#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13UIViewportStabilizerSubsystem.generated.h"

class UOCGameUIRootWidget;
class UWidget;

UCLASS()
class OSTERCONFLICT_API UOCR13UIViewportStabilizerSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

private:
    TMap<TWeakObjectPtr<UWidget>, ESlateVisibility> StartupSuppressedWidgets;
    bool bStartupIsolationActive = false;

    void StabilizeDeployment(UOCGameUIRootWidget* Root) const;
    void ApplyStartupIsolation(UOCGameUIRootWidget* Root, bool bEnable);
};
