# S08 Test Matrix

| ID | Перевірка | Очікуваний результат |
|---|---|---|
| S08-01 | Dedicated server стартує | World sector + enterable house spawned |
| S08-02 | Client дивиться на двері в межах дистанції | HUD показує OPEN/CLOSE prompt |
| S08-03 | Client натискає E на дверях | Двері плавно відкриваються |
| S08-04 | Другий client спостерігає двері | Стан дверей однаковий |
| S08-05 | E поза interaction distance | Двері не змінюють стан |
| S08-06 | E поруч, але приціл не на дверях | Двері не змінюють стан |
| S08-07 | Character проходить крізь відкриті двері | Вхід усередину не блокується shell geometry |
| S08-08 | Character проходить між кімнатами | Interior doorway не заблокований |
| S08-09 | Постріл у фронтальне вікно | Glass pane руйнується |
| S08-10 | Постріл у заднє вікно | Glass pane руйнується |
| S08-11 | Постріл у бокове вікно | Glass pane руйнується |
| S08-12 | Другий client після пострілу | Бачить broken state того самого вікна |
| S08-13 | Новий client підключається після руйнування | Бачить відсутнє скло без старого shard burst |
| S08-14 | Shard burst після руйнування | Уламки короткочасні й автоматично прибираються |
| S08-15 | Revive біля будинку | Revive має вищий пріоритет за world interaction |
| S08-16 | Weapon/ammo pickup поруч | S04 pickup logic працює, якщо world interactable не у focus |
| S08-17 | A/B/C Conquest | Capture/tickets/spawn не регресували |
| S08-18 | Повторний раунд | Enterable house/window state не спричиняє crash |

## Примітка
S08 source verification не замінює реальний multiplayer PIE/dedicated-server test у UE 5.8. Особливо перевірити collision doorway, component transforms та cosmetic shard physics.
