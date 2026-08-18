from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
PROJECT = ROOT / "OsterConflict"
SOURCE = PROJECT / "Source" / "OsterConflict"
CONFIG = PROJECT / "Config" / "DefaultGame.ini"


def fail(message: str) -> None:
    raise SystemExit(f"R13 DYNAMIC COOK PATHS VERIFY FAIL: {message}")


if not SOURCE.is_dir():
    fail(f"missing source directory: {SOURCE.relative_to(ROOT)}")
if not CONFIG.is_file():
    fail(f"missing packaging config: {CONFIG.relative_to(ROOT)}")

config_text = CONFIG.read_text(encoding="utf-8", errors="replace")
always_cook_paths = set(re.findall(r'DirectoriesToAlwaysCook=\(Path="([^"]+)"\)', config_text))
map_paths = set(re.findall(r'MapsToCook=\(FilePath="([^"]+)"\)', config_text))

# Only inspect files that actually participate in runtime/object-path loading. Extract all /Game/... literals from
# those files so indirect patterns such as const TCHAR* MeshPath = TEXT("/Game/..."); LoadObject(..., MeshPath)
# are covered as well as direct LoadObject(TEXT("/Game/...")) calls.
loader_markers = (
    "LoadObject<",
    "LoadClass<",
    "StaticLoadObject",
    "StaticLoadClass",
    "ConstructorHelpers::FObjectFinder",
    "FSoftObjectPath",
)
path_pattern = re.compile(r'/Game/[A-Za-z0-9_./\-]+')

seen_paths: dict[str, set[str]] = {}
for source_file in sorted(list(SOURCE.rglob("*.cpp")) + list(SOURCE.rglob("*.h"))):
    text = source_file.read_text(encoding="utf-8", errors="replace")
    if not any(marker in text for marker in loader_markers):
        continue
    for raw_path in path_pattern.findall(text):
        package_path = raw_path.rstrip(".)")
        # Object references commonly end in .Asset or .Blueprint_C. Cooking is directory-based here, so only
        # the top-level /Game/<Root> package directory matters for non-map content.
        parts = package_path.split("/")
        if len(parts) < 3:
            continue
        root_path = f"/Game/{parts[2]}"
        seen_paths.setdefault(root_path, set()).add(str(source_file.relative_to(ROOT)).replace("\\", "/"))

missing: list[str] = []
for root_path, files in sorted(seen_paths.items()):
    if root_path == "/Game/Maps":
        if not map_paths:
            missing.append(f"{root_path}: runtime map path(s) found but MapsToCook is empty ({', '.join(sorted(files))})")
        continue
    if root_path not in always_cook_paths:
        missing.append(f"{root_path}: dynamic string asset root is not DirectoriesToAlwaysCook ({', '.join(sorted(files))})")

if missing:
    print("R13 dynamic cook path verification: FAIL")
    for item in missing:
        print(" -", item)
    raise SystemExit(1)

print("R13 DYNAMIC COOK PATHS VERIFY: PASS")
print("Dynamic /Game roots covered:", ", ".join(sorted(seen_paths)))
