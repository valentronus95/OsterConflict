#!/usr/bin/env python3
from pass45_runtime_route_contract import ROOT, require, forbid, validate_runtime_route

engine = (ROOT / "OsterConflict/Config/DefaultEngine.ini").read_text(encoding="utf-8", errors="replace")
route = validate_runtime_route()
start = route["start"]
normal = route["normal"]
evidence = route["evidence"]

for needle in ("[/Script/WindowsTargetPlatform.WindowsTargetSettings]", "DefaultGraphicsRHI=DefaultGraphicsRHI_DX11", "+D3D11TargetedShaderFormats=PCD3D_SM5", "+D3D12TargetedShaderFormats=PCD3D_SM6", "r.DynamicGlobalIlluminationMethod=0", "r.ReflectionMethod=0", "r.Shadow.Virtual.Enable=0", "r.RayTracing=False", "r.HDR.EnableHDROutput=0"):
    require(engine, needle, "boot-safe renderer config")
for flag in ("-d3d11", "-sm5", "-nohdr"):
    require(normal, flag, "canonical gameplay renderer")
for needle in ('set "RHI_FLAGS=-d3d11 -sm5 -nohdr"', 'if /I "%OC_RHI_COMPAT%"=="1"', 'set "RHI_FLAGS=-d3d11 -sm5 -nohdr -norhithread"'):
    require(normal, needle, "Pass45 RHI modes")
require(start, "SAFE СУМІСНІСТЬ", "compatibility route")
for forbidden in ("-d3d12", "-sm6"):
    forbid(normal.lower(), forbidden, "normal launcher")
for marker in ("PASS14_PERF_SAMPLE", "PASS14_PERF_30FPS_READY", "PASS14_PERF_BELOW_TARGET"):
    require(evidence, marker, "runtime renderer/FPS evidence")

print("DX11 SM5 RENDER TARGET PASS 23: PASS")
print("- renderer flags live in the gameplay owner, not duplicated in START_HERE")
print("- project remains DX11/SM5 boot-safe with explicit compatibility fallback")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime decides crash/performance acceptance")
