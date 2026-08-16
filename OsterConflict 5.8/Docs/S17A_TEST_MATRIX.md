# S17A Test Matrix

1. Launch client with `-Frontend`; frontend is visible and movement/look are locked.
2. Enter Username + `127.0.0.1:7777`, Connect; server receives name via travel option.
3. Escape toggles frontend overlay; gameplay resumes when closed.
4. Deployment shows current username/team/faction/role/squad and Human/Bot population.
5. Request Team 1/2 before deploy; authority accepts only balanced human switch.
6. Cycle Role and Squad; replicated state updates in deployment roster.
7. Select BASE and deploy; chosen team base is preferred.
8. Own objective A, select A, respawn; A forward spawn is preferred.
9. Lose/contest A before respawn; selection safely falls back rather than spawning illegally.
10. Hold TAB; rich scoreboard shows two teams, bots, squad/role/K/D/R/Score/Ping.
11. Press T; chat input opens with mouse/UI mode. Send ALL, TEAM and SQUAD messages.
12. Verify TEAM/SQUAD recipients only receive permitted messages.
13. Sandbox: F10 opens rich admin panel; Previous/Next/Execute call existing server actions.
14. Close all modal UI; mouse disappears and gameplay movement/look return.
15. Disable/omit Rich UI root in a test subclass; Canvas HUD fallback remains functional.
