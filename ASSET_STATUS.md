# OSTER CONFLICT — ASSET STATUS

Date: 2026-09-04  
Branch: `fix/pass45-asset-import-fail-closed-20260904`  
Base: `main@a1ad0e200611911102c48180956d82f73d0d8fc3`  
Last verified code checkpoint: `ade66ec199b925269060f8f9d9131b6a821dc8fb` — **18/18 SUCCESS**  
PR: #98 — Draft, unmerged, mergeable  
Branch relation at verified code checkpoint: **ahead 58 / behind 0**, merge-base = `main@a1ad0e2`  
Changed-file scope: **20 files**; increase from 17 is intentional: dirty-source guard + its verifier + CI wiring.

## 1. ГОЛОВНА ТАБЛИЦЯ ПРОГРЕСУ

Кожен із 10 етапів = 10% загального прогресу.

| № | Етап | Стан | Виконано | Вклад | Що вже зроблено | Що лишилось |
|---:|---|---|---:|---:|---|---|
| 1 | Local inbox / intake contract | DONE | 100% | +10% | `models_game_OC` lifecycle і safe local-only policy визначені | Нічого |
| 2 | Prepare / extract / classify | DONE | 100% | +10% | ZIP/loose sources безпечно готуються і класифікуються | Нічого |
| 3 | Exact duplicate removal | DONE | 100% | +10% | SHA-256 dedupe працює до UE import | Нічого |
| 4 | Fab / Marketplace / project discovery | DONE | 100% | +10% | Скануються `/Game` і project/plugin mounts | Нічого |
| 5 | Production import logic | DONE | 100% | +10% | HMMWV, M2, BTR-4, M249, Remington 870 import paths готові | Нічого по коду |
| 6 | Fail-closed aggregate result | DONE | 100% | +10% | GAP/exception блокує фальшивий PASS; stale import PASS markers очищаються | Нічого |
| 7 | GitHub source / regression CI | DONE | 100% | +10% | `ade66ec`: 18/18 SUCCESS; exact-head, dirty-source, snapshot freshness і final runtime guards активні | Нічого |
| 8 | Local UE 5.8 import result | WAIT | 0% підтверджено | +0% | Import не може бути PASS без exact remote HEAD, clean tracked runtime source, explicit current import result=0 і fresh `LOCAL_ASSET_STATUS.txt/json` | Потрібен фактичний локальний UE 5.8 import |
| 9 | Live gameplay/runtime hookup | WAIT | 0% підтверджено | +0% | Gameplay/material/weapon/current-run evidence мають freshness cleanup; final runtime snapshot fail-closed | Потрібні live inbox/world/material/gameplay результати |
| 10 | Direct visual acceptance + ZIP cleanup | WAIT | 0% підтверджено | +0% | Автоматичний pipeline навмисно не піднімає visual acceptance | Треба фактично побачити assets у грі; ZIP видаляти тільки після PASS |

### ЗАГАЛЬНИЙ ПРОГРЕС

| Показник | Значення |
|---|---:|
| Завершено етапів | **7 / 10** |
| Загальний прогрес | **70%** |
| Залишилось | **30%** |
| Source/code/CI частина | **100% на `ade66ec`** |
| Local UE import acceptance | **0% підтверджено** |
| Live runtime acceptance | **0% підтверджено** |
| Direct visual acceptance | **0% підтверджено** |
| Failed/cancelled workflows на verified code head | **0 / 18** |

Поточний factual стан: **70%**. Наступні 30% неможливо чесно закрити без локального UE/runtime/visual доказу.

Шлях: **70% → local UE import PASS = 80% → live runtime PASS = 90% → direct visual acceptance + safe ZIP cleanup = 100%.**

## 2. PRODUCTION ASSET MATRIX

| Asset | Стан | Підтверджено | Чого бракує |
|---|---|---:|---|
| HMMWV | WAIT | 25% | Fresh UE import, live use, scale/orientation/material visual proof |
| M2 Browning | WAIT | 25% | Fresh UE import, HMMWV mount, pitch/muzzle/material visual proof |
| BTR-4 | WAIT | 25% | Fresh UE result, live use, proportions/orientation/material visual proof |
| M249 | WAIT | 25% | Exact local payload/UE result/runtime/visual proof |
| Remington 870 | WAIT | 25% | Exact local payload/UE result/runtime/visual proof |
| M16/M4 family | GAP | 0% READY | Dedicated verified payload не підтверджений; fresh manifest може змінити цей висновок, якщо local/Fab payload фактично є |

