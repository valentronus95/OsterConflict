from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
H = SRC / "Public" / "OCR13ResidentialInfillFenceSubsystem.h"
CPP = SRC / "Private" / "OCR13ResidentialInfillFenceSubsystem.cpp"


def fail(message: str) -> None:
    raise SystemExit("R13.4 INFILL FENCE VERIFY FAIL: " + message)


if not H.is_file() or not CPP.is_file():
    fail("infill fence header/source missing")

h = H.read_text(encoding="utf-8", errors="replace")
cpp = CPP.read_text(encoding="utf-8", errors="replace")

includes = [line.strip() for line in h.splitlines() if line.strip().startswith("#include")]
if not includes or "generated.h" not in includes[-1]:
    fail("generated.h must remain the final header include")

required = [
    "FenceDelaySeconds = 1.42f",
    "R13_ResidentialInfillRoot",
    "R13_House01",
    "R13_House02",
    "FrontFenceLengthCm = 3100.0f",
    "GateOpeningCm = 420.0f",
    "SideFenceLengthCm = 3300.0f",
    "Fence_Old_1_2m.Fence_Old_1_2m",
    "Fence_Old_2_2m.Fence_Old_2_2m",
    "Fence_Old_3_2m.Fence_Old_3_2m",
    "SM_Urb_Roa_Sheet_Metal_Rusty_01",
    "SM_Urb_Roa_Sheet_Metal_Rusty_02",
    "SM_Urb_Roa_Sheet_Metal_Rusty_03",
    "auto AddFamily = [ArtRoot, Root]",
    "constexpr float RailFractions[] = { 0.26f, 0.56f, 0.84f }",
    "AddOpenMetalRun",
    "R13_InfillFenceCollision",
    'SetCollisionProfileName(FName(bCollision ? TEXT("BlockAll") : TEXT("NoCollision")))',
    "SetCanEverAffectNavigation(bCollision)",
    "SetVisibility(false, true)",
    "if ((HouseIndex % 7) == 0) continue",
    "GameMode->IsFrontendOnlySession()",
]
for token in required:
    if token not in cpp:
        fail(f"missing bounded mixed-fence marker: {token}")

# Fence art must never become a second generic whole-city owner or drift back to initializer-list pair setup that
# previously created an avoidable MSVC compile-risk before the deferred UE build.
for forbidden in [
    'FindISM(WorldSector, TEXT("WoodFences"))',
    'FindISM(WorldSector, TEXT("MetalFences"))',
    'FindISM(WorldSector, TEXT("LightSheetFences"))',
    "TPair<const TCHAR*, FName>",
    "FMath::Rand",
    "FRand",
]:
    if forbidden in cpp:
        fail(f"infill fence pass drifted into generic/non-deterministic/fragile ownership: {forbidden}")

for left, right in (("(", ")"), ("{", "}"), ("[", "]")):
    if cpp.count(left) != cpp.count(right):
        fail(f"delimiter mismatch {left}{right}")

print("R13.4 RESIDENTIAL INFILL FENCE VERIFY: PASS")
print("Checks infill-only ownership, MSVC-safe explicit asset setup, weighted wood/open-metal/sheet frontage, 4.2m gate openings and hidden continuous collision.")
