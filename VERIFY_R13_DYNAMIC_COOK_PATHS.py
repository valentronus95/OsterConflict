from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
PROJECT = ROOT / "OsterConflict"
CONTENT = PROJECT / "Content"
SOURCE = PROJECT / "Source" / "OsterConflict"
CONFIG = PROJECT / "Config" / "DefaultGame.ini"


def fail(message: str) -> None:
    raise SystemExit(f"R13 DYNAMIC COOK PATHS VERIFY FAIL: {message}")


if not CONTENT.is_dir():
    fail(f"missing content directory: {CONTENT.relative_to(ROOT)}")
if not SOURCE.is_dir():
    fail(f"missing source directory: {SOURCE.relative_to(ROOT)}")
if not CONFIG.is_file():
    fail(f"missing packaging config: {CONFIG.relative_to(ROOT)}")

config_text = CONFIG.read_text(encoding="utf-8", errors="replace")
always_cook_paths = set(re.findall(r'DirectoriesToAlwaysCook=\(Path="([^"]+)"\)', config_text))
map_paths = set(re.findall(r'MapsToCook=\(FilePath="([^"]+)"\)', config_text))

# Inspect only files that participate in runtime/object-path loading. Extract every /Game/... literal from those
# files so indirect patterns such as const TCHAR* MeshPath = TEXT("/Game/..."); LoadObject(..., MeshPath) are
# covered as well as direct LoadObject(TEXT("/Game/...")) calls.
loader_markers = (
    "LoadObject<",
    "LoadClass<",
    "StaticLoadObject",
    "StaticLoadClass",
    "ConstructorHelpers::FObjectFinder",
    "FSoftObjectPath",
)
path_pattern = re.compile(r'/Game/[A-Za-z0-9_./\-]+')

dynamic_paths: dict[str, set[str]] = {}
for source_file in sorted(list(SOURCE.rglob("*.cpp")) + list(SOURCE.rglob("*.h"))):
    text = source_file.read_text(encoding="utf-8", errors="replace")
    if not any(marker in text for marker in loader_markers):
        continue
    relative_file = str(source_file.relative_to(ROOT)).replace("\\", "/")
    for raw_path in path_pattern.findall(text):
        package_path = raw_path.rstrip(".)")
        if not package_path.startswith("/Game/"):
            continue
        dynamic_paths.setdefault(package_path, set()).add(relative_file)


def covered_by_directory(package_path: str) -> bool:
    for cook_path in always_cook_paths:
        prefix = cook_path.rstrip("/")
        if package_path == prefix or package_path.startswith(prefix + "/"):
            return True
    return False


def covered_by_map(package_path: str) -> bool:
    for map_path in map_paths:
        if package_path == map_path or package_path.startswith(map_path + "."):
            return True
    return False


def game_directory_to_local(game_path: str) -> Path:
    relative = game_path.removeprefix("/Game/").strip("/")
    return CONTENT / Path(relative)


def dynamic_asset_to_local(package_path: str) -> Path:
    relative = package_path.removeprefix("/Game/")
    pieces = relative.split("/")
    leaf = pieces[-1]
    # Object/class references use /Game/Folder/Package.Object or Package.Blueprint_C. The .uasset filename is
    # always the package part before the first dot in the final path segment.
    package_leaf = leaf.split(".", 1)[0]
    pieces[-1] = package_leaf + ".uasset"
    return CONTENT.joinpath(*pieces)


missing: list[str] = []
seen_roots: set[str] = set()

for cook_path in sorted(always_cook_paths):
    if not cook_path.startswith("/Game/"):
        continue
    local_dir = game_directory_to_local(cook_path)
    if not local_dir.is_dir():
        missing.append(
            f"{cook_path}: DirectoriesToAlwaysCook points to missing directory {local_dir.relative_to(ROOT)}"
        )

for package_path, files in sorted(dynamic_paths.items()):
    parts = package_path.split("/")
    if len(parts) >= 3:
        seen_roots.add(f"/Game/{parts[2]}")

    if package_path.startswith("/Game/Maps/"):
        if not covered_by_map(package_path):
            missing.append(
                f"{package_path}: runtime map path is not covered by MapsToCook ({', '.join(sorted(files))})"
            )
        continue

    if not covered_by_directory(package_path):
        missing.append(
            f"{package_path}: dynamic string asset path is not covered by DirectoriesToAlwaysCook "
            f"({', '.join(sorted(files))})"
        )
        continue

    local_asset = dynamic_asset_to_local(package_path)
    if not local_asset.is_file():
        missing.append(
            f"{package_path}: dynamic asset does not resolve to committed {local_asset.relative_to(ROOT)} "
            f"({', '.join(sorted(files))})"
        )

if missing:
    print("R13 dynamic cook path verification: FAIL")
    for item in missing:
        print(" -", item)
    raise SystemExit(1)

print("R13 DYNAMIC COOK PATHS VERIFY: PASS")
print("Dynamic /Game roots covered:", ", ".join(sorted(seen_roots)))
print(f"Dynamic package literals covered and present: {len(dynamic_paths)}")
print(f"DirectoriesToAlwaysCook verified on disk: {len(always_cook_paths)}")