`25%` означає тільки підтверджений source/import support, а не готовність asset у грі.

GitHub code search не дав достатнього tracked evidence для M16/M4/M4A1. Це не виключає local/Fab payload; потрібен fresh `runtime_bindings.json`.

## 3. OTHER LOCAL / FAB ASSET FAMILIES

AK-47, MP5, M1911, M700, M14, MAC-10, TEC-9, Lever Action, інші weapon/launcher assets, pickups/vehicle props, buildings, props/furniture/fences, foliage/trees/grass, roads/sidewalks, ground/terrain, water/river, character skins, HUD/UI і Fab/Marketplace/project-plugin meshes мають intake/classification support, але їхній current runtime/visual стан лишається **PENDING** до fresh manifest і live runtime proof.

## 4. ЩО БЛОКУЄ НАСТУПНІ 30%

| Блокер | Чого немає | Що дасть результат |
|---|---|---|
| Fresh local UE import manifest | `runtime_bindings.json` current run | exact discovered/imported/bound/unbound counts |
| Production vehicle result | fresh `production_import_success.txt` | HMMWV/M2/BTR-4 state |
| Exact weapon result | fresh `production_weapon_import_result.txt` | M249/Remington state |
| Local inbox runtime proof | fresh `local_inbox_runtime_validation.txt` | bound assets реально відкриваються у gameplay |
| Local world runtime proof | fresh `local_world_runtime_validation.txt` | buildings/props/foliage/roads/water у live Oster runtime |
| Gameplay evidence | fresh `R14_CURRENT_GAMEPLAY.log` | vehicles/input/FPS/runtime ownership |
| Material evidence | fresh `PASS45_STRICT_MATERIAL_GATE.log` | material/dependency PASS/GAP |
| Consolidated status | fresh `LOCAL_ASSET_STATUS.txt/json` | current-run import/runtime verdict |
| Visual inspection | пряме спостереження в UE/game | final manual acceptance перед cleanup |

Fresh локального runtime FAIL зараз також не підтверджено, бо current local run ще не доступний цьому робочому процесу.

## 5. FAIL-CLOSED ACCEPTANCE CONTRACT

### 5.1 Exact remote HEAD before import

`START_HERE.cmd` виконує `:verify_current_asset_source` до importer:

- Git missing → `66`;
- branch unknown → `67`;
- fetch failed → `68`;
- local/remote HEAD unknown → `69`;
- local HEAD != `origin/<current branch>` → `70`;
- тільки exact match дозволяє asset ingest.

### 5.2 Dirty tracked runtime/source guard

Знайдений у цьому checkpoint пропуск: local HEAD міг збігатися з GitHub, але tracked runtime/source файли могли мати незакомічені зміни. Тоді snapshot приписував би локально змінений код чистому GitHub SHA.

Закрито на `ade66ec`:

- `IMPORT_ALL_LOCAL_INBOX_UE58.cmd` до evidence cleanup/LFS/build/UE import перевіряє tracked runtime/source paths;
- перевіряються launcher-и, collector/verifier, production/material CMD, `OsterConflict/Scripts`, `OsterConflict/Source`;
- untracked local payloads і `Content` не блокуються;
- dirty tracked runtime/source → import code `59`;
- `VERIFY_PASS45_ASSET_SOURCE_CLEAN_GUARD.py` + CI захищають порядок і scope guard-а.

### 5.3 Explicit current import result

Знайдений у цьому checkpoint другий пропуск: standalone collector міг побачити старі PASS sentinels/manifests і показати `LOCAL_UE_IMPORT=PASS` без explicit current import code.

Закрито на `ade66ec`:

- `import_result is None` → `LOCAL_UE_IMPORT=PENDING_CURRENT_RUN`;
- `import_result != 0` → `FAIL`;
- `PASS` можливий тільки при explicit current `import_result=0` плюс vehicle/weapon/binding/all-models-bound PASS;
- regression verifier контролює цей порядок.

### 5.4 Fresh consolidated snapshot

Перед collection видаляються старі `LOCAL_ASSET_STATUS.txt/json`.

