from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
PUBLIC = SRC / "Public"
PRIVATE = SRC / "Private"


def fail(message: str) -> None:
    raise SystemExit("R13 UHT HEADER SANITY VERIFY FAIL: " + message)


def uclass_macro_end(text: str, start: int, header_name: str) -> int:
    """Return the first character after a UCLASS(...) invocation.

    UCLASS metadata can contain nested parentheses, e.g.
    UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent)). A flat regex that stops at the first ')'
    mis-parses valid Unreal headers, so walk balanced parentheses and quoted strings instead.
    """
    pos = start
    while pos < len(text) and text[pos].isspace():
        pos += 1

    if pos >= len(text) or text[pos] != "(":
        return pos

    depth = 0
    quote = None
    escaped = False
    while pos < len(text):
        ch = text[pos]
        if quote is not None:
            if escaped:
                escaped = False
            elif ch == "\\":
                escaped = True
            elif ch == quote:
                quote = None
        else:
            if ch in ('"', "'"):
                quote = ch
            elif ch == "(":
                depth += 1
            elif ch == ")":
                depth -= 1
                if depth == 0:
                    return pos + 1
                if depth < 0:
                    break
        pos += 1

    fail(f"{header_name}: unbalanced UCLASS metadata parentheses")
    return pos


def exported_uclass_names(text: str, header_name: str) -> list[str]:
    names: list[str] = []
    for macro in re.finditer(r"\bUCLASS\b", text):
        declaration_start = uclass_macro_end(text, macro.end(), header_name)
        declaration = re.match(
            r"\s*class\s+OSTERCONFLICT_API\s+([A-Za-z_]\w*)\b",
            text[declaration_start:],
            re.MULTILINE,
        )
        if not declaration:
            fail(f"{header_name}: could not identify exported reflected class after UCLASS")
        names.append(declaration.group(1))
    return names


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

    # Catch copy/paste failures in reflected exported classes without mis-parsing nested UCLASS metadata.
    if "UCLASS" in text:
        exported_uclass_names(text, header.name)

for cpp in sorted(PRIVATE.glob("*.cpp")):
    text = cpp.read_text(encoding="utf-8", errors="replace")
    if ".generated.h\"" in text:
        fail(f"{cpp.name}: generated.h must never be included from a cpp file")

if checked < 10:
    fail(f"suspiciously few reflected headers checked: {checked}")

print("R13 UHT HEADER SANITY VERIFY: PASS")
print(f"Checked {checked} reflected public headers for one matching final generated.h include, GENERATED_BODY and cpp hygiene.")
