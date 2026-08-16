# S18A — Performance budgets

Це acceptance targets, а не вже виміряні результати.

## Client target
- Основна ціль: стабільний 60 FPS profile на цільовому 1080p PC після появи production assets.
- 60 FPS frame budget: 16.67 ms.
- Не приймати “average 60” як достатній результат, якщо є регулярні frame spikes.
- Перевіряти Game Thread, Render Thread і GPU окремо в Unreal Insights.

## Dedicated server target
- Server tick target: 60 Hz у поточному prototype config.
- Game-thread робота повинна вкладатися в tick budget без сталого backlog.
- Обов'язкові 8 і 16 population tests.
- Окремо перевіряти 16 AI, 8 human+8 AI і 16 humans, бо вони навантажують різні системи.

## Network
- Не допускати сталого упору в MaxClientRate/MaxInternetClientRate.
- У Networking Insights шукати найважчі Actors, Properties і RPC.
- Cosmetic blood/debris/ambient one-shots не повинні генерувати gameplay replication flood.
- Static interactables мають низьку update frequency; state changes примусово роблять ForceNetUpdate.
- Replication Graph — умовний наступний крок після вимірювання, не автоматична “оптимізація”.

## Runtime presentation budgets
| Система | Balanced | LowCPU | Quality |
|---|---:|---:|---:|
| Persistent corpses | 16 | 10 | 20 |
| AI think multiplier | 1.00x | 1.35x | 0.90x |
| Ambient update | 5 Hz client | 5 Hz client | 5 Hz client |
| Ambient dedicated-server tick | Off | Off | Off |
| Door idle tick | Off | Off | Off |

## Art/content budgets після імпорту assets
- Foliage: HISM/Foliage System, cull distances, density scalability.
- Будинки: HLOD/World Partition для distant кварталів.
- Інтер'єри: не тримати сотні унікальних movable actors там, де достатньо instancing/static geometry.
- Дрібні blood/debris FX: локальні, короткоживучі, distance culled.
- Skeletal meshes: LOD/animation budget/visibility-based ticking після появи фінальних моделей.
- Audio: concurrency/virtualization, особливо weapon tails, ambience, vehicles.
