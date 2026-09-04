# OSTER CONFLICT — ASSET STATUS

Date: 2026-09-05  
Branch: `fix/pass45-asset-import-fail-closed-20260904`  
Base/current main: `a1ad0e200611911102c48180956d82f73d0d8fc3`  
Last fully verified branch checkpoint: `95e190a1cbdb55b65473a2b82fa0e0c507d58f6f` — **19/19 SUCCESS**  
Last code-changing checkpoint: `13a49efaaee04eaf1cb786cc4604099e927038a7` — **19/19 SUCCESS**  
PR: #98 — Draft, unmerged, mergeable  
Branch relation at verified checkpoint: **ahead 96 / behind 0**, merge-base = current `main@a1ad0e2`  
Changed-file scope: **28 files**, intentional asset/runtime/finalization scope  
Fresh connected local UE/runtime evidence: **not found**

## 1. ГОЛОВНИЙ ПРОГРЕС ТЗ

Кожен із 10 етапів = 10% factual progress.

| № | Етап | Стан | Вклад | Що закрито | Що лишилось |
|---:|---|---|---:|---|---|
| 1 | Local inbox / intake contract | DONE | +10% | `models_game_OC`, local-only lifecycle, ZIP/loose/Fab intake | Нічого |
| 2 | Prepare / extract / classify | DONE | +10% | safe ZIP extraction, nested ZIP accounting, package conflict protection | Нічого по коду |
| 3 | Exact duplicate removal | DONE | +10% | SHA-256 dedupe до import; duplicate nested ZIP не дає false failure | Нічого |
| 4 | Fab / Marketplace / project discovery | DONE | +10% | `/Game`, Content, Plugins/Fab/project discovery | Нічого по коду |
| 5 | Production import logic | DONE | +10% | HMMWV, M2, BTR-4, M249, Remington 870 production paths | Нічого по коду |
| 6 | Fail-closed aggregate/binding result | DONE | +10% | GAP/UNBOUND/import failure не можуть перетворитися на aggregate PASS | Нічого по коду |
| 7 | Source/regression/finalization CI | DONE | +10% | exact HEAD/source freshness/runtime/finalization guards; `95e190a` = 19/19 SUCCESS | Нічого по GitHub-коду |
| 8 | Local UE 5.8 import result | WAIT | +0% | pipeline готовий і fail-closed | Потрібен фактичний fresh local UE import |
| 9 | Live gameplay/runtime hookup | WAIT | +0% | runtime/material/world/evidence gates готові | Потрібен factual full runtime PASS |
| 10 | Direct visual acceptance + safe ZIP cleanup | WAIT | +0% | finalizer + manual Y/N + SHA-256 cleanup готові | Потрібен factual visual inspection після runtime PASS |

### ФАКТИЧНЕ ВИКОНАННЯ

- Завершено: **7 / 10 етапів**.
- Загальний factual progress: **70%**.
- Залишилось: **30%**.
- Source/code/CI lifecycle: **100% реалізований**; current verified branch checkpoint `95e190a` має **19/19 SUCCESS**.
- Local UE import acceptance: **0% підтверджено**.
- Live runtime acceptance: **0% підтверджено**.
- Direct visual acceptance/cleanup: **0% підтверджено**.

Шлях: **70% → local UE import PASS = 80% → live runtime PASS = 90% → manual visual acceptance + safe ZIP cleanup = 100%.**

## 2. ОСТАННІЙ DEEP AUDIT — ЗАКРИТІ ПРОПУСКИ

Перевірений ланцюг:

`prepare ZIP → base import/binding → weapon normalization → collector → runtime evidence attribution → finalizer/cleanup`.

### 2.1 Explicit `UNBOUND` більше не може сховатися

Раніше `source_status=UNBOUND` не завжди переносився в `unbound_models`, тому нестандартно названа mesh `.uasset` теоретично могла лишити `all_models_bound=true`.

Тепер:

- кожен explicit `source_status=UNBOUND` обов'язково стає blocker;
- dedupe unbound rows зберігається;
- `all_models_bound` рахується тільки після reconciliation;
- regression guard фіксує цей порядок.

### 2.2 Weapon normalizer більше не «лікує» import failure назвою файла

Раніше factual `asset_load_failed/UNBOUND` міг бути переписаний у `BOUND`, якщо filename підходив під regex зброї.

Тепер factual `UNBOUND` ніколи не підвищується до `BOUND` лише класифікацією назви. Після normalization усі explicit `UNBOUND` повторно reconciled до blocker list.

### 2.3 Collector незалежно перевіряє consistency bindings

`LOCAL_UE_IMPORT=PASS` тепер одночасно вимагає:

- explicit current `import_result=0`;
- production vehicle PASS;
- production weapon PASS;
- binding success sentinel;
- `all_models_bound=true`;
- `unbound_models=[]`;
- `source_status_counts.UNBOUND=0`.

Пошкоджений aggregate flag сам PASS не дає.

### 2.4 Runtime evidence прив'язаний до exact source SHA

`AUTOMATED_RUNTIME_EVIDENCE=PASS` можливий тільки якщо evidence містить exact `SOURCE_SHA=<current snapshot SHA>`. PASS іншого HEAD стає `STALE_SOURCE`.

