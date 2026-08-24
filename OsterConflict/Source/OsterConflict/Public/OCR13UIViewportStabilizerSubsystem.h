#pragma once

#include "CoreMinimal.h"
#include "Components/SlateWrapperTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13UIViewportStabilizerSubsystem.generated.h"

class AOCPlayerController;
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
    virtual void Deinitialize() override;

private:
    TMap<TWeakObjectPtr<UWidget>, ESlateVisibility> StartupSuppressedWidgets;
    TWeakObjectPtr<UOCGameUIRootWidget> CachedRoot;
    TWeakObjectPtr<AOCPlayerController> CachedController;
    float UpdateAccumulator = 0.0f;
    bool bStartupIsolationActive = false;
    bool bWorldRenderingSuppressed = false;
    bool bDeploymentStabilized = false;
    bool bLastDeploymentVisible = false;
    bool bUpdateBudgetLogged = false;

    UOCGameUIRootWidget* ResolveRoot(UWorld* World, AOCPlayerController* PC);
    void StabilizeDeployment(UOCGameUIRootWidget* Root) const;
    void ApplyStartupIsolation(UOCGameUIRootWidget* Root, bool bEnable);
    void SetWorldRenderingSuppressed(bool bSuppress);
};
