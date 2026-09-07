#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCWorldAuthoringLabelSuppressionSubsystem.generated.h"

/**
 * Runtime-only presentation guard for source-authored Oster sector labels.
 *
 * The sector labels remain useful authoring landmarks in the editor, but they are not gameplay content and must
 * never render as giant world-space text in a player-facing runtime session.
 */
UCLASS()
class OSTERCONFLICT_API UOCWorldAuthoringLabelSuppressionSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
};
