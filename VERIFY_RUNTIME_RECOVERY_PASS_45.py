#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parent
PRIVATE = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private"
errors: list[str] = []


def read(path: Path) -> str:
    if not path.is_file():
        errors.append(f"missing file: {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def require(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


tz = read(ROOT / "PASS45_RUNTIME_RECOVERY_TZ.md")
recovery_tz = read(ROOT / "GAME_RECOVERY.md")
launcher = read(ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd")
startup = read(PRIVATE / "OCLandmarkStartupCoordinatorSubsystem.cpp")
local_weapon_override = read(PRIVATE / "OCLocalInboxWeaponOverrideSubsystem.cpp")
imported_weapon_bridge = read(PRIVATE / "OCPass45ImportedWeaponBridgeSubsystem.cpp")
local_asset_resolver = read(PRIVATE / "OCPass45LocalAssetResolver.cpp")
real_weapon_fallback = read(PRIVATE / "OCRealWeaponFallbackSubsystem.cpp")

require("RUNTIME REJECTED" in tz and "RUNTIME ACCEPTANCE DEFERRED" in tz,
        "Pass45 runtime rejection/deferred acceptance truth was lost")
for flag in ("runtime_acceptance=0", "merge_permitted=0"):
    require(flag in tz, f"Pass45 factual acceptance flag missing: {flag}")

require("fix/pass45-runtime-rejection-material-closure-20260826" in recovery_tz,
        "GAME_RECOVERY single working branch contract is missing")
require("Definition of Done" in recovery_tz and "UE 5.8 runtime" in recovery_tz,
        "GAME_RECOVERY no longer requires factual UE 5.8 runtime acceptance")

for token in ("-d3d11", "-sm5", "-nohdr", "-nosplash"):
    require(token in launcher, f"normal launcher renderer token missing: {token}")
require('set "RHI_FLAGS=-d3d11 -sm5 -nohdr -nosplash"' in launcher,
        "normal DX11/SM5 RHI-thread baseline is missing")
require('if /I "%OC_RHI_COMPAT%"=="1"' in launcher and "-norhithread" in launcher,
        "explicit no-RHI-thread compatibility route is missing")
require("-d3d12" not in launcher.lower() and "-sm6" not in launcher.lower(),
        "normal recovery route must not force D3D12/SM6")
require("t.MaxFPS 60" in launcher,
        "normal recovery route lost the 60 FPS thermal guard")

parts = launcher.split(":quick_normal_game", 1)
require(len(parts) == 2, "quick-normal launcher section is missing")
if len(parts) == 2:
    quick = parts[1]
    require("-windowed" in quick.lower(), "quick-normal route is not windowed")
    res_x = re.search(r"-ResX=(\d+)", quick, re.IGNORECASE)
    res_y = re.search(r"-ResY=(\d+)", quick, re.IGNORECASE)
    require(res_x is not None and int(res_x.group(1)) >= 1280,
            "quick-normal horizontal resolution is below 1280")
    require(res_y is not None and int(res_y.group(1)) >= 720,
            "quick-normal vertical resolution is below 720")

for token in (
    "CancelHistoricalStageTimers",
    "GAME_RECOVERY_WORLD_PREP_TIMERS_CANCELLED",
    "UOCGameRecoveryStadiumActivationSubsystem",
    "IsStadiumPresentationReady",
    "GAME_RECOVERY_WORLD_READY",
    "stadium_ready=1",
    "post_spawn_landmark_materialization=0",
):
    require(token in startup, f"current staged landmark startup contract missing: {token}")

require("LoadObject<" not in local_weapon_override,
        "local inbox weapon override regained blocking LoadObject")
for token in (
    "RequestAsyncLoad(",
    "FSoftObjectPath(ObjectPath).ResolveObject()",
    "OC_ProductionWeaponVisual",
    "GetWeaponVisualRoot()",
    "GAME_RECOVERY_LOCAL_WEAPON_PRELOAD_BEGIN",
    "GAME_RECOVERY_LOCAL_WEAPON_PRELOAD_GAP",
    "resident_asset=1",
    "visual_root_unscaled=1",
    "physics_root_preserved=1",
    "sync_load=0",
):
    require(token in local_weapon_override,
            f"local inbox async/resident weapon contract missing: {token}")

for stale_call in (
    "OCPass45FindLocalSkeletalMeshStrict(Query.Roots",
    "OCPass45FindLocalStaticMeshStrict(Query.Roots",
):
    require(stale_call not in imported_weapon_bridge,
            f"imported weapon bridge regained sync resolver call: {stale_call}")
for token in (
    "OCPass45FindLocalSkeletalMeshPathStrict",
    "OCPass45FindLocalStaticMeshPathStrict",
    "RequestAsyncLoad(",
    "AssetPath.ResolveObject()",
    "OC_LocalInboxWeaponBound",
    "OC_LocalInboxWeaponPreloadPending",
    "GAME_RECOVERY_IMPORTED_WEAPON_PRELOAD_BEGIN",
    "GAME_RECOVERY_IMPORTED_WEAPON_PRELOAD_GAP",
    "resident_asset=1",
    "blocking_asset_loads=0",
    "sync_load=0",
):
    require(token in imported_weapon_bridge,
            f"imported bridge async/resident ownership contract missing: {token}")

for token in (
    "FSoftObjectPath ResolvePath(",
    "FSoftObjectPath(Best.GetObjectPathString())",
    "OCPass45FindLocalStaticMeshPathStrict",
    "OCPass45FindLocalSkeletalMeshPathStrict",
):
    require(token in local_asset_resolver,
            f"metadata-only local asset resolver contract missing: {token}")

# Real fallback meshes are normal-game safety content. They used to synchronously load four packages at world begin
# and another AK package during refresh. They must now preload together and be consumed resident-only.
require("LoadObject<" not in real_weapon_fallback,
        "real weapon fallback regained blocking LoadObject")
for token in (
    "BuildFallbackPreloadPaths()",
    "RequestAsyncLoad(",
    "CompleteFallbackPreload",
    "ResolveResidentStaticMesh",
    "FSoftObjectPath(Path).ResolveObject()",
    "GAME_RECOVERY_REAL_WEAPON_FALLBACK_PRELOAD_BEGIN",
    "GAME_RECOVERY_REAL_WEAPON_FALLBACK_PRELOAD_GAP",
    "GAME_RECOVERY_REAL_WEAPON_FALLBACK_PRELOAD_READY",
    "resident_until_deinitialize=1",
    "resident_only=1",
    "sync_load=0",
    "AuthoredAKFallback.Get()",
):
    require(token in real_weapon_fallback,
            f"real weapon fallback async/resident contract missing: {token}")

delegated = (
    "VERIFY_SLATE_RENDER_TARGET_STARTUP_PASS_43.py",
    "VERIFY_DX11_SM5_RENDER_TARGET_PASS_23.py",
    "VERIFY_PASS45_GRENADE_TYPE_PRESENTATION_LIFECYCLE.py",
    "VERIFY_PASS45_GRENADE_SMOKE_PRIMITIVE_RETIREMENT.py",
    "VERIFY_PASS45_BTR4_MATERIAL_STATE.py",
    "VERIFY_PASS45_BTR4_AXIS_REMOTE_OPTIC.py",
    "VERIFY_PASS45_HMMWV_M2_HIERARCHY.py",
    "VERIFY_GAME_RECOVERY_STADIUM_PRELOAD.py",
    "VERIFY_PASS45_REFERENCE_DRIVEN_RESIDENTIAL_RETIREMENT.py",
    "VERIFY_PASS45_PRIMITIVE_WEAPON_RETIREMENT.py",
    "VERIFY_PASS45_WEAPON_AUDIO_FALLBACK.py",
)

for name in delegated:
    script = ROOT / name
    if not script.is_file():
        errors.append(f"delegated current source gate missing: {name}")
        continue
    print(f"[GAME_RECOVERY] running {name}")
    result = subprocess.run([sys.executable, str(script)], cwd=ROOT, check=False)
    if result.returncode != 0:
        errors.append(f"delegated current source gate failed: {name} rc={result.returncode}")

if errors:
    print("RUNTIME RECOVERY PASS 45: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("RUNTIME RECOVERY PASS 45: PASS")
print("- current recovery gate delegates specialized source contracts instead of duplicating stale assertions")
print("- DX11/SM5 startup, grenades, production vehicles, stadium, weapon proxy retirement and reference-driven map rules are guarded")
print("- both normal-game local weapon visual owners async-load missing assets and bind only resident production meshes")
print("- real weapon fallback meshes preload asynchronously and refresh uses only resident meshes")
print("- weapon fallback audio preloads before first use; shot/reload/manual-action/impact never issue blocking LoadObject")
print("- imported weapon bridge uses metadata-only AssetRegistry path selection and yields to LocalInbox ownership")
print("- staged landmark readiness uses current GAME_RECOVERY markers")
print("STATUS: SOURCE CONTRACT ONLY; factual UE 5.8 runtime remains authoritative")
