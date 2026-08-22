from pathlib import Path

ROOT = Path(__file__).resolve().parent
MAIN = ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd"
STRICT = ROOT / "RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd"
PASS8 = ROOT / "VERIFY_RUNTIME_RECONCILE_PASS_8.py"

for path in (MAIN, STRICT, PASS8):
    if not path.is_file():
        raise SystemExit(f"MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: missing {path.name}")

main = MAIN.read_text(encoding="utf-8")
strict = STRICT.read_text(encoding="utf-8")

required_main = (
    'if /I "%OC_FORCE_ACCEPTANCE%"=="1" set "IS_ACCEPTANCE=1"',
    'if "%IS_ACCEPTANCE%"=="1" if /I "%CURRENT_BRANCH%"=="main"',
    'VERIFY_RUNTIME_RECONCILE_PASS_8.py',
    'PASS7_PRODUCTION_VEHICLE_RUNTIME_FAIL',
    'PASS7_PRODUCTION_WEAPON_RUNTIME_FAIL',
    'PASS7_PRODUCTION_VEHICLES_READY',
    'PASS7_PRODUCTION_WEAPONS_READY',
    'PASS7_MUSEUM_BASES_READY',
    'git fetch origin "%FETCH_BRANCH%"',
    'git rev-parse "%REMOTE_REF%"',
)
for marker in required_main:
    if marker not in main:
        raise SystemExit(f"MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: missing main marker {marker!r}")

required_strict = (
    'set "OC_FORCE_ACCEPTANCE=1"',
    'call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"',
    'if not "%RC%"=="0"',
    'AUTOMATED RUNTIME EVIDENCE GATES PASSED',
)
for marker in required_strict:
    if marker not in strict:
        raise SystemExit(f"MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: missing strict wrapper marker {marker!r}")

if 'set "OC_FORCE_ACCEPTANCE=0"' in strict:
    raise SystemExit("MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: strict wrapper disables its own acceptance flag")

print("MAIN RUNTIME ACCEPTANCE LAUNCHER SOURCE CONTRACT PASS")
print("- normal main launcher remains available")
print("- strict wrapper can force automated runtime evidence checks on current main")
print("- Pass 8 source verification runs before the UE build when present")
print("- Museum BASE + production weapons + production vehicles are mandatory runtime markers in strict mode")
print("STATUS: SOURCE VERIFIED ONLY; local Windows UE 5.8 execution is still required")
