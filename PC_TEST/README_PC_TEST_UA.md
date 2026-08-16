# Oster Conflict — реальна перевірка на Windows / Unreal Engine 5.8

## Що саме дає цей комплект

`START_HERE.cmd` — основна точка входу. Пункт **Compile only** запускає тільки реальну UBT-компіляцію Editor/Client/Server, а пункт **Full validation** запускає повний Unreal pipeline:

1. перевірка UE 5.8 і Visual Studio C++;
2. QueryTargets / toolchain preflight;
3. Generate Project Files;
4. UBT compile `OsterConflictEditor`;
5. UBT compile `OsterConflictClient`;
6. UBT compile `OsterConflictServer`;
7. localization GatherText;
8. запуск `UnrealEditor-Cmd` і створення `/Game/Maps/OsterConflict_Runtime.umap`;
9. `OsterConflict.Release.*` Automation;
10. cook/package Windows Client;
11. cook/package Dedicated Server;
12. packaged server + 2 clients smoke test;
13. створення `PC_TEST/TEST_RESULTS/OSTER_UE58_TEST_RESULTS_*.zip` з логами.

## Що потрібно встановити

### 1. Unreal Engine 5.8 source build

Для повного Dedicated Server тесту потрібна **source build** UE 5.8. Звичайна Launcher-версія може бути корисна для Editor-тесту, але не є достатньою для повного dedicated-server gate.

Після отримання Epic Games Unreal Engine source:

```bat
Setup.bat
GenerateProjectFiles.bat
```

Потім зібрати Unreal Editor/UE5 у Visual Studio.

### 2. Visual Studio 2022

У Visual Studio Installer увімкнути `Game development with C++` та Windows 10/11 SDK. Бажано також C++ profiling tools і AddressSanitizer.

### 3. Python

Бажано мати `python` у PATH. Він потрібний для наших source-verifier/post-build audit scripts. Сам UBT compile може працювати й без системного Python.

## Як запустити

1. Розпакувати ZIP у короткий шлях, наприклад:

```text
D:\OsterTest\
```

2. Не класти проєкт у OneDrive/Dropbox і не використовувати шлях із десятками вкладених папок.
3. Запустити `START_HERE.cmd`.
4. Першим вибрати **1. Compile only**. Після PASS Editor/Client/Server запустити **2. Full validation**.
5. Вказати корінь UE 5.8, наприклад:

```text
D:\UnrealEngine-5.8
```

Це папка, всередині якої є `Engine` і `GenerateProjectFiles.bat`.

## Якщо тест впав

**Не виправляти код навмання.** Комплект сам збере:

```text
PC_TEST\TEST_RESULTS\<timestamp>\SUMMARY.txt
PC_TEST\TEST_RESULTS\<timestamp>\Logs\...
PC_TEST\TEST_RESULTS\OSTER_UE58_TEST_RESULTS_<timestamp>.zip
```

Потрібен саме цей ZIP. По ньому можна визначити першу UHT/C++/link/cook/runtime root cause.

## Важлива межа статусу

`PASS` Full validation доводить, що конкретна машина реально пройшла compile/automation/package/smoke. Він не доводить S19B production map, фінальні assets або S20A performance budgets, доки для них не виконані окремі acceptance runs.

## R7 (із збереженим R6 Launch/Runtime hardening)

- `START_HERE.cmd` is the preferred entry point.
- First run should use **Compile only**, then **Full validation**.
- `PRELAUNCH_CHECK.ps1` validates UE 5.8 source tools, VS/MSVC, Windows SDK, disk/path, port 7777 and captures system/DxDiag evidence.
- Standalone client now opens Frontend automatically unless `-NoFrontend` is supplied.
- Packaged smoke URLs explicitly carry `Protocol=18`.
- Client packaging requests prerequisites.
- After a successful Full run, `RUN_LOCAL_GAME_AFTER_BUILD.cmd` starts the newest packaged dedicated server and a visible Frontend client.


## R7 runtime evidence
- `SMOKE_LOCAL.ps1` writes explicit `SmokeLogs/Server.log`, `Client_SmokeAlpha.log`, `Client_SmokeBravo.log`.
- PASS requires `OC_SERVER_READY`, then both `Human joined: SmokeAlpha` and `Human joined: SmokeBravo`.
- A live process without these markers is a FAIL, not a smoke PASS.
- Client/Server packaged defaults point at `/Game/Maps/OsterConflict_Runtime`.

- Full validation після Automation вимагає експортований `AutomationReports/index.json`; відсутній report = FAIL до packaging.


## R8 — UE 5.8 TargetRules fix

Перший реальний UBT запуск на Windows виявив blocker до компіляції C++: project Editor target використовував backward-compatible build defaults, які UE 5.8 не дозволяє змінювати для target-а зі спільними UnrealEditor build products. У R8 всі чотири targets використовують `BuildSettingsVersion.V7`, `EngineIncludeOrderVersion.Unreal5_8` та `CppStandardVersion.Cpp20`. Для першої Editor-компіляції з установленою Launcher-версією UE 5.8 додано `BUILD_EDITOR_LAUNCHER_UE58.cmd`.
