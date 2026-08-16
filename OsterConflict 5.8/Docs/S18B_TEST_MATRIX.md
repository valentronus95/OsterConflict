# S18B Test Matrix

| ID | Test | PASS |
|---|---|---|
| B01 | UE_ROOT/toolchain check | required |
| B02 | Development Editor C++/UHT compile | required |
| B03 | Create `/Game/Maps/OsterConflict_Runtime.umap` | required |
| B04 | `OsterConflict.Release.*` automation suite | required |
| B05 | Development Client BuildCookRun | required |
| B06 | Development Server BuildCookRun | required |
| B07 | Client EXE + cooked container audit | required |
| B08 | Server EXE + cooked container audit | required |
| B09 | SHA-256 build manifest | required |
| B10 | Packaged dedicated server survives warm-up | required |
| B11 | 2 packaged clients connect + auto-deploy | required |
| B12 | No Fatal/Assertion marker in smoke logs | required |
| B13 | 8-population 30 min soak | RC gate |
| B14 | 16-population 30 min soak | RC gate |
| B15 | Settings persistence in packaged client | RC gate |
| B16 | Conquest 2 complete rounds | RC gate |
| B17 | Sandbox core interactions smoke | RC gate |
| B18 | Insights/network baseline archived | RC gate |

Source-only verifier can validate scripts/contracts, but **B01–B18 are not marked PASS until run on a real UE 5.8 Windows build machine**.
