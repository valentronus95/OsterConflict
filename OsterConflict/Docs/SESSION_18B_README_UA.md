# S18B — First Build / Release Candidate Gate

Статус цього архіву: **build-ready source milestone**, але НЕ підтверджений compiled RC.

## Що додано
- Windows PowerShell build pipeline з `UE_ROOT`.
- UBT Development Editor build.
- Автоматичне створення project-owned `/Game/Maps/OsterConflict_Runtime.umap` через Unreal Editor Python.
- C++ automation smoke tests `OsterConflict.Release.*`.
- Development Client та Dedicated Server BuildCookRun packaging.
- Post-build EXE/data-container audit + SHA-256 manifest.
- Local packaged smoke test: server + 2 clients + `?AutoDeploy=1`.
- Log/crash/trace collector.
- Build fingerprint `oc.BuildInfo` / startup log.

## Чому `.exe` все ще не лежить у цьому ZIP
Поточне середовище розробки не містить Unreal Engine 5.8 source toolchain. S18B автоматизує той самий build/cook/test/package процес, який треба виконати на Windows-машині з UE 5.8 source build. До фактичного PASS цього pipeline жоден `.exe` не вважається готовим.

## Найкоротший запуск на build-машині
1. Встановити/зібрати UE 5.8 source build та Visual Studio toolchain.
2. `set UE_ROOT=D:\UnrealEngine-5.8`
3. Запустити `Scripts\S18B\BUILD_S18B_ALL.bat`.
4. Після PASS запустити `Scripts\S18B\SMOKE_LOCAL.ps1`.
5. Зберегти `Build\S18B\S18B_BUILD_MANIFEST.json`, AutomationReports і логи.

## Release blocker
Якщо compile, UHT, map generation, automation, cook, package або packaged smoke не PASS — RC не існує. Красивий номер версії не лікує помилки компілятора.
