# Audio UX benchmark — відкриті референси

Мета — взяти системні принципи, а не копіювати чужі звуки.

## Arma Reforger

Офіційні project settings розділяють загальний Volume, SFX, Music, Voice Chat, Dialogue та UI. Для Oster Conflict це підтверджує необхідність окремих mixer categories замість одного Master slider.

Source: https://community.bistudio.com/wiki/Arma_Reforger%3AResource_Manager%3A_Options

Arma Reforger Audio Editor також будує поведінку звуку як параметричні signal chains, що підтримує наш підхід DataAsset/MetaSound із залежністю від speed/load/distance/context.

Source: https://community.bistudio.com/wiki/Arma_Reforger%3AAudio_Editor

## Battlefield 6

Офіційні update notes 2026 згадують окремі 3D Audio settings / Headphone Width, а також регулярне тюнення audibility кроків, squad/spot ping, vehicle warning і vehicle/turret sounds. Висновок: просторовий output і читабельність пріоритетних сигналів важливіші за максимальну кількість фонового шуму.

Sources:
- https://www.ea.com/games/battlefield/battlefield-6/news/battlefield-6-game-update-1-3-2-0
- https://www.ea.com/games/battlefield/battlefield-6/news/battlefield-6-game-update-1-3-1-0

## Unreal Engine 5.8

- Sound Classes дозволяють групувати звуки та змінювати параметри всієї категорії.
- Attenuation підтримує distance falloff, occlusion, reverb sends та priority attenuation.
- MetaSounds дає параметричні DSP graphs, придатні для engines, wind, explosions та layered ambience.

Sources:
- https://dev.epicgames.com/documentation/unreal-engine/sound-classes-in-unreal-engine
- https://dev.epicgames.com/documentation/unreal-engine/sound-attenuation-in-unreal-engine
- https://dev.epicgames.com/documentation/unreal-engine/metasounds-quick-start

## Висновок для Oster Conflict

Залишаємо детальний mixer, але не створюємо десятки малозрозумілих користувацьких sliders. У Advanced можна пізніше відкрити додаткові категорії; базовий екран має бути читабельним: Master, SFX/Weapons, Vehicles, Characters, Environment, Music, UI, Voice Chat та Output/Dynamic Range.
