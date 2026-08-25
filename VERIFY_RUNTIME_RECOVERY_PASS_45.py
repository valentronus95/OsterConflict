#!/usr/bin/env python3
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private"
PUB = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public"
SCRIPTS = ROOT / "OsterConflict" / "Scripts"

errors = []


def text(path: Path) -> str:
    if not path.is_file():
        errors.append(f"missing file: {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def req(cond: bool, message: str) -> None:
    if not cond:
        errors.append(message)


def has_all(haystack: str, needles) -> bool:
    return all(n in haystack for n in needles)


tz = text(ROOT / "PASS45_RUNTIME_RECOVERY_TZ.md")
manifest = text(ROOT / "RUNTIME_EVIDENCE" / "2026-08-24_PASS44_REJECTED" / "MANIFEST.md")
ledger = text(ROOT / "OSTER_CONFLICT_WORK_LEDGER.md")
start = text(ROOT / "START_HERE.cmd")
launcher = text(ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd")
perf = text(SRC / "OCPerformanceSampleSubsystem.cpp")
landmark = text(SRC / "OCR146LandmarkSeparationSubsystem.cpp")
ownership = text(SRC / "OCLandmarkShellOwnershipGuardSubsystem.cpp")
ownership_h = text(PUB / "OCLandmarkShellOwnershipGuardSubsystem.h")
tactical = text(SRC / "OCTacticalMapVisual.cpp")
foliage = text(SRC / "OCFoliageRuntimeGuardSubsystem.cpp")
foliage_h = text(PUB / "OCFoliageRuntimeGuardSubsystem.h")
budget = text(SRC / "OCWorldRenderBudgetPass17Subsystem.cpp")
world = text(SRC / "OCWorldSectorOster.cpp")
trees = text(SRC / "OCR145MuseumTreeLayoutSubsystem.cpp")
weapon_preflight = text(SCRIPTS / "verify_required_weapon_assets.py")
pass43 = text(ROOT / "VERIFY_SLATE_RENDER_TARGET_STARTUP_PASS_43.py")
pass23 = text(ROOT / "VERIFY_DX11_SM5_RENDER_TARGET_PASS_23.py")

evidence_sheet = ROOT / "RUNTIME_EVIDENCE" / "2026-08-24_PASS44_REJECTED" / "pass44_runtime_evidence_20260824.jpg"
req(evidence_sheet.is_file(), "Pass 44 rejected runtime evidence sheet is missing")
req("Pass 44 = RUNTIME REJECTED" in manifest, "evidence manifest does not classify Pass 44 as RUNTIME REJECTED")
req("PASS 45 RUNTIME RECOVERY TZ" in tz, "canonical Pass 45 corrective TZ is missing")
req("RUNTIME REJECTED" in ledger and "PASS 45 ACTIVE" in ledger, "ledger is not switched to rejected Pass 44 / active Pass 45")

# RHI A/B: normal DX11/SM5/no-HDR with normal threading, explicit compatibility-only -norhithread.
req('set "RHI_FLAGS=-d3d11 -sm5 -nohdr"' in launcher, "normal RHI-thread baseline is missing")
req('if /I "%OC_RHI_COMPAT%"=="1"' in launcher, "explicit RHI compatibility selector is missing")
req('set "RHI_FLAGS=-d3d11 -sm5 -nohdr -norhithread"' in launcher, "no-RHI-thread compatibility route is missing")
req('set "RHI_MODE=dx11_sm5_rhi_thread"' in launcher, "normal RHI mode marker is missing")
req('set "RHI_MODE=dx11_sm5_no_rhi_thread_compat"' in launcher, "compatibility RHI mode marker is missing")
req("SAFE СУМІСНІСТЬ" in start and 'set "OC_RHI_COMPAT=1"' in start,
    "START_HERE does not expose the compatibility A/B route")
req("-d3d12" not in launcher.lower() and "-sm6" not in launcher.lower(),
    "Pass 45 normal gameplay must not re-enable D3D12/SM6 while startup recovery is unresolved")
req("-nullrhi" in launcher and "-run=pythonscript" in launcher,
    "isolated weapon preflight must remain NullRHI")

# Frontend/gameplay evidence must be independent and must not mutate quality to hide performance.
for marker in (
    "PASS45_RHI_MODE",
    "PASS45_FRONTEND_PERF_BASELINE",
    "PASS45_FRONTEND_PERF_BELOW_TARGET",
    "PASS45_GAMEPLAY_PERF_BASELINE",
    "PASS45_GAMEPLAY_PERF_BELOW_TARGET",
):
    req(marker in perf, f"performance sampler missing {marker}")
req("quality_mutation=0" in perf, "Pass 45 performance evidence must not mutate visual quality")
req("PC->GetPawn() == nullptr" in perf, "frontend performance sample is not pawn-less")
req("PC->GetPawn() == nullptr" in perf and "PASS45_FRONTEND_PERF_BASELINE" in perf,
    "menu-at-8-FPS evidence path is not represented")

# The old ~8 second 0.20 x 40 full-world reconciliation loop is forbidden.
req("SeparationValidationDelaySeconds = 6.25f" in landmark, "one-shot delayed landmark reconciliation missing")
req("PASS45_LANDMARK_RECONCILIATION_BUDGET_READY" in landmark,
    "Pass 45 landmark reconciliation budget marker missing")
req("full_world_scan_passes=%d further_periodic_scan=0" in landmark,
    "landmark subsystem does not prove periodic full-world scans are retired")
req("SeparationStartupGuardIntervalSeconds" not in landmark,
    "obsolete 0.20 s landmark startup scan interval returned")
req("SeparationStartupGuardPassCount" not in landmark,
    "obsolete 40-pass landmark startup scan count returned")
req("SetTimer(\n        StartupGuardTimer" in landmark and "false);" in landmark,
    "landmark reconciliation must be a one-shot timer")

# Pass 45 shell authority: R13.8 is the single Museum shell; R13.7 is a reference/detail parent only.
for needle in (
    'MuseumReferenceLayerTag(TEXT("R137_MuseumPhotoModel"))',
    'MuseumShellTag(TEXT("R138_MuseumHighFidelityArchitecture"))',
    'SilpoShellTag(TEXT("R140_SilpoPhotoModel"))',
    'CultureHouseShellTag(TEXT("R146_CultureHouseAuthoritative"))',
    "PASS45_LANDMARK_SINGLE_SHELL_CONTRACT_READY",
    "PASS45_SINGLE_LANDMARK_SHELL_OWNERS_READY",
    "periodic_owner_scan=0",
):
    req(needle in ownership, f"single landmark shell ownership contract missing: {needle}")
req(has_all(ownership_h, (
        "exactly one current visible shell owner",
        "Museum shell: R13.8 segmented architecture.",
        "Silpo shell: R14.0 photo model.",
        "Culture House shell: R14.6 authoritative model.",
        "not counted as a second Museum shell",
    )),
    "landmark guard header does not document the Pass 45 one-shell-per-site authority")
req("MuseumPrototypeTag" not in ownership,
    "old Pass 21 Museum prototype-as-second-shell concept returned")
req("MuseumReferenceLayerCount == 1 && MuseumShellCount == 1" in ownership,
    "Museum readiness does not separate reference layer from the single shell owner")

# Tactical map topology must come from the retained 640x630 user reference, not source blockout ISMs.
for needle in (
    "Pass45ReferenceWidthPx = 640.0f",
    "Pass45ReferenceHeightPx = 630.0f",
    "Pass45ReferenceRoads[]",
    "ReferencePixelToSectorLocal",
    "PASS45_TACTICAL_REFERENCE_TOPOLOGY_READY",
    "procedural_road_ism=0",
    "procedural_sidewalk_ism=0",
    "procedural_building_ism=0",
    "FOCGeoReference::Museum()",
    "FOCGeoReference::CultureHouse()",
    "FOCGeoReference::Silpo()",
    "FOCGeoReference::Stadium()",
    "FOCGeoReference::CentralPark()",
):
    req(needle in tactical, f"Pass 45 tactical topology contract missing: {needle}")

map_build = re.search(
    r"void\s+UOCTacticalMapWidget::BuildProductionVisualLayer\(\)\s*\{(?P<body>.*)\n\}",
    tactical,
    re.S,
)
if not map_build:
    errors.append("could not isolate BuildProductionVisualLayer")
else:
    map_body = map_build.group("body")
    for forbidden_call in (
        "Sector->GetTacticalRoads()",
        "Sector->GetTacticalSidewalks()",
        "Sector->GetTacticalBuildings()",
        "Sector->GetTacticalResidentialRoofs()",
        "Sector->GetTacticalLandmarkBlocks()",
        "Sector->GetTacticalLandmarkRoofs()",
    ):
        req(forbidden_call not in map_body,
            f"tactical map still derives topology from procedural source ISM: {forbidden_call}")
    req("for (const FPass45ReferenceRoadSegment& Segment : Pass45ReferenceRoads)" in map_body,
        "tactical map does not render the Pass 45 reference-traced road layer")

# Compact render budget: no historical 1300 m family culls.
req("PASS45_COMPACT_WORLD_CULL_BUDGET_READY" in budget, "compact world cull marker missing")
req("130000" not in budget, "historical 1300 m source-family cull distance returned")
req('TEXT("GrassMown"),              0,  16000' in budget,
    "ground-cover cull was not reduced for compact sector")
req('TEXT("ResidentialDetails"),  6000,  24000' in budget,
    "residential detail cull was not reduced for compact sector")

# Primitive tree source still exists as historical authoring data, but must be visually retired at runtime.
for family in (
    "TreeTrunks", "TreeCrowns", "SovietPoplarTrunks", "SovietPoplarCrowns",
    "BirchTrunks", "BirchCrowns", "PineTrunks", "PineCrowns",
):
    req(f'TEXT("{family}")' in foliage, f"foliage guard does not own primitive family retirement: {family}")
req("RetireSourceTreeProxies" in foliage and "bTreeProxyRetirementObserved" in foliage_h,
    "primitive tree retirement path is incomplete")
req("PASS45_PRIMITIVE_TREE_PROXIES_RETIRED" in foliage,
    "primitive tree retirement runtime marker missing")
req("cylinder_sphere_visible=0" in foliage,
    "primitive tree retirement marker does not assert hidden proxy visuals")
req("oak_asset_verified=0" in foliage,
    "Pass 45 must not invent a verified oak asset")

# Verified real pine candidates already exist in current content usage; do not replace them with BasicShapes.
req(has_all(trees, ["SM_Pine_Tree_01", "SM_Pine_Tree_03"]),
    "known real pine assets are no longer referenced by museum tree owner")
req("/Engine/BasicShapes/Cylinder" in world and "/Engine/BasicShapes/Sphere" in world,
    "source verifier expects historical proxy authoring to remain available for audit; source layout unexpectedly changed")

# Every required weapon now gets mesh -> material slot -> texture dependency truth in the fresh NullRHI process.
for needle in (
    "DEPENDENCY_REPORT_PATH",
    "required_weapon_material_texture_dependencies.json",
    "DEPENDENCY_SUCCESS_SENTINEL",
    "unreal.MaterialEditingLibrary.get_used_textures(material)",
    "is_placeholder_texture",
    "missing_texture_dependency",
    "placeholder_texture_dependency",
    "TEXTURE_DEPENDENCY_SUMMARY=",
    "texture_dependency_result",
    "PASS45_WEAPON_DEPENDENCY_AUDIT_COMPLETE",
):
    req(needle in weapon_preflight, f"weapon material/texture dependency audit missing: {needle}")
req('("M16"' not in weapon_preflight and '("M4"' not in weapon_preflight,
    "weapon preflight invents unverified M16/M4 production payloads")
req("white/default slots are NOT production-ready" in weapon_preflight,
    "weapon preflight no longer preserves the user-observed white-material failure semantics")

# Stale renderer verifiers must not force -norhithread back into every normal run.
req("explicit Pass 45 compatibility A/B" in pass43,
    "Pass 43 verifier was not forward-ported for Pass 45 RHI A/B")
req("compatibility A/B route" in pass23,
    "Pass 23 verifier was not forward-ported for Pass 45 RHI A/B")

if errors:
    print("RUNTIME RECOVERY PASS 45: FAIL")
    for e in errors:
        print("[FAIL]", e)
    raise SystemExit(1)

print("RUNTIME RECOVERY PASS 45: PASS")
print("- Pass 44 runtime rejection and screenshot evidence are preserved")
print("- normal DX11/SM5 route restores normal RHI threading; -norhithread is explicit compatibility A/B")
print("- frontend and gameplay performance are sampled separately without quality mutation")
print("- 40-pass landmark full-world startup scan is retired to one delayed reconciliation")
print("- Museum/Silpo/Culture use one current shell owner per site")
print("- tactical map renders reference-traced compact Oster topology instead of procedural world ISMs")
print("- compact-sector culling replaces historical 700-1300 m broad family budgets")
print("- primitive Cylinder/Sphere tree families are hidden from normal gameplay")
print("- real pine candidates remain; oak is explicitly unverified rather than fabricated")
print("- all 11 required weapons emit mesh/material/texture-dependency truth in fresh UE preflight")
print("STATUS: SOURCE CONTRACT ONLY; factual UE 5.8 runtime remains authoritative")
