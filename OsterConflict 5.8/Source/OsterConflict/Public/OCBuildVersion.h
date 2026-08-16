#pragma once

#include "CoreMinimal.h"

/** S18B build fingerprint used by logs, smoke tests and release manifests. */
namespace OCBuildVersion
{
    inline constexpr const TCHAR* Milestone = TEXT("S18B");
    inline constexpr const TCHAR* ProjectVersion = TEXT("0.0.18B-S18B");
    inline constexpr int32 NetworkProtocol = 18;
    inline constexpr const TCHAR* ReleaseMap = TEXT("/Game/Maps/OsterConflict_Runtime");
}
