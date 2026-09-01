#!/usr/bin/env python3
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent


def read(rel: str) -> str:
    path = ROOT / rel
    if not path.is_file():
        raise SystemExit(f"PASS45 REMINGTON870 SOURCE INTAKE: FAIL\n[FAIL] missing file: {rel}")
    return path.read_text(encoding="utf-8", errors="replace")


errors: list[str] = []


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


contract = read("_DOCS/PASS45_REMINGTON870_SOURCE_INTAKE.md")
third_party_register = read("_DOCS/THIRD_PARTY_CODE_AND_ASSET_REGISTER.md")
remote_audit = read("PASS45_REMINGTON870_REMOTE_CANDIDATE_AUDIT.py")
variants = read("OsterConflict/Source/OsterConflict/Private/OCWeaponVariants.cpp")

for needle in (
    "/Game/Production/Weapons/Remington870/SM_Remington870.SM_Remington870",
    "PASS45_WEAPON_PRODUCTION_VISUAL_GAP weapon=Remington870 primitive_visible=0 real_fallback_pending=1",
):
    req(needle in variants, f"canonical Remington fail-closed production contract missing: {needle}")

for needle in (
    "OSTER SOURCE ACQUIRED / UE 5.8 IMPORT PENDING / RUNTIME UNACCEPTED",
    "SOURCE_ACQUIRED_APPROVED_FOR_UE_IMPORT",
    "Parking-Master/FPS",
    "ed07ea542111c2149c5dab735e752824d0b0541c",
    "models/weapons/shotgun.glb",
    "f822d184d96ede43d79a6f691d69cbe7cf60e686",
    "20621580",
    "eea11de7e9d24b6683962b8388c319eb",
    "8sianDude",
    "CC-BY-4.0",
    "animation index `2` for fire",
    "index `3` for ordinary/easy reload",
    "index `4` for full/empty reload",
    "SOURCE_ASSETS/PASS45/Remington870/MANIFEST.json",
    "SOURCE_ASSETS/PASS45/Remington870/remington_870_8siandude_ccby4.glb",
    "177285c68fd693ff1570f3025fae5890128eae17",
    "147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2",
    "5 animations, 6 meshes, 109 nodes and 4 skins",
    "fire index 2 -> 71 channels",
    "easy reload index 3 -> 71 channels",
    "full reload index 4 -> 72 channels",
    "static-only geometry presented as completed skeletal/manual-action content -> reject",
    "remote Git blob pin is useful acquisition evidence but is not an Oster-owned `source_sha256` record",
    "_DOCS/THIRD_PARTY_CODE_AND_ASSET_REGISTER.md",
    "runtime_ready=false",
    "ue58_import_pending=true",
    "item16_checked=false",
    "oster_source_bytes_acquired=1",
    "accepted_remington870_source_for_ue_import=1",
    "tracked_production_package=0",
    "ue58_runtime_acceptance=0",
):
    req(needle in contract, f"Remington source-intake contract missing current fail-closed evidence: {needle}")

for needle in (
    'REPO = "Parking-Master/FPS"',
    'COMMIT = "ed07ea542111c2149c5dab735e752824d0b0541c"',
    'PATH = "models/weapons/shotgun.glb"',
    'EXPECTED_GIT_BLOB_SHA1 = "f822d184d96ede43d79a6f691d69cbe7cf60e686"',
    "EXPECTED_SIZE = 20621580",
    'UPSTREAM_MODEL_ID = "eea11de7e9d24b6683962b8388c319eb"',
    'UPSTREAM_CREATOR = "8sianDude"',
    'LICENSE_ID = "CC-BY-4.0"',
    "git_blob_sha1(data)",
    "len(animations) < 5",
    'for index, semantic in ((2, "fire"), (3, "easy_reload"), (4, "full_reload"))',
    "if len(skins) < 1",
):
    req(needle in remote_audit, f"remote Remington candidate audit is no longer pinned/fail-closed: {needle}")

for needle in (
    "PASS45-3P-WEAPON-001",
    "Remington 870 animated source donor",
    "8sianDude",
    "https://sketchfab.com/3d-models/remington-870-eea11de7e9d24b6683962b8388c319eb",
    "CC-BY-4.0",
    "ed07ea542111c2149c5dab735e752824d0b0541c",
    "f822d184d96ede43d79a6f691d69cbe7cf60e686",
    "147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2",
    "SOURCE_ASSETS/PASS45/Remington870/remington_870_8siandude_ccby4.glb",
    "SOURCE_ASSETS/PASS45/Remington870/MANIFEST.json",
    "ATTRIBUTION_REQUIRED: yes",
    "RUNTIME_DEPENDENCY: not yet",
    "UE_5_8_BUILD_EVIDENCE: pending",
    "VISUAL_AUDIO_ACCEPTANCE: pending",
    "CUTOVER_COMMIT: pending",
):
    req(needle in third_party_register,
        f"mandatory Remington870 third-party actual-import record missing evidence: {needle}")

production_asset = ROOT / "OsterConflict" / "Content" / "Production" / "Weapons" / "Remington870" / "SM_Remington870.uasset"
manifest_path = ROOT / "SOURCE_ASSETS" / "PASS45" / "Remington870" / "MANIFEST.json"
source_path = ROOT / "SOURCE_ASSETS" / "PASS45" / "Remington870" / "remington_870_8siandude_ccby4.glb"

