# Tactical Map Production Polish Pass 2

Source branch: `feat/tactical-map-polish-pass-2`

This pass is intentionally limited to overview readability and tactical chrome. It does not change map geography, capture-point ownership, squad networking, input bindings, or world placement.

Implemented:
- semantic Lucide POI icons for museum, park, Silpo and stadium;
- edge-safe POI/objective placement;
- stronger 38/29 px objective backplates with contested-state emphasis;
- quieter overview treatment for very small residential footprints;
- softer grid alpha;
- compact 310 px legend panel with 42 px row rhythm;
- existing scale bar, player emphasis and LIVE/WORLD SYNC treatment retained.

Status remains `CODED_UNTESTED` until UE 5.8 runtime screenshot confirms spacing and readability.
