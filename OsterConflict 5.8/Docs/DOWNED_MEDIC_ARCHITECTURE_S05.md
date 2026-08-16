# S05 Architecture — Downed / Medic

## Authority ownership

### Health component
Server owns:
- transition Alive -> Downed -> Dead;
- bleed-out timer;
- lethal/finisher rules;
- revive health;
- final death event.

Replicated:
- `CurrentHealth`;
- `LifeState`;
- `DownedEndServerTime`.

### Character
Server owns:
- revive target;
- revive completion timestamp;
- give-up completion timestamp;
- revive validation and cancellation;
- final give-up request.

Client owns only presentation/input intent:
- HUD progress;
- camera lowering;
- input request RPCs.

## State transition table

| Current | Event | Result |
|---|---|---|
| Alive | Health <= 0, ordinary hit | Downed |
| Alive | Health <= 0, >= instant-death threshold | Dead |
| Downed | 60 s elapsed | Dead |
| Downed | qualifying finisher damage | Dead |
| Downed | hold Space completed | Dead |
| Downed | valid medic revive completed | Alive at 35 HP |
| Dead | respawn timer | new Character pawn |

## Revive validation
A revive completes only if the server still confirms:
1. reviver has authority-side medic capability;
2. reviver is Alive;
3. target is Downed;
4. target != reviver;
5. target is within `ReviveDistance`;
6. visibility trace is not blocked.

The validation runs when revive begins and again every server tick while a target is assigned, then once more at completion.

## S06 integration contract
S06 must replace the prototype permissive medic configuration with:
- `TeamId` in PlayerState;
- same-team requirement;
- role/loadout capability assignment;
- enemy Downed players cannot be revived;
- optional enemy finish/interrogate interactions can be separate systems.