req(manifest_path.is_file(), "acquired Remington870 source is missing mandatory MANIFEST.json")
req(source_path.is_file(), "acquired Remington870 source GLB/LFS pointer is missing")

if source_path.is_file():
    pointer = source_path.read_text(encoding="utf-8", errors="replace")
    req("version https://git-lfs.github.com/spec/v1" in pointer,
        "Remington870 source must remain Git LFS controlled in source-verification checkout")
    req("oid sha256:147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2" in pointer,
        "Remington870 Git LFS OID drifted from pinned acquired source SHA-256")
    req("size 20621580" in pointer,
        "Remington870 Git LFS pointer size drifted from pinned acquired source size")

if production_asset.is_file():
    req(manifest_path.is_file(),
        "tracked Remington870 production asset exists without mandatory SOURCE_ASSETS/PASS45/Remington870/MANIFEST.json")

if manifest_path.is_file():
    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    except Exception as exc:
        errors.append(f"Remington870 manifest is invalid JSON: {exc}")
        manifest = {}

    req(manifest.get("schema") == 1, "Remington870 manifest schema must be 1")
    req(manifest.get("weapon") == "Remington870", "Remington870 manifest weapon identity mismatch")
    req(manifest.get("status") == "APPROVED_FOR_UE_IMPORT",
        "Remington870 manifest must be APPROVED_FOR_UE_IMPORT before production package introduction")

    for key in ("source_name", "source_url", "source_model_id", "license_id", "license_url", "attribution", "derivative_notes"):
        value = manifest.get(key)
        req(isinstance(value, str) and bool(value.strip()), f"Remington870 manifest missing non-empty {key}")

    req(manifest.get("source_name") == "Remington 870", "Remington870 source_name drifted")
    req(manifest.get("source_model_id") == "eea11de7e9d24b6683962b8388c319eb", "Remington870 model id drifted")
    req(manifest.get("license_id") == "CC-BY-4.0", "Remington870 license id drifted")
    req(manifest.get("source_git_blob_sha1") == "f822d184d96ede43d79a6f691d69cbe7cf60e686", "Remington870 source Git blob pin drifted")
    req(manifest.get("source_transport_commit") == "ed07ea542111c2149c5dab735e752824d0b0541c", "Remington870 transport commit drifted")
    req(manifest.get("source_transport_path") == "models/weapons/shotgun.glb", "Remington870 transport path drifted")
    req(manifest.get("source_bytes") == 20621580, "Remington870 acquired byte size drifted")
    req(manifest.get("source_sha256") == "147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2",
        "Remington870 acquired source SHA-256 drifted")
    req(manifest.get("donor_animation_count") == 5, "Remington870 donor animation count drifted")
    req(manifest.get("donor_mesh_count") == 6, "Remington870 donor mesh count drifted")
    req(manifest.get("donor_node_count") == 109, "Remington870 donor node count drifted")
    req(manifest.get("donor_skin_count") == 4, "Remington870 donor skin count drifted")
    req(manifest.get("proven_donor_action_channels") == {
        "easy_reload_index_3": 71,
        "fire_index_2": 71,
        "full_reload_index_4": 72,
    }, "Remington870 donor action-channel probe drifted")

    req(manifest.get("public_repo_allowed") is True,
        "Remington870 source is not confirmed redistributable in this public repository")
    source_sha = manifest.get("source_sha256", "")
    req(isinstance(source_sha, str) and re.fullmatch(r"[0-9a-f]{64}", source_sha) is not None,
        "Remington870 manifest requires exact lowercase SHA-256 of acquired source bytes")
    req(manifest.get("rigged_or_articulated") is True,
        "Remington870 accepted source must prove rigged/articulated moving-part capability")
    req(manifest.get("animation_capable") is True,
        "Remington870 accepted source must prove animation capability")

    clips = manifest.get("intended_fp_clips")
    req(isinstance(clips, list), "Remington870 intended_fp_clips must be a list")
    if isinstance(clips, list):
        normalized = {str(item).strip().lower() for item in clips}
        req({"ironsight", "fire", "reload", "dryfire"}.issubset(normalized),
            "Remington870 manifest must preserve intended ironsight/fire/reload/dryfire FP coverage")

    # Intake approval is deliberately not runtime acceptance.
    req(manifest.get("runtime_ready") is False,
        "Remington870 intake manifest may not claim runtime_ready before current-head UE 5.8 acceptance")
    req(manifest.get("ue58_import_pending") is True,
        "Remington870 intake manifest must remain ue58_import_pending until factual UE 5.8 import acceptance")
    req(manifest.get("item16_checked") is False,
        "Remington870 intake alone may not close PASS45 item 16")

if errors:
    print("PASS45 REMINGTON870 SOURCE INTAKE: FAIL")
    for error in errors:
        print(f"[FAIL] {error}")
    raise SystemExit(1)

if production_asset.is_file():
    print("PASS45 REMINGTON870 SOURCE INTAKE: PASS source_acquired=1 manifest_guard=1 third_party_register=1 production_asset=1 runtime_acceptance=0")
else:
    print("PASS45 REMINGTON870 SOURCE INTAKE: PASS source_acquired=1 manifest_guard=1 third_party_register=1 production_asset=0 ue58_import_pending=1 runtime_acceptance=0")
