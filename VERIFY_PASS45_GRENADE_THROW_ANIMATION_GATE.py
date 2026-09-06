#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
CHARACTER_VISUAL = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCCharacterVisualComponent.cpp"
CHARACTER_VISUAL_HEADER = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCCharacterVisualComponent.h"
THROW_SOUND = ROOT / "OsterConflict" / "Content" / "R13" / "Audio" / "snd_throw1.uasset"
RUNTIME_GATE = ROOT / "VERIFY_PASS45_GRENADE_THROW_ANIMATION_RUNTIME.py"
GENERAL_EVIDENCE = ROOT / "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"
LAUNCHER = ROOT / "RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS45 GRENADE THROW ANIMATION GATE: FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


visual = read(CHARACTER_VISUAL)
visual_header = read(CHARACTER_VISUAL_HEADER)
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

# The repository already carries an authored throw sound. It must be consumed by the factual replicated throw
# event, not left orphaned or played from the input press before authoritative spawn succeeds.
req(THROW_SOUND.is_file() and THROW_SOUND.stat().st_size > 0,
    "tracked authored grenade throw sound is missing or empty")
for needle in (
    "/Game/R13/Audio/snd_throw1.snd_throw1",
    "PASS45_GRENADE_THROW_AUDIO_RUNTIME_READY",
    "PASS45_GRENADE_THROW_AUDIO_CONTENT_GAP",
    "UGameplayStatics::PlaySound2D",
    "UGameplayStatics::PlaySoundAtLocation",
    "replicated_event=1",
    "gameplay_authority=0",
):
    req(needle in visual, f"grenade throw authored-audio contract missing: {needle}")
req("TObjectPtr<USoundBase> GrenadeThrowSound" in visual_header,
    "character presentation component no longer retains the authored grenade throw sound")

# The dedicated runtime gate must demand factual authored animation evidence, not merely the cosmetic bridge.
for needle in (
    "PASS45_GRENADE_THROW_PRESENTATION_BRIDGE_READY",
    "PASS45_GRENADE_THROW_AUTHORED_ANIMATION_CONTENT_GAP",
    "PASS45_GRENADE_THROW_AUTHORED_ANIMATION_RUNTIME_READY",
    "PASS45_GRENADE_THROW_AUDIO_RUNTIME_READY",
    "PASS45_GRENADE_THROW_AUDIO_CONTENT_GAP",
    "authored first-person grenade hand/throw/recover animation is still a content gap",
):
    req(needle in runtime_gate, f"runtime grenade animation gate missing: {needle}")

# The generic final evidence path must enforce the same truth. Otherwise the dedicated animation gate can fail while
# the general PASS45 evidence file still prints PASS, creating two contradictory acceptance authorities.
req('require(gameplay, "PASS45_GRENADE_THROW_AUTHORED_ANIMATION_RUNTIME_READY"' in general_evidence,
    "general runtime evidence can pass without factual authored grenade throw animation readiness")
req('forbid(gameplay, "PASS45_GRENADE_THROW_AUTHORED_ANIMATION_CONTENT_GAP"' in general_evidence,
    "general runtime evidence can pass while the authored grenade throw animation content gap is logged")
req('require(gameplay, "PASS45_GRENADE_THROW_AUDIO_RUNTIME_READY"' in general_evidence,
    "general runtime evidence can pass without factual authored grenade throw audio readiness")
req('forbid(gameplay, "PASS45_GRENADE_THROW_AUDIO_CONTENT_GAP"' in general_evidence,
    "general runtime evidence can pass while authored grenade throw audio is missing")
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
print("- tracked authored throw audio is wired to the factual replicated throw presentation event")
print("- dedicated and general runtime evidence both require factual authored hand/throw/recover READY evidence")
print("- contradictory generic PASS while the grenade animation gate fails is no longer allowed")
print("STATUS: FAIL-HONEST SOURCE GATE; authored animation content and direct UE 5.8 visual acceptance remain pending")
