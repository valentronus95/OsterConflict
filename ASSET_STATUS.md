# OSTER CONFLICT — ASSET STATUS

Date: 2026-09-04  
Branch: `fix/pass45-asset-import-fail-closed-20260904`  
Base: `main@a1ad0e200611911102c48180956d82f73d0d8fc3`  
Last verified code checkpoint: `31b679558a9c752a6e9291f30a97755daf720577`  
PR: #98 — Draft, unmerged  
GitHub CI on the last verified checkpoint: **9/9 SUCCESS**

## 1. ГОЛОВНА ТАБЛИЦЯ ПРОГРЕСУ

Індикатори:

- 🟢 `ГОТОВО` — пункт фактично закритий.
- 🟡 `ОЧІКУЄ / ЧАСТКОВО` — код готовий, але бракує локального UE/runtime/visual доказу.
- 🔴 `ПРОБЛЕМА / CONTENT GAP` — відсутній потрібний asset або є фактичний провал.

Кожен із 10 етапів = **10% загального прогресу**.

| № | Етап | Індикатор | Виконано в етапі | Вклад у загальний прогрес | Що вже зроблено | Що лишилось / проблема |
|---:|---|:---:|---:|---:|---|---|
| 1 | Local inbox / intake contract | 🟢 | 100% | +10% | `models_game_OC` lifecycle і safe local-only policy визначені | Нічого |
| 2 | Prepare / extract / classify | 🟢 | 100% | +10% | ZIP/loose sources безпечно розпаковуються і класифікуються | Нічого |
| 3 | Exact duplicate removal | 🟢 | 100% | +10% | SHA-256 dedupe працює до UE import | Нічого |
| 4 | Fab / Marketplace / project discovery | 🟢 | 100% | +10% | Скануються `/Game` і project/plugin content mounts | Нічого |
| 5 | Production import logic | 🟢 | 100% | +10% | Є import paths для HMMWV, M2, BTR-4, M249, Remington 870 | Нічого по коду |
| 6 | Fail-closed aggregate result | 🟢 | 100% | +10% | GAP/exception блокує фальшивий PASS; stale PASS markers видаляються | Нічого |
| 7 | GitHub source / regression CI | 🟢 | 100% | +10% | Exact-head source checks пройшли 9/9 | Нічого |
| 8 | Local UE 5.8 import result | 🟡 | 0% | +0% | Importer і `LOCAL_ASSET_STATUS` collector готові | Потрібен фактичний прогін на локальному UE 5.8 |
| 9 | Live gameplay/runtime hookup | 🟡 | 0% | +0% | Runtime validators і canonical evidence route готові | Потрібні live inbox/world/material/gameplay результати |
| 10 | Direct visual acceptance + ZIP cleanup | 🟡 | 0% | +0% | Правила acceptance/cleanup визначені | Треба побачити assets у грі; ZIP видаляти тільки після PASS |

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
| Фактичних failed GitHub checks на останньому verified checkpoint | **0** | 🟢 |

**Поточний стан: 🟡 70% — кодова частина закрита, решта блокується відсутністю локального UE/runtime/visual доказу, а не незакритим GitHub-кодом.**

## 2. PRODUCTION ASSET MATRIX

Для кожного production asset використовується 4 фактичні ворота:

1. source/import support;
2. UE import result;
3. live runtime hookup;
4. direct visual acceptance.

Відсоток нижче — кількість фактично підтверджених воріт, а не оцінка часу.

| Asset | Індикатор | Підтверджено | Поточний стан | Чого бракує |
|---|:---:|---:|---|---|
| HMMWV | 🟡 | **25%** | Import support готовий | Fresh UE import, live vehicle use, scale/orientation/material visual proof |
| M2 Browning | 🟡 | **25%** | Import support готовий | Fresh UE import, HMMWV mount runtime, pitch/muzzle/material visual proof |
| BTR-4 | 🟡 | **25%** | Import/fallback support готовий | Fresh UE result, live use, proportions/orientation/material visual proof |
| M249 | 🟡 | **25%** | Exact-source importer готовий | Перевірити exact local payload, UE result, runtime, visual |
| Remington 870 | 🟡 | **25%** | Exact-source importer готовий | Перевірити exact local payload, UE result, runtime, visual |
| M16/M4 family | 🔴 | **0% READY** | Generic classifier є, dedicated verified payload нема | Потрібен фактичний production payload і весь UE/runtime/visual цикл |

`25%` тут означає лише: **код/import support існує**. Це не означає, що asset уже реально працює в грі.

## 3. OTHER LOCAL / FAB ASSET FAMILIES

