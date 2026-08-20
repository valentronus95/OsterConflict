#include "OCR13FirstPersonFallbackSubsystem.h"

#include "OCCharacter.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"

bool UOCR13FirstPersonFallbackSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13FirstPersonFallbackSubsystem::Tick(float DeltaTime)
{
    ScanAccumulator += DeltaTime;
    if (ScanAccumulator < 0.20f) return;
    ScanAccumulator = 0.0f;

    UWorld* World = GetWorld();
    if (!World) return;

    for (TActorIterator<AOCCharacter> It(World); It; ++It)
    {
        AOCCharacter* Character = *It;
        if (!Character || !Character->IsLocallyControlled()) continue;

        bool bHasProductionArms = false;
        TArray<USkeletalMeshComponent*> SkeletalMeshes;
        Character->GetComponents<USkeletalMeshComponent>(SkeletalMeshes);
        for (USkeletalMeshComponent* Component : SkeletalMeshes)
        {
            if (!Component || Component->GetFName() != TEXT("FirstPersonArms")) continue;
            bHasProductionArms = Component->GetSkeletalMeshAsset() != nullptr && Component->IsVisible();
            break;
        }

        TArray<UStaticMeshComponent*> StaticMeshes;
        Character->GetComponents<UStaticMeshComponent>(StaticMeshes);
        for (UStaticMeshComponent* Component : StaticMeshes)
        {
            if (!Component) continue;
            const FName Name = Component->GetFName();
            const bool bFirstPersonProxy = Name == TEXT("FPProxyArmL") || Name == TEXT("FPProxyArmR") ||
                Name == TEXT("FPProxyHandL") || Name == TEXT("FPProxyHandR");
            if (!bFirstPersonProxy) continue;

            Component->SetOnlyOwnerSee(true);
            Component->SetHiddenInGame(false, true);
            Component->SetVisibility(!bHasProductionArms, true);
        }
    }
}

TStatId UOCR13FirstPersonFallbackSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13FirstPersonFallbackSubsystem, STATGROUP_Tickables);
}
