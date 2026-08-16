import unreal

MAP_PATH = "/Game/Maps/OsterConflict_Runtime"

unreal.log("[S18B] Creating deterministic project-owned release map: " + MAP_PATH)
world = unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
if not world:
    raise RuntimeError("Unable to create blank map")

# Gameplay content is spawned by AOCGameMode at runtime in the current source-only milestone.
# The important S18B step is owning the .umap inside /Game so cook/package no longer depends on /Engine/Maps/Entry.
if not unreal.EditorLoadingAndSavingUtils.save_map(world, MAP_PATH):
    raise RuntimeError("Unable to save release map: " + MAP_PATH)

unreal.log("[S18B] RELEASE_MAP_READY " + MAP_PATH)
