#!/usr/bin/env python3
from pass45_runtime_route_contract import require, forbid, validate_runtime_route

route = validate_runtime_route()
start = route["start"]
normal = route["normal"]
evidence = route["evidence"]

for needle in ("1. ЗВИЧАЙНА ГРА", "2. ПОВНИЙ RUNTIME-ТЕСТ ^(ПАКЕТНИЙ^)", "3. SAFE СУМІСНІСТЬ", "4. ВІДКРИТИ UNREAL EDITOR", 'set "OC_RHI_COMPAT=1"', 'set "OC_RHI_COMPAT=0"'):
    require(start, needle, "single launcher UI")
for needle in ('set "RHI_FLAGS=-d3d11 -sm5 -nohdr"', 'set "RHI_FLAGS=-d3d11 -sm5 -nohdr -norhithread"', 'set "RHI_MODE=dx11_sm5_rhi_thread"', 'set "RHI_MODE=dx11_sm5_no_rhi_thread_compat"'):
    require(normal, needle, "renderer owner")
for forbidden in ("-d3d12", "-dx12", "-sm6"):
    forbid(normal.lower(), forbidden, "normal gameplay renderer")
require(normal, "[LOCAL CHANGE]", "local-change visibility")
for marker in ("PASS29_MAIN_START_DIRECT_HOST_QUEUED", "PASS19_PLAYABLE_WEAPON_SET_READY", "PASS14_PERF_30FPS_READY"):
    require(evidence, marker, "canonical runtime evidence")

print("SINGLE LAUNCHER / D3D11 PASS22/PASS45 SOURCE CONTRACT PASS")
print("- START_HERE delegates; RUN_R14_CURRENT_GAMEPLAY owns DX11/SM5 flags")
print("- -norhithread remains compatibility-only")
print("STATUS: SOURCE CONTRACT ONLY; local UE runtime must confirm stability and FPS")
