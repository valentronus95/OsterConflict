from pathlib import Path

ROOT = Path(__file__).resolve().parent
LFS_CHECK = ROOT / "PC_TEST" / "CHECK_R13_LFS_PAYLOADS.ps1"
LAUNCHER = ROOT / "RUN_R11_LISTEN_TEST.cmd"
PACKAGING = ROOT / "OsterConflict" / "Config" / "DefaultGame.ini"


def fail(message: str) -> None:
    raise SystemExit(f"R13 LFS LAUNCH GATE VERIFY FAIL: {message}")


for path in (LFS_CHECK, LAUNCHER, PACKAGING):
    if not path.is_file():
        fail(f"missing required file: {path.relative_to(ROOT)}")

lfs = LFS_CHECK.read_text(encoding="utf-8", errors="replace")
launcher = LAUNCHER.read_text(encoding="utf-8", errors="replace")
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
]
for token in LAUNCH_REQUIRED:
    if token not in launcher:
        fail(f"listen launcher LFS gate missing: {token}")

lfs_call = launcher.find('powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%LFS_CHECK%"')
editor_start = launcher.find('start "Oster Conflict R13"')
if lfs_call < 0 or editor_start < 0 or lfs_call >= editor_start:
    fail("LFS payload check must execute before starting UnrealEditor")

if 'DirectoriesToAlwaysCook=' not in packaging:
    fail("packaging config must expose runtime cook directories for the LFS gate")

if "Get-ChildItem $ContentRoot -Recurse" in lfs:
    fail("LFS gate must not scan/block unrelated unused Content trees")

print("R13 LFS LAUNCH GATE VERIFY: PASS")
print("Checks runtime-cooked LFS payload detection and pre-Editor launch ordering.")
