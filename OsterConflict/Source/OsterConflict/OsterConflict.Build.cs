using UnrealBuildTool;

public class OsterConflict : ModuleRules
{
    public OsterConflict(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // OsterConflict contains a number of implementation-local helpers with intentionally common
        // names (AddBox, MakeMID, MakeISM, etc.). Unreal unity builds concatenate multiple .cpp files
        // into one translation unit, which breaks those otherwise file-local anonymous namespaces.
        // Compile source files independently for deterministic UE 5.8 builds and lower per-action
        // memory pressure on the current 16 GB development machine.
        bUseUnity = false;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "RHI",
            "EnhancedInput",
            "InputCore",
            "NetCore",
            "AIModule",
            "NavigationSystem",
            "GameplayTasks",
            "UMG",
            "Slate",
            "SlateCore",
            "MoviePlayer",
            "Niagara"
        });
    }
}