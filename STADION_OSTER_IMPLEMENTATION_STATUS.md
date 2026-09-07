# OSTER CONFLICT — STADION OSTER IMPLEMENTATION STATUS

Статус: `SOURCE VERIFIED / RUNTIME EVIDENCE PENDING`
Інтеграційна гілка: `main`
Історичний Draft PR `#14`: `CLOSED / SUPERSEDED`
Оновлено: `2026-08-22`

## Що вже є в актуальному `main`

- canonical WGS84 anchor: `50.949360, 30.884660`;
- один authoritative owner: `UOCR13StadiumSurfaceSubsystem`;
- runtime tag/root: `R13_StadionOsterAuthoritative` / `R13_StadionOsterSiteRoot`;
- старий delayed museum/stadium builder retired і більше не створює geometry;
- legacy `StadiumGeometry` / `StadiumDetails` приховуються, а shared fences очищаються лише в stadium zone;
- поле 105 × 68 м, розмітка, повнорозмірні ворота, тренувальні ворота, спортзона, вхідна синьо-жовта конструкція;
- окремий `TextRenderComponent` з написом `СТАДІОН ОСТЕР`;
- imported rural houses / trees / fences підключені до site owner;
- сегментовані ґрунтові стежки замість road-like прямих смуг;
- stadium XY береться з georeference, а Z підбирається runtime line trace по фактичному terrain;
- стара гігантська зелена `GrassApron`-підкладка прибрана, щоб не перекривати terrain/foliage прямокутною плитою;
- `VERIFY_R13_STADION_OSTER.py` підключений до source verification;
- 17-frame reference index збережений; пошкоджений ZIP не маскується як валідний payload і позначений `RESTORE_REQUIRED`, доки його не відновлено з оригіналів.

## Active PASS45 continuation — 2026-08-30

On open/unmerged PR #94, source head `9d04baab648fe75ccb0e6903365f438c55230609` makes the Stadion Oster perimeter tree owner directly select committed `HillTree_02` and `ScotsPineTall_01`. Legacy `SM_Tree_Var01/04` authoring is forbidden and the source has no late tree remap. This is source-verified only; current-head UE 5.8 visual/runtime evidence remains pending.

## Pass 9 — runtime evidence

Гілка: `fix/runtime-acceptance-stadion-pass-9-20260822`

Додано `UOCR13StadiumRuntimeValidationSubsystem`, який у реальному gameplay world перевіряє:

1. рівно один actor з tag `R13_StadionOsterAuthoritative`;
2. наявність основного pitch, running surface, pitch lines, sports metal, footpaths, entrance, houses, trees і replacement fences;
3. відсутність obsolete `StadionOsterGrassApron`;
4. наявність `StadionOsterEntranceText`;
5. XY фактичного pitch відносно canonical georef;
6. Z pitch відносно нового terrain line trace;
7. що legacy `StadiumGeometry` / `StadiumDetails` не повернулися у visible state.

Runtime markers:

- success: `PASS9_STADION_OSTER_READY`;
- failure: `PASS9_STADION_OSTER_RUNTIME_FAIL`.

Окремий Windows launcher: `RUN_R14_STADION_RUNTIME_ACCEPTANCE.cmd`.
Він спочатку запускає повний strict `RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd`, а потім вимагає stadium READY marker і відхиляє stadium FAIL marker з того самого UE log.

## Що все ще не можна називати VERIFIED

До повного `VERIFIED` все ще потрібні:

1. успішний UE 5.8 compile/build на Windows;
2. локальний gameplay test через `RUN_R14_STADION_RUNTIME_ACCEPTANCE.cmd`;
3. `PASS9_STADION_OSTER_READY` без `PASS9_STADION_OSTER_RUNTIME_FAIL`;
4. ручна перевірка collision воріт, replacement fences та entrance structure;
5. відсутність z-fighting / duplicate surfaces / delayed flicker;
6. візуальне зіставлення входу, дерев, стежок, житлової межі та спортзон із canonical reference set;
7. перевірка напису `СТАДІОН ОСТЕР`: правильний бік, кирилиця, без mirrored rendering;
8. відновлення пошкодженого stadium reference ZIP з оригінальних 17 кадрів.

## Історія інтеграції

Старий Draft PR `#14` закрито як superseded. Його корисна stadium-реалізація вже була forward-ported у `main`, а `main` після цього отримав новіші fixes, зокрема terrain-Z snap і прибирання oversized green slab. Прямий merge старого PR тепер був би відкатом частини актуального stadium code.
