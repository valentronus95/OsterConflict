#!/usr/bin/env python3
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parent
MANIFEST = ROOT / "SOURCE_ASSETS" / "PASS45" / "Remington870" / "MANIFEST.json"
REPORT = ROOT / "_DOCS" / "PASS45_REMINGTON870_DONOR_MOTION_AUDIT_2026-09-01.md"

errors: list[str] = []


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


def near(actual: object, expected: float, tolerance: float = 1e-6) -> bool:
    return isinstance(actual, (int, float)) and math.isclose(
        float(actual), expected, rel_tol=0.0, abs_tol=tolerance
    )


if not MANIFEST.is_file():
    errors.append("missing pinned Remington870 manifest")
    manifest = {}
else:
    try:
        manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    except Exception as exc:
        errors.append(f"invalid Remington870 manifest JSON: {exc}")
        manifest = {}

if not REPORT.is_file():
    errors.append("missing Remington870 donor motion audit report")
    report = ""
else:
    report = REPORT.read_text(encoding="utf-8", errors="replace")

req(
    manifest.get("source_sha256")
    == "147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2",
    "motion evidence is no longer bound to the exact acquired donor SHA-256",
)
req(manifest.get("runtime_ready") is False, "donor motion audit may not claim runtime_ready")
req(manifest.get("ue58_import_pending") is True, "donor motion audit may not clear UE 5.8 import pending")
req(manifest.get("item16_checked") is False, "donor motion evidence alone may not close PASS45 item 16")

expected_fingerprints = {
    "fire_index_2": "5446382d16de5fa56ad784bce41e80be3ae70dc76c03a8a6456e660957f9e5e1",
    "easy_reload_index_3": "4e96b343cc6268a6239d104ae635abf6dbab5609b01dbca37c956e406e26083f",
    "full_reload_index_4": "a93ebf3318791345771e8f775d718d2f89427f568378249d1077090687f29487",
}
action_targets = manifest.get("donor_action_targets")
req(isinstance(action_targets, dict), "manifest missing donor_action_targets")
if isinstance(action_targets, dict):
    for key, expected_fingerprint in expected_fingerprints.items():
        row = action_targets.get(key)
        req(isinstance(row, dict), f"manifest missing action-target row {key}")
        if isinstance(row, dict):
            req(row.get("unique_target_nodes") == 56, f"{key} unique target-node count drifted")
            req(
                row.get("target_fingerprint_sha256") == expected_fingerprint,
                f"{key} target fingerprint drifted",
            )
            names = row.get("target_node_names")
            req(isinstance(names, list), f"{key} target-node names missing")
            if isinstance(names, list):
                for required_name in ("Rif_059", "Trigger_060", "PBody_058", "Pmag_061"):
                    req(required_name in names, f"{key} lost weapon-side target {required_name}")

expected_motion = {
    "fire_index_2": {
        "PBody_058": (0.050905, 24.037146),
        "Pmag_061": (0.050845, 24.037145),
    },
    "easy_reload_index_3": {
        "PBody_058": (0.067139, 25.570902),
        "Pmag_061": (0.606719, 76.062898),
    },
    "full_reload_index_4": {
        "PBody_058": (0.070092, 25.570902),
        "Pmag_061": (0.606719, 76.062898),
    },
}
weapon_motion = manifest.get("donor_weapon_motion")
req(isinstance(weapon_motion, dict), "manifest missing donor_weapon_motion")
if isinstance(weapon_motion, dict):
    for clip_key, nodes in expected_motion.items():
        clip = weapon_motion.get(clip_key)
        req(isinstance(clip, dict), f"manifest missing weapon-motion clip {clip_key}")
        if not isinstance(clip, dict):
            continue
        for node_name, (translation, rotation) in nodes.items():
            row = clip.get(node_name)
            req(isinstance(row, dict), f"{clip_key} missing motion row {node_name}")
            if not isinstance(row, dict):
                continue
            req(
                near(row.get("translation_max_displacement"), translation),
                f"{clip_key} {node_name} translation motion drifted",
            )
            req(
                near(row.get("rotation_max_angle_degrees"), rotation),
                f"{clip_key} {node_name} rotation motion drifted",
            )

if isinstance(weapon_motion, dict):
    fire = weapon_motion.get("fire_index_2") or {}
    easy = weapon_motion.get("easy_reload_index_3") or {}
    fire_pmag = (fire.get("Pmag_061") or {}).get("translation_max_displacement")
    easy_pmag = (easy.get("Pmag_061") or {}).get("translation_max_displacement")
    req(
        isinstance(fire_pmag, (int, float))
        and isinstance(easy_pmag, (int, float))
        and float(easy_pmag) > float(fire_pmag) * 5.0,
        "reload-specific Pmag_061 motion is no longer materially distinct from fire motion",
    )

for needle in (
    "ARTICULATED_RELOAD_MOTION_PROVEN",
    "PUMP_NODE_IDENTITY_UNPROVEN",
    "STANDALONE_PUMP_CLIP_UNPROVEN",
    "must not be silently re-labelled as the authoritative pump cycle",
    "A second Remington donor must not be stacked",
    "production_cutover=0",
    "runtime_acceptance=0",
    "item16_checked=0",
):
    req(needle in report, f"fail-closed donor motion report missing: {needle}")

if errors:
    print("PASS45 REMINGTON870 DONOR MOTION AUDIT: FAIL")
    for error in errors:
        print(f"[FAIL] {error}")
    raise SystemExit(1)

print(
    "PASS45 REMINGTON870 DONOR MOTION AUDIT: PASS "
    "articulated_reload_motion=1 pump_node_identity=0 standalone_pump_clip=0 "
    "production_cutover=0 runtime_acceptance=0 item16_checked=0"
)
