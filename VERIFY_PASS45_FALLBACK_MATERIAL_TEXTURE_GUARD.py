#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SOURCE = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCRealWeaponFallbackSubsystem.cpp"

if not SOURCE.is_file():
    raise SystemExit(f"PASS45 FALLBACK MATERIAL TEXTURE GUARD FAIL: missing {SOURCE.relative_to(ROOT)}")

source = SOURCE.read_text(encoding="utf-8", errors="replace")
errors = []


def require(needle: str, label: str) -> None:
    if needle not in source:
        errors.append(f"{label}: missing {needle!r}")


for needle, label in (
    ('#include "Engine/Texture.h"', "texture type include"),
    ('bool IsPlaceholderTexture(const UTexture* Texture)', "placeholder texture predicate"),
    ('GetUsedTextures(', "runtime material dependency enumeration"),
    ('EMaterialQualityLevel::High', "material quality used for dependency enumeration"),
    ('ERHIFeatureLevel::SM5', "SM5 dependency enumeration"),
    ('UsedTextures.IsEmpty()', "zero-texture dependency rejection"),
    ('DefaultTexture', "default texture rejection"),
    ('WhiteSquareTexture', "white placeholder texture rejection"),
    ('Component->ComponentHasTag(ProductionVisualTag)', "production visual material audit"),
    ('Component->ComponentHasTag(RealFallbackComponentTag)', "real fallback material audit"),
    ('IsMissingOrDefaultMaterial(Component->GetMaterial(Slot))', "slot-level dependency audit"),
    ('PASS44_WEAPON_AUTHORED_MATERIAL_GAP', "fail-visible material gap marker"),
    ('PASS44_WEAPON_AUTHORED_MATERIAL_READY', "per-weapon ready marker"),
    ('PASS36_WEAPON_MATERIAL_AUDIT_READY', "rack material ready marker"),
    ('RackGapWeapons == 0', "rack READY gated by zero factual material gaps"),
):
    require(needle, label)

# A named material alone is not item-18 readiness. The texture-dependency test must happen inside the same
# predicate consumed by AuditAndRepairWeaponMaterials, before the rack READY marker can be emitted.
predicate_pos = source.find('bool IsMissingOrDefaultMaterial(const UMaterialInterface* Material)')
textures_pos = source.find('GetUsedTextures(', predicate_pos)
predicate_end = source.find('\n    }\n}', predicate_pos)
audit_pos = source.find('int32 UOCRealWeaponFallbackSubsystem::AuditAndRepairWeaponMaterials')
ready_pos = source.find('PASS36_WEAPON_MATERIAL_AUDIT_READY')
if not (predicate_pos >= 0 and textures_pos > predicate_pos and predicate_end > textures_pos):
    errors.append("GetUsedTextures must remain inside IsMissingOrDefaultMaterial")
if not (audit_pos > predicate_end and ready_pos > audit_pos):
    errors.append("material predicate/audit/READY ordering is inconsistent")

if errors:
    print("PASS45 FALLBACK MATERIAL TEXTURE GUARD: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 FALLBACK MATERIAL TEXTURE GUARD: PASS")
print("- production and explicit real-fallback visuals require non-placeholder authored materials")
print("- every accepted material must expose at least one non-placeholder SM5 render texture dependency")
print("- zero-texture/default/white-placeholder material state cannot emit rack MATERIAL_AUDIT_READY")
print("STATUS: SOURCE CONTRACT ONLY; UE 5.8 fresh-load/runtime visual acceptance remains required")
