# Weapon Audio Architecture — S15A

## Потік події
1. Гравець/AI просить постріл.
2. Сервер перевіряє cadence, ammo, reload і виконує authoritative trace/damage.
3. Сервер класифікує оточення джерела: Outdoor / SemiIndoor / Indoor.
4. Сервер multicast-ить компактну audio event подію: origin, trace end, suppressor, environment, seed.
5. Кожен клієнт сам визначає свою дистанцію до джерела та обирає Near або Distant presentation.
6. Bullet-crack рахується тільки локально від траєкторії до listener; це косметика, не gameplay.

## Мікс
- Close weapon report: основний постріл.
- Indoor layer: коротший, жорсткіший early-reflection profile.
- Distant tail: окремий запис/MetaSound layer на великій дистанції.
- Mechanical layer: переважно власник зброї, менш гучний.
- Bullet crack: окремий supersonic transient біля listener.
- Impact: матеріал-залежний one-shot.

## Майбутній content pass
Для кожної сім'ї зброї бажано 5–12 варіантів близького пострілу, 3–8 distant tails, 3–6 механічних шарів і окремі indoor/suppressed набори. Рандомізація повинна уникати очевидного повторення, а MetaSound може додати pitch/volume variation та layered DSP без зміни C++ API.
