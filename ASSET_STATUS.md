# OSTER CONFLICT — ASSET STATUS

Date: 2026-09-04  
Branch: `fix/pass45-asset-import-fail-closed-20260904`  
Base: `main@a1ad0e200611911102c48180956d82f73d0d8fc3`  
Last verified code checkpoint: `dffad3234f39c1e76404c52453d8e5b93c31f100`  
PR: #98 — Draft, unmerged  
Exact-head GitHub CI on `dffad323`: **18/18 SUCCESS**

## 1. ГОЛОВНА ТАБЛИЦЯ ПРОГРЕСУ

Індикатори: 🟢 готово · 🟡 очікує локального доказу · 🔴 фактичний GAP/FAIL.

Кожен із 10 етапів = **10% загального прогресу**.

| № | Етап | Індикатор | Виконано | Вклад | Що вже зроблено | Що лишилось |
|---:|---|:---:|---:|---:|---|---|
| 1 | Local inbox / intake contract | 🟢 | 100% | +10% | `models_game_OC` lifecycle і safe local-only policy визначені | Нічого |
| 2 | Prepare / extract / classify | 🟢 | 100% | +10% | ZIP/loose sources безпечно готуються і класифікуються | Нічого |
| 3 | Exact duplicate removal | 🟢 | 100% | +10% | SHA-256 dedupe працює до UE import | Нічого |
| 4 | Fab / Marketplace / project discovery | 🟢 | 100% | +10% | Скануються `/Game` і project/plugin mounts | Нічого |
| 5 | Production import logic | 🟢 | 100% | +10% | HMMWV, M2, BTR-4, M249, Remington 870 import paths готові | Нічого по коду |
| 6 | Fail-closed aggregate result | 🟢 | 100% | +10% | GAP/exception блокує фальшивий PASS; stale PASS markers очищаються | Нічого |
| 7 | GitHub source / regression CI | 🟢 | 100% | +10% | Exact-head `dffad323`: **18/18 SUCCESS** | Нічого |
| 8 | Local UE 5.8 import result | 🟡 | 0% підтверджено | +0% | Import pipeline, failed-import snapshot і exact import code готові | Потрібен фактичний локальний UE 5.8 import |
| 9 | Live gameplay/runtime hookup | 🟡 | 0% підтверджено | +0% | Pre-merge runtime дозволений; кожен ранній runtime FAIL тепер оновлює snapshot з exact runtime code | Потрібні live inbox/world/material/gameplay результати |
| 10 | Direct visual acceptance + ZIP cleanup | 🟡 | 0% підтверджено | +0% | Acceptance/cleanup contract готовий | Треба побачити assets у грі; ZIP видаляти тільки після PASS |

### ЗАГАЛЬНИЙ ПРОГРЕС

| Показник | Значення | Індикатор |
|---|---:|:---:|
| Завершено етапів | **7 / 10** | 🟢 |
| Загальний прогрес | **70%** | 🟡 |
| Залишилось | **30%** | 🟡 |
| Source/code/CI частина | **100%** | 🟢 |
| Local UE import acceptance | **0% підтверджено** | 🟡 |
| Live runtime acceptance | **0% підтверджено** | 🟡 |
| Direct visual acceptance | **0% підтверджено** | 🟡 |
| Failed GitHub checks на verified code checkpoint | **0 / 18** | 🟢 |

**Поточний стан: 🟡 70%. Source-side підготовка завершена; наступні 30% потребують фактичного локального UE/runtime/visual доказу.**

Шлях до 100%: **70% → local UE import PASS = 80% → live runtime PASS = 90% → direct visual acceptance + safe cleanup = 100%.**

## 2. PRODUCTION ASSET MATRIX

Для кожного production asset є 4 ворота: source/import support → UE import → live runtime → direct visual acceptance.

| Asset | Індикатор | Підтверджено | Поточний стан | Чого бракує |
|---|:---:|---:|---|---|
| HMMWV | 🟡 | **25%** | Import support готовий | Fresh UE import, live use, scale/orientation/material visual proof |
| M2 Browning | 🟡 | **25%** | Import support готовий | Fresh UE import, HMMWV mount, pitch/muzzle/material visual proof |
| BTR-4 | 🟡 | **25%** | Local FBX або Oster-authored fallback support готовий | Fresh UE result, live use, proportions/orientation/material visual proof |
| M249 | 🟡 | **25%** | Exact-source importer готовий | Exact local payload/UE result/runtime/visual proof |
| Remington 870 | 🟡 | **25%** | Exact-source importer готовий | Exact local payload/UE result/runtime/visual proof |
| M16/M4 family | 🔴 | **0% READY** | Generic classifier є; dedicated verified payload не підтверджений | Потрібен фактичний production payload і весь UE/runtime/visual цикл |

`25%` означає тільки підтверджений source/import support, а не готовність asset у грі.

## 3. OTHER LOCAL / FAB ASSET FAMILIES

