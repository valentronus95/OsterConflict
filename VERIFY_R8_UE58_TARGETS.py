from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / 'OsterConflict' / 'Source'
TARGETS = [
    'OsterConflict.Target.cs',
    'OsterConflictEditor.Target.cs',
    'OsterConflictClient.Target.cs',
    'OsterConflictServer.Target.cs',
]
required = [
    'DefaultBuildSettings = BuildSettingsVersion.V7;',
    'IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;',
    'CppStandard = CppStandardVersion.Cpp20;',
]
for name in TARGETS:
    p = SRC / name
    if not p.exists():
        raise SystemExit(f'FAIL missing target: {p}')
    text = p.read_text(encoding='utf-8')
    for marker in required:
        if marker not in text:
            raise SystemExit(f'FAIL {name}: missing {marker}')
    for forbidden in ['BuildEnvironment = TargetBuildEnvironment.Unique', 'bOverrideBuildEnvironment = true']:
        if forbidden in text:
            raise SystemExit(f'FAIL {name}: workaround found instead of UE5.8 defaults: {forbidden}')
print('R8 UE5.8 TARGET RULES VERIFY: PASS (4 targets x 3 required settings)')
