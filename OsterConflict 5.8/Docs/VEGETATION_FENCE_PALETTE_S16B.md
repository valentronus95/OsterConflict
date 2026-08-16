# S16B — VEGETATION & FENCE PALETTE

## Радянський міський каркас
Українські матеріали про озеленення радянського періоду прямо згадують тополю, клен, березу, липу та сосну. У грі це базовий історично правдоподібний palette для старих парків, алей та громадських територій.

Proxy families:
- `SovietPoplar*` — високий вузький силует тополі;
- `TreeTrunks/TreeCrowns` — змішаний broadleaf proxy, у final content розділяється на липу/клен/каштан та ін.;
- `Birch*` — березова група;
- `Pine*` — сосна/хвойна група.

## Трава
- `GrassMown` — стадіон, доглянуті civic/park зони;
- `GrassRough` — узбіччя, приватний сектор, нерівномірно скошені ділянки;
- `GrassWetland` — заплава Десни/Остра, вологі високі трави.

Representative lawn/urban species for final material/foliage references: Poa pratensis, Festuca rubra, Lolium perenne; for ordinary park lawns and disturbed verges also Taraxacum officinale, Plantago major, Dactylis glomerata, Trifolium repens. Species are visual-reference guidance, not a botanical survey of every map cell.

## Паркани
- `WoodFences` — основний приватний street-facing family;
- `MetalFences` — суцільний метал;
- `LightSheetFences` — легкий листовий/профільний/шифероподібний вигляд;
- public/landmark fences remain in `Fences`.

Street-facing fences are generally 1.9–2.35 m in the source-only proxy pass and should block most direct views into private yards. Final art must add wear, repairs, gates, wicket doors, paint variation and material-specific audio/destruction.
