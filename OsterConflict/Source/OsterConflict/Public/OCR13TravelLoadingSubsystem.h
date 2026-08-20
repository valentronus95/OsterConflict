#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OCR13TravelLoadingSubsystem.generated.h"

class SWidget;

/** Keeps a deliberate loading presentation visible across frontend -> gameplay map travel. */
UCLASS()
class OSTERCONFLICT_API UOCR13TravelLoadingSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

private:
    TSharedPtr<SWidget> LoadingWidget;

    void HandlePreLoadMap(const FString& MapName);
    void HandlePostLoadMap(UWorld* LoadedWorld);
    void ShowOverlay();
    void HideOverlay();
};