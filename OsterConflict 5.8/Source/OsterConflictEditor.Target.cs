using UnrealBuildTool;
using System.Collections.Generic;

public class OsterConflictEditorTarget : TargetRules
{
    public OsterConflictEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        CppStandard = CppStandardVersion.Cpp20;

        ExtraModuleNames.Add("OsterConflict");
    }
}
