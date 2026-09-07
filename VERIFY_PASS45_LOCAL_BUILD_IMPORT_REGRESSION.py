#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
TACTICAL = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCTacticalMapVisual.cpp"
PICKUP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCPickupGunTruck.cpp"
STREET = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCPass45StreetInfrastructureSubsystem.cpp"
IMPORTER = ROOT / "OsterConflict" / "Scripts" / "import_production_vehicle_assets.py"
BATCH_RUNTIME = ROOT / "OsterConflict" / "Scripts" / "pass45_batch_runtime.py"
ASSET_WRAPPER = ROOT / "OsterConflict" / "IMPORT_ALL_LOCAL_INBOX_UE58.cmd"
ASSET_AUDIT = ROOT / "OsterConflict" / "Scripts" / "audit_local_model_inbox.ps1"
ASSET_PREPARE = ROOT / "OsterConflict" / "Scripts" / "prepare_all_local_inbox_assets.ps1"
LEDGER = ROOT / "OSTER_CONFLICT_WORK_LEDGER.md"

errors = []


def read(path: Path) -> str:
    if not path.is_file():
        errors.append(f"missing file: {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def require(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


tactical = read(TACTICAL)
pickup = read(PICKUP)
street = read(STREET)
importer = read(IMPORTER)
batch_runtime = read(BATCH_RUNTIME)
asset_wrapper = read(ASSET_WRAPPER)
asset_audit = read(ASSET_AUDIT)
asset_prepare = read(ASSET_PREPARE)
ledger = read(LEDGER)

# Local UE 5.8.1 / MSVC 14.51 factual build rejected the FVector2D table when it was constexpr.
require(
    "const FPass45ReferenceRoadSegment Pass45ReferenceRoads[]" in tactical,
    "Pass45 road table is not a normal const table",
)
require(
    "constexpr FPass45ReferenceRoadSegment Pass45ReferenceRoads[]" not in tactical,
    "UE 5.8 C2131 regression returned: FVector2D road table must not be constexpr",
)
require(
    "UE_ARRAY_COUNT(Pass45ReferenceRoads)" in tactical,
    "road-table count contract was lost while fixing C2131",
)

# UE 5.8 compile-time format validation requires UE_LOG format arguments to be TCHAR arrays.
require(
    'PASS45_HMMWV_M2_PRODUCTION_VISUAL_GAP exact_m2=0' in pickup,
    "HMMWV/M2 fail-closed log marker was lost while fixing UE_LOG formatting",
)
require(
    'Gun truck mounted-gun visual missing:' in pickup,
    "pickup fallback diagnostic was lost while fixing UE_LOG formatting",
)
require(
    '? TEXT("PASS45_HMMWV_M2_PRODUCTION_VISUAL_GAP' not in pickup,
    "UE 5.8 C2338 regression returned: UE_LOG format string must not be selected by ternary expression",
)

# UE 5.8 TActorIterator constructors take UWorld*, not a dereferenced UWorld&.
require(
    'TActorIterator<AStaticMeshActor> Existing(World)' in street,
    "street infrastructure static-mesh iterator does not receive UWorld*",
)
require(
    'TActorIterator<AOCWorldSectorOster> It(World)' in street,
    "street infrastructure sector iterator does not receive UWorld*",
)
require(
    'TActorIterator<AStaticMeshActor> Existing(*World)' not in street and
    'TActorIterator<AOCWorldSectorOster> It(*World)' not in street,
    "UE 5.8 iterator regression returned: TActorIterator receives dereferenced UWorld",
)

# Local UE 5.8 Interchange rejected the deprecated bAutoDetectMeshType property for GLB HMMWV/M2 intake.
require(
    'common_meshes.set_editor_property("convert_statics_with_animated_transform_to_skeletals", False)' in importer,
    "current UE 5.8 static-mesh animated-transform policy is missing",
)
require(
    'set_editor_property("auto_detect_mesh_type"' not in importer,
    "deprecated UE 5.8 auto_detect_mesh_type property returned",
)
require(
    'common_meshes.set_editor_property("force_all_mesh_as_type", force_mesh_type.IFMT_STATIC_MESH)' in importer,
    "HMMWV/M2 GLB intake no longer explicitly forces StaticMesh",
)
require(
    'mesh_pipeline.set_editor_property("import_skeletal_meshes", False)' in importer,
    "HMMWV/M2 GLB intake no longer explicitly disables skeletal import",
)

# Windows cmd.exe dispatch must keep call/path/args as separate argv elements.
require(
    'def cmd_batch(path: Path, *args: str) -> list[str]:' in batch_runtime,
    "PASS45 batch runtime is missing the argv-safe Windows batch helper",
)
require(
    'return [comspec, "/d", "/c", "call", str(path), *[str(arg) for arg in args]]' in batch_runtime,
    "PASS45 batch runtime no longer dispatches batch path/args as separate argv elements",
)
require(
    'f\'call "{path}"\'' not in batch_runtime,
    "regression returned: batch runtime embeds quoted command path in one cmd /c argument",
)
require(
    'build_cmd = cmd_batch(' in batch_runtime,
    "UE Build.bat is not routed through the argv-safe batch helper",
)
require(
    'cmd_batch(material_gate)' in batch_runtime,
    "strict material gate is not routed through the argv-safe batch helper",
)

# Native Windows PowerShell must not receive a quoted project directory ending in a backslash.
require(
    'set "PS_PROJECT_DIR=%~dp0."' in asset_wrapper,
    "asset intake is missing the dot-qualified PowerShell project directory",
)
require(
    '-ProjectDir "%PS_PROJECT_DIR%"' in asset_wrapper,
    "asset intake PowerShell calls do not use the safe project directory",
)
require(
    '-ProjectDir "%PROJECT_DIR%"' not in asset_wrapper,
    "PowerShell trailing-backslash quoting regression returned in asset intake",
)

# Windows PowerShell 5.1 reads UTF-8-without-BOM source as ANSI. Keep these two launcher scripts ASCII-only
# and preserve Ukrainian filename matching through .NET regex \uXXXX escapes instead of literal Cyrillic.
require(
    asset_audit.isascii(),
    "audit_local_model_inbox.ps1 contains non-ASCII source and can be mojibake-parsed by Windows PowerShell 5.1",
)
require(
    asset_prepare.isascii(),
    "prepare_all_local_inbox_assets.ps1 contains non-ASCII source and can be mojibake-parsed by Windows PowerShell 5.1",
)
require(
    "\\u0431\\u0443\\u0446\\u0435\\u0444" in asset_audit and "\\u0431\\u0443\\u0446\\u0435\\u0444" in asset_prepare,
    "ASCII-safe Ukrainian category regex escapes are missing",
)
require(
    "$inventory.status = if (" not in asset_audit,
    "PowerShell parser-sensitive inline if assignment returned in local asset audit",
)
require(
    "$inventory.status = 'UNSAFE_ARCHIVE_PRESENT'" in asset_audit and "$inventory.status = 'PASS'" in asset_audit,
    "local asset audit no longer has explicit PASS/unsafe status assignment",
)
require(
    "[System.IO.Path]::GetRelativePath" not in asset_prepare,
    "PowerShell 5.1/.NET Framework-incompatible Path.GetRelativePath returned in local inbox preparation",
)
require(
    "Substring($root.Length)" in asset_prepare,
    "PowerShell 5.1-safe relative package path calculation is missing",
)
require(
    "Expand-SafeZip -Archive $archive -Destination $stage" in asset_prepare,
    "local inbox safe ZIP helper is not invoked through its declared Destination parameter",
)

# Batch summaries must surface actionable failures instead of optional UE profiler/capture diagnostics.
require(
    'HARMLESS_UE_DIAGNOSTIC' in batch_runtime and 'aqProf\\.dll' in batch_runtime,
    "batch runtime no longer filters harmless UE profiler DLL diagnostics",
)
require(
    'WinPixGpuCapturer\\.dll' in batch_runtime and 'PixWinPlugin' in batch_runtime and 'RenderDocPlugin' in batch_runtime,
    "batch runtime no longer filters non-causal PIX/RenderDoc startup diagnostics",
)
require(
    'DECISIVE = re.compile(' in batch_runtime and '_collect_context(lines, DECISIVE' in batch_runtime,
    "batch runtime no longer prioritizes fail-closed/compiler/parser diagnostics",
)
require(
    'ParserError' in batch_runtime and 'index - 2' in batch_runtime,
    "batch runtime no longer emits parser-error context",
)
require(
    'normalize_windows_returncode' in batch_runtime and '0x100000000' in batch_runtime,
    "batch runtime no longer normalizes unsigned Windows -1 exit codes",
)

# Status must remain factual: source fix exists, but a later local build/import must verify it.
require(
    "LOCAL UE BUILD REJECTED" in ledger,
    "ledger does not preserve the factual 2026-08-25 local UE build rejection",
)
require(
    "C2131" in ledger and "auto_detect_mesh_type" in ledger,
    "ledger does not name both observed regressions",
)
require(
    "CODED_UNTESTED" in ledger,
    "ledger must not promote the new build/import fix to VERIFIED before a later local UE run",
)

if errors:
    print("PASS45 LOCAL BUILD/IMPORT REGRESSION: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 LOCAL BUILD/IMPORT REGRESSION: PASS")
print("- UE 5.8 FVector2D tactical-road table is no longer constexpr")
print("- UE_LOG format strings cannot regress to ternary TEXT selection")
print("- street TActorIterator calls keep UWorld* instead of invalid UWorld&")
print("- HMMWV/M2 Interchange intake uses the current UE 5.8 static-mesh policy")
print("- Windows batch dispatch cannot regress to embedded escaped command quotes")
print("- PowerShell paths, encoding, APIs and audit status remain Windows PowerShell 5.1-safe")
print("- batch summaries suppress profiler/PIX/RenderDoc noise and prioritize real failures")
print("- factual local build rejection remains recorded; fixes are CODED_UNTESTED until rerun")
