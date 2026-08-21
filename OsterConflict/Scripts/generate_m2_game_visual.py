"""Generate an external-only, game-visual M2-style heavy machine gun mesh as GLB.

This is an authored visual asset for Oster Conflict. It intentionally omits internal
mechanisms and manufacturing-grade dimensions. +X is the weapon/barrel forward axis.
"""

from __future__ import annotations

import json
import math
import struct
from pathlib import Path


METAL = (48, 52, 52, 255)
METAL_2 = (63, 66, 64, 255)
EDGE = (32, 34, 34, 255)
BLACK = (18, 19, 19, 255)
GRIP = (58, 49, 39, 255)
STEEL = (80, 84, 82, 255)


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

    def add_box(self, extents, center, color=METAL):
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

    @staticmethod
    def _basis(axis):
        ax, ay, az = axis
        length = math.sqrt(ax*ax + ay*ay + az*az)
        ax, ay, az = ax/length, ay/length, az/length
        hx, hy, hz = (0.0, 0.0, 1.0) if abs(az) < 0.85 else (0.0, 1.0, 0.0)
        ux = ay*hz - az*hy
        uy = az*hx - ax*hz
        uz = ax*hy - ay*hx
        ul = math.sqrt(ux*ux + uy*uy + uz*uz)
        ux, uy, uz = ux/ul, uy/ul, uz/ul
        vx = ay*uz - az*uy
        vy = az*ux - ax*uz
        vz = ax*uy - ay*ux
        return (ax, ay, az), (ux, uy, uz), (vx, vy, vz)

    def add_cylinder(self, length, radius, center, axis=(1,0,0), color=METAL_2, sections=16):
        a, u, v = self._basis(axis)
        half = length * 0.5
        cx, cy, cz = center
        verts = []
        for end in (-half, half):
            for i in range(sections):
                ang = 2.0 * math.pi * i / sections
                radial = tuple(radius * (math.cos(ang)*u[j] + math.sin(ang)*v[j]) for j in range(3))
                verts.append((
                    cx + end*a[0] + radial[0],
                    cy + end*a[1] + radial[1],
                    cz + end*a[2] + radial[2],
                ))
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

    # Receiver, feed area, and external cover.
    m.add_box((52,18,20), (-19,0,0), METAL)
    m.add_box((45,15,7), (-17,0,-12), EDGE)
    m.add_box((43,19.5,5), (-17,0,12.5), METAL_2)
    m.add_box((30,5,3), (-18,0,16.3), EDGE)
    m.add_box((13,5,8), (-2,-11.5,3), METAL_2)
    m.add_box((13,5,8), (-2,11.5,3), METAL_2)
    m.add_box((4,19,22), (-47,0,0), EDGE)

    # Heavy barrel silhouette.
    m.add_cylinder(66,6.2,(38,0,2),(1,0,0),METAL_2,20)
    m.add_cylinder(7,7.7,(6,0,2),(1,0,0),EDGE,20)
    m.add_cylinder(6,7.2,(70,0,2),(1,0,0),EDGE,20)
    m.add_cylinder(58,2.8,(99,0,2),(1,0,0),STEEL,18)
    m.add_cylinder(13,4.4,(131,0,2),(1,0,0),EDGE,18)
    m.add_cylinder(7,3.5,(140.5,0,2),(1,0,0),BLACK,18)

    # Dark recessed-looking barrel-jacket vents. These are purely presentation detail.
    for x in (16,25,34,43,52,61):
        for ang in tuple(2*math.pi*i/6 for i in range(6)):
            y = 6.0 * math.cos(ang)
            z = 2.0 + 6.0 * math.sin(ang)
            axis = (0.0, math.cos(ang), math.sin(ang))
            m.add_cylinder(0.45,1.25,(x,y,z),axis,BLACK,10)

    # Charging handle and exterior controls.
    m.add_cylinder(25,0.9,(-25,11.5,2),(1,0,0),EDGE,10)
    m.add_cylinder(6,1.1,(-35,14,2),(0,1,0),METAL_2,10)
    m.add_cylinder(7,1.8,(-35,18.5,2),(0,1,0),GRIP,12)

    # Spade grips / rear presentation.
    m.add_cylinder(8,1.6,(-52,0,3),(1,0,0),EDGE,12)
    for side in (-1,1):
        y = side * 8
        m.add_cylinder(13,1.4,(-55,y,3),(0,0,1),EDGE,12)
        m.add_cylinder(11,2.1,(-57,y,12),(0,0,1),GRIP,12)
        m.add_box((8,4,3),(-53,y,18),EDGE)
        m.add_box((5,1.6,6),(-51,side*3.0,7),METAL_2)

    # Sights.
    m.add_box((8,5,4),(-32,0,17),EDGE)
    m.add_box((2.5,1.8,10),(-31,0,23),METAL_2)
    m.add_cylinder(1.6,2.4,(-31,0,29),(0,1,0),BLACK,12)
    m.add_box((5,4,4),(68,0,9),EDGE)
    m.add_box((2,1.5,8),(69,0,14),METAL_2)

    # Vehicle pintle attachment only. No tripod/internal weapon mechanism.
    m.add_box((14,10,7),(-8,0,-17),EDGE)
    m.add_cylinder(10,4.0,(-8,0,-24),(0,0,1),BLACK,16)

    # Receiver presentation details.
    for x in (-38,-29,-20,-11,0):
        for side in (-1,1):
            m.add_cylinder(0.7,1.1,(x,side*9.25,2),(0,side,0),STEEL,8)
    m.add_box((8,4,2.5),(-36,0,16.5),EDGE)
    m.add_cylinder(7,0.9,(-36,0,17.5),(0,1,0),STEEL,8)
    m.add_box((18,0.8,8),(-22,9.3,-2),BLACK)
    m.add_box((18,0.8,8),(-22,-9.3,-2),BLACK)
    return m


