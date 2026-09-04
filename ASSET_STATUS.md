# OSTER CONFLICT — ASSET STATUS

Date: 2026-09-05  
Branch: `fix/pass45-asset-import-fail-closed-20260904`  
Base/current main: `a1ad0e200611911102c48180956d82f73d0d8fc3`  
Last fully verified code checkpoint: `13a49efaaee04eaf1cb786cc4604099e927038a7` — **19/19 SUCCESS**  
PR: #98 — Draft, unmerged, mergeable  
Branch relation at verified code checkpoint: **ahead 93 / behind 0**, merge-base = current `main@a1ad0e2`  
Changed-file scope: **28 files**, intentional asset/runtime/finalization scope  
Fresh connected local UE/runtime evidence: **not found**

## 1. ГОЛОВНИЙ ПРОГРЕС ТЗ

Кожен із 10 етапів = 10% загального factual progress.

| № | Етап | Стан | Вклад | Що вже закрито | Що лишилось |
|---:|---|---|---:|---|---|
| 1 | Local inbox / intake contract | DONE | +10% | `models_game_OC`, local-only lifecycle, ZIP/loose/Fab intake | Нічого |
| 2 | Prepare / extract / classify | DONE | +10% | safe ZIP extraction, nested ZIP accounting, package conflict protection | Нічого по коду |
| 3 | Exact duplicate removal | DONE | +10% | SHA-256 dedupe до import; duplicate nested ZIP не створює false failure | Нічого |
| 4 | Fab / Marketplace / project discovery | DONE | +10% | `/Game`, Content, Plugins/Fab/project discovery | Нічого по коду |
| 5 | Production import logic | DONE | +10% | HMMWV, M2, BTR-4, M249, Remington 870 production paths | Нічого по коду |
| 6 | Fail-closed aggregate/binding result | DONE | +10% | GAP/UNBOUND/import failure не можуть перетворитися на aggregate PASS | Нічого по коду |
| 7 | Source/regression/finalization CI | DONE | +10% | exact HEAD/source freshness/runtime/finalization guards, `13a49ef` = 19/19 SUCCESS | Нічого по GitHub-коду |
| 8 | Local UE 5.8 import result | WAIT | +0% | pipeline готовий і fail-closed | Потрібен фактичний fresh local UE import |
| 9 | Live gameplay/runtime hookup | WAIT | +0% | runtime/material/world/evidence gates готові | Потрібен factual full runtime PASS |
| 10 | Direct visual acceptance + safe ZIP cleanup | WAIT | +0% | finalizer і manual Y/N/hash cleanup готові | Потрібен фактичний visual inspection після runtime PASS |

### ФАКТИЧНЕ ВИКОНАННЯ

- Завершено: **7 / 10 етапів**.
- Загальний factual progress: **70%**.
- Залишилось: **30%**.
- Source/code/CI lifecycle: **100% реалізований і перевірений на `13a49ef`**.
- Local UE import acceptance: **0% підтверджено**.
- Live runtime acceptance: **0% підтверджено**.
- Direct visual acceptance/cleanup: **0% підтверджено**.
- Exact code-head CI: **19/19 SUCCESS**, failed = 0.

Шлях закриття: **70% → local UE import PASS = 80% → live runtime PASS = 90% → manual visual acceptance + safe ZIP cleanup = 100%.**

## 2. АУДИТ ПРОПУЩЕНИХ ПРОБЛЕМ — 2026-09-05

Цей pass був окремим deep audit ланцюга:

`prepare ZIP → base import/binding → weapon normalization → consolidated collector → runtime evidence attribution → finalizer/cleanup`.

Знайдені нижче пропуски були реальними false-green або silent-skip шляхами і вже закриті.

### 2.1 Explicit `source_status=UNBOUND` міг не блокувати `all_models_bound`

Проблема:

- `import_all_local_inbox_assets.py` записував factual `UNBOUND`, наприклад `asset_load_failed`;
- у `unbound_models` переносились не всі такі рядки, а тільки вибрані heuristic model sources;
- нестандартно названа `.uasset` mesh могла лишитися `source_status=UNBOUND`, але `all_models_bound=true`.

Виправлено:

- кожен explicit `source_status=UNBOUND` тепер обов'язково переноситься в `unbound_models`;
- dedupe unbound rows зберігається;
- `all_models_bound` обчислюється тільки після повного reconciliation;
- regression guard перевіряє порядок.

