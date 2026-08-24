#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
ENGINE = ROOT / "OsterConflict" / "Config" / "DefaultEngine.ini"
START = ROOT / "START_HERE.cmd"
NORMAL = ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd"
RECOVERY = ROOT / "RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd"

errors = []

def req(cond, msg):
    if not cond:
        errors.append(msg)

engine = ENGINE.read_text(encoding="utf-8", errors="replace")
start = START.read_text(encoding="utf-8", errors="replace")
normal = NORMAL.read_text(encoding="utf-8", errors="replace")
recovery = RECOVERY.read_text(encoding="utf-8", errors="replace")

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

# Pass 45 preserves the DX11/SM5/no-HDR renderer isolation. The latest factual menu-at-8-FPS run makes
# -norhithread a diagnostic compatibility route rather than a permanent normal-game requirement.
for name, text in [("START_HERE", start), ("normal launcher", normal), ("recovery launcher", recovery)]:
    for flag in ["-d3d11", "-sm5", "-nohdr"]:
        req(flag in text, f"{name} is missing safe renderer flag {flag}")

req('set "RHI_FLAGS=-d3d11 -sm5 -nohdr"' in normal,
    "normal launcher does not expose the Pass 45 RHI-thread baseline")
req('if /I "%OC_RHI_COMPAT%"=="1"' in normal,
    "normal launcher is missing explicit Pass 45 compatibility selection")
req('-norhithread' in normal,
    "normal launcher lost the explicit no-RHI-thread compatibility fallback")
req('SAFE СУМІСНІСТЬ' in start and 'set "OC_RHI_COMPAT=1"' in start,
    "START_HERE does not expose the Pass 45 compatibility A/B route")
req('set "OC_RHI_COMPAT=0"' in start,
    "START_HERE normal route does not explicitly select normal RHI threading")

req("-d3d12" not in normal.lower(), "normal launcher still forces D3D12")
req("-sm6" not in normal.lower(), "normal launcher still forces SM6")
req("fix/dx11-sm5-" in normal, "normal launcher cannot test the isolated Pass 23 branch")
req("fix/runtime-recovery-" in normal, "normal launcher cannot test the active Pass 45 branch")
req("fix/dx11-sm5-" in recovery, "recovery launcher cannot test the isolated Pass 23 branch")

if errors:
    print("DX11 SM5 RENDER TARGET PASS 23: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("DX11 SM5 RENDER TARGET PASS 23: PASS")
print("- project RHI is pinned to DX11 and DX11 shaders target SM5")
print("- boot renderer disables HDR, Lumen/reflections/VSM/ray tracing before frontend Slate creation")
print("- Pass 45 normal route uses DX11/SM5/no-HDR with normal RHI threading")
print("- -norhithread remains available only as an explicit compatibility A/B route")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime decides crash/performance acceptance")
