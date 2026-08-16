# Oster Conflict — S05: Downed / Medic / Revive

Статус: source milestone.
Engine target: Unreal Engine 5.8.

## Що додано

### Життєві стани
`UOCHealthComponent` тепер має явну модель:

- `Alive`
- `Downed`
- `Dead`

Звичайне летальне ушкодження переводить персонажа в `Downed`, а не одразу в respawn.
Важке одиничне ушкодження (`InstantDeathDamageThreshold`, за замовчуванням 180) може одразу завершити смерть.
Повторне ушкодження по Downed-цілі від 10 damage завершує смерть.

### Bleed-out
- Downed time: 60 секунд.
- Таймер існує на сервері.
- Для HUD реплікується кінцевий server timestamp.
- Клієнт рахує залишок через синхронізований `AGameStateBase::GetServerWorldTimeSeconds()`.

### Повзання
У S05 це gameplay-прототип повзання, а не фінальна prone-анімація:
- зброя ховається;
- sprint / ADS / fire / reload / weapon switching блокуються;
- персонаж переходить у crouched/downed presentation;
- камера опускається;
- швидкість руху обмежена до 95 cm/s;
- WASD лишається активним, щоб доповзти до укриття.

Фінальна анімація лежачи, переходи, IK і wounded animation set мають бути додані разом із character art/animation pass.

### Revive
- `E` біля Downed-персонажа: почати revive.
- Треба утримувати `E` 3 секунди.
- Сервер перевіряє:
  - reviver живий;
  - ціль Downed;
  - ціль не є самим reviver;
  - дистанція <= 220 cm;
  - line-of-sight не перекритий;
  - reviver має `bHasMedicCapability`.
- Перевірка повторюється під час revive.
- Відпускання `E`, вихід із дистанції, перешкода або зміна стану цілі скасовують revive.
- Після revive: 35 HP, потім стандартна CoD-style регенерація.
- Reviver отримує +1 Revive і +50 Score.

`bHasMedicCapability=true` для всіх prototype characters у S05, щоб механіку можна було тестувати без системи класів. У S06/S07+ це поле має задаватися loadout/class rules, а team-check буде доданий разом із двома командами.

### Give up
- У Downed стані `Space` більше не стрибає.
- Утримання Space 2 секунди запускає добровільну смерть.
- Відпускання Space скасовує процес.
- HUD показує progress bar.
- Сервер ще раз перевіряє, що персонаж досі Downed.

### HUD
Downed HUD:
- `DOWNED`;
- `BLEED OUT mm:ss`;
- `WAIT FOR MEDIC | CRAWL TO COVER`;
- `HOLD SPACE TO GIVE UP`;
- progress bar здачі.

Reviver HUD:
- `HOLD E REVIVE <name>`;
- `REVIVING <name>`;
- progress bar.

Scoreboard:
- додано колонку `R` = Revives.

## Kill/death accounting
- Downed ще не є Death.
- Якщо гравця підняли, Death не додається.
- Якщо гравець bleeding-out / give-up / finished, тоді реєструється Death і запускається старий respawn flow.
- При bleed-out kill attribution лишається за останнім damage instigator.
- Якщо Downed-гравця добив інший attacker, attribution оновлюється на finisher.

## Не входить у S05
- дві команди і friendly-only revive — S06;
- окремий medic class/loadout — після team/loadout framework;
- prone/crawl animation asset — character animation pass;
- кров, decals, ragdoll, dismemberment — S14;
- revive animation/sound — S15 + animation pass;
- revive syringe/medkit mesh — art pass.

## Перевірка
Запустити `VERIFY_S05.py` з кореня архіву.