| Family | Intake support | Runtime state | Індикатор | Що потрібно для зеленого |
|---|---|---|:---:|---|
| AK-47 | supported | PENDING | 🟡 | `runtime_bindings.json` + gameplay load |
| MP5 | supported | PENDING | 🟡 | manifest + gameplay load |
| M1911 | supported | PENDING | 🟡 | manifest + gameplay load |
| M700 | supported | PENDING | 🟡 | manifest + gameplay + visual calibration |
| M14 | supported | PENDING | 🟡 | manifest + gameplay load |
| MAC-10 | supported | PENDING | 🟡 | manifest + gameplay load |
| TEC-9 | supported | PENDING | 🟡 | manifest + gameplay load |
| Lever Action | supported | PENDING | 🟡 | manifest + gameplay + visual calibration |
| Other weapon/launcher assets | supported/classified | PENDING | 🟡 | exact bound asset paths + runtime |
| Pickups / vehicle props | supported/classified | PENDING | 🟡 | live placement/load proof |
| Buildings | supported/classified | PENDING | 🟡 | `local_world_runtime_validation.txt` |
| Props / furniture / fences | supported/classified | PENDING | 🟡 | live world validation |
| Foliage / trees / grass | supported/classified | PENDING | 🟡 | live placement + visual proof |
| Roads / sidewalks | supported/classified | PENDING | 🟡 | live placement + visual proof |
| Ground / terrain | supported/classified | PENDING | 🟡 | live placement + visual proof |
| Water / river assets | supported/classified | PENDING | 🟡 | live placement + visual proof |
| Character skins | skeletal import supported | PENDING | 🟡 | compatible skeleton count + live character proof |
| HUD/UI textures/widgets | supported | PENDING | 🟡 | manifest + actual HUD usage |
| Fab/Marketplace/project-plugin meshes | automatic mount discovery | PENDING | 🟡 | exact discovered/bound/failure counts |

## 4. ЩО САМЕ ЗАРАЗ БЛОКУЄ НАСТУПНІ 30%

| Блокер | Індикатор | Чого немає зараз | Що дасть результат |
|---|:---:|---|---|
| Fresh local UE import manifest | 🟡 | `runtime_bindings.json` поточного запуску | точні discovered/imported/bound/unbound counts |
| Production vehicles import result | 🟡 | fresh `production_import_success.txt` | статус HMMWV/M2/BTR-4 |
| Exact weapons import result | 🟡 | fresh `production_weapon_import_result.txt` | статус M249/Remington |
| Local inbox runtime proof | 🟡 | `local_inbox_runtime_validation.txt` | чи реально відкриваються bound assets у gameplay |
| Local world runtime proof | 🟡 | `local_world_runtime_validation.txt` | чи buildings/props/foliage/roads/water реально підключені до Остра |
| Gameplay evidence | 🟡 | fresh `R14_CURRENT_GAMEPLAY.log` | vehicles/input/FPS/runtime ownership |
| Material evidence | 🟡 | fresh `PASS45_STRICT_MATERIAL_GATE.log` | production material/dependency PASS/GAP |
| Visual inspection | 🟡 | пряме спостереження в UE/game | останній acceptance перед READY/cleanup |

Наразі **червоного runtime FAIL не підтверджено**, бо свіжого локального UE-прогону ще немає. Червоним позначений тільки відомий `M16/M4` content gap, де немає verified dedicated payload.

## 5. ЄДИНИЙ ЛОКАЛЬНИЙ STATUS SNAPSHOT

`COLLECT_LOCAL_ASSET_STATUS.py` уже підключений до canonical runtime evidence verifier і запускається **як при PASS, так і при FAIL**.

Після фактичного локального прогону він створює:

- `OsterConflict/Saved/AssetStatus/LOCAL_ASSET_STATUS.txt`
- `OsterConflict/Saved/AssetStatus/LOCAL_ASSET_STATUS.json`

У snapshot входять:

- prepared counts;
- discovered/imported/bound/unbound counts;
- category counts;
- HMMWV / M2 / BTR-4 status;
- M249 / Remington status;
- local inbox/world runtime status;
- material/evidence state;
- exact GAP reasons;
- список відсутніх evidence files.

Це прибирає ручне полювання по `Saved` і логах, але не підміняє сам UE runtime proof.

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

## 7. ЗАКРИТО В PR #98

- 🟢 aggregate importer fail-closed для production vehicle/exact-weapon GAP;
- 🟢 stale global/sub-import PASS sentinels очищаються перед fresh run;
- 🟢 required production failures записуються як `UNBOUND`;
- 🟢 старі Pass38/Pass37/Pass33 залежності від видаленого `RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd` прибрані;
- 🟢 `COLLECT_LOCAL_ASSET_STATUS.py` збирає один `.txt` + `.json` snapshot;
- 🟢 snapshot створюється і після runtime PASS, і після runtime FAIL;
- 🟢 source/regression CI останнього verified checkpoint пройшов 9/9.

## 8. ПЕРШИЙ НЕЗАКРИТИЙ CHECKPOINT

### 🟡 `LOCAL-UE-ASSET-001`

**Завдання:** виконати current full asset/runtime path на фактичному локальному UE 5.8 проекті та отримати свіжий `LOCAL_ASSET_STATUS` snapshot.

Після цього можна буде без припущень:

1. заповнити точні `prepared / discovered / imported / bound / unbound` цифри;
2. перевести фактично підтверджені assets із 🟡 у 🟢;
3. фактичні помилки перевести в 🔴 з конкретною причиною;
4. закрити етап 8 і підняти загальний прогрес із **70% до 80%**, якщо local UE import PASS;
5. потім закрити live runtime етап 9 до **90%**;
6. після direct visual acceptance і safe source cleanup закрити етап 10 до **100%**.

## 9. CONTINUATION RULE

Кожен наступний asset-pass повинен оновити цю таблицю, а не писати прогрес лише текстом:

- current branch/head/PR;
- 🟢 / 🟡 / 🔴 для кожного етапу;
- загальний % виконання;
- скільки % лишилось;
- exact asset rows, які змінилися;
- factual GAPs;
- перший незакритий checkpoint;
- чого конкретно не вистачає для наступних +10%.

Не повторювати завершені етапи й не підміняти відсутній локальний UE доказ source-only припущенням.
