from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
PUBLIC = SRC / "Public"
PRIVATE = SRC / "Private"


def fail(message: str) -> None:
    raise SystemExit("R13 UHT HEADER SANITY VERIFY FAIL: " + message)


if not PUBLIC.is_dir() or not PRIVATE.is_dir():
    fail("missing OsterConflict Public/Private source directories")

checked = 0
for header in sorted(PUBLIC.glob("*.h")):
    text = header.read_text(encoding="utf-8", errors="replace")
    if "UCLASS" not in text and "USTRUCT" not in text and "UENUM" not in text:
        continue

    checked += 1
    includes = [line.strip() for line in text.splitlines() if line.strip().startswith("#include")]
    generated = [line for line in includes if ".generated.h\"" in line]

    if len(generated) != 1:
        fail(f"{header.name}: expected exactly one generated.h include, found {len(generated)}")
    if includes[-1] != generated[0]:
        fail(f"{header.name}: generated.h must remain the final include")
    if header.stem + ".generated.h" not in generated[0]:
        fail(f"{header.name}: generated include does not match header basename: {generated[0]}")

    if "UCLASS" in text and "GENERATED_BODY()" not in text:
        fail(f"{header.name}: UCLASS header lacks GENERATED_BODY()")

    # Catch a common copy/paste failure where a reflected class accidentally uses another class's API macro/body name.
    class_matches = re.findall(
        r'UCLASS\s*\([^)]*\)?\s*\n\s*class\s+OSTERCONFLICT_API\s+(\w+)', text, re.MULTILINE
    )
    if "UCLASS" in text and not class_matches:
        # UCLASS() with no arguments is the dominant form in this project; allow whitespace but still require exported class.
        class_matches = re.findall(
            r'UCLASS\s*\(\s*\)\s*\n\s*class\s+OSTERCONFLICT_API\s+(\w+)', text, re.MULTILINE
        )
    if "UCLASS" in text and not class_matches:
        fail(f"{header.name}: could not identify exported reflected class after UCLASS")

for cpp in sorted(PRIVATE.glob("*.cpp")):
    text = cpp.read_text(encoding="utf-8", errors="replace")
    if ".generated.h\"" in text:
        fail(f"{cpp.name}: generated.h must never be included from a cpp file")

if checked < 10:
    fail(f"suspiciously few reflected headers checked: {checked}")

print("R13 UHT HEADER SANITY VERIFY: PASS")
print(f"Checked {checked} reflected public headers for one matching final generated.h include, GENERATED_BODY and cpp hygiene.")
