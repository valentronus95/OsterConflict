# AI architecture — S13

## Authority

AI controllers існують і приймають рішення на сервері. Клієнти отримують стандартну replicated position/weapon/health/objective state.

## Perception and navigation

`UAIPerceptionComponent` використовується для sight events. Для source-only fallback controller додатково перевіряє LOS до character actors, але fallback зберігає налаштовані sight radius та field-of-view, тобто не дає боту 360° «всезнання». Переміщення йде через `AAIController::MoveToActor/MoveToLocation` по NavMesh. Runtime Recast generation увімкнена в конфігурації, тому карта, яка створюється кодом, може отримати навігацію під час запуску після додавання NavMesh bounds у production level.

## Decision priority

1. Якщо controller веде машину — рух до cached objective.
2. Якщо medic бачить доступного downed союзника і поблизу немає близької загрози — revive.
3. Видимий ворог — combat.
4. Вільна далека objective — за можливості знайти машину.
5. Рух до A/B/C.
6. Перед собою бот перевіряє interactable door/gate й відкриває її.

## Difficulty

- Easy: повільна реакція, більша похибка, менше поле зору.
- Normal: baseline.
- Hard: швидша реакція, краща точність і cover search.
- Veteran: найкоротша реакція та найменша штучна похибка, але weapon spread/reload/ammo все одно не обходяться.

Важливо: складність не дає ботам додаткового HP або прихованого damage multiplier. Вони грають тією самою weapon/health framework, що й гравці.

## Lifecycle / cleanup

- При round reset AI controller коректно виходить із vehicle, видаляє старий bot pawn і отримує новий spawn.
- Sandbox `Clear AI bots` також спочатку повертає AI-driver з машини в character pawn, тому очищення ботів не повинно знищувати саму тестову машину.
- Якщо medic змінює пріоритет із revive на combat/objective, активний revive timer скасовується сервером.
