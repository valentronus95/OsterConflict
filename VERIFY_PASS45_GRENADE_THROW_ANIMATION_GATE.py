#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
CHARACTER_VISUAL = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCCharacterVisualComponent.cpp"
RUNTIME_GATE = ROOT / "VERIFY_PASS45_GRENADE_THROW_ANIMATION_RUNTIME.py"
GENERAL_EVIDENCE = ROOT / "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"
LAUNCHER = ROOT / "RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS45 GRENADE THROW ANIMATION GATE: FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


visual = read(CHARACTER_VISUAL)
runtime_gate = read(RUNTIME_GATE)
general_evidence = read(GENERAL_EVIDENCE)
launcher = read(LAUNCHER)
errors: list[str] = []


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


# The current native presentation layer must remain fail-honest until an exact authored sequence is supplied.
for needle in (
    "EOCCharacterActionEvent::GrenadeThrow",
    "PASS45_GRENADE_THROW_AUTHORED_ANIMATION_CONTENT_GAP",
    "native_authored_sequence=0",
    "blueprint_hook_dispatched=1",
    "second_gameplay_timer=0",
    "runtime_visual_acceptance=0",
):
    req(needle in visual, f"grenade throw source content-gap truth missing: {needle}")
req("PASS45_GRENADE_THROW_AUTHORED_ANIMATION_RUNTIME_READY" not in visual,
    "native source falsely emits runtime READY without an authored grenade throw sequence")

# The dedicated runtime gate must demand factual authored animation evidence, not merely the cosmetic bridge.
for needle in (
    "PASS45_GRENADE_THROW_PRESENTATION_BRIDGE_READY",
    "PASS45_GRENADE_THROW_AUTHORED_ANIMATION_CONTENT_GAP",
    "PASS45_GRENADE_THROW_AUTHORED_ANIMATION_RUNTIME_READY",
    "authored first-person grenade hand/throw/recover animation is still a content gap",
):
    req(needle in runtime_gate, f"runtime grenade animation gate missing: {needle}")

# The generic final evidence path must enforce the same truth. Otherwise the dedicated animation gate can fail while
# the general PASS45 evidence file still prints PASS, creating two contradictory acceptance authorities.
req('require(gameplay, "PASS45_GRENADE_THROW_AUTHORED_ANIMATION_RUNTIME_READY"' in general_evidence,
    "general runtime evidence can pass without factual authored grenade throw animation readiness")
req('forbid(gameplay, "PASS45_GRENADE_THROW_AUTHORED_ANIMATION_CONTENT_GAP"' in general_evidence,
    "general runtime evidence can pass while the authored grenade throw animation content gap is logged")
req('"GRENADE_AUTHORED_THROW_ANIMATION=PASS\\n"' in general_evidence,
    "general evidence output has no explicit authored grenade throw animation contract")

for needle in (
    "GRENADE_ANIM_VERIFY",
    "VERIFY_PASS45_GRENADE_THROW_ANIMATION_RUNTIME.py",
    "Verifying authored first-person grenade hand/throw/recover animation evidence",
    "GRENADE_ANIM_RC",
    "A presentation event bridge alone cannot satisfy item 24",
):
    req(needle in launcher, f"main runtime acceptance launcher lost grenade animation gate wiring: {needle}")

if errors:
    print("PASS45 GRENADE THROW ANIMATION GATE: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 GRENADE THROW ANIMATION GATE: PASS")
print("- current native grenade throw presentation is explicitly CONTENT GAP, not READY")
print("- no second gameplay timer was introduced")
print("- dedicated and general runtime evidence both require factual authored hand/throw/recover READY evidence")
print("- contradictory generic PASS while the grenade animation gate fails is no longer allowed")
print("STATUS: FAIL-HONEST SOURCE GATE; authored animation content and direct UE 5.8 visual acceptance remain pending")