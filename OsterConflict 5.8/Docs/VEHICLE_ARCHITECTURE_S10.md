# Vehicle Architecture S10

`AOCVehicleBase`
- APawn, який може бути possessed водієм.
- UBoxComponent є фізичним коренем.
- Чотири raycast suspension points прикладають spring/damper forces.
- Drive/grip/steering обраховуються тільки authority.
- ReplicateMovement передає результат клієнтам.
- DriverCharacter реплікується як seat occupancy state.
- Character переходить у прихований `bInVehicle` стан і відновлюється після виходу.

`AOCCivilianVehicle`
- три source-only габаритні варіанти;
- спільна фізична база;
- різна маса та максимальна швидкість.

`AOCVehicleSpawnPoint`
- тримає одну активну машину;
- після переходу машини у Wrecked запускає respawn;
- wreck живе окремо і прибирається пізніше.

## Відділення gameplay від cosmetics
Gameplay state реплікується: driver, input state, health, damage stage, destroyed state, actor movement.
Косметика damage stage застосовується локально через `OnRep_DamageState`.
