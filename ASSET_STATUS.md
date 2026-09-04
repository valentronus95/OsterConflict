# OSTER CONFLICT — ASSET STATUS

Date: 2026-09-04  
Branch: `fix/pass45-asset-import-fail-closed-20260904`  
Base/current main: `a1ad0e200611911102c48180956d82f73d0d8fc3`  
Last fully verified code checkpoint: `15090c44eda2448e1c4e4ce3a85ea906b2b7841f` — **19/19 SUCCESS**  
PR: #98 — Draft, unmerged, mergeable  
Branch relation at verified code checkpoint: **ahead 77 / behind 0**, merge-base = current `main@a1ad0e2`  
Changed-file scope: **25 files**, intentional asset/runtime/finalization scope  
PR review threads: **0** · submitted reviews: **0**

## 1. ГОЛОВНА ТАБЛИЦЯ ПРОГРЕСУ

Кожен із 10 етапів = 10% загального прогресу.

| № | Етап | Стан | Виконано | Вклад | Що вже зроблено | Що лишилось |
|---:|---|---|---:|---:|---|---|
| 1 | Local inbox / intake contract | DONE | 100% | +10% | `models_game_OC` lifecycle і safe local-only policy визначені | Нічого |
| 2 | Prepare / extract / classify | DONE | 100% | +10% | ZIP/loose sources безпечно готуються й класифікуються | Нічого |
| 3 | Exact duplicate removal | DONE | 100% | +10% | SHA-256 dedupe працює до UE import | Нічого |
| 4 | Fab / Marketplace / project discovery | DONE | 100% | +10% | Скануються `/Game` і project/plugin mounts | Нічого |
| 5 | Production import logic | DONE | 100% | +10% | HMMWV, M2, BTR-4, M249, Remington 870 import paths готові | Нічого по коду |
| 6 | Fail-closed aggregate result | DONE | 100% | +10% | GAP/exception блокує фальшивий PASS; stale import PASS markers очищаються | Нічого |
| 7 | GitHub source / regression CI | DONE | 100% | +10% | `15090c4`: **19/19 SUCCESS**; exact-head, dirty-source, freshness, runtime і finalization guards активні | Нічого |
| 8 | Local UE 5.8 import result | WAIT | 0% підтверджено | +0% | Import PASS можливий лише на exact remote HEAD, clean tracked runtime source, explicit current import result=0 і fresh snapshot | Потрібен фактичний local UE 5.8 import |
| 9 | Live gameplay/runtime hookup | WAIT | 0% підтверджено | +0% | Current gameplay/material/weapon/world evidence fail-closed; final runtime snapshot обов'язковий | Потрібен factual full runtime PASS |
| 10 | Direct visual acceptance + safe ZIP cleanup | WAIT | 0% підтверджено | +0% | **Кодовий шлях завершений і CI-захищений**: finalization вбудований у `START_HERE`, manual Y/N + exact SHA/hash cleanup | Потрібен фактичний visual inspection після успішного runtime і factual M16/M4 payload |

### ЗАГАЛЬНИЙ ПРОГРЕС

| Показник | Значення |
|---|---:|
| Завершено етапів | **7 / 10** |
| Загальний factual прогрес | **70%** |
| Залишилось | **30%** |
| Source/code/CI частина | **100% на `15090c4`** |
| Local UE import acceptance | **0% підтверджено** |
| Live runtime acceptance | **0% підтверджено** |
| Direct visual acceptance | **0% підтверджено** |
| Failed/cancelled workflows на verified code head | **0 / 19** |

Поточний стан: **70%**. Кодовий lifecycle від intake до final cleanup тепер повний, але останні 30% не можна чесно зарахувати без локального UE/runtime/manual evidence.

Шлях: **70% → local UE import PASS = 80% → live runtime PASS = 90% → direct visual acceptance + safe ZIP cleanup = 100%.**

## 2. PRODUCTION ASSET MATRIX

