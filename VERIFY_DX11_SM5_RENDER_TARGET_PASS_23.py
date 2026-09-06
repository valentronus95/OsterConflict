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

# Current GAME_RECOVERY graphics profile keeps the stable DX11/SM5 backend while restoring
# DX11-safe visual quality. Do not regress to the old flat diagnostic profile merely to satisfy Pass23.
for token in (
    "r.DynamicGlobalIlluminationMethod=2",
    "r.ReflectionMethod=2",
    "r.Shadow.Virtual.Enable=0",
    "r.AntiAliasingMethod=2",
    "r.GenerateMeshDistanceFields=False",
    "r.RayTracing=False",
    "r.HDR.EnableHDROutput=0",
):
    req(token in engine, f"current DX11-safe renderer token missing: {token}")

for name, text in (("START_HERE", start), ("normal launcher", normal), ("recovery launcher", recovery)):
    for flag in ("-d3d11", "-sm5", "-nohdr"):
        req(flag in text, f"{name} is missing safe renderer flag {flag}")

req('set "RHI_FLAGS=-d3d11 -sm5 -nohdr -nosplash"' in normal,
    "normal launcher does not expose the current Pass45 RHI-thread baseline")
req('if /I "%OC_RHI_COMPAT%"=="1"' in normal,
    "normal launcher is missing explicit Pass45 compatibility selection")
req("-norhithread" in normal,
    "normal launcher lost the no-RHI-thread compatibility fallback")
req("SAFE СУМІСНІСТЬ" in start and 'set "OC_RHI_COMPAT=1"' in start,
    "START_HERE does not expose the Pass45 compatibility A/B route")
req('set "OC_RHI_COMPAT=0"' in start,
    "START_HERE normal route does not explicitly select normal RHI threading")
req("t.MaxFPS 60" in normal and "r.ScreenPercentage 100" in normal,
    "normal DX11 route lost the current 60 FPS / 100% render-scale guard")

req("-d3d12" not in normal.lower(), "normal launcher still forces D3D12")
req("-sm6" not in normal.lower(), "normal launcher still forces SM6")
req("fix/dx11-sm5-" in normal, "normal launcher cannot test the isolated Pass23 branch")
req("fix/runtime-recovery-" in normal, "normal launcher cannot test runtime-recovery branches")
req("fix/pass45-runtime-rejection-" in normal, "normal launcher cannot test the canonical Pass45 recovery branch")
req("fix/dx11-sm5-" in recovery, "recovery launcher cannot test the isolated Pass23 branch")

if errors:
    print("DX11 SM5 RENDER TARGET PASS 23: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("DX11 SM5 RENDER TARGET PASS 23: PASS")
print("- project remains pinned to DX11/SM5 with HDR, VSM, ray tracing and DX12-only paths disabled")
print("- current DX11-safe GI/reflections/TAA quality profile is preserved instead of the retired flat diagnostic profile")
print("- normal route uses RHI threading; -norhithread remains explicit compatibility-only")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime decides crash/performance acceptance")
