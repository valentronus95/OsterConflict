# S16A TEST MATRIX — Oster reference-driven map

1. **Geo origin** — Museum anchor resolves to ~0,0 local XY.
2. **College geo transform** — College is west/north of museum and no longer uses an arbitrary hardcoded S07 coordinate.
3. **Park correction** — Central City Park and culture-house park reference resolve to two different anchors.
4. **Map bounds** — source proxy is 2.4 × 2.4 km and all active base spawns remain inside bounds.
5. **A/B/C gameplay** — capture points still spawn from Museum/Stadium, Central Park and College anchors.
6. **Respawn** — both teams receive valid base spawns after map expansion.
7. **Road continuity** — central museum, college corridor, park approaches and peripheral vehicle routes remain traversable.
8. **Hydrography proxies** — Desna/floodplain and Oster-river proxies do not block player movement unless crossed by an authored bridge/shore collision later.
9. **Reference markers** — verified anchor markers appear at museum, college, both park references and secondary heritage points in source-only debug layout.
10. **Museum silhouette** — central upper volume, wings, gabled roof, porch/steps and facade window rhythm remain present.
11. **Stadium** — 105×68 pitch proxy, track/apron, fence, stand/service massing remain present.
12. **College** — 4-storey main block, entrance, window grid and campus B-grade massing remain present.
13. **Residential variation** — multiple blocks contain different house dimensions, sheds, porches/annexes and incomplete fences.
14. **S08 enterable house** — interactive test house still spawns and doors/windows/light/gate continue functioning.
15. **Vehicles/AI** — vehicle spawn and AI systems still receive a usable world; no old 1.8 km literal assumptions remain in active GameMode.
16. **Regression** — VERIFY_S04.py through VERIFY_S16A.py all pass from extracted archive.