| Family | Intake | Runtime | Індикатор | Для зеленого |
|---|---|---|:---:|---|
| AK-47 | supported | PENDING | 🟡 | manifest + gameplay load |
| MP5 | supported | PENDING | 🟡 | manifest + gameplay load |
| M1911 | supported | PENDING | 🟡 | manifest + gameplay load |
| M700 | supported | PENDING | 🟡 | manifest + gameplay + visual calibration |
| M14 | supported | PENDING | 🟡 | manifest + gameplay load |
| MAC-10 | supported | PENDING | 🟡 | manifest + gameplay load |
| TEC-9 | supported | PENDING | 🟡 | manifest + gameplay load |
| Lever Action | supported | PENDING | 🟡 | manifest + gameplay + visual calibration |
| Other weapon/launcher assets | classified | PENDING | 🟡 | bound paths + runtime |
| Pickups / vehicle props | classified | PENDING | 🟡 | live placement/load proof |
| Buildings | classified | PENDING | 🟡 | live world validation |
| Props / furniture / fences | classified | PENDING | 🟡 | live world validation |
| Foliage / trees / grass | classified | PENDING | 🟡 | live placement + visual proof |
| Roads / sidewalks | classified | PENDING | 🟡 | live placement + visual proof |
| Ground / terrain | classified | PENDING | 🟡 | live placement + visual proof |
| Water / river | classified | PENDING | 🟡 | live placement + visual proof |
| Character skins | skeletal import supported | PENDING | 🟡 | compatible skeleton count + live character proof |
| HUD/UI | supported | PENDING | 🟡 | manifest + actual HUD use |
| Fab/Marketplace/project-plugin meshes | automatic mount discovery | PENDING | 🟡 | exact discovered/bound/failure counts |

## 4. ЩО БЛОКУЄ НАСТУПНІ 30%

| Блокер | Індикатор | Чого немає | Що дасть результат |
|---|:---:|---|---|
| Fresh local UE import manifest | 🟡 | `runtime_bindings.json` поточного запуску | discovered/imported/bound/unbound counts |
| Production vehicle result | 🟡 | fresh `production_import_success.txt` | HMMWV/M2/BTR-4 state |
| Exact weapon result | 🟡 | fresh `production_weapon_import_result.txt` | M249/Remington state |
| Local inbox runtime proof | 🟡 | `local_inbox_runtime_validation.txt` | чи bound assets реально відкриваються у gameplay |
| Local world runtime proof | 🟡 | `local_world_runtime_validation.txt` | buildings/props/foliage/roads/water у live Oster runtime |
| Gameplay evidence | 🟡 | fresh `R14_CURRENT_GAMEPLAY.log` | vehicles/input/FPS/runtime ownership |
| Material evidence | 🟡 | fresh `PASS45_STRICT_MATERIAL_GATE.log` | material/dependency PASS/GAP |
| Visual inspection | 🟡 | пряме спостереження в UE/game | останній acceptance перед READY/cleanup |

Свіжого локального runtime FAIL зараз не підтверджено, бо локального прогону ще немає. Відомий червоний content gap: **M16/M4 dedicated production payload не підтверджений**.

## 5. LOCAL_ASSET_STATUS SNAPSHOT

`COLLECT_LOCAL_ASSET_STATUS.py` створює зведення одразу після asset import і оновлює його на runtime-етапі.

Якщо import падає, snapshot містить:

- `LOCAL_UE_IMPORT=FAIL`;
- точний `IMPORT_RESULT_CODE=<код>`;
- підготовлені/bound/unbound counts, якщо вони вже встигли сформуватися;
- production vehicle/weapon states;
- exact GAP reasons;
- список відсутніх evidence files.

Якщо import пройшов, але full runtime падає раніше canonical evidence verifier, `START_HERE` тепер теж оновлює snapshot перед виходом. Покриті ранні провали:

- gameplay launcher / acceptance;
- missing або failed local inbox runtime proof;
- missing або failed local world runtime proof;
- strict material gate;
- відсутній Python для evidence stage;
- canonical runtime evidence FAIL.

У такому випадку snapshot містить:

- `LIVE_RUNTIME_HOOKUP=FAIL`;
- точний `RUNTIME_RESULT_CODE=<код>`;
- наявні import/binding/material/runtime факти на момент провалу.

Canonical runtime evidence при досягненні фінальної стадії так само оновлює цей snapshot для PASS або FAIL.

Файли:

- `OsterConflict/Saved/AssetStatus/LOCAL_ASSET_STATUS.txt`
- `OsterConflict/Saved/AssetStatus/LOCAL_ASSET_STATUS.json`

Direct visual acceptance автоматично не підвищується: `PENDING_MANUAL_OBSERVATION` зберігається до фактичного огляду.

## 6. LOCAL FILES, ЯКИХ ЩЕ НЕМАЄ В GITHUB / LIBRARY

