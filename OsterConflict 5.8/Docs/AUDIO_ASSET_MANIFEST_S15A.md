# Audio Asset Manifest — S15A

## Правило ліцензування
У репозиторій дозволено додавати тільки: власні записи; власно синтезовані/згенеровані звуки з дозволеною ліцензією; CC0/royalty-free assets із перевіреними умовами; придбані assets, ліцензія яких дозволяє використання у грі. Не копіювати raw audio з Arma, Battlefield, Call of Duty чи інших ігор.

## Naming
`SFX_WPN_<family>_Shot_Close_01`
`SFX_WPN_<family>_Shot_Indoor_01`
`SFX_WPN_<family>_Shot_Suppressed_01`
`SFX_WPN_<family>_Tail_Distant_01`
`SFX_WPN_<family>_Mech_01`
`SFX_WPN_<family>_Reload_Start_01`
`SFX_WPN_<family>_Reload_End_01`
`SFX_Ballistic_Crack_01`
`SFX_Impact_Wood_01`, `Metal`, `Glass`, `Masonry`, `Dirt`, `Flesh`

## Мінімальний контент для Alpha
| Категорія | Мінімум | Ціль |
|---|---:|---:|
| Close unsuppressed / weapon family | 4 | 8-12 |
| Indoor / weapon family | 3 | 6-8 |
| Suppressed / weapon family | 3 | 6-8 |
| Distant tails / family | 3 | 6-10 |
| Mechanical / family | 3 | 6 |
| Reload phases / weapon | 2 | 4-8 |
| Bullet cracks | 4 | 8 |
| Impact / surface | 4 | 8-12 |

## Unreal content target
Final assets should use attenuation/spatialization settings. MetaSound Sources are preferred for random variation, layer control and future indoor/distant processing, while C++ only emits semantic gameplay audio events.
