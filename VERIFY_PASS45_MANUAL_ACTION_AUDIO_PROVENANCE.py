#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
PROVENANCE = ROOT / "PASS45_MANUAL_ACTION_AUDIO_PROVENANCE.md"
INTAKE = ROOT / "PASS45_MANUAL_ACTION_AUDIO_INTAKE.py"
GITATTRIBUTES = ROOT / ".gitattributes"
AUDIO_CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWeaponAudioComponent.cpp"
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


provenance = read(PROVENANCE)
intake = read(INTAKE)
gitattributes = read(GITATTRIBUTES)
audio_cpp = read(AUDIO_CPP)
tz = read(TZ)

# Pin exact public provenance and identity limits. These are candidate source contracts, not proof that bytes
# have been acquired or that UE can load/audibly present them.
for needle in (
    "https://freesound.org/people/C-V/sounds/523401/",
    "Lever action cocking.wav",
    "Creative Commons 0 (CC0)",
    "0.958 s",
    "**not** evidence of an exact Stein/Marlin/Model-1894 recording",
    "https://freesound.org/people/rammbostein/sounds/263459/",
    "Mosin Nagant Bolt.wav",
    "6.500 s",
    "**not** an M700 recording",
    "https://opengameart.org/content/the-free-firearm-sound-library",
    "beb2f4041f3d6740fa0aeaf0e71159bd65a78c1b",
    "Folder names such as Model 1894 / Savage 10 / Mosin Nagant are **not sufficient evidence**",
):
    req(needle in provenance, f"manual-action audio provenance contract missing: {needle}")

# Audit/write intake must pin both exact LQ preview URLs and exact transport hashes. A dynamically selected
# HQ/LQ variant is not the same audited byte stream and must never silently replace the pinned donor.
for needle in (
    '"expected_transport_url": "https://cdn.freesound.org/previews/523/523401_9-lq.mp3"',
    '"expected_transport_sha256": "7785b4db5b512cec45da227097789dab4510aafec1f7e5d9f260669f54ed75ab"',
    '"expected_transport_url": "https://cdn.freesound.org/previews/263/263459_3988807-lq.mp3"',
    '"expected_transport_sha256": "635a4fd88454a032a476445237befb536ab532c1bdf573249653011bff4dde9e"',
    'return expected_url, "freesound_public_preview_pinned"',
    'pinned public preview is no longer advertised by source page',
    'write mode forbidden without pinned transport URL',
    'transport URL drift',
):
    req(needle in intake, f"manual-action intake transport pin missing: {needle}")
req('return source_urls[0], "freesound_public_preview"' not in intake,
    "manual-action intake can still silently select a different Freesound preview variant")
req('candidates.sort(key=lambda u: ("-hq." not in u.lower()' not in intake,
    "manual-action intake still prefers a dynamic HQ preview over the audited LQ transport")
for needle in (
    "https://cdn.freesound.org/previews/523/523401_9-lq.mp3",
    "7785b4db5b512cec45da227097789dab4510aafec1f7e5d9f260669f54ed75ab",
    "https://cdn.freesound.org/previews/263/263459_3988807-lq.mp3",
    "635a4fd88454a032a476445237befb536ab532c1bdf573249653011bff4dde9e",
    "exact audited LQ transport URLs and SHA-256 values",
):
    req(needle in provenance, f"manual-action provenance lost pinned transport truth: {needle}")

# Repository policy already routes WAV through LFS. The intake contract must not encourage ordinary Git blobs
# for source WAV data just because the files are small.
req("*.wav filter=lfs" in gitattributes,
    "repository no longer protects WAV payloads with Git LFS")
req("Do not bypass LFS" in provenance,
    "manual-action provenance no longer forbids bypassing the repository WAV/LFS policy")

# No candidate is allowed to become a silent source-only promotion. Until accepted bytes are actually present
# and imported, the runtime fallback profile must keep bolt and lever unassigned/fail-visible.
req("RepositoryFallbackProfile->PumpCycle.Add(Pump);" in audio_cpp,
    "tracked PumpCycle fallback disappeared while item 16 is still open")
for forbidden in (
    "RepositoryFallbackProfile->BoltCycle.Add",
    "RepositoryFallbackProfile->LeverCycle.Add",
):
    req(forbidden not in audio_cpp,
        f"unaccepted manual-action donor was wired into runtime prematurely: {forbidden}")

for needle in (
    "Current repository-owned BoltCycle: **CONTENT GAP**",
    "Current repository-owned LeverCycle: **CONTENT GAP**",
    "No URL, title, tag, filename or folder name by itself counts as runtime content.",
    "Item 16 remains unchecked",
    "PR #94 remains OPEN / UNMERGED",
):
    req(needle in provenance, f"manual-action audio fail-closed rule missing: {needle}")

# Canonical TZ must remain honest about current runtime and the still-open item. This verifier protects source
# provenance only and cannot upgrade a runtime-dependent checklist item.
req("RUNTIME REJECTED 2026-08-31" in tz,
    "canonical Pass45 TZ lost current factual runtime rejection")
req("Replace procedural manual-action fallback cues" in tz,
    "canonical item 16 text is no longer discoverable")
req("real bolt/pump/lever sound content" in tz,
    "canonical item 16 lost the real manual-action audio requirement")

if errors:
    print("PASS45 MANUAL-ACTION AUDIO PROVENANCE: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 MANUAL-ACTION AUDIO PROVENANCE: PASS")
print("- real CC0 lever-action and bolt-action donor sources are pinned with identity limits")
print("- audited LQ preview URLs and transport hashes are both fail-closed; HQ/LQ drift cannot silently replace bytes")
print("- WAV/LFS policy is protected; source URLs cannot impersonate committed runtime content")
print("- PumpCycle remains tracked while BoltCycle/LeverCycle remain explicit content gaps")
print("- item 16 stays open until payload import, authored moving-part animation and UE 5.8 acceptance")
print("STATUS: SOURCE PROVENANCE VERIFIED; AUDIO BYTES / UE IMPORT / RUNTIME ACCEPTANCE PENDING")
