"""Generate an external-only BTR-4-inspired 8x8 game visual as GLB.

This mesh exists to remove the green/proxy runtime shell when the user's local FBX is
not available in the worktree. It is deliberately a presentation model, not an
engineering/manufacturing reference.

Internal modeling coordinates are +X forward, +Y lateral, +Z up. glTF is Y-up, so the
exported root carries an explicit -90 degree X rotation that maps internal +Z to glTF
+Y while preserving +X as vehicle forward. Unreal may then perform its normal glTF
coordinate conversion without leaving the BTR lying on its side.

Pass45 material rule: the generated fallback must carry an explicit authored glTF
PBR material. Vertex colors are presentation data, not a substitute for a material
slot contract. Unreal must never have to invent DefaultMaterial for this asset.
"""

from __future__ import annotations

import json
import math
import struct
from pathlib import Path

ARMOR = (63, 78, 64, 255)
ARMOR_DARK = (48, 61, 51, 255)
ARMOR_LIGHT = (79, 94, 77, 255)
BLACK = (20, 22, 21, 255)
RUBBER = (25, 26, 25, 255)
STEEL = (86, 90, 84, 255)
GLASS = (54, 66, 65, 255)

BTR4_MATERIAL_NAME = "M_BTR4_OC_Authored"
# Quaternion [x, y, z, w]. Internal mesh math is Z-up; glTF is Y-up.
# -90 degrees around X maps internal +Z -> glTF +Y and preserves +X forward.
BTR4_Z_UP_TO_GLTF_Y_UP_ROTATION = [-0.7071067811865476, 0.0, 0.0, 0.7071067811865476]


class MeshBuilder:
    def __init__(self):
        self.positions: list[tuple[float, float, float]] = []
        self.colors: list[tuple[int, int, int, int]] = []
        self.indices: list[int] = []

    def _add_vertices(self, verts, color):
        base = len(self.positions)
        self.positions.extend(tuple(map(float, v)) for v in verts)
        self.colors.extend([color] * len(verts))
        return base

    def add_box(self, extents, center, color=ARMOR):
        ex, ey, ez = (v * 0.5 for v in extents)
        cx, cy, cz = center
        verts = [
            (cx-ex, cy-ey, cz-ez), (cx+ex, cy-ey, cz-ez),
            (cx+ex, cy+ey, cz-ez), (cx-ex, cy+ey, cz-ez),
            (cx-ex, cy-ey, cz+ez), (cx+ex, cy-ey, cz+ez),
            (cx+ex, cy+ey, cz+ez), (cx-ex, cy+ey, cz+ez),
        ]
        faces = [
            (0,2,1),(0,3,2), (4,5,6),(4,6,7),
            (0,1,5),(0,5,4), (1,2,6),(1,6,5),
            (2,3,7),(2,7,6), (3,0,4),(3,4,7),
        ]
        base = self._add_vertices(verts, color)
        for tri in faces:
            self.indices.extend(base + i for i in tri)

    def add_wedge_x(self, length, width, low_z, high_z, center_x, center_y, base_z, color=ARMOR):
        """Simple sloped prism. Nose points toward +X."""
        hx = length * 0.5
        hy = width * 0.5
        x0, x1 = center_x - hx, center_x + hx
        y0, y1 = center_y - hy, center_y + hy
        z0 = base_z
        verts = [
            (x0,y0,z0), (x1,y0,z0), (x1,y1,z0), (x0,y1,z0),
            (x0,y0,z0+high_z), (x1,y0,z0+low_z),
            (x1,y1,z0+low_z), (x0,y1,z0+high_z),
        ]
        faces = [
            (0,2,1),(0,3,2), (4,5,6),(4,6,7),
            (0,1,5),(0,5,4), (1,2,6),(1,6,5),
            (2,3,7),(2,7,6), (3,0,4),(3,4,7),
        ]
        base = self._add_vertices(verts, color)
        for tri in faces:
            self.indices.extend(base + i for i in tri)

    @staticmethod
    def _basis(axis):
        ax, ay, az = axis
        length = math.sqrt(ax*ax + ay*ay + az*az)
        ax, ay, az = ax/length, ay/length, az/length
        helper = (0.0, 0.0, 1.0) if abs(az) < 0.85 else (0.0, 1.0, 0.0)
        hx, hy, hz = helper
        ux = ay*hz - az*hy
        uy = az*hx - ax*hz
        uz = ax*hy - ay*hx
        ul = math.sqrt(ux*ux + uy*uy + uz*uz)
        ux, uy, uz = ux/ul, uy/ul, uz/ul
        vx = ay*uz - az*uy
        vy = az*ux - ax*uz
        vz = ax*uy - ay*ux
        return (ax, ay, az), (ux, uy, uz), (vx, vy, vz)

    def add_cylinder(self, length, radius, center, axis=(1,0,0), color=STEEL, sections=18):
        a, u, v = self._basis(axis)
        half = length * 0.5
        cx, cy, cz = center
        verts = []
        for end in (-half, half):
            for i in range(sections):
                ang = 2.0 * math.pi * i / sections
                radial = tuple(radius * (math.cos(ang)*u[j] + math.sin(ang)*v[j]) for j in range(3))
                verts.append((cx + end*a[0] + radial[0], cx*0 + cy + end*a[1] + radial[1], cz + end*a[2] + radial[2]))
        start_center = len(verts)
        verts.append((cx-half*a[0], cy-half*a[1], cz-half*a[2]))
        end_center = len(verts)
        verts.append((cx+half*a[0], cy+half*a[1], cz+half*a[2]))
        base = self._add_vertices(verts, color)
        for i in range(sections):
            n = (i + 1) % sections
            a0, a1 = base+i, base+n
            b0, b1 = base+sections+i, base+sections+n
            self.indices += [a0,b0,b1, a0,b1,a1]
            self.indices += [base+start_center, a1, a0]
            self.indices += [base+end_center, b0, b1]


