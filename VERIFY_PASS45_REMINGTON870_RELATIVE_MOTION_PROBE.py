#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
PROBE = ROOT / "PASS45_REMINGTON870_RELATIVE_MOTION_PROBE.py"
WORKFLOW = ROOT / ".github/workflows/pass45-remington870-remote-candidate-audit.yml"


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 REMINGTON870 RELATIVE MOTION PROBE GUARD: FAIL\n[FAIL] {message}")


def require(condition: bool, message: str) -> None:
    if not condition:
        fail(message)


def main() -> None:
    require(PROBE.is_file(), f"missing probe: {PROBE.name}")
    require(WORKFLOW.is_file(), f"missing workflow: {WORKFLOW.relative_to(ROOT)}")
    probe = PROBE.read_text(encoding="utf-8", errors="replace")
    workflow = WORKFLOW.read_text(encoding="utf-8", errors="replace")

    for needle in (
        'import PASS45_REMINGTON870_REMOTE_CANDIDATE_AUDIT as remote',
        'import PASS45_REMINGTON870_SOURCE_ACQUIRE as acquire',
        'EXPECTED_SHA256 = "147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2"',
        'PBODY_NAME = "PBody_058"',
        'PMAG_NAME = "Pmag_061"',
        'EXPECTED_SHARED_PARENT = "Root_01"',
        'identity = remote.verify_pinned_bytes(data)',
        'binary_payload = acquire.glb_binary_chunk(data)',
        '"translation", 3',
        '"rotation", 4',
        'quaternion_multiply(quaternion_inverse(pbody), pmag)',
        'quaternion_multiply(quaternion_inverse(origin), current)',
        '"sibling_relative_translation_measured": 1',
        '"sibling_relative_rotation_measured": 1',
        '"fire_relative_rotation_degrees"',
        '"easy_reload_relative_rotation_degrees"',
        '"full_reload_relative_rotation_degrees"',
        '"pump_node_identity": "UNPROVEN"',
        '"standalone_pump_clip": "UNPROVEN"',
        '"fire_clip_internal_pump_phase": "UNPROVEN"',
        '"ue58_import_pending": 1',
        '"runtime_acceptance": 0',
        '"item16_checked": 0',
    ):
        require(needle in probe, f"probe contract missing: {needle}")

    for forbidden in (
        "remote.fetch_bytes(",
        "urllib",
        "requests.",
        "AssetImportTask",
        "/Game/Production/Weapons/Remington870",
        '"pump_node_identity": "PROVEN"',
        '"standalone_pump_clip": "PROVEN"',
        '"fire_clip_internal_pump_phase": "PROVEN"',
        '"runtime_acceptance": 1',
        '"item16_checked": 1',
    ):
        require(forbidden not in probe, f"probe regained forbidden network/import/promotion behavior: {forbidden}")

    for needle in (
        "PASS45_REMINGTON870_RELATIVE_MOTION_PROBE.py",
        'id: relative_motion',
        'python PASS45_REMINGTON870_RELATIVE_MOTION_PROBE.py "$RUNNER_TEMP/remington870_candidate.glb"',
        "steps.relative_motion.outputs.fire_relative_peak",
        "steps.relative_motion.outputs.easy_reload_relative_peak",
        "steps.relative_motion.outputs.full_reload_relative_peak",
        "steps.relative_motion.outputs.fire_relative_rotation_degrees",
        "steps.relative_motion.outputs.easy_reload_relative_rotation_degrees",
        "steps.relative_motion.outputs.full_reload_relative_rotation_degrees",
        "steps.relative_motion.outputs.sibling_relative_rotation_measured",
        "steps.relative_motion.outputs.shared_parent",
        "steps.relative_motion.outputs.pump_node_identity",
        "steps.relative_motion.outputs.fire_clip_internal_pump_phase",
        "steps.relative_motion.outputs.runtime_acceptance",
        "steps.relative_motion.outputs.item16_checked",
    ):
        require(needle in workflow, f"workflow does not enforce relative-motion proof: {needle}")

    print(
        "PASS45 REMINGTON870 RELATIVE MOTION PROBE GUARD: PASS "
        "local_exact_bytes_only=1 sibling_translation=1 sibling_rotation=1 "
        "pump_identity_unproven=1 fire_internal_pump_phase_unproven=1 "
        "runtime_acceptance=0 item16_checked=0"
    )


if __name__ == "__main__":
    main()
