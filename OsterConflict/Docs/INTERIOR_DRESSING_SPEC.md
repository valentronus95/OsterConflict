# INTERIOR_DRESSING_SPEC — S14A

## Мета
Доступні приватні будинки Остра мають відрізнятися плануванням, станом і побутовим наповненням. Не робити квартал із клонованих ідеальних prefab-інтер'єрів.

## Профіль
- `InteriorSeed`: детермінований seed.
- `Condition`: Worn / Ordinary / Maintained.
- `LayoutVariant`: варіант розташування декоративних груп.

## Базові decorative prop groups
- диван / крісло;
- столи та різні стільці;
- шафи / тумби / полиці;
- проста кухня / стільниця;
- холодильник;
- плита;
- телевізор або монітор;
- робочий стіл;
- настільний ПК / tower;
- ноутбук;
- коробки / пакети / дрібний побутовий clutter.

## Правила
- S14A props не є loot і не мають interaction RPC.
- Не реплікувати кожен предмет окремим Actor.
- Використовувати ISM/HISM/Level Instances/Packed Actors там, де це доречно.
- Не перекривати двері, вікна, revive-space і основні navigation/combat corridors.
- Різний Condition впливає на completeness, clutter, rotation/placement variation, але не змінює gameplay collision хаотично.
- Публічні landmark фасади можуть бути reference-driven; приватні інтер'єри не копіюються 1:1.
