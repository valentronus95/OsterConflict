#!/usr/bin/env python3
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent
REPORT = ROOT / "_DOCS/PASS45_REMINGTON870_STRUCTURE_AUDIT_2026-09-02.json"
AUDITOR = ROOT / "PASS45_REMINGTON870_STRUCTURE_AUDIT.py"

EXPECTED_SHA256 = "147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2"


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 REMINGTON870 STRUCTURE AUDIT GUARD: FAIL\n[FAIL] {message}")


def require(condition: bool, message: str) -> None:
    if not condition:
        fail(message)


def one_influenced_primitive(target: dict, *, skin_index: int, mesh_index: int, node_name: str) -> dict:
    matching: list[dict] = []
    for skin in target.get("skin_influence") or []:
        if skin.get("skin_index") != skin_index:
            continue
        for mesh in skin.get("skinned_meshes") or []:
            if mesh.get("mesh_index") != mesh_index or mesh.get("node_name") != node_name:
                continue
            matching.extend(mesh.get("primitives") or [])
    require(len(matching) == 1, f"expected one influenced primitive for {target.get('node_name')} skin={skin_index} mesh={mesh_index}, found {len(matching)}")
    return matching[0]


def main() -> None:
    require(REPORT.is_file(), f"missing persisted evidence: {REPORT.relative_to(ROOT)}")
    require(AUDITOR.is_file(), f"missing auditor: {AUDITOR.relative_to(ROOT)}")

    doc = json.loads(REPORT.read_text(encoding="utf-8"))
    require(doc.get("schema") == 2, "persisted evidence is not schema 2")
    require(doc.get("audit") == "PASS45_REMINGTON870_STRUCTURE_AUDIT", "audit identity drifted")
    require(doc.get("status") == "STRUCTURAL_GEOMETRY_SKIN_EVIDENCE_ONLY", "fail-closed source-evidence status drifted")
    require(doc.get("source_sha256") == EXPECTED_SHA256, "pinned donor SHA-256 drifted")
    require(doc.get("source_bytes") == 20621580, "pinned donor byte count drifted")
    require(doc.get("source_git_blob_sha1") == "f822d184d96ede43d79a6f691d69cbe7cf60e686", "pinned transport Git blob drifted")

    for key, expected in (
        ("pump_node_identity", "UNPROVEN"),
        ("standalone_pump_clip", "UNPROVEN"),
        ("ue58_import_pending", True),
        ("visual_inspection_required", True),
        ("production_cutover", False),
        ("runtime_acceptance", False),
        ("item16_checked", False),
    ):
        require(doc.get(key) == expected, f"acceptance boundary drifted: {key}={doc.get(key)!r}")

    targets = doc.get("targets") or {}
    require(set(targets) == {"PBody_058", "Pmag_061", "Rif_059", "Trigger_060"}, "target set drifted")

    pbody = targets["PBody_058"]
    pmag = targets["Pmag_061"]
    require(pbody.get("node_index") == 62, "PBody_058 node index drifted")
    require(pmag.get("node_index") == 64, "Pmag_061 node index drifted")
    require((pbody.get("parent") or {}).get("node_name") == "Root_01", "PBody_058 parent drifted")
    require((pmag.get("parent") or {}).get("node_name") == "Root_01", "Pmag_061 parent drifted")
    require(pmag.get("mesh_bindings_in_subtree") == [], "Pmag_061 unexpectedly gained a direct mesh binding")
    require([row.get("node_name") for row in pmag.get("children") or []] == ["Pmag_end_079"], "Pmag_061 child topology drifted")

    pbody_primitive = one_influenced_primitive(pbody, skin_index=1, mesh_index=3, node_name="Object_91")
    require(pbody_primitive.get("influenced_vertex_count") == 7318, "PBody_058 influenced vertex count drifted")
    require(pbody_primitive.get("max_vertex_weight") == 1.0, "PBody_058 max weight drifted")
    require(pbody_primitive.get("mean_nonzero_vertex_weight") == 1.0, "PBody_058 mean weight drifted")

    pmag_primitive = one_influenced_primitive(pmag, skin_index=3, mesh_index=5, node_name="Object_95")
    require(pmag_primitive.get("influenced_vertex_count") == 4411, "Pmag_061 influenced vertex count drifted")
    require(pmag_primitive.get("max_vertex_weight") == 1.0, "Pmag_061 max weight drifted")
    require(pmag_primitive.get("mean_nonzero_vertex_weight") == 1.0, "Pmag_061 mean weight drifted")
    require((pmag_primitive.get("full_position_bounds") or {}).get("vertex_count") == 4411, "Pmag_061 controlled mesh vertex count drifted")
    require((pmag_primitive.get("full_position_bounds") or {}).get("extent") == [0.03682, 0.150998, 0.091527], "Pmag_061 controlled mesh bounds drifted")

    for semantic in ("easy_reload_index_3", "full_reload_index_4"):
        row = (pmag.get("action_translation") or {}).get(semantic) or {}
        require(row.get("translation_channel") is True, f"Pmag_061 {semantic} translation channel disappeared")
        require(row.get("peak_displacement") == 0.606719, f"Pmag_061 {semantic} displacement drifted")

    auditor = AUDITOR.read_text(encoding="utf-8", errors="replace")
    for forbidden in (
        '"pump_node_identity": "PROVEN"',
        '"standalone_pump_clip": "PROVEN"',
        '"runtime_acceptance": True',
        '"item16_checked": True',
        '/Game/Production/Weapons/Remington870',
    ):
        require(forbidden not in auditor, f"auditor regained forbidden promotion claim/path: {forbidden}")

    print(
        "PASS45 REMINGTON870 STRUCTURE AUDIT GUARD: PASS "
        "pinned_source=1 hierarchy=1 skin_influence=1 pmag_controlled_mesh_vertices=4411 "
        "reload_motion=1 pump_identity_unproven=1 standalone_pump_clip_unproven=1 "
        "ue58_import_pending=1 visual_inspection_required=1 production_cutover=0 "
        "runtime_acceptance=0 item16_checked=0"
    )


if __name__ == "__main__":
    main()
