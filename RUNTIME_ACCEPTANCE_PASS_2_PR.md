# PR summary

Second runtime correction pass from 2026-08-22 playtest evidence.

Root-cause fixes:
- remove menu geometry conflict that visibly shifts the startup layout;
- reassert Museum BASE placement at runtime for serialized spawn actors;
- defer dense grass population until same-world frontend transitions into gameplay;
- separate camera-based ballistic trace origin from local visible muzzle/tracer origin;
- gate normal gameplay on real HMMWV/M2/BTR production ingest instead of proxy acceptance.

Includes focused source-contract CI. UE 5.8 runtime acceptance remains pending.
