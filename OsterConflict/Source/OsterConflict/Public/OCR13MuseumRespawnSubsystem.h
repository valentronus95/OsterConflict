#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13MuseumRespawnSubsystem.generated.h"

class AOCCharacter;
class AOCPlayerController;

/**
 * Keeps human location-testing spawns beside the Oster museum instead of the central park.
 *
 * The museum itself remains anchored by AOCWorldSectorOster::MuseumAnchor(). The offsets used here are
 * gameplay-only staging points around the photographed approach, not claims about real-world survey geometry.
 * Bots are deliberately excluded.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13MuseumRespawnSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

private:
    bool PlaceHumanNearMuseum(AOCPlayerController* PC, AOCCharacter* Character) const;
    bool ResolveGroundedCandidate(const FVector& XYLocation, const AActor* IgnoreActor,
        FVector& OutCharacterLocation) const;

    TMap<TWeakObjectPtr<AOCPlayerController>, TWeakObjectPtr<AOCCharacter>> LastMuseumPlacedPawn;
};
