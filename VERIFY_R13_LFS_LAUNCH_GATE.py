from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parent
LFS_CHECK = ROOT / "PC_TEST" / "CHECK_R13_LFS_PAYLOADS.ps1"
LAUNCHER = ROOT / "RUN_R11_LISTEN_TEST.cmd"
FULL = ROOT / "RUN_PC_TEST.cmd"
CLEAN_FULL = ROOT / "RUN_CLEAN_FULL_TEST.cmd"
COMPILE_ONLY = ROOT / "RUN_COMPILE_ONLY.cmd"
PACKAGING = ROOT / "OsterConflict" / "Config" / "DefaultGame.ini"


def fail(message: str) -> None:
    raise SystemExit(f"R13 LFS LAUNCH GATE VERIFY FAIL: {message}")


for path in (LFS_CHECK, LAUNCHER, FULL, CLEAN_FULL, COMPILE_ONLY, PACKAGING):
    if not path.is_file():
        fail(f"missing required file: {path.relative_to(ROOT)}")

lfs = LFS_CHECK.read_text(encoding="utf-8", errors="replace")
launcher = LAUNCHER.read_text(encoding="utf-8", errors="replace")
full = FULL.read_text(encoding="utf-8", errors="replace")
clean_full = CLEAN_FULL.read_text(encoding="utf-8", errors="replace")
compile_only = COMPILE_ONLY.read_text(encoding="utf-8", errors="replace")
packaging = PACKAGING.read_text(encoding="utf-8", errors="replace")

LFS_REQUIRED = [
    "DirectoriesToAlwaysCook",
    "Test-GitLfsPointer",
    "version https://git-lfs.github.com/spec/v1",
    "Get-ChildItem -LiteralPath $LocalDir -Recurse -File",
    "'.uasset','.umap'",
    "R13 GAMEPLAY LAUNCH BLOCKED: GIT LFS CONTENT IS NOT READY",
    "git lfs pull",
    "Do not replace the pointer files manually.",
]
for token in LFS_REQUIRED:
    if token not in lfs:
        fail(f"LFS checker guard missing: {token}")

LAUNCH_REQUIRED = [
    'set "LFS_CHECK=%~dp0PC_TEST\\CHECK_R13_LFS_PAYLOADS.ps1"',
    'if not exist "%LFS_CHECK%"',
    'powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%LFS_CHECK%" -ProjectRoot "%PROJECT_ROOT%"',
    'set "LFS_RC=%ERRORLEVEL%"',
    'if not "%LFS_RC%"=="0"',
    '-NoScreenMessages',
]
for token in LAUNCH_REQUIRED:
    if token not in launcher:
        fail(f"listen launcher R13.1 gate missing: {token}")

lfs_call = launcher.find('powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%LFS_CHECK%"')
editor_start = launcher.find('start "Oster Conflict R13.1"')
if lfs_call < 0 or editor_start < 0 or lfs_call >= editor_start:
    fail("LFS payload check must execute before starting the R13.1 UnrealEditor player-facing path")

FULL_REQUIRED = [
    'set "LFS_CHECK=%~dp0PC_TEST\\CHECK_R13_LFS_PAYLOADS.ps1"',
    'powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%LFS_CHECK%" -ProjectRoot "%~dp0OsterConflict"',
    'set "LFS_RC=%ERRORLEVEL%"',
    'if not "%LFS_RC%"=="0"',
]
for label, script in (("full validation", full), ("clean full validation", clean_full)):
    for token in FULL_REQUIRED:
        if token not in script:
            fail(f"{label} LFS gate missing: {token}")
    lfs_pos = script.find('powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%LFS_CHECK%"')
    validation_pos = script.find('RUN_UE58_PC_VALIDATION.ps1')
    if lfs_pos < 0 or validation_pos < 0 or lfs_pos >= validation_pos:
        fail(f"{label} must check LFS payloads before starting UE validation")

if "CHECK_R13_LFS_PAYLOADS.ps1" in compile_only:
    fail("compile-only path must remain independent of heavy Git LFS runtime payloads")

if 'DirectoriesToAlwaysCook=' not in packaging:
    fail("packaging config must expose runtime cook directories for the LFS gate")

if "Get-ChildItem $ContentRoot -Recurse" in lfs:
    fail("LFS gate must not scan/block unrelated unused Content trees")

# Parse the PowerShell gate without executing it. GitHub source CI intentionally checks out LFS pointers, so running
# the gate there would correctly fail for the wrong reason; the language parser still catches syntax errors.
shell = shutil.which("pwsh") or shutil.which("powershell.exe") or shutil.which("powershell")
if shell:
    escaped_path = str(LFS_CHECK).replace("'", "''")
    parse_command = (
        "$tokens=$null; $errors=$null; "
        f"[System.Management.Automation.Language.Parser]::ParseFile('{escaped_path}', [ref]$tokens, [ref]$errors) | Out-Null; "
        "if($errors.Count -gt 0){ $errors | ForEach-Object { Write-Error $_.Message }; exit 1 }"
    )
    result = subprocess.run(
        [shell, "-NoProfile", "-Command", parse_command],
        cwd=ROOT,
        text=True,
        capture_output=True,
    )
    if result.returncode != 0:
        detail = (result.stdout + "\n" + result.stderr).strip()
        fail("PowerShell parser rejected CHECK_R13_LFS_PAYLOADS.ps1" + (f": {detail}" if detail else ""))

print("R13 LFS LAUNCH GATE VERIFY: PASS")
print("Checks runtime-cooked LFS payload detection, PowerShell syntax, full/package gating and R13.1 player-facing launch ordering.")
