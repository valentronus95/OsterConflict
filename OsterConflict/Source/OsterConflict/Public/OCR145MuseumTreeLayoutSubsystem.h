#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR145MuseumTreeLayoutSubsystem.generated.h"

/** Replaces the symmetric R13.7 museum tree seeds with the asymmetrical photo-oriented site layout. */
UCLASS()
class OSTERCONFLICT_API UOCR145MuseumTreeLayoutSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ReplaceMuseumTrees(UWorld& World) const;
};
