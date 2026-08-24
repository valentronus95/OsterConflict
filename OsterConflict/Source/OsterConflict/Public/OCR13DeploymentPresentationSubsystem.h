#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13DeploymentPresentationSubsystem.generated.h"

class AOCPlayerController;
class UBackgroundBlur;
class UBorder;
class UOCGameUIRootWidget;

/** Visual-only R13.6 deployment restyle. Gameplay selection/state remains owned by OCR13DeploymentFlowSubsystem. */
UCLASS()
class OSTERCONFLICT_API UOCR13DeploymentPresentationSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

private:
    UOCGameUIRootWidget* ResolveRoot(UWorld* World, AOCPlayerController* PC);
    void EnsurePresentation(UOCGameUIRootWidget* Root);
    void ApplyWidgetStyle(UBorder* FlowPanel);
    void SetPresentationVisible(bool bVisible);

    TWeakObjectPtr<UOCGameUIRootWidget> ActiveRoot;
    TWeakObjectPtr<AOCPlayerController> ActiveController;
    TWeakObjectPtr<UBackgroundBlur> BackdropBlur;
    TWeakObjectPtr<UBorder> BackdropShade;
    TWeakObjectPtr<UBorder> StyledFlowPanel;
    float UpdateAccumulator = 0.0f;
    bool bStyleApplied = false;
    bool bPresentationVisibilityValid = false;
    bool bLastPresentationVisible = false;
    bool bUpdateBudgetLogged = false;
};