| File/report | Для чого | Стан |
|---|---|:---:|
| `OsterConflict/Saved/LocalModelInbox/prepared_sources.json` | prepared sources | 🟡 NOT AVAILABLE |
| `OsterConflict/Saved/LocalModelInbox/runtime_bindings.json` | bound/unbound/categories/GAPs | 🟡 NOT AVAILABLE |
| `OsterConflict/Saved/LocalModelInbox/runtime_bindings_success.txt` | aggregate binding PASS | 🟡 NOT AVAILABLE |
| `OsterConflict/Saved/ProductionAssetImportCache/production_import_success.txt` | HMMWV/M2/BTR state | 🟡 NOT AVAILABLE |
| `OsterConflict/Saved/ProductionAssetImportCache/production_weapon_import_result.txt` | M249/Remington state | 🟡 NOT AVAILABLE |
| `OsterConflict/Saved/AutomationReports/ProductionModels/local_inbox_runtime_validation.txt` | live inbox proof | 🟡 NOT AVAILABLE |
| `OsterConflict/Saved/AutomationReports/ProductionModels/local_world_runtime_validation.txt` | live world proof | 🟡 NOT AVAILABLE |
| `Logs/R14_CURRENT_GAMEPLAY.log` | gameplay/runtime/FPS proof | 🟡 NOT AVAILABLE |
| `Logs/PASS45_STRICT_MATERIAL_GATE.log` | material/dependency proof | 🟡 NOT AVAILABLE |
| `OsterConflict/Saved/AssetStatus/LOCAL_ASSET_STATUS.txt` | consolidated human report | 🟡 NOT GENERATED YET |
| `OsterConflict/Saved/AssetStatus/LOCAL_ASSET_STATUS.json` | consolidated machine report | 🟡 NOT GENERATED YET |

## 7. ЗАКРИТО В ЦЬОМУ CHECKPOINT

- 🟢 failed asset import snapshot зберігає exact `IMPORT_RESULT_CODE` і `LOCAL_UE_IMPORT=FAIL`;
- 🟢 early full-runtime failures тепер теж refresh-ять `LOCAL_ASSET_STATUS` до виходу;
- 🟢 exact runtime failure code передається через `PASS45_RUNTIME_RC` і записується як `RUNTIME_RESULT_CODE`;
- 🟢 ненульовий runtime code однозначно дає `LIVE_RUNTIME_HOOKUP=FAIL`, а не `PENDING_OR_GAP`;
- 🟢 regression-guard вимагає snapshot coverage для gameplay/inbox/world/material/Python/evidence failure paths;
- 🟢 поточні `fix/pass45-asset-*` гілки дозволені для pre-merge runtime acceptance;
- 🟢 Pass19/Pass15/Pass20/Pass22/Pass23 forward-port’нуті з видалених per-pass acceptance launchers на `START_HERE` + canonical Pass45 evidence;
- 🟢 Pass3 перевіряє audit у його реальному власнику `IMPORT_ALL_LOCAL_INBOX_UE58.cmd`, а не вимагає дубль у `START_HERE`;
- 🟢 Pass4 звіряє фактичну current source-recovery truth, а не старий текст повідомлення;
- 🟢 BTR verifier відповідає поточному intake: local FBX або Oster-authored generated fallback;
- 🟢 exact-head source/regression CI на `dffad323`: **18/18 SUCCESS**;
- 🟢 PR #98 лишається Draft/unmerged до локального UE runtime acceptance.

## 8. ПЕРШИЙ НЕЗАКРИТИЙ CHECKPOINT

### 🟡 `LOCAL-UE-ASSET-001`

**Завдання:** виконати current full asset/runtime path на фактичному локальному UE 5.8 проекті та отримати fresh `LOCAL_ASSET_STATUS`.

Результат цього checkpoint:

1. exact `prepared / discovered / imported / bound / unbound` цифри;
2. proven assets переводяться 🟡 → 🟢;
3. factual import failures переводяться в 🔴 з exact `IMPORT_RESULT_CODE` і GAP reason;
4. factual runtime failures переводяться в 🔴 з exact `RUNTIME_RESULT_CODE` і stage evidence;
5. local UE import PASS закриває етап 8: **70% → 80%**;
6. live runtime PASS закриває етап 9: **80% → 90%**;
7. direct visual acceptance + safe source ZIP cleanup закриває етап 10: **90% → 100%**.

## 9. CONTINUATION RULE

Кожен наступний asset-pass повинен оновити цю таблицю, а не писати прогрес лише текстом:

- current branch/head/PR;
- 🟢 / 🟡 / 🔴 для кожного етапу;
- загальний % виконання і % залишку;
- exact asset rows, які змінилися;
- factual GAPs;
- перший незакритий checkpoint;
- чого конкретно не вистачає для наступних +10%.

Не повторювати завершені етапи й не підміняти відсутній локальний UE доказ source-only припущенням.