| Asset | Стан | Підтверджено | Чого бракує |
|---|---|---:|---|
| HMMWV | WAIT | 25% | Fresh UE import, live use, scale/orientation/material visual proof |
| M2 Browning | WAIT | 25% | Fresh UE import, HMMWV mount, pitch/muzzle/material visual proof |
| BTR-4 | WAIT | 25% | Fresh UE result, live use, proportions/orientation/material visual proof |
| M249 | WAIT | 25% | Exact local payload/UE result/runtime/visual proof |
| Remington 870 | WAIT | 25% | Exact local payload/UE result/runtime/visual proof |
| M16/M4 family | GAP | 0% READY | Fresh `runtime_bindings.json` має фактично показати `M16_M4 >= 1`; без цього finalization до 100% заблокований |

`25%` означає тільки підтверджений source/import support, не готовність asset у грі.

GitHub tracked evidence досі не дає підстав зняти M16/M4 gap. Local/Fab payload може існувати, але це повинен довести fresh current-run manifest.

## 3. OTHER LOCAL / FAB ASSET FAMILIES

AK-47, MP5, M1911, M700, M14, MAC-10, TEC-9, Lever Action, інші weapon/launcher assets, pickups/vehicle props, buildings, props/furniture/fences, foliage/trees/grass, roads/sidewalks, ground/terrain, water/river, character skins, HUD/UI та Fab/Marketplace/project-plugin meshes мають intake/classification support, але current runtime/visual state лишається **PENDING** до fresh manifest і live proof.

## 4. FAIL-CLOSED ACCEPTANCE CONTRACT

### 4.1 Exact remote HEAD before import

`START_HERE.cmd` до UE import виконує fetch + exact comparison:

- Git missing → `66`;
- branch unknown → `67`;
- fetch failed → `68`;
- local/remote HEAD unknown → `69`;
- local HEAD != `origin/<current branch>` → `70`;
- лише exact match дозволяє asset ingest.

### 4.2 Dirty tracked runtime/source guard

`IMPORT_ALL_LOCAL_INBOX_UE58.cmd` до evidence cleanup/LFS/build/UE import перевіряє tracked runtime/source files:

- launcher-и;
- collector/evidence verifier;
- production/material CMD;
- `OsterConflict/Scripts`;
- `OsterConflict/Source`.

Untracked local payloads та `Content` не блокуються. Dirty tracked runtime/source → code `59`.

### 4.3 Explicit current import result + fresh snapshot

`COLLECT_LOCAL_ASSET_STATUS.py` тепер schema `oster-conflict-local-asset-status-v4`.

- `import_result is None` → `LOCAL_UE_IMPORT=PENDING_CURRENT_RUN`;
- current nonzero import code → `FAIL`;
- `PASS` можливий лише при explicit current `import_result=0` + required vehicle/weapon/binding/all-models-bound PASS;
- stale `LOCAL_ASSET_STATUS.txt/json` видаляються перед fresh collection;
- collector missing → `62`;
- Python missing → `63`;
- collector nonzero → `64`;
- missing txt/json → `65`;
- importer success + snapshot failure = overall nonzero.

### 4.4 Runtime/material/weapon freshness

- `START_HERE` очищає old local inbox/world runtime reports перед current gameplay;
- `RUN_R14_CURRENT_GAMEPLAY.cmd` очищає gameplay/preflight logs/sentinels;
- strict material gate очищає old weapon report/sentinel/material log;
- canonical evidence verifier вимагає current gameplay + material + exact weapon dependency proof;
- final runtime PASS fail-closed, якщо final consolidated status не записався.

`RUNTIME_SCOPE`:

| Scope | Значення |
|---|---|
| `IMPORT_ONLY` | runtime/material/evidence = `PENDING_CURRENT_RUN` |
| `CURRENT_RUN_FAILED` | runtime FAIL, exact current runtime code; old PASS не успадковується |
| `CURRENT_RUN_COMPLETED` | PASS дозволений лише за current runtime/material/evidence |

## 5. STAGE 10 — MANUAL VISUAL ACCEPTANCE + ZIP CLEANUP

Знайдений у цьому pass фактичний пропуск: tracker вимагав visual acceptance/cleanup, але в коді не було безпечного способу їх зафіксувати й завершити.

