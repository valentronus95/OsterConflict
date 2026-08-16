# S15B Test Matrix

| # | Тест | Pass |
|---|---|---|
| 1 | Dedicated server + 2 clients, відкрити двері. | Server не відтворює звук; обидва клієнти отримують semantic event. |
| 2 | 10 разів відкрити/закрити двері. | Variant bank не зациклюється на одному sample, якщо в bank >1 asset. |
| 3 | Ворота/світло/вікно/руйнування. | Подія маршрутизується у правильний world bank. |
| 4 | Frag/flash/smoke. | Frag/flash мають small explosion hook; smoke не відтворює вибуховий report. |
| 5 | Museum/Park/College/Residential ambient zones. | Ambient лише локальний, має рідкі one-shots, при виході bed fade-out. |
| 6 | Ambience checkbox OFF. | Background bed та one-shots замовкають, gameplay не змінюється. |
| 7 | Master 0%, потім 100%. | Усі routed categories змінюють effective volume. |
| 8 | Weapons checkbox OFF. | Постріли/weapon state presentation muted; damage/shot gameplay працює. |
| 9 | Vehicle external vs driver/gunner. | Interior/exterior mix змінюється; engine pitch реагує на load/speed. |
|10| Handbrake на швидкості. | Skid layer активний тільки за релевантних умов. |
|11| Light/heavy vehicle collision. | Різні collision banks. |
|12| Character walk/sprint. | Footstep cadence різний; sprint може додавати gear rustle. |
|13| Character damage/downed/revive/death. | Відповідні character banks; без gameplay effect. |
|14| Menu Music checkbox OFF. | Background music зупинена/не стартує, UI SFX можуть лишатися. |
|15| Music 20%, UI 80%. | Категорії незалежні. |
|16| SaveAudioSettings + restart. | 0–100%, toggles, output/dynamic-range збережені. |
|17| Combat audio stress. | Ambience не маскує близькі кроки/постріли; decorative sources підлягають culling/concurrency. |
