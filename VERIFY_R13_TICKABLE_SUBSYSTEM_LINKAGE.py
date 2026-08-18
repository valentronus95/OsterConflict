from pathlib import Path
import re
import runpy

ROOT = Path(__file__).resolve().parent
SOURCE = ROOT / "OsterConflict" / "Source" / "OsterConflict"
PUBLIC = SOURCE / "Public"
PRIVATE = SOURCE / "Private"


def fail(message: str) -> None:
    raise SystemExit(f"R13 TICKABLE SUBSYSTEM LINKAGE VERIFY FAIL: {message}")


uht_gate = ROOT / "VERIFY_R13_UHT_HEADER_SANITY.py"
if not uht_gate.is_file():
    fail("global UHT header sanity gate is missing")
runpy.run_path(str(uht_gate), run_name="__main__")

if not PUBLIC.is_dir() or not PRIVATE.is_dir():
    fail("missing OsterConflict Public/Private source directories")

checked = 0
for header in sorted(PUBLIC.glob("*.h")):
    text = header.read_text(encoding="utf-8", errors="replace")
    if "UTickableWorldSubsystem" not in text or "GetStatId() const override" not in text:
        continue

    class_match = re.search(r'class\s+OSTERCONFLICT_API\s+(\w+)\s*:\s*public\s+UTickableWorldSubsystem', text)
    if not class_match:
        fail(f"could not identify tickable subsystem class in {header.relative_to(ROOT)}")
    class_name = class_match.group(1)
    checked += 1

    inline_body = re.search(r'GetStatId\s*\(\s*\)\s*const\s+override\s*\{', text) is not None
    if inline_body:
        if "RETURN_QUICK_DECLARE_CYCLE_STAT" not in text:
            fail(f"{class_name}: inline GetStatId body lacks RETURN_QUICK_DECLARE_CYCLE_STAT")
        continue

    cpp = PRIVATE / f"{header.stem}.cpp"
    if not cpp.is_file():
        fail(f"{class_name}: declared GetStatId override has no matching cpp file")
    cpp_text = cpp.read_text(encoding="utf-8", errors="replace")
    signature = f"{class_name}::GetStatId() const"
    if signature not in cpp_text:
        fail(f"{class_name}: GetStatId override is declared but no definition was found in {cpp.relative_to(ROOT)}")

if checked == 0:
    fail("no tickable subsystem GetStatId declarations were inspected")

print("R13 TICKABLE SUBSYSTEM LINKAGE VERIFY: PASS")
print(f"Checked global reflected-header UHT hygiene plus {checked} tickable subsystem GetStatId override(s) for linkable definitions.")
