#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent


def read(rel: str) -> str:
    path = ROOT / rel
    if not path.is_file():
        raise SystemExit(f"PASS45 REMINGTON870 TRANSPORT CONSUMER CONTRACT: FAIL\n[FAIL] missing file: {rel}")
    return path.read_text(encoding="utf-8", errors="replace")


errors: list[str] = []


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


auditor = read("PASS45_REMINGTON870_REMOTE_CANDIDATE_AUDIT.py")
workflow = read(".github/workflows/pass45-remington870-remote-candidate-audit.yml")
intake = read("_DOCS/PASS45_REMINGTON870_SOURCE_INTAKE.md")

for needle in (
    'REPO = "Parking-Master/FPS"',
    'COMMIT = "ed07ea542111c2149c5dab735e752824d0b0541c"',
    'PATH = "models/weapons/shotgun.glb"',
    'EXPECTED_GIT_BLOB_SHA1 = "f822d184d96ede43d79a6f691d69cbe7cf60e686"',
    'CONSUMER_PATH = "src.html"',
    'EXPECTED_CONSUMER_GIT_BLOB_SHA1 = "18400e77c5b54b44e38dfd5cfd37a70efd19d43b"',
    'EXPECTED_CONSUMER_SIZE = 125420',
    'sub.add_parser("audit-consumer-contract")',
    'fire_expr = "gunsAnimations[weaponDefs.indexOf(currentWeapon)][2]"',
    'easy_reload_expr = "gunsAnimations[weaponDefs.indexOf(currentWeapon)][3]"',
    'full_reload_expr = "gunsAnimations[weaponDefs.indexOf(currentWeapon)][4]"',
    '"shotgun_fire_routes_shared_fire": 1',
    '"separate_consumer_pump_invocation": 0',
    '"fire_clip_internal_pump_phase": "UNPROVEN"',
    '"pump_node_identity": "UNPROVEN"',
    '"ue58_import_pending": 1',
    '"runtime_acceptance": 0',
    '"item16_checked": 0',
):
    req(needle in auditor, f"remote auditor lost pinned consumer evidence: {needle}")

for forbidden in (
    '"runtime_acceptance": 1',
    '"item16_checked": 1',
    '/Game/Production/Weapons/Remington870',
    'AssetImportTask',
):
    req(forbidden not in auditor, f"consumer audit must not claim production/runtime authority: {forbidden}")

for needle in (
    'python PASS45_REMINGTON870_REMOTE_CANDIDATE_AUDIT.py audit-consumer-contract',
    'steps.consumer.outputs.consumer_git_blob_sha1',
    '18400e77c5b54b44e38dfd5cfd37a70efd19d43b',
    'steps.consumer.outputs.consumer_fire_clip_index',
    'steps.consumer.outputs.consumer_easy_reload_clip_index',
    'steps.consumer.outputs.consumer_full_reload_clip_index',
    'steps.consumer.outputs.separate_consumer_pump_invocation',
    'steps.consumer.outputs.fire_clip_internal_pump_phase',
    'steps.consumer.outputs.runtime_acceptance',
    'steps.consumer.outputs.item16_checked',
):
    req(needle in workflow, f"remote workflow does not enforce consumer contract: {needle}")

for needle in (
    'animation index `2` for fire',
    'index `3` for ordinary/easy reload',
    'index `4` for full/empty reload',
    'Direct first-person pump/action visual acceptance: `PENDING`',
    'runtime_ready=false',
    'ue58_import_pending=true',
    'item16_checked=false',
):
    req(needle in intake, f"source-intake contract lost fail-closed item-16 state: {needle}")

if errors:
    print("PASS45 REMINGTON870 TRANSPORT CONSUMER CONTRACT: FAIL")
    for error in errors:
        print(f"[FAIL] {error}")
    raise SystemExit(1)

print(
    "PASS45 REMINGTON870 TRANSPORT CONSUMER CONTRACT: PASS "
    "fire_index=2 easy_reload_index=3 full_reload_index=4 "
    "separate_consumer_pump_invocation=0 fire_clip_internal_pump_phase=UNPROVEN "
    "runtime_acceptance=0 item16_checked=0"
)
