from pathlib import Path
import csv
import sys

ROOT = Path(__file__).resolve().parent
LEDGER = ROOT / "PASS45_CONTENT_INTEGRATION_LEDGER.csv"

if not LEDGER.exists():
    print("PASS45_GAMEPLAY_CONTENT_INTEGRATION_FAIL reason=missing_ledger runtime_acceptance=0")
    sys.exit(1)

with LEDGER.open("r", encoding="utf-8-sig", newline="") as fh:
    rows = list(csv.DictReader(fh))

if not rows:
    print("PASS45_GAMEPLAY_CONTENT_INTEGRATION_FAIL reason=empty_ledger runtime_acceptance=0")
    sys.exit(1)

required_prefixes = (
    "CI-ARMS-", "CI-ANIM-", "CI-HUD-", "CI-INT-", "CI-WPN-",
    "CI-VFX-", "CI-AUDIO-", "CI-WORLD-", "CI-PROP-", "CI-VEH-",
)

missing_roles = [prefix for prefix in required_prefixes if not any((r.get("AssetId") or "").startswith(prefix) for r in rows)]
if missing_roles:
    print("PASS45_GAMEPLAY_CONTENT_INTEGRATION_FAIL reason=missing_role_rows runtime_acceptance=0")
    for prefix in missing_roles:
        print(f"- missing role: {prefix}")
    sys.exit(1)

accepted_states = {"INTEGRATED", "RUNTIME_ACCEPTED", "EXCLUDED_WITH_REASON"}
blocking = []
for row in rows:
    state = (row.get("State") or "").strip().upper()
    asset_id = (row.get("AssetId") or "UNKNOWN").strip()
    # Candidate-only alternatives may remain unselected without blocking the canonical path.
    if state in {"CANDIDATE_ONLY", "NOT_SELECTED"}:
        continue
    if state not in accepted_states:
        blocking.append((asset_id, state or "MISSING_STATE", row.get("TargetIntegration", "")))

if blocking:
    print(f"PASS45_GAMEPLAY_CONTENT_INTEGRATION_PENDING blocking={len(blocking)} runtime_acceptance=0")
    for asset_id, state, target in blocking:
        print(f"- {asset_id}: state={state} target={target}")
    print("NOTE: downloaded/content-present assets are not integration completion.")
    sys.exit(2)

print("PASS45_GAMEPLAY_CONTENT_INTEGRATION_READY integrated_or_excluded=1 runtime_acceptance=pending_or_recorded")