### 2.2 Weapon normalizer міг перетворити реальний import failure на `BOUND`

Проблема:

- `normalize_local_weapon_categories.py` міг побачити factual `UNBOUND`, наприклад `asset_load_failed`;
- якщо ім'я source/path підходило під regex `AR15`, `AK74`, іншу зброю, normalizer міг переписати рядок у `BOUND`;
- фактичне завантаження asset таким чином підмінялося класифікацією по назві.

Виправлено:

- factual `UNBOUND` rows normalizer більше ніколи не підвищує до `BOUND`;
- після normalization виконується незалежний reconciliation усіх залишених `source_status=UNBOUND`;
- тільки після цього перераховується `all_models_bound` і success sentinel;
- CI trigger включає `normalize_local_weapon_categories.py`.

### 2.3 Collector міг занадто довіряти `all_models_bound=true`

Проблема:

- `COLLECT_LOCAL_ASSET_STATUS.py` для `LOCAL_UE_IMPORT=PASS` перевіряв aggregate flag/sentinel;
- inconsistent manifest теоретично міг мати `all_models_bound=true`, але explicit `UNBOUND` rows.

Виправлено:

Current import PASS тепер одночасно вимагає:

- explicit current `import_result=0`;
- production vehicle PASS;
- production weapon PASS;
- binding success sentinel;
- `all_models_bound=true`;
- `unbound_models=[]`;
- `source_status_counts.UNBOUND=0`.

Тому один пошкоджений aggregate flag більше не може сам дати 80%.

### 2.4 Runtime evidence PASS не був жорстко прив'язаний до exact source SHA у collector

Проблема:

- canonical evidence file вже записував `SOURCE_SHA`;
- collector дивився переважно на `PASS45_RUNTIME_AUTOMATED_EVIDENCE=PASS`;
- при нестандартному/ручному виклику collector теоретично можна було змішати current snapshot зі stale evidence іншого HEAD.

Виправлено:

`AUTOMATED_RUNTIME_EVIDENCE=PASS` тепер можливий тільки якщо:

- current runtime result explicit `0`;
- snapshot має відомий source SHA;
- evidence має `PASS45_RUNTIME_AUTOMATED_EVIDENCE=PASS`;
- evidence має exact `SOURCE_SHA=<current snapshot SHA>`.

PASS з іншого SHA стає `STALE_SOURCE`.

### 2.5 Nested ZIP depth-limit міг мовчки пропустити assets

Проблема:

- nested ZIP глибше 4 рівнів отримував warning і `continue`;
- його не було у manifest як failure;
- prepare міг лишитися PASS, хоча частина payload не була оброблена.

Виправлено:

- deep archive записується в manifest як `NESTED_DEPTH_LIMIT`;
- `error=nested_zip_depth_limit_exceeded`;
- він входить у `$unsafeCount`;
- manifest → `UNSAFE_ARCHIVE_PRESENT`;
- prepare завершується code `40`, а не green PASS;
- SHA-dedupe виконується **до** depth rejection, тому exact duplicate уже обробленого archive не створює false failure.

### 2.6 Finalizer отримав незалежні бар'єри

Finalizer тепер окремо вимагає:

- exact schema `oster-conflict-local-asset-status-v4`;
- `source_sha == current HEAD`;
- `import_result_code == 0`;
- `runtime_result_code == 0`;
- `RUNTIME_SCOPE=CURRENT_RUN_COMPLETED`;
- import/runtime/material/evidence stages = PASS;
- production vehicles/weapons = PASS;
- `all_models_bound=true`;
- `unbound=[]`;
- summary `unbound_models=0`;
- `source_status_counts.UNBOUND=0`;
- `M16_M4 >= 1`;
- prepared status `PASS` або factual `NO_INBOX`;
- no package conflicts;
- source ZIP cleanup тільки для manifest-proven SHA-256.

## 3. ЩО ПЕРЕВІРЕНО І НЕ Є ПРОПУСКОМ

- `NO_INBOX` не успадковує stale archive rows: manifest створюється заново з порожніми arrays.
- unknown/unproven ZIP блокує cleanup **до першого видалення**.
- Fab-only/`NO_INBOX` може коректно завершити zero-ZIP cleanup після інших PASS gates.
- fresh ingest анулює старі manual visual/cleanup records.
- dirty tracked runtime/source блокує import до UE execution.
- exact `local HEAD == origin/current branch` перевіряється до asset import і знову перед final acceptance.
- stale consolidated snapshot не може пережити fresh collection.
- production HMMWV/M2/BTR і exact M249/Remington мають окремі mandatory result gates.
- `RUN_ALL_VERIFY.py` запускає source-clean і finalization guards.
- після останніх source changes exact code head `13a49ef` пройшов **19/19 SUCCESS**.

