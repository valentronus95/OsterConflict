#include "OCPass45ImportedWeaponBridgeSubsystem.h"

#include "Engine/StreamableManager.h"

namespace
{
    constexpr int32 InitialWeaponRefreshPasses = 2;
}

bool UOCPass45ImportedWeaponBridgeSubsystem::IsInitialWeaponPresentationReady() const
{
    if (RefreshPass < InitialWeaponRefreshPasses) return false;

    for (const TSharedPtr<FStreamableHandle>& Handle : ResidentPreloadHandles)
    {
        if (Handle.IsValid() && !Handle->HasLoadCompleted()) return false;
    }
    return true;
}

float UOCPass45ImportedWeaponBridgeSubsystem::GetInitialWeaponPresentationProgress() const
{
    const float ScanProgress = FMath::Clamp(
        static_cast<float>(RefreshPass) / static_cast<float>(InitialWeaponRefreshPasses), 0.0f, 1.0f);
    if (RefreshPass < InitialWeaponRefreshPasses) return ScanProgress * 0.50f;

    if (ResidentPreloadHandles.IsEmpty()) return 1.0f;

    int32 SettledLoads = 0;
    for (const TSharedPtr<FStreamableHandle>& Handle : ResidentPreloadHandles)
    {
        if (!Handle.IsValid() || Handle->HasLoadCompleted()) ++SettledLoads;
    }

    const float LoadProgress = static_cast<float>(SettledLoads) /
        static_cast<float>(ResidentPreloadHandles.Num());
    return FMath::Clamp(0.50f + LoadProgress * 0.50f, 0.0f, 1.0f);
}
