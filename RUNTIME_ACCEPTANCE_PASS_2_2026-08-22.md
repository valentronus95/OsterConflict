# Runtime acceptance pass 2 — 2026-08-22

Статус: `CODED_UNTESTED`

Цей pass створено після повторного playtest, де runtime все ще показав: зміщення main menu після START, старий BASE spawn далеко від музею, відсутню траву, camera-origin tracer/muzzle presentation, civilian pickup/proxy mounted gun та proxy BTR.

## Виявлені конкретні причини

1. **Main menu jump** — `OCR13UIViewportStabilizerSubsystem` повторно перезаписував canonical geometry `R13_MenuPanel`: frontend owner задавав `(112,92)` / `440x760`, stabilizer задавав `(90,60)` / `470x780`. Stabilizer тепер має право міняти тільки isolation/Z-order, не position/size.
2. **Старий BASE spawn** — museum relocation була гарантована лише через `AOCTeamSpawnPoint::ConfigureServer`. Serialized `PlayerStart` міг увійти в runtime без повторного source-builder configure. `BeginPlay` тепер authority-side перевіряє base spawn і повторно прив'язує його до `MuseumAnchor`, якщо він лишився далеко.
3. **Трави немає після frontend** — `UOCDenseGroundFoliageSubsystem::OnWorldBeginPlay` раніше робив постійний `return`, якщо світ стартував як frontend-only. Frontend і gameplay використовують той самий `OsterConflict_Runtime`, тому другого `OnWorldBeginPlay` після START не було. Тепер subsystem чекає, доки frontend flag зникне, і після цього один раз виконує `Populate`.
4. **Tracer/muzzle летить із camera origin** — server hit validation правильно використовує camera trace, але той самий `TraceOrigin` передавався у visual FX. Локальний visual start тепер окремо виводиться з фактичної видимої геометрії current weapon; damage trace лишається camera-authored.
5. **Pickup/BTR/M2 proxies** — normal gameplay launcher раніше міг пройти з generated approximation або без HMMWV import. Тепер normal gameplay gate викликає повний `IMPORT_PRODUCTION_VEHICLES_UE58.cmd` і не запускає матч, якщо немає реальних source assets для HMMWV + M2 + BTR-4.

## Runtime contract після цього pass

- main menu не повинен змінювати position/size при START;
- main-menu START переводить у deployment без стрибка композиції;
- після вибору team/squad/role/base і deployment START з'являється blocking 0–100% loading overlay;
- після 100% player possession відбувається на BASE біля Museum;
- біля primary BASE є test weapon rack;
- grass population запускається після переходу frontend → gameplay;
- ballistic hit calculation лишається camera-based, але локальний muzzle/tracer починається з visible firearm geometry;
- normal gameplay не запускається з approximation HMMWV/M2/BTR як начебто production content.

## Не вважати перевіреним до локального UE 5.8 acceptance

Цей connector session не виконує локальний Unreal Editor build/runtime. Source і launcher changes мають статус `CODED_UNTESTED`, доки Windows UE 5.8 build та фактичний playtest не підтвердять поведінку.