### 2.5 Nested ZIP більше не може мовчки зникнути

Nested ZIP глибше дозволеного ліміту тепер:

- записується як `NESTED_DEPTH_LIMIT`;
- має `error=nested_zip_depth_limit_exceeded`;
- входить у unsafe count;
- переводить manifest у `UNSAFE_ARCHIVE_PRESENT`;
- завершує prepare кодом `40`.

SHA-dedupe виконується до depth rejection, тому exact duplicate вже обробленого archive не створює false failure.

## 3. FINALIZER — НЕЗАЛЕЖНИЙ FAIL-CLOSED КОНТРАКТ

До manual acceptance/cleanup finalizer окремо вимагає:

- schema `oster-conflict-local-asset-status-v4`;
- `source_sha == current HEAD`;
- `import_result_code == 0`;
- `runtime_result_code == 0`;
- `runtime_scope == CURRENT_RUN_COMPLETED`;
- import/runtime/material/evidence stages = PASS;
- production vehicles/weapons = PASS;
- `all_models_bound=true`;
- `unbound=[]`;
- summary `unbound_models=0`;
- `source_status_counts.UNBOUND=0`;
- factual `M16_M4 >= 1`;
- prepared status `PASS` або factual `NO_INBOX`;
- no package conflicts;
- ZIP cleanup лише для manifest-proven SHA-256.

Також перевірено:

- `NO_INBOX` створює новий порожній manifest і не успадковує stale archive rows;
- unknown/unproven ZIP блокує cleanup **до першого видалення**;
- Fab-only/`NO_INBOX` може завершити zero-ZIP cleanup після інших PASS gates;
- fresh ingest анулює старі manual visual/cleanup records;
- dirty tracked runtime/source блокує import до UE execution;
- exact local HEAD перевіряється проти `origin/<branch>` до import і перед final acceptance;
- stale consolidated snapshot не може пережити fresh collection.

## 4. PRODUCTION ASSET MATRIX

| Asset | Factual стан | Підтверджено | Що ще треба |
|---|---|---|---|
| HMMWV | WAIT | source/import support | fresh UE import + live use + visual proof |
| M2 Browning | WAIT | source/import support | fresh UE import + mount/pitch/muzzle/material visual proof |
| BTR-4 | WAIT | source/import support | fresh UE result + live use + visual proof |
| M249 | WAIT | exact importer support | fresh source/UE/runtime/visual proof |
| Remington 870 | WAIT | exact importer support | fresh source/UE/runtime/visual proof |
| M16/M4 family | GAP | classifier/runtime category support | fresh manifest має довести actual bound `M16_M4 >= 1` |

M16/M4 залишається **factual content GAP**, а не code-classifier GAP. Local/Fab payload може існувати, але лише fresh current-run `runtime_bindings.json` може це підтвердити.

Інші local/Fab families: AK-47, MP5, M1911, M700, M14, MAC-10, TEC-9, Lever Action, інша зброя, pickups, buildings, props/furniture/fences, foliage, roads, terrain, water, character skins, HUD/UI — intake support є, factual runtime/visual proof ще PENDING.

## 5. FRESH LOCAL EVIDENCE STATUS

Повторний connected conversation/Library search на checkpoint після `95e190a` **не знайшов fresh current-head**:

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

Пошук повернув лише старі/сторонні матеріали. Вони current acceptance не підтверджують.

## 6. ПЕРШИЙ НЕЗАКРИТИЙ CHECKPOINT

### `LOCAL-UE-ASSET-001`

Наступна factual робота:

1. локальна гілка має бути синхронізована з current PR head;
2. запускати тільки `START_HERE.cmd`;
3. вибрати `2. ПОВНИЙ RUNTIME-ТЕСТ`;
4. pipeline виконає prepare/import/binding/runtime/material/evidence/finalization preflight;
5. fresh `LOCAL_ASSET_STATUS.txt/json` визначить factual result.

Результат:

- import PASS → **70% → 80%**;
- runtime PASS → **80% → 90%**;
- manual visual PASS + hash-proven ZIP cleanup → **90% → 100%**;
- GAP/UNBOUND/stale source/missing M16-M4 лишає відповідний етап незакритим.

## 7. CHECKPOINT CONTINUATION — 2026-09-05

Після попереднього tracker checkpoint виконано без повторення DONE роботи:

- current branch head `95e190a` звірено;
- `main` лишився `a1ad0e2`;
- branch relation на verified checkpoint: **ahead 96 / behind 0**;
- PR #98 лишається Draft/unmerged/mergeable;
- `95e190a` має **19/19 completed SUCCESS**, failed/cancelled = 0;
- fresh current-head local UE/runtime evidence знову не знайдено;
- progress тому лишається **70% factual**;
- перший реально незакритий пункт не змінився: `LOCAL-UE-ASSET-001`.

## 8. CONTINUATION RULE

Наступний pass:

- не повторює DONE 1–7;
- спочатку звіряє current branch/head/main/PR/CI;
- читає fresh consolidated `LOCAL_ASSET_STATUS` першим, якщо він з'явився;
- individual logs читаються тільки для конкретного FAIL/GAP;
- progress підвищується лише за factual local UE/runtime/manual evidence;
- PR #98 не merge до local UE/runtime acceptance.