Тепер це закрито по коду.

### 5.1 Один user-facing launcher

`START_HERE.cmd` лишається **єдиним** user-facing launcher. Окремий `FINALIZE_ASSET_ACCEPTANCE_AND_CLEANUP.cmd`, створений під час аудиту, був видалений до фінального checkpoint, щоб не повернути старий хаос із багатьма запускалками.

Після `2. ПОВНИЙ RUNTIME-ТЕСТ`:

1. проходить import;
2. live gameplay/runtime;
3. local inbox/world proof;
4. strict material/weapon gate;
5. canonical automated evidence;
6. запускається **non-destructive finalization preflight**;
7. лише якщо preflight PASS, `START_HERE` показує visual checklist і питає Y/N;
8. `N` → visual лишається PENDING, ZIP не видаляються;
9. `Y` → finalizer повторно перевіряє все і тільки тоді записує manual PASS та виконує safe ZIP cleanup.

### 5.2 Finalization preflight

`OsterConflict/Scripts/finalize_asset_acceptance.py` вимагає:

- exact current `HEAD == origin/<current branch>`;
- clean tracked runtime/acceptance source;
- `LOCAL_ASSET_STATUS.source_sha == HEAD`;
- `RUNTIME_SCOPE=CURRENT_RUN_COMPLETED`;
- local UE import/runtime/material/automated evidence = PASS;
- production vehicles = PASS;
- production weapons = PASS;
- `all_models_bound=true`;
- `unbound=[]`;
- fresh `category_counts.M16_M4 >= 1`;
- prepared manifest = `PASS` або factual `NO_INBOX`;
- no prepared conflicts;
- every source ZIP selected for cleanup must have SHA-256 present in current prepared manifest as `EXTRACTED`.

Якщо будь-який ZIP у `models_game_OC` не доведений current manifest/hash, cleanup відмовляється **до першого видалення**.

Fab-only / `NO_INBOX` case підтриманий: після повного acceptance дозволений zero-ZIP cleanup.

### 5.3 Manual evidence

Після явного `Y` записуються:

- `OsterConflict/Saved/AssetStatus/MANUAL_VISUAL_ACCEPTANCE.json`
- `OsterConflict/Saved/AssetStatus/MANUAL_VISUAL_ACCEPTANCE.txt`

Запис містить exact source SHA, UTC timestamp і checklist: HMMWV, M2, BTR-4, weapon models including M16/M4, world assets, skins/HUD та відсутність obvious placeholder/broken material/absurd scale/detached mesh.

### 5.4 Safe source ZIP cleanup evidence

Cleanup видаляє **тільки** source ZIP у `models_game_OC`, SHA-256 яких доведений current `prepared_sources.json`.

Записуються:

- `OsterConflict/Saved/AssetStatus/ACCEPTED_ZIP_CLEANUP.json`
- `OsterConflict/Saved/AssetStatus/ACCEPTED_ZIP_CLEANUP.txt`

Evidence містить source SHA, exact deleted paths, SHA-256, bytes, count і result.

Якщо cleanup частково падає, результат = FAIL і 100% не досягається.

### 5.5 Stale manual PASS заборонений

Будь-який новий asset ingest автоматично видаляє старі:

- `MANUAL_VISUAL_ACCEPTANCE.json/.txt`;
- `ACCEPTED_ZIP_CLEANUP.json/.txt`.

Тому не можна один раз отримати visual PASS, потім додати новий Fab/ZIP asset і успадкувати старі 100%.

`LOCAL_ASSET_STATUS` v4 окремо показує:

- `DIRECT_VISUAL_ACCEPTANCE=...`;
- `SOURCE_ZIP_CLEANUP=...`.

Manual/cleanup record з іншого SHA позначається stale і не дає PASS.

## 6. REGRESSION / CI AUDIT ЦЬОГО CHECKPOINT

Додано/посилено:

