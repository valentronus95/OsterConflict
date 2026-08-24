#!/usr/bin/env python3
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private"
PUB = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public"

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
foliage = text(SRC / "OCFoliageRuntimeGuardSubsystem.cpp")
foliage_h = text(PUB / "OCFoliageRuntimeGuardSubsystem.h")
budget = text(SRC / "OCWorldRenderBudgetPass17Subsystem.cpp")
world = text(SRC / "OCWorldSectorOster.cpp")
trees = text(SRC / "OCR145MuseumTreeLayoutSubsystem.cpp")
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
    "source verifier expects historical proxy authoring to remain visible for audit; source layout unexpectedly changed")

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
print("- compact-sector culling replaces historical 700-1300 m broad family budgets")
print("- primitive Cylinder/Sphere tree families are hidden from normal gameplay")
print("- real pine candidates remain; oak is explicitly unverified rather than fabricated")
print("STATUS: SOURCE CONTRACT ONLY; factual UE 5.8 runtime remains authoritative")
