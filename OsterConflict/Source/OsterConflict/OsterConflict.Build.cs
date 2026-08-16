using UnrealBuildTool;

public class OsterConflict : ModuleRules
{
    public OsterConflict(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "EnhancedInput",
            "InputCore",
            "NetCore",
            "AIModule",
            "NavigationSystem",
            "GameplayTasks",
            "UMG",
            "Slate",
            "SlateCore"
        });
    }
}