- `VERIFY_PASS45_ASSET_SOURCE_CLEAN_GUARD.py`;
- `VERIFY_PASS45_ASSET_FINALIZATION_GUARD.py`;
- `VERIFY_PASS45_LOCAL_BUILD_IMPORT_REGRESSION.py`;
- `RUN_ALL_VERIFY.py` тепер запускає source-clean і finalization guards;
- local build/import workflow запускає всі три asset guards;
- `VERIFY_MAIN_RUNTIME_ACCEPTANCE_LAUNCHER.py`, `VERIFY_PASS45_STRICT_RUNTIME_ACCEPTANCE_HARNESS.py`, `VERIFY_RUNTIME_ACCEPTANCE_PASS_33.py` оновлені під automated→manual boundary без послаблення gameplay/material/FPS gates.

Під час аудиту CI реально спіймав старі verifier assumptions. Вони були виправлені, а не обійдені.

Final verified code head `15090c4`:

- **19/19 workflows SUCCESS**;
- failed = 0;
- cancelled = 0;
- Source verification = SUCCESS;
- Main runtime acceptance launcher = SUCCESS;
- Strict runtime acceptance harness = SUCCESS;
- Pass37 + Pass33 compatibility chain = SUCCESS;
- Pass36 / Pass38 / DX11 / normal route / single launcher / material audits = SUCCESS.

## 7. LOCAL FILES, ЯКИХ ЩЕ НЕМАЄ В CONNECTED EVIDENCE

- `OsterConflict/Saved/LocalModelInbox/prepared_sources.json`
- `OsterConflict/Saved/LocalModelInbox/runtime_bindings.json`
- `OsterConflict/Saved/LocalModelInbox/runtime_bindings_success.txt`
- `OsterConflict/Saved/ProductionAssetImportCache/production_import_success.txt`
- `OsterConflict/Saved/ProductionAssetImportCache/production_weapon_import_result.txt`
- `OsterConflict/Saved/AutomationReports/ProductionModels/local_inbox_runtime_validation.txt`
- `OsterConflict/Saved/AutomationReports/ProductionModels/local_world_runtime_validation.txt`
- `Logs/R14_CURRENT_GAMEPLAY.log`
- `Logs/PASS45_STRICT_MATERIAL_GATE.log`
- `OsterConflict/Saved/AssetStatus/LOCAL_ASSET_STATUS.txt`
- `OsterConflict/Saved/AssetStatus/LOCAL_ASSET_STATUS.json`
- `OsterConflict/Saved/AssetStatus/MANUAL_VISUAL_ACCEPTANCE.json/.txt`
- `OsterConflict/Saved/AssetStatus/ACCEPTED_ZIP_CLEANUP.json/.txt`

Повторний connected Library/conversation search не знайшов fresh current-run evidence. Старі локальні логи 2026-08-22…26 і historical HMMWV/M2/BTR-4 PASS не закривають current acceptance.

## 8. ПЕРШИЙ НЕЗАКРИТИЙ CHECKPOINT

### `LOCAL-UE-ASSET-001`

Фактичне наступне завдання: виконати current full asset/runtime path на локальному UE 5.8 project через **`START_HERE.cmd` → `2. ПОВНИЙ RUNTIME-ТЕСТ`**.

Для 80% потрібен fresh current `LOCAL_ASSET_STATUS` із import PASS.  
Для 90% потрібен current full runtime/material/evidence PASS.  
Для 100% finalization preflight додатково не дозволить пройти з M16/M4 gap; після factual payload + runtime PASS він попросить явний visual Y/N і лише після `Y` виконає manifest/hash-proven source ZIP cleanup.

Поки fresh local UE evidence немає, factual progress лишається **70%**.

## 9. CONTINUATION RULE

Наступний pass:

- не повторює DONE 1–7;
- починає з `LOCAL-UE-ASSET-001` або з fresh локального result, якщо він уже з'явився;
- оновлює exact branch/head/PR/CI;
- оновлює asset rows за current manifest;
- не підвищує % без factual evidence;
- не merge PR #98 до local UE/runtime acceptance;
- не видаляє source ZIP поза manifest/hash-proven finalization route.
