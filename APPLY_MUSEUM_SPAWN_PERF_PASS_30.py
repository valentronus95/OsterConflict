from pathlib import Path

ROOT = Path(__file__).resolve().parent
R137 = ROOT / "OsterConflict/Source/OsterConflict/Private/OCR137MuseumPhotoModelSubsystem.cpp"
R138 = ROOT / "OsterConflict/Source/OsterConflict/Private/OCR138MuseumInteractiveArchitectureSubsystem.cpp"
R143 = ROOT / "OsterConflict/Source/OsterConflict/Private/OCR143MuseumSiteVegetationSubsystem.cpp"
R145 = ROOT / "OsterConflict/Source/OsterConflict/Private/OCR145MuseumTreeLayoutSubsystem.cpp"
FOLIAGE = ROOT / "OsterConflict/Source/OsterConflict/Private/OCDenseGroundFoliageSubsystem.cpp"
LEDGER = ROOT / "OSTER_CONFLICT_WORK_LEDGER.md"


def patch(path: Path, old: str, new: str, label: str, count: int = 1):
    text = path.read_text(encoding="utf-8")
    found = text.count(old)
    if found < count:
        raise SystemExit(f"Pass 30 anchor missing for {label}: expected >= {count}, found {found}")
    text = text.replace(old, new, count)
    path.write_text(text, encoding="utf-8")


# The generic source landmark cleanup was too tight around a museum whose procedural layers extend well
# beyond the central body. Widen only the museum landmark-family cleanup, not general building deletion.
patch(R137,
      "constexpr float SourceMuseumCleanupRadiusCm = 3600.0f;",
      "constexpr float SourceMuseumCleanupRadiusCm = 5000.0f;",
      "museum source-shell cleanup radius")

# R13.8 creates dozens of individual static wall pieces. They are the authoritative collision shell but do
# not need each piece to cast a dynamic shadow in the LowCPU gameplay route.
patch(R138,
      "        Component->SetGenerateOverlapEvents(false);\n        Component->SetCanEverAffectNavigation(bCollision);\n        Component->ComponentTags.Add(TEXT(\"MuseumStructural\"));",
      "        Component->SetGenerateOverlapEvents(false);\n        Component->SetCanEverAffectNavigation(bCollision);\n        Component->SetCastShadow(false);\n        Component->ComponentTags.Add(TEXT(\"MuseumStructural\"));",
      "R138 structural shadow budget")

patch(R138,
      "        Component->SetGenerateOverlapEvents(false);\n        Component->SetCanEverAffectNavigation(bCollision);\n        Owner->AddInstanceComponent(Component);",
      "        Component->SetGenerateOverlapEvents(false);\n        Component->SetCanEverAffectNavigation(bCollision);\n        Component->SetCastShadow(false);\n        Component->SetCullDistances(0, 30000);\n        Owner->AddInstanceComponent(Component);",
      "R138 detail shadow/cull budget")

# The three provisional interior slabs are unsupported by interior references and were exactly the large pale
# blockers visible in the user's runtime screenshot. Keep the walkable floor, remove speculative partitions.
old_partitions = '''    AddSection(Architecture, Root, Cube, Interior, TEXT("InteriorPartitionLeft"),
        Museum + FVector(-480.0f, 80.0f, 225.0f), FVector(700.0f, 18.0f, 300.0f));
    AddSection(Architecture, Root, Cube, Interior, TEXT("InteriorPartitionRight"),
        Museum + FVector(480.0f, 80.0f, 225.0f), FVector(700.0f, 18.0f, 300.0f));
    AddSection(Architecture, Root, Cube, Interior, TEXT("InteriorPartitionHeader"),
        Museum + FVector(0.0f, 80.0f, 350.0f), FVector(260.0f, 18.0f, 80.0f));
'''
new_partitions = '''    // Pass 30: no speculative interior partitions. Runtime showed these pale slabs as false walls that
    // trapped/obscured the player. Interior room geometry must wait for actual interior references.
    UE_LOG(LogTemp, Display, TEXT("PASS30_MUSEUM_SPECULATIVE_INTERIOR_REMOVED"));
'''
patch(R138, old_partitions, new_partitions, "remove speculative R138 interior slabs")

