#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
VISUAL_COMPONENT = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCCharacterVisualComponent.cpp"
EVIDENCE = ROOT / "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"
STRICT_HARNESS = ROOT / "VERIFY_PASS45_STRICT_RUNTIME_ACCEPTANCE_HARNESS.py"
TZ = ROOT / "PASS45_RUNTIME_RECOVERY_TZ.md"

errors: list[str] = []


def read(path: Path) -> str:
    if not path.is_file():
        errors.append(f"missing file: {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


visual = read(VISUAL_COMPONENT)
evidence = read(EVIDENCE)
strict = read(STRICT_HARNESS)
tz = read(TZ)

# Current source truth: the replicated GrenadeThrow bridge exists, but no accepted native authored hand/pull/throw/
# recover animation has been wired. This marker must remain explicit until actual content replaces it.
req("EOCCharacterActionEvent::GrenadeThrow" in visual,
    "grenade throw presentation event is no longer handled by the character visual component")
req("PASS45_GRENADE_THROW_AUTHORED_ANIMATION_CONTENT_GAP" in visual,
    "missing authored grenade throw animation is no longer fail-visible")
req("native_authored_sequence=0" in visual and "runtime_visual_acceptance=0" in visual,
    "grenade authored-animation content gap no longer records factual native/runtime state")
req("second_gameplay_timer=0" in visual,
    "grenade cosmetic bridge can no longer prove that it owns no second gameplay timer")

# Final automated runtime evidence must not pass merely because the event bridge fired. It needs factual authored
# animation readiness and must reject the known gap marker. This intentionally makes current strict acceptance fail
# until real authored content is wired and emits the READY marker.
req('require(gameplay, "PASS45_GRENADE_THROW_AUTHORED_ANIMATION_RUNTIME_READY"' in evidence,
    "runtime evidence does not require factual authored grenade throw animation readiness")
req('forbid(gameplay, "PASS45_GRENADE_THROW_AUTHORED_ANIMATION_CONTENT_GAP"' in evidence,
    "runtime evidence can still pass while authored grenade throw animation is explicitly missing")
req('"GRENADE_AUTHORED_THROW_ANIMATION=PASS\\n"' in evidence,
    "successful evidence output has no explicit authored grenade throw animation contract")

# The existing strict-harness source guard must continue to validate the ordnance evidence contract. A dedicated
# grenade guard owns the exact animation-gap strings so the generic strict harness does not need to duplicate every
# current content-gap implementation detail.
req("PASS45_GRENADE_THROW_PRESENTATION_BRIDGE_READY" in strict,
    "strict harness no longer protects the grenade presentation bridge evidence route")
req("VISUAL_ACCEPTANCE=PENDING_MANUAL_OBSERVATION" in strict,
    "strict harness can falsely promote automated evidence to visual acceptance")

# Canonical acceptance remains visual, not just event-driven.
for needle in (
    "visible first-person throw presentation",
    "authored throw animation is visibly coherent in first person",
    "second gameplay timer",
):
    req(needle in tz, f"canonical grenade acceptance lost authored presentation requirement: {needle}")

if errors:
    print("PASS45 GRENADE AUTHORED THROW ACCEPTANCE: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 GRENADE AUTHORED THROW ACCEPTANCE: PASS")
print("- replicated GrenadeThrow bridge remains present but cannot impersonate authored animation")
print("- current missing native hand/pull/throw/recover sequence stays explicit as CONTENT GAP")
print("- final strict runtime evidence requires factual authored-animation READY and rejects the gap marker")
print("- cosmetic presentation remains forbidden from owning a second gameplay timer")
print("STATUS: FAIL-CLOSED SOURCE CONTRACT; authored grenade throw animation + local UE 5.8 visual acceptance remain pending")