def _pad4(data: bytes, pad=b"\x00") -> bytes:
    return data + pad * ((4 - len(data) % 4) % 4)


def build_m2_glb(output_path):
    mesh = build_visual_mesh()

    pos_bytes = b"".join(struct.pack("<fff", *p) for p in mesh.positions)
    color_bytes = bytes(channel for rgba in mesh.colors for channel in rgba)
    index_bytes = b"".join(struct.pack("<I", i) for i in mesh.indices)

    pos_offset = 0
    color_offset = len(pos_bytes)
    index_offset = color_offset + len(color_bytes)
    binary = _pad4(pos_bytes + color_bytes + index_bytes)

    xs = [p[0] for p in mesh.positions]
    ys = [p[1] for p in mesh.positions]
    zs = [p[2] for p in mesh.positions]

    document = {
        "asset": {
            "version": "2.0",
            "generator": "Oster Conflict authored game-visual M2 generator",
            "extras": {
                "purpose": "external game visual only",
                "manufacturing_accuracy": False,
            },
        },
        "scene": 0,
        "scenes": [{"nodes": [0]}],
        "nodes": [{"mesh": 0, "name": "M2_Browning_GameVisual"}],
        "meshes": [{
            "name": "M2_Browning_GameVisual",
            "primitives": [{
                "attributes": {"POSITION": 0, "COLOR_0": 1},
                "indices": 2,
                "mode": 4,
            }],
        }],
        "buffers": [{"byteLength": len(binary)}],
        "bufferViews": [
            {"buffer": 0, "byteOffset": pos_offset, "byteLength": len(pos_bytes), "target": 34962},
            {"buffer": 0, "byteOffset": color_offset, "byteLength": len(color_bytes), "target": 34962},
            {"buffer": 0, "byteOffset": index_offset, "byteLength": len(index_bytes), "target": 34963},
        ],
        "accessors": [
            {
                "bufferView": 0, "byteOffset": 0, "componentType": 5126,
                "count": len(mesh.positions), "type": "VEC3",
                "min": [min(xs), min(ys), min(zs)],
                "max": [max(xs), max(ys), max(zs)],
            },
            {
                "bufferView": 1, "byteOffset": 0, "componentType": 5121,
                "normalized": True, "count": len(mesh.colors), "type": "VEC4",
            },
            {
                "bufferView": 2, "byteOffset": 0, "componentType": 5125,
                "count": len(mesh.indices), "type": "SCALAR",
            },
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
    destination = Path(__file__).with_name("m2_browning_oc_authored.glb")
    build_m2_glb(destination)
    print(destination)
