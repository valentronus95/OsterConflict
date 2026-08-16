# S14A acceptance matrix

1. Start 16 target population with bot fill: total converges to 16.
2. Human joins at 16/16 where at least one slot is bot: join succeeds and one bot is removed.
3. Continue joins: bots monotonically fall as humans increase.
4. At 16 human players: additional human receives SERVER_FULL_HUMANS.
5. Human leaves: after refill delay a bot restores target population when bot fill is enabled.
6. Replacement does not intentionally destroy the entire vehicle actor when the removed bot is a driver.
7. Teams remain approximately balanced while filler bots are added/removed.
8. New PlayerState exposes human/bot marker, squad, leader and ready state to clients.
9. Squad auto-assignment never exceeds four members with default settings.
10. Human entering a bot-led squad takes leadership.
11. Leader leaving causes deterministic leader repair.
12. SquadAttack A makes same-squad bots prefer objective A when they have no higher-priority combat/revive task.
13. SquadMoveHere/Regroup provide a movement target to same-squad bots.
14. Global chat reaches both teams.
15. Team chat reaches only same-team human clients.
16. Squad chat reaches only same-team same-squad human clients.
17. Chat messages over 120 chars are clipped and newline characters are removed.
18. Chat spam faster than 0.6 s is dropped server-side.
19. F8 deployment panel displays username/team/squad/role/ready and human+bot population.
20. F2/F3/F4 change role/squad/ready through server RPCs.
21. Household prop dressing contains sofa/table/chairs/kitchen/fridge/computer/laptop/storage/clutter proxies.
22. Household dressing has no loot/interact RPCs and is not replicated per prop.
23. S04-S13 regression verifiers still pass.
