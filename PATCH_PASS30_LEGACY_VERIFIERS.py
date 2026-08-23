from pathlib import Path

ROOT = Path(__file__).resolve().parent


def replace_once(path: Path, old: str, new: str, label: str) -> None:
    text = path.read_text(encoding="utf-8")
    if old not in text:
        raise SystemExit(f"Pass 30 verifier anchor missing: {label}")
    path.write_text(text.replace(old, new, 1), encoding="utf-8")


# 1) World-model verifier: the rural-cabin frame asset remains checked in for other buildings, but
# museum runtime must not stretch it after the user's screenshot proved that presentation invalid.
world = ROOT / "VERIFY_OSTER_WORLD_MODELS_PASS.py"
replace_once(world,
'''        # Museum styled windows keep the replicated AOCBreakableWindow glass state, but their visible
        # frame now prefers a checked-in authored frame profile rather than six BasicShape cubes.
        require_text(museum_window_cpp, "Window_Frame_Part.Window_Frame_Part", "museum authored frame path")
        require_text(museum_window_cpp, "FitAuthoredFramePart", "museum frame bounds-fitting helper")
        require_text(museum_window_cpp, "FQuat::FindBetweenNormals", "museum frame longest-axis orientation")
        require_text(museum_window_cpp, "OC_AuthoredMuseumWindowFrame", "museum authored frame tag")
        require_text(museum_window_cpp, "Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);",
                     "museum frame visual-only collision")
        require_text(museum_window_cpp, "!Component->ComponentHasTag(AuthoredMuseumFrameTag)",
                     "museum authored materials preserved")
        require_text(museum_window_cpp, "GlassPane->SetMaterial(0, Glass);",
                     "museum breakable glass material preserved")
''',
'''        # Pass 30 supersedes the generic rural-cabin frame for Museum only. Runtime screenshots proved
        # longest-axis fitting produced oversized/rusty strips, so the museum window now uses a clean,
        # lightweight collision-free frame while keeping the replicated breakable glass state.
        require_text(museum_window_cpp, "PASS30_MUSEUM_WINDOW_FRAME_CLEAN_READY", "museum clean frame marker")
        require("Window_Frame_Part.Window_Frame_Part" not in museum_window_cpp,
                "museum must not reuse the distorted rural-cabin frame")
        require("FitAuthoredFramePart" not in museum_window_cpp,
                "museum must not restore axis-stretched frame fitting")
        require_text(museum_window_cpp, "Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);",
                     "museum frame visual-only collision")
        require_text(museum_window_cpp, "Component->SetCastShadow(false);",
                     "museum frame lightweight shadow contract")
        require_text(museum_window_cpp, "GlassPane->SetMaterial(0, Glass);",
                     "museum breakable glass material preserved")
''',
"world-model museum frame contract")

# 2) Pass 7: previous verifier treated <=35 m from MuseumAnchor as success. That exact contract is what
# allowed a BASE inside the building. Require the new exclusion/recovery path instead.
pass7 = ROOT / "VERIFY_RUNTIME_ACCEPTANCE_PASS_7.py"
replace_once(pass7,
'''require(spawn_guard, 'FVector::DistSquared2D(Point->GetActorLocation(), Museum) > FMath::Square(3500.0f)', "stale BASE relocation guard")
require(spawn_guard, 'Point->ConfigureServer(Team, true, NAME_None);', "canonical BASE repair path")
require(spawn_guard, 'EnsurePrimary(EOCTeam::TeamOne, TeamOnePrimary);', "TeamOne primary BASE guarantee")
require(spawn_guard, 'EnsurePrimary(EOCTeam::TeamTwo, TeamTwoPrimary);', "TeamTwo primary BASE guarantee")
require(spawn_guard, 'GetRequestedDeploymentSpawn()', "actual BASE-selected pawn validation")
require(spawn_guard, 'PASS15_BASE_DEPLOYMENT_NEAR_MUSEUM', "actual pawn Museum evidence")
require(spawn_guard, 'PASS15_BASE_DEPLOYMENT_RECOVERED', "legacy fallback correction path")
require(spawn_guard, 'PASS7_MUSEUM_BASES_READY', "runtime Museum BASE evidence marker")
''',
'''require(spawn_guard, 'constexpr float MuseumNoSpawnRadiusCm = 3000.0f;', "Museum no-spawn exclusion")
require(spawn_guard, 'Point->ConfigureServer(Team, true, NAME_None);', "canonical BASE repair path")
require(spawn_guard, 'EnsurePrimary(EOCTeam::TeamOne, TeamOnePrimary);', "TeamOne primary BASE guarantee")
require(spawn_guard, 'EnsurePrimary(EOCTeam::TeamTwo, TeamTwoPrimary);', "TeamTwo primary BASE guarantee")
require(spawn_guard, 'GetRequestedDeploymentSpawn()', "actual BASE-selected pawn validation")
require(spawn_guard, 'PASS30_MUSEUM_EXTERIOR_BASES_READY', "exterior Museum BASE evidence")
require(spawn_guard, 'PASS30_BASE_DEPLOYMENT_OUTSIDE_MUSEUM', "actual exterior pawn evidence")
require(spawn_guard, 'PASS30_BASE_DEPLOYMENT_RECOVERED_OUTSIDE_MUSEUM', "interior BASE recovery path")
require(spawn_guard, 'PASS7_MUSEUM_BASES_READY', "Pass 7 compatibility readiness marker")
require(team_spawn, 'PASS30_BASE_RELOCATED_OUTSIDE_MUSEUM', "canonical exterior BASE relocation")
require(team_spawn, 'FVector(-2600.0f, -3200.0f, 120.0f)', "TeamOne exterior BASE offset")
require(team_spawn, 'FVector(2600.0f, -3200.0f, 120.0f)', "TeamTwo exterior BASE offset")
''',
"Pass 7 museum BASE contract")

