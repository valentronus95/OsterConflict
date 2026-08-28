#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
TZ = ROOT / "PASS45_RUNTIME_RECOVERY_TZ.md"
BINDINGS = ROOT / "PASS45_REFERENCE_PACK_BINDINGS.md"
SILPO = ROOT / "_DOCS" / "REFERENCE_PACKS" / "LOC_SILPO_002_OSTER_SILPO" / "REFERENCE_SPEC.md"
CULTURE = ROOT / "_DOCS" / "REFERENCE_PACKS" / "LOC_CULTURE_003_OSTER_CULTURE_HOUSE" / "REFERENCE_SPEC.md"

errors: list[str] = []


def read(path: Path) -> str:
    if not path.is_file():
        errors.append(f"missing reference-contract file: {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


tz = read(TZ)
bindings = read(BINDINGS)
silpo = read(SILPO)
culture = read(CULTURE)

# Canonical execution truth stays in the main TZ; detailed visual contracts are explicitly bound by repository path.
req("single canonical active TZ for Pass 45" in tz,
    "canonical Pass45 execution/status ownership marker missing")
req("PASS45_REFERENCE_PACK_BINDINGS.md" in tz,
    "canonical Pass45 TZ does not explicitly bind the reference-pack index")
for path in (
    "_DOCS/REFERENCE_PACKS/LOC_SILPO_002_OSTER_SILPO/REFERENCE_SPEC.md",
    "_DOCS/REFERENCE_PACKS/LOC_CULTURE_003_OSTER_CULTURE_HOUSE/REFERENCE_SPEC.md",
):
    req(path in tz, f"canonical Pass45 TZ missing bound location spec: {path}")
    req(path in bindings, f"binding index missing location spec: {path}")

req("Latest factual gameplay evidence: 2026-08-27" in tz,
    "canonical TZ latest runtime evidence date drifted from 2026-08-27")
req("RUNTIME REJECTED 2026-08-27" in tz,
    "canonical TZ no longer preserves latest factual runtime rejection")

# Silpo: temporal state, hero identity, interior/context and water-tower uncertainty must remain explicit.
for needle in (
    "2020 GRAPHITE STATE",
    "They must not be blended into one impossible building",
    "high stepped parapet",
    "volumetric Silpo logo",
    "lower entrance wing",
    "orange square lane-number signs with blue numerals",
    "LOC_TOWER_002A_OSTER_WATER_TOWER",
    "exact world transform: `PROVISIONAL`",
    "CAM-SILPO-01_FRONT_WIDE",
    "CAM-SILPO-07_WATER_TOWER_SIGHTLINE",
):
    req(needle in silpo, f"Silpo reference truth missing: {needle}")

for needle in (
    "2017–2019 light facade",
    "2020 graphite state",
    "LOC_TOWER_002A_OSTER_WATER_TOWER",
    "NORMATIVE USER EVIDENCE",
):
    req(needle in bindings, f"Silpo binding lost required scope/truth: {needle}")

# Culture House: verified identity must never make provisional geometry look photo-verified.
for needle in (
    "Hranovskoho Street 3",
    "former synagogue",
    "old park directly by the Culture House",
    "PROVISIONAL WORKING HYPOTHESIS, NOT PHOTO-VERIFIED EXACT GEOMETRY",
    "exact yaw/bearing: `PROVISIONAL`",
    "CUL-CAM-01_FRONT_WIDE",
    "CUL-CAM-05_SITE_CONTEXT",
    "later user-supplied Culture House photo pack supersedes",
):
    req(needle in culture, f"Culture House reference truth missing: {needle}")

for needle in (
    "VERIFIED IDENTITY+SITE",
    "VISUAL GEOMETRY PARTLY PROVISIONAL",
    "current six-column source facade as a **PROVISIONAL WORKING HYPOTHESIS**",
):
    req(needle in bindings, f"Culture House binding lost provisional-evidence boundary: {needle}")

# Main Gate E/K must consume the bound evidence and keep source green separate from runtime acceptance.
for needle in (
    "active location contracts in `PASS45_REFERENCE_PACK_BINDINGS.md`",
    "MUS-CAM-01..07",
    "CAM-SILPO-01_FRONT_WIDE",
    "CAM-SILPO-07_WATER_TOWER_SIGHTLINE",
    "CUL-CAM-01_FRONT_WIDE",
    "CUL-CAM-05_SITE_CONTEXT",
    "source tests alone cannot close this gate",
):
    req(needle in tz, f"Pass45 Gate E/K lost bound reference acceptance rule: {needle}")

# Prevent evidence bleed among the three hero landmarks.
for text, where in ((silpo, "Silpo"), (culture, "Culture House"), (bindings, "binding index")):
    req("Museum" in text and "Culture House" in text and "Silpo" in text,
        f"{where} reference separation no longer names all three independent landmark owners")

if errors:
    print("PASS45 LOCATION REFERENCE BINDINGS: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 LOCATION REFERENCE BINDINGS: PASS")
print("- canonical Pass45 TZ explicitly consumes the repository reference binding index")
print("- Silpo user evidence is normalized into a selected-period facade/interior/street/water-tower contract")
print("- water-tower silhouette is evidence-owned while exact transform remains fail-visible PROVISIONAL")
print("- Culture House identity/site facts are separated from its still-provisional exact visual geometry")
print("- Museum/Silpo/Culture House keep independent evidence namespaces and direct screenshot gates")
print("STATUS: REFERENCE CONTRACT SOURCE-GUARDED; CURRENT-HEAD UE 5.8 VISUAL ACCEPTANCE STILL REQUIRED")
