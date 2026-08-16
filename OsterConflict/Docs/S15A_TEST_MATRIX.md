# S15A Test Matrix

1. Dedicated server: no audio device/playback attempts.
2. Close outdoor AR shot: near report fires on clients.
3. Indoor shot: Indoor profile selected when roof + surrounding walls are detected.
4. Suppressor installed: suppressed set selected; fallback lowers volume if suppressed set empty.
5. Listener beyond near radius: distant tail selected.
6. Listener near hitscan segment but not shooter: bullet crack fires for supersonic profile.
7. Shotgun profile can disable supersonic crack.
8. Empty magazine trigger: dry-fire is rate limited.
9. Reload start/end/cancel each produce one semantic audio event.
10. Fire mode switch produces state audio event.
11. Impact sound routes by Flesh/Glass/Wood/Metal/Masonry/Dirt.
12. Missing audio profile: gameplay still works and no crash occurs.
13. Two clients at different distances can hear different presentation for the same server-confirmed shot.
14. `oc.Audio.Debug 1` exposes event routing in development build.
