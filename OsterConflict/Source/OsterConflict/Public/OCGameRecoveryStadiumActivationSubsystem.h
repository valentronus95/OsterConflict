#pragma once

#include "CoreMinimal.h"
#include "OCR13StadiumSurfaceSubsystem.h"
#include "OCGameRecoveryStadiumActivationSubsystem.generated.h"

struct FStreamableHandle;

/**
 * Concrete GAME_RECOVERY activation path for the canonical Stadion Oster owner.
 * The historical stadium implementation stays abstract and is invoked only after its presentation payload
 * is resident, so startup never revives the old blocking package-load path.
 */
UCLASS()
class OSTERCONFLICT_API UOCGameRecoveryStadiumActivationSubsystem final : public UOCR13StadiumSurfaceSubsystem
{
    GENERATED_BODY()

public:
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

    bool IsStadiumPresentationReady() const { return bPresentationReady; }

private:
    void BeginStadiumPreload();
    void CompleteStadiumPreload();

    TSharedPtr<FStreamableHandle> PreloadHandle;
    bool bPreloadRequested = false;
    bool bPreloadFailed = false;
    bool bPresentationReady = false;
};
