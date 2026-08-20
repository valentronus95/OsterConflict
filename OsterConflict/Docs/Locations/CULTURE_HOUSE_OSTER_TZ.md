# OSTER CONFLICT — ТЗ: ОСТЕРСЬКИЙ МІСЬКИЙ БУДИНОК КУЛЬТУРИ

Status: `CODED_UNTESTED`
Integration branch: `main`
Runtime owner: `UOCR146CultureHousePhotoModelSubsystem`
Address: м. Остер, вул. Грановського, 3

## 1. Головний контракт

Будинок культури є окремим landmark і не може використовувати координати, shell, parcel або runtime owner музею, Сільпо, стадіону чи парку.

Єдина runtime-точка істини: `FOCGeoReference::CultureHouse()`.

Поточний public-map anchor:
- latitude: `50.948694`
- longitude: `30.881435`
- confidence: `B`

Адреса `вул. Грановського, 3` підтверджена публічними/офіційними записами. Координата використовується як map placement, а не як кадастрове або геодезичне вимірювання. Точний bearing фасаду поки не видається за survey-grade факт.

## 2. Ownership

Authoritative actor tags:
- `R146_CultureHouseAuthoritative`
- `CultureHouseOster_Hranovskoho3`

Заборонено:
- спавнити Будинок культури на `Museum()`;
- спавнити його на `Silpo()`;
- вставляти shell Будинку культури всередину іншої будівлі;
- залишати одночасно legacy `R13_CultureHousePhotoModel` і current R14.6 owner;
- робити пізній багатосекундний replacement уже видимої чужої локації.

## 3. Поточна модель R14.6

Photo-driven/current-history silhouette відновлений як один атомарний site root:
- одноповерховий високий зал;
- шестиколонний портик;
- три високі вхідні прольоти;
- арочне верхнє скління;
- фронтон;
- широкий ступінчастий вхід;
- окремий підхід до входу;
- консервативні бокові та задні вікна;
- карниз та водостоки як detail layer.

Уся геометрія задається локально від одного root. Перенесення site anchor переносить будівлю цілком, а не окремими несинхронізованими шматками.

## 4. Map separation

`UOCR146LandmarkSeparationSubsystem` до reveal current landmark owners:
- чистить generic source-building instances у локальних parcel-зонах Museum / Silpo / Culture House;
- прибирає legacy mixed-location actors, якщо вони повернуться через regression;
- не переносить current owners один поверх одного;
- прибирає стару synthetic north-civic grove + пряму умовну park-link смугу, які не були підтвердженою вулицею/parcel Будинку культури.

## 5. Acceptance

До `VERIFIED` потрібно:
1. UE 5.8 build `Succeeded`;
2. current-main Sandbox playtest;
3. музей видно тільки на музейній ділянці;
4. Сільпо видно тільки на вул. Богдана Хмельницького, 54;
5. Будинок культури видно тільки на вул. Грановського, 3;
6. немає building-in-building / duplicate shells / z-fighting;
7. немає legacy mixed civic/Silpo scene;
8. уточнити yaw/parcel footprint лише після надійнішої картографічної або фото-прив'язки.
