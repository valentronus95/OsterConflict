from pathlib import Path

ROOT = Path(__file__).resolve().parent


def read(rel: str) -> str:
    path = ROOT / rel
    if not path.is_file():
        raise AssertionError(f"missing canonical runtime file: {rel}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, where: str) -> None:
    if needle not in text:
        raise AssertionError(f"{where}: missing {needle!r}")


def forbid(text: str, needle: str, where: str) -> None:
    if needle in text:
        raise AssertionError(f"{where}: forbidden {needle!r}")


def validate_runtime_route() -> dict[str, str]:
    start = read("START_HERE.cmd")
    batch_cmd = read("RUN_PASS45_BATCH_RUNTIME_TEST.cmd")
    batch = read("OsterConflict/Scripts/run_pass45_batch_runtime_test.ps1")
    normal = read("RUN_R14_CURRENT_GAMEPLAY.cmd")
    material = read("OsterConflict/RUN_PASS45_STRICT_MATERIAL_GATE.cmd")
    evidence = read("VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py")
    finalizer = read("OsterConflict/Scripts/finalize_asset_acceptance.py")
    all_assets = read("OsterConflict/IMPORT_ALL_LOCAL_INBOX_UE58.cmd")

    for marker in (
        "Єдиний користувацький launcher/test entrypoint: START_HERE.cmd.",
        "2. ПОВНИЙ RUNTIME-ТЕСТ ^(ПАКЕТНИЙ^)",
        'set "CURRENT_GAMEPLAY=%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"',
        'set "BATCH_RUNTIME=%~dp0RUN_PASS45_BATCH_RUNTIME_TEST.cmd"',
        'call "%BATCH_RUNTIME%"',
        'set "OC_RHI_COMPAT=1"',
        'set "OC_RHI_COMPAT=0"',
        'call "%CURRENT_GAMEPLAY%"',
    ):
        require(start, marker, "START_HERE delegation")
    for stale in (
        ":full_runtime_test",
        ":ingest_all_assets",
        "PASS45_REIMPORT_STEIN_WEAPON_MATERIALS_UE58.cmd",
        "PASS45_IMPORT_MANUAL_ACTION_AUDIO_UE58.cmd",
        "PASS45_IMPORT_REMINGTON870_PRODUCTION_UE58.cmd",
        "RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd",
        "RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd",
        "FINALIZE_ASSET_ACCEPTANCE_AND_CLEANUP.cmd",
    ):
        forbid(start, stale, "START_HERE single-owner contract")

    require(batch_cmd, 'powershell -NoProfile -ExecutionPolicy Bypass -File "%RUNNER%"', "batch wrapper")
    for marker in (
        "IMPORT_ALL_LOCAL_INBOX_UE58.cmd",
        "RUN_PASS45_STRICT_MATERIAL_GATE.cmd",
        "RUN_R14_CURRENT_GAMEPLAY.cmd",
        "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py",
        "finalize_asset_acceptance.py",
        "PASS45_BATCH_RUNTIME_REPORT.txt",
        "Tracked Changes:",
        "FORMAL_ACCEPTANCE=BLOCKED_DIRTY_OR_NONEXACT_SOURCE",
        '& $Command @Arguments',
        '$env:OC_FORCE_ACCEPTANCE = "0"',
        '$env:OC_FORCE_ACCEPTANCE = "1"',
        "Single gameplay runtime",
        "FAILURE TAILS:",
        'Read-Host "Ви реально оглянули assets і приймаєте їх візуальний стан? [Y/N]"',
        '@($Finalizer, "--preflight")',
        '@($Finalizer, "--accept-visual")',
    ):
        require(batch, marker, "packet runtime owner")
    for stale in (
        "PASS45_REIMPORT_STEIN_WEAPON_MATERIALS_UE58.cmd",
        "PASS45_IMPORT_MANUAL_ACTION_AUDIO_UE58.cmd",
        "PASS45_IMPORT_REMINGTON870_PRODUCTION_UE58.cmd",
        "cmd /c",
        '\\"C:',
    ):
        forbid(batch, stale, "packet runtime owner")

    for marker in (
        'set "RHI_FLAGS=-d3d11 -sm5 -nohdr"',
        'if /I "%OC_RHI_COMPAT%"=="1"',
        'set "RHI_FLAGS=-d3d11 -sm5 -nohdr -norhithread"',
        'if "%IS_ACCEPTANCE%"=="1" (',
        'call "%PRODUCTION_IMPORT%"',
    ):
        require(normal, marker, "canonical gameplay owner")

    for marker in (
        "PASS45_AUTHORED_WEAPON_MATERIALS=PASS",
        "PASS45_WEAPON_DEPENDENCY_REPORT=PASS",
        "PASS45_PRODUCTION_VEHICLE_VISUALS_VALIDATED_READY",
    ):
        require(material, marker, "strict material gate")
    for marker in (
        "PASS45_RUNTIME_AUTOMATED_EVIDENCE=PASS",
        "VISUAL_ACCEPTANCE=PENDING_MANUAL_OBSERVATION",
        "PASS14_PERF_30FPS_READY",
        "PASS14_PERF_BELOW_TARGET",
        "PASS19_PLAYABLE_WEAPON_SET_READY",
        "PASS19_PLAYABLE_WEAPON_SET_FAIL",
    ):
        require(evidence, marker, "canonical runtime evidence")
    for marker in (
        'preflight_only = "--preflight" in args',
        'accept_visual = "--accept-visual" in args',
        "run_preflight()",
        "write_manual_acceptance(head)",
    ):
        require(finalizer, marker, "manual finalizer")
    for marker in (
        "DIRTY_ACCEPTANCE_SOURCE",
        "lfs pull origin",
        "import_all_project_assets.py",
        "normalize_local_weapon_categories.py",
    ):
        require(all_assets, marker, "aggregate asset intake")

    return {
        "start": start,
        "batch_cmd": batch_cmd,
        "batch": batch,
        "normal": normal,
        "material": material,
        "evidence": evidence,
        "finalizer": finalizer,
        "all_assets": all_assets,
    }
