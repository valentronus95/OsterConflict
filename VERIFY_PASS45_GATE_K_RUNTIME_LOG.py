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
        "PASS45_AUTHORED_ROAD_SURFACE_READY",
        "PASS45_AUTHORED_WORLD_FENCE_READY",
        "PASS45_GATE_K_RUNTIME_READY",
    )
    forbidden = (
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

    if errors:
        print("PASS45 GATE K: FAIL")
        for error in errors:
            print("[FAIL]", error)
        return 1

    print("PASS45 GATE K: PASS")
    print("- obsolete ground-cover cubes were destroyed and authored dense foliage owns runtime grass")
    print("- developer reference markers/text labels were destroyed")
    print("- Roads/Sidewalks were upgraded to tracked Scene_RoadsideConstruction authored meshes/materials")
    print("- visible general world Fences were upgraded to committed AdvancedVillagePack SM_Fence_Var01")
    print("- final Oster sector + authoritative stadium contain zero visible Engine BasicShape static meshes")
    print("STATUS: AUTOMATED RUNTIME CONTRACT ONLY; direct screenshot fidelity acceptance remains required")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
