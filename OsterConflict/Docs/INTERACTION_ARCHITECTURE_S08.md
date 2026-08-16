# S08 Interaction Architecture

## Мета
Не прив'язувати клавішу `E` окремим hardcode до кожного нового об'єкта.

## Потік

`AOCCharacter::ServerInteract()`

1. Перевіряє, що Character живий.
2. Перевіряє friendly downed target для revive.
3. Робить server-side line trace перед очима Character.
4. Якщо trace потрапив у `AOCInteractableActor`, викликається `CanInteractServer()`.
5. Тільки після успішної перевірки викликається `InteractServer()`.
6. Якщо world interaction відсутня, перевіряються weapon/ammo pickups.

## AOCInteractableActor
Базовий replicated Actor.

Віртуальний API:
- `GetInteractionPrompt()`;
- `CanInteractServer()`;
- `InteractServer()`;
- `GetMaxInteractionDistance()`.

Це мінімальна основа. Вона не диктує конкретну дію.

## AOCInteractableDoor
Gameplay state:
- `bOpen` replicated.

Локальна презентація:
- DoorLeaf інтерполює yaw до `0` або `OpenYawDegrees`.

Таким чином мережа передає **стан**, а не кожен кадр анімації дверей.

## AOCBreakableWindow
Gameplay state:
- `bBroken` replicated + RepNotify.

Authority:
- damage приходить через штатний `ApplyPointDamage` weapon pipeline;
- сервер визначає момент руйнування;
- після руйнування pane collision вимикається.

Cosmetic:
- `NetMulticast, Unreliable` запускає короткий shard burst;
- уламки не використовуються для gameplay damage або authoritative collision;
- вони ховаються через кілька секунд;
- join-in-progress не відтворює старий burst.

## Чому це важливо
Мережевий стан дверей/вікна має бути однаковим для всіх. Фізична траєкторія шести дрібних уламків не повинна споживати реплікацію і впливати на результат бою.

## Наступне розширення
На `AOCInteractableActor` можна без зміни Character API посадити:
- двері з замком;
- ворота;
- стаціонарну зброю;
- ammo resupply station;
- repair station;
- вибухівку/інженерні об'єкти;
- генератор/вимикач;
- objective-specific interactions.