# Museum-specific vegetation is decorative. Keep it close and cheap instead of rendering it across hundreds of metres.
patch(R143, "TEXT(\"R143Museum_GrassA\"), 38000);", "TEXT(\"R143Museum_GrassA\"), 15000);", "R143 grass A cull")
patch(R143, "TEXT(\"R143Museum_GrassB\"), 38000);", "TEXT(\"R143Museum_GrassB\"), 15000);", "R143 grass B cull")
patch(R143, "TEXT(\"R143Museum_GroundPlantA\"), 32000);", "TEXT(\"R143Museum_GroundPlantA\"), 12000);", "R143 plant A cull")
patch(R143, "TEXT(\"R143Museum_GroundPlantB\"), 32000);", "TEXT(\"R143Museum_GroundPlantB\"), 12000);", "R143 plant B cull")

# Fourteen detailed museum trees were all shadow-casting, nav-affecting and visible up to 1 km. Keep collision,
# but remove dynamic-nav/shadow cost and cap draw distance around the landmark.
patch(R145,
      "        Component->SetCanEverAffectNavigation(true);\n        Component->SetCastShadow(true);\n        Component->SetCullDistances(0, CullEndCm);",
      "        Component->SetCanEverAffectNavigation(false);\n        Component->SetCastShadow(false);\n        Component->SetCullDistances(0, FMath::Min(CullEndCm, 45000));",
      "R145 tree budget")

# The global grass pass was still processing thousands of traces/instances for ~30 seconds after deployment.
# Cut both construction work and persistent HISM density for the laptop LowCPU route.
patch(FOLIAGE, "constexpr float GridStep = 2200.0f;", "constexpr float GridStep = 4000.0f;", "dense foliage grid")
patch(FOLIAGE, "constexpr int32 CellsPerBatch = 12;", "constexpr int32 CellsPerBatch = 4;", "dense foliage batch")
patch(FOLIAGE, "FName(*FString::Printf(TEXT(\"DenseGrass_%d\"), Index)), 9000));",
      "FName(*FString::Printf(TEXT(\"DenseGrass_%d\"), Index)), 6000));", "dense grass cull")
patch(FOLIAGE, "GroundPlants = MakeFoliageHISM(Owner, Root, GroundPlantMesh, TEXT(\"DenseGroundPlants\"), 6500);",
      "GroundPlants = MakeFoliageHISM(Owner, Root, GroundPlantMesh, TEXT(\"DenseGroundPlants\"), 5000);", "dense plants cull")
patch(FOLIAGE, "Flowers = MakeFoliageHISM(Owner, Root, FlowerMesh, TEXT(\"DenseFlowers\"), 4500);",
      "Flowers = MakeFoliageHISM(Owner, Root, FlowerMesh, TEXT(\"DenseFlowers\"), 3500);", "dense flowers cull")
patch(FOLIAGE,
      "TEXT(\"PASS15_FOLIAGE_BUDGET_READY grid_cm=%.0f cells_per_batch=%d grass_cull_cm=9000\"),",
      "TEXT(\"PASS30_FOLIAGE_BUDGET_READY grid_cm=%.0f cells_per_batch=%d grass_cull_cm=6000\"),",
      "dense foliage runtime marker")

ledger = LEDGER.read_text(encoding="utf-8")
ledger += '''\n\n## 2026-08-23 — Pass 30 museum spawn / overlap / FPS correction\n\n- Runtime after Pass 29 reached gameplay, proving the reported START Slate crash no longer blocks this route.\n- New runtime evidence: BASE spawned the player inside/against museum geometry; movement was effectively blocked; FPS fell from ~45 to ~8; provisional pale interior slabs and distorted window-frame geometry were visible.\n- Root cause in source: canonical primary BASE was only 14.5 m from `MuseumAnchor`, while the authored museum footprint and vestibule occupy that same area. The museum guard also accepted any BASE pawn within 35 m, including the building interior.\n- Pass 30 moves primary BASE spawns to ~41 m exterior front-side positions and secondary bases farther out, adds a 30 m museum no-spawn exclusion radius, and recovers any BASE deployment that appears inside that radius.\n- Distorted stretched rural-cabin window-frame meshes are removed from museum windows in favor of lightweight clean frame geometry until a museum-specific authored frame exists.\n- Unsupported R13.8 interior partition slabs are removed. Generic landmark shell cleanup around the museum is widened to 50 m.\n- Museum structural/detail shadows, tree shadow/nav cost, museum vegetation cull ranges, and global dense-foliage density/batching are reduced for the current LowCPU runtime.\n- Status remains CODED_UNTESTED until the next UE 5.8 playtest confirms exterior spawn, movement, no duplicate shell/slabs and measured FPS.\n'''
LEDGER.write_text(ledger, encoding="utf-8")

print("PASS30 PATCH APPLIED")
