# GAMEPLAY BALANCE BASELINE v1 — P0_VERTICAL_SLICE

Canonical source defaults for the first working profile. These values are tuning defaults, not promises for later P1/P2 releases.

| Setting | Baseline | Source |
|---|---:|---|
| Max human slots | 16 | `AOCGameMode::MaxPlayerSlots` |
| Target total population | 16 | `AOCGameMode::TargetPopulation` |
| Starting tickets per team | 200 | `AOCGameMode::StartingTickets` |
| Ticket bleed interval | 5.0 s | `AOCGameMode::TicketBleedInterval` |
| Friendly fire | Off | `AOCGameMode::bFriendlyFire` |
| Respawn delay | 3.0 s | `AOCGameMode::RespawnDelay` |
| Downed duration | 60.0 s | `UOCHealthComponent::DownedDuration` |
| Give-up hold | 2.0 s | `AOCCharacter::GiveUpHoldSeconds` |
| Corpse lifetime source baseline | 30.0 s | `AOCGameMode::CorpseLifetimeSeconds` |
| Hard corpse cap | 20 | `AOCGameMode::MaxPersistentCorpses` |
| Bot refill delay | 3.0 s | `AOCGameMode::BotRefillDelay` |
| Squad size | 4 | `AOCGameMode::MaxSquadSize` |
| Dedicated server tick target | 30 Hz | `DefaultEngine.ini` |

## Change policy
A tuning change is allowed without changing network protocol if it does not change serialization/RPC compatibility, but the value must be recorded in CHANGELOG and the relevant gameplay/performance tests must be rerun.