def build_visual_mesh():
    m = MeshBuilder()

    # Lower hull and characteristic sloped front.
    m.add_box((520, 232, 92), (-28, 0, 22), ARMOR_DARK)
    m.add_wedge_x(150, 224, 54, 100, 292, 0, 8, ARMOR)
    m.add_box((360, 220, 90), (-62, 0, 104), ARMOR)
    m.add_wedge_x(118, 218, 50, 82, 170, 0, 87, ARMOR_LIGHT)
    m.add_box((100, 214, 74), (-280, 0, 84), ARMOR_DARK)

    # Roof blocks and external side armor details.
    m.add_box((230, 196, 20), (-55, 0, 160), ARMOR_LIGHT)
    m.add_box((84, 190, 16), (125, 0, 151), ARMOR)
    for side in (-1, 1):
        y = side * 116
        m.add_box((410, 10, 45), (-65, y, 78), ARMOR_DARK)
        m.add_box((112, 8, 38), (214, y, 70), ARMOR)
        for x in (-205, -70, 70, 205):
            m.add_box((82, 9, 15), (x, y, 17), ARMOR_LIGHT)

    # Eight wheels. Internal cylinder axis Y is the wheel axle.
    axle_x = (-205, -70, 70, 205)
    for x in axle_x:
        for side in (-1, 1):
            y = side * 128
            m.add_cylinder(30, 43, (x, y, -26), (0,1,0), RUBBER, 20)
            m.add_cylinder(33, 21, (x, y, -26), (0,1,0), STEEL, 16)

    m.add_cylinder(38, 54, (-35, 0, 194), (0,0,1), ARMOR_DARK, 20)
    m.add_box((84, 92, 44), (-28, 0, 216), ARMOR)
    m.add_wedge_x(54, 88, 26, 38, 38, 0, 199, ARMOR_LIGHT)
    m.add_cylinder(168, 5.2, (93, 0, 220), (1,0,0), BLACK, 18)
    m.add_cylinder(25, 8.0, (181, 0, 220), (1,0,0), STEEL, 18)
    m.add_box((78, 18, 18), (54, -36, 206), ARMOR_DARK)

    m.add_box((62, 72, 5), (96, -48, 172), ARMOR_DARK)
    m.add_box((62, 72, 5), (96, 48, 172), ARMOR_DARK)
    m.add_box((46, 48, 7), (-170, 0, 174), ARMOR_DARK)
    m.add_box((22, 54, 13), (217, -58, 137), GLASS)
    m.add_box((22, 54, 13), (217, 58, 137), GLASS)
    m.add_box((8, 52, 31), (328, -82, 55), BLACK)
    m.add_box((8, 52, 31), (328, 82, 55), BLACK)
    m.add_box((7, 74, 30), (-332, 0, 76), ARMOR_LIGHT)
    m.add_box((10, 8, 24), (305, -104, 76), BLACK)
    m.add_box((10, 8, 24), (305, 104, 76), BLACK)

    m.add_box((22, 232, 16), (342, 0, 3), ARMOR_DARK)
    m.add_box((18, 228, 16), (-338, 0, 8), ARMOR_DARK)
    m.add_cylinder(110, 1.6, (-95, -68, 240), (0,0,1), BLACK, 10)
    m.add_cylinder(96, 1.4, (-120, 65, 232), (0,0,1), BLACK, 10)

    return m


