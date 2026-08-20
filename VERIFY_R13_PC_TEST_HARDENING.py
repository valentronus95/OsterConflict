from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parent

required = [
    ROOT / ".gitignore",
    ROOT / "RUN_ALL_VERIFY.py",
    ROOT / "VERIFY_S05.py",
    ROOT / "PC_TEST" / "PRELAUNCH_CHECK.ps1",
    ROOT / "PC_TEST" / "RUN_UE58_PC_VALIDATION.ps1",
]
missing = [str(path.relative_to(ROOT)) for path in required if not path.exists()]
if missing:
    print("R13 PC test hardening verification: FAIL")
    print("Missing:", *missing, sep="\n - ")
    sys.exit(1)

ignore = (ROOT / ".gitignore").read_text(encoding="utf-8")
runner = (ROOT / "RUN_ALL_VERIFY.py").read_text(encoding="utf-8")
s05 = (ROOT / "VERIFY_S05.py").read_text(encoding="utf-8")
prelaunch = (ROOT / "PC_TEST" / "PRELAUNCH_CHECK.ps1").read_text(encoding="utf-8")
validation = (ROOT / "PC_TEST" / "RUN_UE58_PC_VALIDATION.ps1").read_text(encoding="utf-8")

checks = [
    ("validation evidence ignored", "PC_TEST/TEST_RESULTS/" in ignore),
    ("generated localization output ignored", "OsterConflict/Content/Localization/Game/" in ignore),
    ("generated cook file-open-order logs ignored", "OsterConflict/Build/**/FileOpenOrder/*.log" in ignore),
    ("UE binaries ignored", "Binaries/" in ignore),
    ("source runner queries Git index", '["git", "ls-files"]' in runner),
    ("source runner protects generated directories", 'GENERATED_DIRS = ("Binaries", "Intermediate", "Saved", "DerivedDataCache")' in runner),
    ("source runner temporarily hides local UE outputs", "hide_local_generated_dirs" in runner and "restore_local_generated_dirs" in runner),
    ("source runner restores outputs in finally", "finally:\n    restore_local_generated_dirs(moved_generated)" in runner),
    ("source runner can recover stale hold", "recover_stale_hold()" in runner),
    ("S05 checks Git index", '"git", "ls-files"' in s05),
    ("S05 tracks generated prefixes", "forbidden_prefixes" in s05),
    ("S05 no longer rejects local Binaries folder", "if (root / forbidden).exists()" not in s05),
    ("SDK version parsing is explicit", "[System.Version]::Parse($_.Name)" in prelaunch),
    ("SDK fallback message names UBT", "UBT will perform authoritative SDK detection" in prelaunch),
    ("static verifier is strict when Python exists", "Static verifier suite' 'FAIL'" in validation and 'throw "Static verifier suite failed with exit code $rc"' in validation),
    ("installed build project files are a clean skip", "Launcher/installed UE build; direct UBT does not require GenerateProjectFiles.bat." in validation),
    ("successful result is highlighted", "Write-Host $PassLine -ForegroundColor Yellow" in validation),
    ("validation derives success from failure message", "$Succeeded=[string]::IsNullOrEmpty($FailureMessage)" in validation),
    ("validation clears failed stage on success", "$FailedStage=''" in validation),
    ("validation only fills failed stage on failure", "if(-not $Succeeded){ $FailedStage=$CurrentStage }" in validation),
]

failed = [name for name, ok in checks if not ok]
if failed:
    print("R13 PC test hardening verification: FAIL")
    print("Failed checks:", *failed, sep="\n - ")
    sys.exit(1)

print("R13 PC test hardening verification: PASS")
print(f"Checked {len(required)} required files and {len(checks)} regression markers.")
