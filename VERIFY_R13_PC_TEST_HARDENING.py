from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parent

required = [
    ROOT / ".gitignore",
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
s05 = (ROOT / "VERIFY_S05.py").read_text(encoding="utf-8")
prelaunch = (ROOT / "PC_TEST" / "PRELAUNCH_CHECK.ps1").read_text(encoding="utf-8")
validation = (ROOT / "PC_TEST" / "RUN_UE58_PC_VALIDATION.ps1").read_text(encoding="utf-8")

checks = [
    ("validation evidence ignored", "PC_TEST/TEST_RESULTS/" in ignore),
    ("UE binaries ignored", "Binaries/" in ignore),
    ("S05 checks Git index", '"git", "ls-files"' in s05),
    ("S05 tracks generated prefixes", "forbidden_prefixes" in s05),
    ("S05 no longer rejects local Binaries folder", "if (root / forbidden).exists()" not in s05),
    ("SDK version parsing is explicit", "[System.Version]::Parse($_.Name)" in prelaunch),
    ("SDK fallback message names UBT", "UBT will perform authoritative SDK detection" in prelaunch),
    ("validation derives success from failure message", "$Succeeded=[string]::IsNullOrEmpty($FailureMessage)" in validation),
    ("validation clears failed stage on success", "$FailedStage=''" in validation),
    ("validation only fills failed stage on failure", "if(-not $Succeeded){ $FailedStage=$CurrentStage }" in validation),
    ("static verifier is documented as required quality gate", "Source verifiers are a required quality gate whenever Python is available." in validation),
    ("static verifier failure is a FAIL stage", "Add-Stage 'Static verifier suite' 'FAIL'" in validation),
    ("static verifier failure aborts validation", 'throw "Static verifier suite failed with exit code $rc"' in validation),
    ("static verifier no longer degrades failure to warning", "Add-Stage 'Static verifier suite' 'WARN'" not in validation),
    ("installed UE skips missing project-file generator", "elseif($InstalledBuild)" in validation and "Launcher/installed UE build; direct UBT does not require GenerateProjectFiles.bat." in validation),
    ("source UE still warns when project-file generator is unexpectedly missing", "GenerateProjectFiles.bat not found; continuing with direct UBT." in validation),
]

failed = [name for name, ok in checks if not ok]
if failed:
    print("R13 PC test hardening verification: FAIL")
    print("Failed checks:", *failed, sep="\n - ")
    sys.exit(1)

print("R13 PC test hardening verification: PASS")
print(f"Checked {len(required)} required files and {len(checks)} regression markers.")
