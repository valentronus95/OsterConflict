#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCLocalInboxAcceptanceSubsystem.generated.h"

class AStaticMeshActor;

/** Strict local acceptance path used only while OC_FORCE_ACCEPTANCE=1. */
UCLASS()
class OSTERCONFLICT_API UOCLocalInboxAcceptanceSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    void ValidateAndExposeAssets();
    void CycleShowcase();
    void WriteReport(bool bPass, const FString& Detail) const;

    TArray<FString> StaticPaths;
    TArray<TWeakObjectPtr<AStaticMeshActor>> ShowcaseActors;
    int32 ShowcaseCursor = 0;
    FTimerHandle ValidationTimer;
    FTimerHandle ShowcaseTimer;
};
