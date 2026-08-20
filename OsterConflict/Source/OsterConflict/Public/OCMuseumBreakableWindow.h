#pragma once

#include "CoreMinimal.h"
#include "OCBreakableWindow.h"
#include "OCMuseumBreakableWindow.generated.h"

class UStaticMeshComponent;

/**
 * Museum-specific visual specialization of the shared replicated breakable window.
 * Keeps the proven gameplay/break state while matching the photographed white frames,
 * upper transom and real glass material without changing windows used by other houses.
 */
UCLASS()
class OSTERCONFLICT_API AOCMuseumBreakableWindow : public AOCBreakableWindow
{
    GENERATED_BODY()

public:
    AOCMuseumBreakableWindow();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Museum|Window")
    TObjectPtr<UStaticMeshComponent> CenterMullion;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Museum|Window")
    TObjectPtr<UStaticMeshComponent> UpperTransom;

private:
    void ApplyMuseumMaterials();
};
