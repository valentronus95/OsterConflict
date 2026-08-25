#!/usr/bin/env python3
"""Source-level Pass45 gate for the repository-safe BTR-4 fallback material contract."""

from __future__ import annotations

import importlib.util
import json
import struct
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent
GENERATOR = ROOT / "OsterConflict" / "Scripts" / "generate_btr4_game_visual.py"
IMPORTER = ROOT / "OsterConflict" / "Scripts" / "import_btr4_production_asset.py"
VEHICLE_IMPORTER = ROOT / "OsterConflict" / "Scripts" / "import_production_vehicle_assets.py"
BTR_SOURCE = ROOT / "OsterConflict" / "SourceAssets" / "Production" / "Vehicles" / "BTR4"


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 BTR4 AUTHORED MATERIAL: FAIL: {message}")


def read(path: Path) -> str:
    if not path.is_file():
        fail(f"missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def load_generator():
    spec = importlib.util.spec_from_file_location("oc_btr4_generator", GENERATOR)
    if spec is None or spec.loader is None:
        fail("cannot load BTR-4 generator")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def read_glb_json(path: Path) -> dict:
    data = path.read_bytes()
    if len(data) < 20:
        fail("generated GLB is too small")
    magic, version, total_length = struct.unpack_from("<III", data, 0)
    if magic != 0x46546C67 or version != 2 or total_length != len(data):
        fail("generated GLB header is invalid")

    offset = 12
    while offset + 8 <= len(data):
        chunk_length, chunk_type = struct.unpack_from("<II", data, offset)
        offset += 8
        payload = data[offset:offset + chunk_length]
        offset += chunk_length
        if chunk_type == 0x4E4F534A:
            return json.loads(payload.decode("utf-8").rstrip("\x00 "))
    fail("generated GLB has no JSON chunk")


generator_text = read(GENERATOR)
importer_text = read(IMPORTER)
vehicle_importer_text = read(VEHICLE_IMPORTER)

for needle in (
    'BTR4_MATERIAL_NAME = "M_BTR4_OC_Authored"',
    '"pass45_authored_material_contract": True',
    '"materials": [{',
    '"material": 0',
    '"metallicFactor": 0.18',
    '"roughnessFactor": 0.72',
):
    if needle not in generator_text:
        fail(f"generator contract missing {needle!r}")

for needle in (
    'build_btr4_glb(GENERATED_SOURCE)',
    'import_glb_combined(GENERATED_SOURCE, DESTINATION, ASSET_NAME)',
    'source_kind = "authored_external_visual"',
):
    if needle not in importer_text:
        fail(f"BTR fallback import path missing {needle!r}")

for needle in (
    'options.set_editor_property("import_materials", True)',
    'options.set_editor_property("import_textures", True)',
):
    if needle not in vehicle_importer_text:
        fail(f"local FBX authored material import contract missing {needle!r}")

if not (BTR_SOURCE / "SOURCE_METADATA.txt").is_file():
    fail("BTR source metadata is missing")

# The repository intentionally does not ship the user-selected FBX/texture payload unless its
# redistribution status is verified. When absent, the generated fallback is therefore authoritative.
local_fbx = BTR_SOURCE / "BTR4_Bucephalus.fbx"
if local_fbx.exists():
    print("- local BTR4 FBX exists; importer must use its authored material/texture path")
else:
    print("- local BTR4 FBX absent; repository-safe authored GLB fallback is the active source path")

module = load_generator()
with tempfile.TemporaryDirectory() as temp_dir:
    output = Path(temp_dir) / "btr4_pass45.glb"
    module.build_btr4_glb(output)
    document = read_glb_json(output)

materials = document.get("materials") or []
if len(materials) != 1:
    fail(f"expected exactly one authored fallback material, found {len(materials)}")
material = materials[0]
if material.get("name") != "M_BTR4_OC_Authored":
    fail(f"unexpected authored material name: {material.get('name')!r}")
pbr = material.get("pbrMetallicRoughness") or {}
if pbr.get("baseColorFactor") != [1.0, 1.0, 1.0, 1.0]:
    fail("neutral PBR baseColorFactor is missing; vertex palette would not be preserved")
if pbr.get("metallicFactor") is None or pbr.get("roughnessFactor") is None:
    fail("explicit PBR metallic/roughness values are missing")

meshes = document.get("meshes") or []
if len(meshes) != 1:
    fail(f"expected one combined BTR mesh, found {len(meshes)}")
primitives = meshes[0].get("primitives") or []
if len(primitives) != 1:
    fail(f"expected one combined BTR primitive, found {len(primitives)}")
primitive = primitives[0]
if primitive.get("material") != 0:
    fail("BTR primitive is not bound to authored material slot 0")
attributes = primitive.get("attributes") or {}
if "COLOR_0" not in attributes:
    fail("BTR fallback lost its authored vertex-color palette")

extras = (document.get("asset") or {}).get("extras") or {}
if extras.get("pass45_authored_material_contract") is not True:
    fail("Pass45 authored material provenance marker missing from generated GLB")

print("PASS45 BTR4 AUTHORED MATERIAL: PASS")
print("- repository-safe BTR-4 fallback GLB carries an explicit authored PBR material")
print("- primitive slot 0 is bound to that material and retains COLOR_0 presentation data")
print("- local FBX path still imports authored materials/textures when the licensed payload is present")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 import/runtime material validation remains authoritative")