def _pad4(data: bytes, pad=b"\x00") -> bytes:
    return data + pad * ((4 - len(data) % 4) % 4)


def build_btr4_glb(output_path):
    mesh = build_visual_mesh()
    pos_bytes = b"".join(struct.pack("<fff", *p) for p in mesh.positions)
    color_bytes = bytes(channel for rgba in mesh.colors for channel in rgba)
    index_bytes = b"".join(struct.pack("<I", i) for i in mesh.indices)

    color_offset = len(pos_bytes)
    index_offset = color_offset + len(color_bytes)
    binary = _pad4(pos_bytes + color_bytes + index_bytes)

    xs = [p[0] for p in mesh.positions]
    ys = [p[1] for p in mesh.positions]
    zs = [p[2] for p in mesh.positions]
    document = {
        "asset": {
            "version": "2.0",
            "generator": "Oster Conflict authored BTR-4 external game visual",
            "extras": {
                "purpose": "external game visual only",
                "engineering_accuracy": False,
                "pass45_authored_material_contract": True,
                "internal_axis_contract": "+X forward, +Z up",
                "gltf_axis_contract": "+X forward, +Y up",
            },
        },
        "scene": 0,
        "scenes": [{"nodes": [0]}],
        "nodes": [{
            "mesh": 0,
            "name": "BTR4_Bucephalus_GameVisual",
            "rotation": BTR4_Z_UP_TO_GLTF_Y_UP_ROTATION,
        }],
        "materials": [{
            "name": BTR4_MATERIAL_NAME,
            "doubleSided": False,
            "pbrMetallicRoughness": {
                "baseColorFactor": [1.0, 1.0, 1.0, 1.0],
                "metallicFactor": 0.18,
                "roughnessFactor": 0.72,
            },
        }],
        "meshes": [{"name": "BTR4_Bucephalus_GameVisual", "primitives": [{
            "attributes": {"POSITION": 0, "COLOR_0": 1},
            "indices": 2,
            "material": 0,
            "mode": 4,
        }]}],
        "buffers": [{"byteLength": len(binary)}],
        "bufferViews": [
            {"buffer": 0, "byteOffset": 0, "byteLength": len(pos_bytes), "target": 34962},
            {"buffer": 0, "byteOffset": color_offset, "byteLength": len(color_bytes), "target": 34962},
            {"buffer": 0, "byteOffset": index_offset, "byteLength": len(index_bytes), "target": 34963},
        ],
        "accessors": [
            {"bufferView": 0, "byteOffset": 0, "componentType": 5126, "count": len(mesh.positions),
             "type": "VEC3", "min": [min(xs), min(ys), min(zs)], "max": [max(xs), max(ys), max(zs)]},
            {"bufferView": 1, "byteOffset": 0, "componentType": 5121, "normalized": True,
             "count": len(mesh.colors), "type": "VEC4"},
            {"bufferView": 2, "byteOffset": 0, "componentType": 5125, "count": len(mesh.indices), "type": "SCALAR"},
        ],
    }

    json_chunk = _pad4(json.dumps(document, separators=(",", ":")).encode("utf-8"), b" ")
    bin_chunk = _pad4(binary)
    total = 12 + 8 + len(json_chunk) + 8 + len(bin_chunk)
    glb = (
        struct.pack("<III", 0x46546C67, 2, total)
        + struct.pack("<II", len(json_chunk), 0x4E4F534A) + json_chunk
        + struct.pack("<II", len(bin_chunk), 0x004E4942) + bin_chunk
    )

    output = Path(output_path)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(glb)
    return output


if __name__ == "__main__":
    destination = Path(__file__).with_name("btr4_bucephalus_oc_authored.glb")
    build_btr4_glb(destination)
    print(destination)
