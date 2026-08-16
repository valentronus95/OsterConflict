# S07 — архітектура карти Остра

## 1. Система координат
Локальний нуль: Остерський краєзнавчий музей.

- `+X` — схід;
- `-X` — захід;
- `+Y` — північ;
- `-Y` — південь;
- `Z` — висота;
- одиниця UE = сантиметр.

Перевага такого підходу: подальші reference points можна переводити з широти/довготи у локальні метричні зміщення без необхідності тягнути GIS runtime у бойову логіку.

## 2. Межі S07
Greybox має основу близько 1.8 x 1.8 км. Це не фінальна межа всієї майбутньої карти Остра. Після S09/S16 світ може бути розширений, а geometry перенесена у World Partition level.

## 3. S07 landmark anchors
| Anchor | Local X | Local Y | Призначення |
|---|---:|---:|---|
| Museum | 0 м | 0 м | географічний нуль, objective A sector |
| Stadium | +150 м | -15 м | gameplay-stylized stadium beside museum |
| College | -333 м | +94 м | coarse real-relative anchor |
| City Park | -612 м | +735 м | coarse real-relative anchor |

`Stadium` поки gameplay-stylized, бо S07 не має достатнього survey/reference pack для сантиметрової прив'язки footprint.

## 4. Runtime geometry
`AOCWorldSectorOster` використовує instanced static meshes із Engine BasicShapes. Це тимчасовий source-only спосіб отримати масштабний playable greybox без бінарних `.uasset`.

Компоненти:
- Ground;
- Roads;
- Sidewalks;
- Buildings;
- LandmarkBlocks;
- Fences;
- TreeTrunks;
- TreeCrowns;
- StadiumGeometry;
- ParkGeometry.

Ця геометрія замінюється modular art kit у S16, але координати/маршрути можуть бути збережені.

## 5. Музей / стадіон
Museum sector має:
- нижній основний об'єм;
- верхній центральний об'єм;
- вхідний виступ;
- бічний об'єм;
- perimeter/garden edge;
- дерева у дворі.

Stadium sector має:
- велике поле;
- дві прості довгі трибуни;
- end structures;
- два goal-frame collision proxies.

## 6. Дорожня мережа
S07 будує не точну копію кожної смуги, а gameplay corridors:
- магістраль музей/стадіон;
- вісь Татарівської;
- вісь Соломії Крушельницької;
- кілька перехресних вулиць;
- підхід до міського парку;
- вузькі приватні вулиці.

У S08 ця мережа уточнюється насамперед уздовж Соломії Крушельницької та приватного сектору.

## 7. Conquest mapping
- `A`: midpoint Museum ↔ Stadium;
- `B`: City Park;
- `C`: College courtyard sector.

Capture radius збільшено порівняно з S06 test arena, щоб точки відповідали великому простору.

## 8. Performance
S07 свідомо використовує instancing для повторюваних будинків, огорож і дерев. Не створюється окремий Actor на кожне дерево/будинок. Це критично для майбутньої великої карти й дешевше для replication/world management.

## 9. Наступні кроки
S08:
- деталізувати вул. Соломії Крушельницької;
- приватні двори;
- перший enterable-house kit;
- interaction door/window foundation.

S09:
- точніше відбудувати міський парк;
- коледж та двір;
- поєднати центральні квартали;
- перевірити піхотні/майбутні автомобільні маршрути.
