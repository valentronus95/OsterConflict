using UnrealBuildTool;
using System.Collections.Generic;

public class OsterConflictClientTarget : TargetRules
{
    public OsterConflictClientTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Client;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        CppStandard = CppStandardVersion.Cpp20;

        ExtraModuleNames.Add("OsterConflict");
    }
}
