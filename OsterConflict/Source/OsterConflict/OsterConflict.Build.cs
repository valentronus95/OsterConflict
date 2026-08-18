using UnrealBuildTool;

public class OsterConflict : ModuleRules
{
    public OsterConflict(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // R13 contains many source-local helpers/constants in anonymous namespaces. They are valid in separate
        // translation units, but Unreal unity blobs merge several .cpp files and make those private names collide
        // (FindISM, MakeISM, MaxSpawnAttempts, etc.). Keep this game module non-unity so source-local helpers retain
        // normal C++ translation-unit isolation. Shared PCHs remain enabled, so only the unity aggregation is disabled.
        bUseUnity = false;

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