Після цього deep audit відомого source-only false-green шляху в перевіреному ланцюгу не залишилось. Це **не** означає, що actual UE runtime уже прийнятий: його фізично ще не запускали на цьому current head у доступному evidence.

## 4. PRODUCTION ASSET MATRIX

| Asset | Factual стан | Що підтверджено | Що ще треба |
|---|---|---|---|
| HMMWV | WAIT | source/import support | fresh UE import + live use + visual proof |
| M2 Browning | WAIT | source/import support | fresh UE import + mount/pitch/muzzle/material visual proof |
| BTR-4 | WAIT | source/import support | fresh UE result + live use + visual proof |
| M249 | WAIT | exact importer support | fresh source/UE/runtime/visual proof |
| Remington 870 | WAIT | exact importer support | fresh source/UE/runtime/visual proof |
| M16/M4 family | GAP | classifier/runtime category support | fresh manifest must prove actual bound `M16_M4 >= 1` |

M16/M4 remains a **factual content gap**, not a code-classifier gap. Local/Fab payload may exist, but only fresh current-run `runtime_bindings.json` can close it.

Other supported local/Fab families such as AK-47, MP5, M1911, M700, M14, MAC-10, TEC-9, Lever Action, other weapons, pickups, buildings, props/furniture/fences, foliage, roads, terrain, water, character skins and HUD/UI remain PENDING factual runtime/visual proof.

## 5. FRESH LOCAL EVIDENCE STATUS

Repeated connected conversation/Library search on 2026-09-05 found **no fresh current-head**:

- `OsterConflict/Saved/LocalModelInbox/prepared_sources.json`
- `OsterConflict/Saved/LocalModelInbox/runtime_bindings.json`
- `OsterConflict/Saved/LocalModelInbox/runtime_bindings_success.txt`
- `OsterConflict/Saved/ProductionAssetImportCache/production_import_success.txt`
- `OsterConflict/Saved/ProductionAssetImportCache/production_weapon_import_result.txt`
- `OsterConflict/Saved/AutomationReports/ProductionModels/local_inbox_runtime_validation.txt`
- `OsterConflict/Saved/AutomationReports/ProductionModels/local_world_runtime_validation.txt`
- `Logs/R14_CURRENT_GAMEPLAY.log`
- `Logs/PASS45_STRICT_MATERIAL_GATE.log`
- `Logs/PASS45_RUNTIME_ACCEPTANCE_EVIDENCE.txt`
- `OsterConflict/Saved/AssetStatus/LOCAL_ASSET_STATUS.txt`
- `OsterConflict/Saved/AssetStatus/LOCAL_ASSET_STATUS.json`

Search повернув лише старі серпневі логи/сторонні файли. Вони не зараховуються для current checkpoint.

## 6. ПЕРШИЙ НЕЗАКРИТИЙ CHECKPOINT

### `LOCAL-UE-ASSET-001`

Єдина наступна factual робота, яку GitHub сам виконати не може:

1. локальна гілка має бути синхронізована з current PR head;
2. запускати тільки `START_HERE.cmd`;
3. вибрати `2. ПОВНИЙ RUNTIME-ТЕСТ`;
4. pipeline сам виконає prepare/import/binding/runtime/material/evidence/finalization preflight;
5. fresh `LOCAL_ASSET_STATUS.txt/json` визначить factual result.

Результат:

- import PASS → **70% → 80%**;
- runtime PASS → **80% → 90%**;
- manual visual PASS + hash-proven ZIP cleanup → **90% → 100%**;
- будь-який GAP/UNBOUND/stale source/missing M16-M4 залишає відповідний етап незакритим.

## 7. CONTINUATION RULE

Наступний pass:

- не повторює DONE 1–7;
- спочатку звіряє current branch/head/main/PR/CI;
- читає fresh consolidated `LOCAL_ASSET_STATUS` першим, якщо він з'явився;
- individual logs читаються тільки для конкретного FAIL/GAP;
- progress підвищується лише за factual local UE/runtime/manual evidence;
- PR #98 не merge до local UE/runtime acceptance.
