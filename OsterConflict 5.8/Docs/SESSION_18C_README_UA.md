# S18C — Build Readiness / Static Preflight

S18C не заявляє Unreal-компіляцію. Його задача — максимально зменшити кількість очевидних build-blocker-ів до першого запуску UE 5.8 toolchain на Windows.

## Додано
- `Scripts/S18C/PREFLIGHT_S18C.py` — UHT/RPC/targets/modules/scripts/archive sanity.
- `Scripts/S18C/WINDOWS_TOOLCHAIN_PREFLIGHT.ps1` — перевірка точного UE 5.8 source build, UBT/UAT/EditorCmd, Python і MSVC.
- `Scripts/S18C/ANALYZE_BUILD_LOG.py` — класифікація першої помилки UHT/C++/link/cook/package/runtime.
- `VERIFY_S18C.py` — контроль milestone.

## Порядок першого build
1. На Windows встановити/підготувати UE 5.8 source build та Visual Studio C++ toolchain.
2. Встановити `UE_ROOT`.
3. Запустити `python Scripts/S18C/PREFLIGHT_S18C.py`.
4. Запустити `powershell -ExecutionPolicy Bypass -File Scripts/S18C/WINDOWS_TOOLCHAIN_PREFLIGHT.ps1`.
5. Запустити `Scripts/S18B/BUILD_S18B_ALL.bat`.
6. Якщо build впаде, не правити навмання: передати відповідний log у `ANALYZE_BUILD_LOG.py`.
7. Після package запустити `Scripts/S18B/SMOKE_LOCAL.ps1`.

## Acceptance
S18C source milestone приймається при PASS статичного preflight, усіх попередніх verifiers, Python syntax audit та чистому ZIP. Справжній RC gate проходиться лише після фактичної UE-компіляції/cook/package/smoke.