# 3) Pass 8: later performance recovery is explicitly allowed to make the grid sparser. 40 m is now
# intentional after the actual 8 FPS playtest, so the old 24 m upper bound is obsolete.
pass8 = ROOT / "VERIFY_RUNTIME_RECONCILE_PASS_8.py"
replace_once(pass8,
'''if not grid or not 900.0 <= float(grid.group(1)) <= 2400.0:
    raise SystemExit("PASS 8 FAIL: foliage grid is missing or outside the supported incremental range")
''',
'''if not grid or not 900.0 <= float(grid.group(1)) <= 5000.0:
    raise SystemExit("PASS 8 FAIL: foliage grid is missing or outside the supported incremental range")
''',
"Pass 8 sparse foliage range")

# 4) Pass 3: replace the historical near-anchor offsets/distance probe with the current exterior BASE
# source contract. BeginPlay still canonicalizes every serialized BASE via ConfigureServer.
pass3 = ROOT / "VERIFY_RUNTIME_ACCEPTANCE_PASS_2.py"
replace_once(pass3,
'''for needle in (
    "void AOCTeamSpawnPoint::BeginPlay()",
    "AOCWorldSectorOster::MuseumAnchor()",
    "FVector::DistSquared2D(GetActorLocation(), Museum)",
    "ConfigureServer(TeamId, true, NAME_None);",
    "SpawnRuntimeBaseWeaponRack",
    "FVector(-1450.0f, -900.0f, 120.0f)",
    "FVector(1450.0f, 900.0f, 120.0f)",
):
    require(spawn_cpp, needle, "runtime museum spawn")
''',
'''for needle in (
    "void AOCTeamSpawnPoint::BeginPlay()",
    "AOCWorldSectorOster::MuseumAnchor()",
    "ConfigureServer(TeamId, true, NAME_None);",
    "SpawnRuntimeBaseWeaponRack",
    "PASS30_BASE_RELOCATED_OUTSIDE_MUSEUM",
    "FVector(-2600.0f, -3200.0f, 120.0f)",
    "FVector(2600.0f, -3200.0f, 120.0f)",
):
    require(spawn_cpp, needle, "runtime museum exterior spawn")
for forbidden in (
    "FVector(-1450.0f, -900.0f, 120.0f)",
    "FVector(1450.0f, 900.0f, 120.0f)",
):
    if forbidden in spawn_cpp:
        raise SystemExit(f"RUNTIME ACCEPTANCE PASS 3 FAIL: old interior-adjacent BASE returned: {forbidden}")
''',
"Pass 3 museum exterior BASE contract")

replace_once(pass3,
'print("- BASE is placed directly beside the Museum test hub")',
'print("- BASE is placed on the exterior Museum approach, outside the building exclusion radius")',
"Pass 3 result wording")

print("PASS30 LEGACY VERIFIERS ALIGNED")
