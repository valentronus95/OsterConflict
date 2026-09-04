#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
ENGINE = ROOT / "OsterConflict" / "Config" / "DefaultEngine.ini"
START = ROOT / "START_HERE.cmd"
NORMAL = ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd"
EVIDENCE = ROOT / "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"

errors = []


def req(cond, msg):
    if not cond:
        errors.append(msg)


engine = ENGINE.read_text(encoding="utf-8", errors="replace")
start = START.read_text(encoding="utf-8", errors="replace")
normal = NORMAL.read_text(encoding="utf-8", errors="replace")
evidence = EVIDENCE.read_text(encoding="utf-8", errors="replace")

req("[/Script/WindowsTargetPlatform.WindowsTargetSettings]" in engine,
    "WindowsTargetSettings section is missing")
req("DefaultGraphicsRHI=DefaultGraphicsRHI_DX11" in engine,
    "project default RHI is not pinned to DX11")
req("+D3D11TargetedShaderFormats=PCD3D_SM5" in engine,
    "DX11 SM5 shader target is missing")
req("+D3D12TargetedShaderFormats=PCD3D_SM6" in engine,
    "future DX12 SM6 shader target is not preserved")
for token in [
    "r.DynamicGlobalIlluminationMethod=0",
    "r.ReflectionMethod=0",
    "r.Shadow.Virtual.Enable=0",
    "r.AntiAliasingMethod=1",
    "r.GenerateMeshDistanceFields=False",
    "r.RayTracing=False",
    "r.HDR.EnableHDROutput=0",
]:
    req(token in engine, f"boot-safe renderer token missing: {token}")

# Current Pass45 has one renderer authority: START_HERE selects normal/compat mode, then the canonical
# gameplay launcher owns the actual DX11/SM5/no-HDR flags. Retired focused recovery launchers are not required.
for flag in ["-d3d11", "-sm5", "-nohdr"]:
    req(flag in start, f"START_HERE is missing safe renderer flag {flag}")
    req(flag in normal, f"normal launcher is missing safe renderer flag {flag}")

req('set "RHI_FLAGS=-d3d11 -sm5 -nohdr"' in normal,
    "normal launcher does not expose the Pass45 RHI-thread baseline")
req('if /I "%OC_RHI_COMPAT%"=="1"' in normal,
    "normal launcher is missing explicit Pass45 compatibility selection")
req('set "RHI_FLAGS=-d3d11 -sm5 -nohdr -norhithread"' in normal,
    "normal launcher lost the explicit no-RHI-thread compatibility fallback")
req('SAFE СУМІСНІСТЬ' in start and 'set "OC_RHI_COMPAT=1"' in start,
    "START_HERE does not expose the Pass45 compatibility A/B route")
req('set "OC_RHI_COMPAT=0"' in start,
    "START_HERE normal route does not explicitly select normal RHI threading")
req('set "CURRENT_GAMEPLAY=%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"' in start,
    "START_HERE no longer owns the canonical gameplay launcher")
req('set "EVIDENCE_VERIFY=%~dp0VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"' in start,
    "START_HERE no longer owns the canonical runtime evidence verifier")

req("-d3d12" not in normal.lower(), "normal launcher still forces D3D12")
req("-sm6" not in normal.lower(), "normal launcher still forces SM6")
req("fix/dx11-sm5-" in normal, "normal launcher cannot test the isolated Pass23 branch")
req("fix/runtime-recovery-" in normal, "normal launcher cannot test runtime recovery branches")
req("fix/pass45-runtime-rejection-" in normal, "normal launcher cannot test Pass45 runtime-rejection branches")

# Renderer acceptance is factual only when the canonical runtime evidence includes the current >=30 FPS
# gate and the low-FPS failure marker. Source checks must not resurrect old per-pass acceptance launchers.
for marker in ("PASS14_PERF_SAMPLE", "PASS14_PERF_30FPS_READY", "PASS14_PERF_BELOW_TARGET"):
    req(marker in evidence, f"canonical runtime evidence lost renderer/performance marker {marker}")
for retired in (
    "RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd",
    "RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd",
):
    req(retired not in start, f"retired renderer/recovery launcher leaked into START_HERE: {retired}")

if errors:
    print("DX11 SM5 RENDER TARGET PASS 23: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("DX11 SM5 RENDER TARGET PASS 23: PASS")
print("- project RHI is pinned to DX11 and DX11 shaders target SM5")
print("- boot renderer disables HDR, Lumen/reflections/VSM/ray tracing before frontend Slate creation")
print("- Pass45 normal route uses DX11/SM5/no-HDR with normal RHI threading")
print("- -norhithread remains available only as an explicit compatibility A/B route")
print("- canonical Pass45 evidence owns runtime renderer/FPS acceptance; retired focused launchers are not required")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime decides crash/performance acceptance")
