#pragma once

#include "CoreMinimal.h"
#include "OCCharacterVisualTypes.h"

/**
 * Current production-asset contract for one visual faction.
 *
 * R14 deliberately records the real current state: all four factions still share the
 * QuantumCharacter body/arms technical base. bFactionUnique* must stay false until an
 * actually distinct production asset is imported, wired and visually approved.
 */
struct FOCCharacterProductionProfile
{
    EOCFactionArchetype Faction = EOCFactionArchetype::UASpecialUnit;
    FString DisplayName;
    FString ThirdPersonBodyObjectPath;
    FString FirstPersonArmsObjectPath;
    bool bFactionUniqueBody = false;
    bool bFactionUniqueArms = false;
};

enum class EOCCharacterProductionModuleType : uint8
{
    Skeletal,
    Static,
};

/** Existing modular QuantumCharacter asset that can participate in R14 appearance sets. */
struct FOCCharacterProductionModule
{
    FName ModuleId = NAME_None;
    FString ObjectPath;
    EOCCharacterProductionModuleType Type = EOCCharacterProductionModuleType::Skeletal;
    bool bCurrentlyUsedByRuntimeSubsystem = false;
};

OSTERCONFLICT_API bool OCHasDeclaredCharacterProductionProfile(EOCFactionArchetype Faction);
OSTERCONFLICT_API FOCCharacterProductionProfile OCResolveCharacterProductionProfile(EOCFactionArchetype Faction);
OSTERCONFLICT_API TArray<FOCCharacterProductionProfile> OCGetDeclaredCharacterProductionProfiles();
OSTERCONFLICT_API TArray<FOCCharacterProductionModule> OCGetDeclaredCharacterProductionModules();
