using UnrealBuildTool;
using System.Collections.Generic;

public class OsterConflictServerTarget : TargetRules
{
    public OsterConflictServerTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Server;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        CppStandard = CppStandardVersion.Cpp20;

        ExtraModuleNames.Add("OsterConflict");
    }
}
