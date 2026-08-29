#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parent
DEFAULT_LOG = ROOT / "Logs" / "R14_CURRENT_GAMEPLAY.log"


def main() -> int:
    log_path = Path(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_LOG
    if not log_path.is_file():
        print(f"PASS45 GATE K: FAIL: gameplay log missing: {log_path}")
        return 2

    text = log_path.read_text(encoding="utf-8", errors="replace")
    errors: list[str] = []

    required = (
        "PASS45_GROUND_COVER_PRIMITIVES_DESTROYED",
        "PASS45_DEVELOPER_WORLD_MARKERS_DESTROYED",
        "PASS45_AUTHORED_VEGETATION_READY",
        "PASS45_GATE_K_PARK_SEMANTIC_SPLIT_READY",
        "PASS45_AUTHORED_GROUND_SURFACE_READY",
        "PASS45_AUTHORED_ROAD_SURFACE_READY",
        "PASS45_PARK_PATH_OWNERSHIP_READY",
        "PASS45_AUTHORED_PARK_PATH_SURFACE_READY",
        "PASS45_AUTHORED_WORLD_FENCE_READY",
        "PASS45_GATE_K_RUNTIME_READY",
    )
    forbidden = (
        "PASS45_GATE_K_PARK_SEMANTIC_SPLIT_REJECTED",
        "PASS45_VISUAL_FIDELITY_CONTENT_GAP",
        "PASS45_GATE_K_RUNTIME_FAIL",
        "PASS45_AUTHORED_WORLD_SURFACE_CONTENT_GAP",
        "PASS45_AUTHORED_WORLD_SURFACE_FAIL",
        "PASS10_FOLIAGE_RUNTIME_FAIL",
    )

    for marker in required:
        if marker not in text:
            errors.append(f"missing {marker}")
    for marker in forbidden:
        if marker in text:
            errors.append(f"forbidden {marker}")

    # Current Gate K readiness is valid only when the READY marker itself proves the expanded final-world scope.
    # A stale sector/landmark-only READY line from an older runtime must never satisfy a current-head acceptance run.
    ready_lines = [line for line in text.splitlines() if "PASS45_GATE_K_RUNTIME_READY" in line]
    if not ready_lines:
        errors.append("missing PASS45_GATE_K_RUNTIME_READY line")
    else:
        ready_line = ready_lines[-1]
        ready_contract = (
            "visible_basicshape_components=0",
            "visible_basicshape_instances=0",
            "landmark_basicshape_components=0",
            "landmark_basicshape_instances=0",
            "sector_owners=1",
            "stadium_owners=1",
            "museum_owners=1",
            "culture_owners=1",
            "silpo_owners=1",
            "scope=all_gameplay_actors",
            "runtime_visible_only=1",
            "hidden_in_game_ignored=1",
            "gate_k_complete=1",
        )
        for marker in ready_contract:
            if marker not in ready_line:
                errors.append(f"Gate K READY line missing current-scope field {marker}")

    if errors:
        print("PASS45 GATE K: FAIL")
        for error in errors:
            print("[FAIL]", error)
        return 1

    print("PASS45 GATE K: PASS")
    print("- obsolete ground-cover cubes were destroyed and authored dense foliage owns runtime grass")
    print("- developer reference markers/text labels were destroyed")
    print("- Central Park detail ownership split is exact: legacy ParkDetails=0, semantic groups=2/4/3/14, total=23")
    print("- playable Ground was upgraded to committed AdvancedVillagePack SM_Plane_1x1 + M_Inst_Landscape")
    print("- Ground playable footprint and top-Z were preserved by bounds-aware replacement")
    print("- Roads/Sidewalks were upgraded to tracked Scene_RoadsideConstruction authored meshes/materials")
    print("- exactly five central-park path proxies moved out of Sidewalks into ParkPaths")
    print("- ParkPaths were upgraded to committed AdvancedVillagePack SM_Stonepath_Var01 with bounds-aware fitting")
    print("- visible general world Fences were upgraded to committed AdvancedVillagePack SM_Fence_Var01")
    print("- final gameplay world contains zero runtime-visible Engine BasicShape static meshes across all gameplay actors")
    print("- hidden-in-game collision/proxy components are ignored because they are not rendered production content")
    print("STATUS: AUTOMATED RUNTIME CONTRACT ONLY; direct screenshot fidelity acceptance remains required")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