- collector missing → `62`;
- Python missing → `63`;
- collector nonzero → `64`;
- missing txt/json output → `65`;
- successful importer + failed snapshot = overall nonzero, не PASS.

### 5.5 Runtime/material/weapon freshness

Перевірено в цьому checkpoint:

- `START_HERE` видаляє старі local inbox/world runtime reports до current gameplay;
- `RUN_R14_CURRENT_GAMEPLAY.cmd` видаляє current gameplay/preflight logs/sentinels до запуску;
- `RUN_PASS45_STRICT_MATERIAL_GATE.cmd` видаляє старі `weapon_runtime_validation.txt`, weapon success sentinel і `PASS45_STRICT_MATERIAL_GATE.log` перед current material gate;
- canonical `VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py` вимагає current gameplay + material + exact weapon dependency evidence;
- final runtime PASS fail-closed, якщо final `LOCAL_ASSET_STATUS` не записався.

### 5.6 Runtime scopes

`COLLECT_LOCAL_ASSET_STATUS.py` schema `oster-conflict-local-asset-status-v3`:

| RUNTIME_SCOPE | Значення |
|---|---|
| `IMPORT_ONLY` | runtime/material/evidence = `PENDING_CURRENT_RUN` |
| `CURRENT_RUN_FAILED` | runtime = FAIL; material/evidence не можуть успадкувати старий PASS |
| `CURRENT_RUN_COMPLETED` | PASS дозволений тільки за current runtime/material/evidence markers |

Direct visual acceptance навмисно лишається `PENDING_MANUAL_OBSERVATION` до фактичного огляду.

## 6. LOCAL FILES, ЯКИХ ЩЕ НЕМАЄ В CONNECTED EVIDENCE

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

Повторний search у connected conversation/Library не знайшов fresh current-run файлів. Результати були сторонні/старі, тому їх не зараховано.

Історичний локальний доказ від 2026-08-26 підтверджує тодішній UE import PASS для HMMWV, M2 і BTR-4, але він лишається baseline і не закриває current `LOCAL-UE-ASSET-001`.

## 7. ЗАКРИТО В ЦЬОМУ CHECKPOINT

- code/tracker checkpoint `5e981c5`: 18/18 SUCCESS;
- знайдено й закрито dirty tracked runtime/source false-attribution gap;
- importer тепер зупиняється code `59` до evidence cleanup, LFS, build і UE import;
- додано окремий regression verifier і CI wiring для dirty-source guard;
- знайдено й закрито stale import-stage false-green у standalone collector;
- `LOCAL_UE_IMPORT=PASS` тепер неможливий без explicit current import result=0;
- material/weapon/gameplay freshness cleanup перевірений, додаткового stale-pass шляху там не знайдено;
- exact-head code checkpoint `ade66ec`: **18/18 SUCCESS**, failed/cancelled = **0/18**;
- branch: **ahead 58 / behind 0**, base/merge-base = `main@a1ad0e2`;
- intentional scope = **20 files**;
- PR #98 лишається Draft/unmerged до local UE runtime acceptance;
- fresh connected local UE/runtime evidence досі немає;
- M16/M4 content gap не знятий без fresh local/Fab manifest.

## 8. ПЕРШИЙ НЕЗАКРИТИЙ CHECKPOINT

### `LOCAL-UE-ASSET-001`

Завдання: виконати current full asset/runtime path на фактичному локальному UE 5.8 project і отримати fresh:

- `OsterConflict/Saved/AssetStatus/LOCAL_ASSET_STATUS.txt`
- `OsterConflict/Saved/AssetStatus/LOCAL_ASSET_STATUS.json`

Current source-side acceptance має чотири fail-closed бар'єри перед 80%: exact remote HEAD → clean tracked runtime source → explicit current import result → mandatory fresh snapshot.

Результат checkpoint:

1. exact prepared/discovered/imported/bound/unbound counts;
2. factual production asset PASS/GAP rows;
3. import PASS → **70% → 80%**;
4. live runtime PASS → **80% → 90%**;
5. direct visual acceptance + safe ZIP cleanup → **90% → 100%**.

## 9. CONTINUATION RULE

Наступний pass продовжує з першого фактично незакритого пункту, не повторює DONE-етапи й оновлює цей tracker factual даними: branch/head/PR, CI, changed asset rows, GAPs, %, missing evidence і next checkpoint.
