from pathlib import Path

ROOT = Path(__file__).resolve().parent
FETCHER = ROOT / "FETCH_R13_S01_KRUSHELNYTSKA_VISICOM_GEOMETRY.py"
WORLD = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWorldSectorOster.cpp"


def fail(message: str) -> None:
    raise SystemExit("R13 S01 KRUSHELNYTSKA GEOMETRY FETCH VERIFY FAIL: " + message)


for path in (FETCHER, WORLD):
    if not path.is_file():
        fail(f"missing file: {path.relative_to(ROOT)}")

fetcher = FETCHER.read_text(encoding="utf-8", errors="replace")
world = WORLD.read_text(encoding="utf-8", errors="replace")

required = [
    'FEATURE_ID = "STR3KJXJOBMQ"',
    'API_BASE = "https://api.visicom.ua/data-api/5.0/uk/feature"',
    'os.environ.get("VISICOM_API_KEY"',
    "quote(api_key, safe='')",
    "OsterConflict-R13-reference-review/1.0",
    'geometry_type == "LineString"',
    'geometry_type == "MultiLineString"',
    'geometry_type == "GeometryCollection"',
    "OriginLatitude",
    "OriginLongitude",
    'OUT_DIR = ROOT / "OsterConflict" / "Saved" / "ReferenceEvidence"',
    "S01_Krushelnytska_Visicom_feature.json",
    "S01_Krushelnytska_Visicom_local_cm.csv",
    "status=REVIEW_ONLY; no runtime/source geometry was modified",
]
for token in required:
    if token not in fetcher:
        fail(f"geometry-fetch safety marker missing: {token}")

# No credential, downloaded geometry or generated centerline belongs in tracked source. The tool must only consume an
# environment key and write review evidence below Unreal Saved/.
for forbidden in [
    "YOUR_API_KEY",
    "api_key = \"",
    "Private/OCWorldSectorOster.cpp",
    "Public/OCLocationSectorS01RoadData.h",
    "Private/OCLocationSectorS01RoadData.cpp",
    "git add",
    "git commit",
    "git push",
]:
    if forbidden in fetcher:
        fail(f"fetcher can mutate source or suggests hardcoded credentials: {forbidden}")

if "FETCH_R13_S01_KRUSHELNYTSKA_VISICOM_GEOMETRY" in world or "STR3KJXJOBMQ" in world:
    fail("review-only Visicom feature acquisition leaked into runtime world construction")

print("R13 S01 KRUSHELNYTSKA GEOMETRY FETCH VERIFY: PASS")
print("Checks exact Visicom street feature ID, environment-only API key, LineString/MultiLineString review extraction, project-local conversion and Saved-only output with zero runtime/source mutation.")
