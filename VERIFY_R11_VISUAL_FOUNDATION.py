from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parent
P = ROOT / 'OsterConflict'
SRC = P / 'Source' / 'OsterConflict'
checks = []

def req(cond, msg):
    if not cond:
        raise SystemExit('R11 VERIFY FAIL: ' + msg)
    checks.append(msg)
    print('PASS:', msg)

def read(path):
    return path.read_text(encoding='utf-8', errors='replace')

env_h = read(SRC/'Public/OCVisualEnvironment.h')
env_cpp = read(SRC/'Private/OCVisualEnvironment.cpp')
gm = read(SRC/'Private/OCGameMode.cpp')
world_h = read(SRC/'Public/OCWorldSectorOster.h')
world = read(SRC/'Private/OCWorldSectorOster.cpp')
weapon_h = read(SRC/'Public/OCWeaponBase.h')
weapon = read(SRC/'Private/OCWeaponBase.cpp')
fx_h = read(SRC/'Public/OCTransientVisualFX.h')
fx = read(SRC/'Private/OCTransientVisualFX.cpp')
char = read(SRC/'Private/OCCharacterVisualComponent.cpp')
vehicle = read(SRC/'Private/OCVehicleBase.cpp')
validation = read(ROOT/'PC_TEST/RUN_UE58_PC_VALIDATION.ps1')
preflight = read(P/'Scripts/S18C/WINDOWS_TOOLCHAIN_PREFLIGHT.ps1')
prelaunch = read(ROOT/'PC_TEST/PRELAUNCH_CHECK.ps1')
start = read(ROOT/'START_HERE.cmd')
quick = read(ROOT/'RUN_R13_LISTEN_TEST.cmd')

req('UDirectionalLightComponent' in env_h and 'USkyAtmosphereComponent' in env_h, 'runtime daylight rig declared')
req('bReplicates = true' in env_cpp and 'bAlwaysRelevant = true' in env_cpp, 'daylight rig reaches network clients')
req('SetAtmosphereSunLight(true)' in env_cpp and 'SetRealTimeCaptureEnabled(true)' in env_cpp, 'sun/sky runtime lighting configured')
old_depth = 'SetFogDensity(0.0085f)' in env_cpp and 'SetVolumetricFog(false)' in env_cpp
r13_neutral = 'SetFogDensity(0.0f)' in env_cpp and 'SetFogMaxOpacity(0.0f)' in env_cpp and 'SetVolumetricFog(false)' in env_cpp
req(old_depth or r13_neutral, 'atmospheric depth or R13 neutral no-fog art-QA mode configured')
req('SpawnActor<AOCVisualEnvironment>' in gm, 'GameMode spawns visual environment')
req(gm.find('SpawnActor<AOCVisualEnvironment>') < gm.find('SpawnActor<AOCWorldSectorOster>'), 'lighting spawns before source world')

req('virtual void BeginPlay() override;' in world_h, 'world has runtime material pass')
for token in ['Tint(Ground','Tint(Roads','Tint(Buildings','Tint(ResidentialRoofs','Tint(TreeCrowns','Tint(Waterways','Tint(StadiumGeometry']:
    req(token in world, f'world palette marker {token}')
req('ReferenceMarkers->SetVisibility(false, true)' in world, 'authoring reference markers hidden in gameplay')
req('Label->SetVisibility(false, true)' in world, 'authoring labels hidden in gameplay')

req('TObjectPtr<USceneComponent> WeaponRoot' in weapon_h, 'weapon has unscaled scene root for composite visuals')
req('BuildSourceOnlyWeaponVisual();' in weapon and 'RifleBarrel' in weapon and 'SniperScope' in weapon and 'PistolGrip' in weapon, 'recognizable composite weapon silhouettes implemented')
req('AOCTransientVisualFX' in weapon and 'ConfigureMuzzle' in weapon and 'ConfigureTracer' in weapon and 'ConfigureImpact' in weapon, 'combat FX use transient scene visuals')
req('DrawDebugLine(GetWorld(), TraceStart' not in weapon and 'DrawDebugPoint(GetWorld(), ImpactLocation' not in weapon, 'weapon fire/impact debug primitives removed')
req('SetLifeSpan' in fx and 'UPointLightComponent' in fx_h and 'BasicShapeMaterial' in fx, 'transient FX self-clean and use lit material geometry')

req('FPProxyArmL"), Cylinder' in char and 'FPProxyHandL"), Sphere' in char, 'first-person proxy arm fallback remains structurally defined')
primitive_arms_hidden = (
    'for (UStaticMeshComponent* Part : FirstPersonProxyParts)' in char and
    'if (Part) Part->SetVisibility(false, true);' in char and
    'Arms->SetVisibility(bHasProductionArms, true);' in char
)
req(primitive_arms_hidden, 'first-person primitive arm fallback is hidden while authored FPS arms are visibility-gated')
req('GetComponents<UStaticMeshComponent>(MeshComponents)' in vehicle and 'CivilianPalette' in vehicle and 'MilitaryBody' in vehicle, 'vehicle source proxies receive readable palettes')

req("$BuildBat=Resolve-Required" in validation and 'RunUBT.bat' not in validation, 'validation no longer requires missing RunUBT.bat')
req('[string[]]$ArgumentList' in validation and '& $Exe @ArgumentList' in validation and '[string[]]$Args' not in validation, 'native child-process arguments do not collide with PowerShell automatic $Args')
req("Launcher/installed UE 5.8 detected; source-only RunUBT.bat is not required." in prelaunch and "Engine\\Build\\BatchFiles\\Build.bat" in prelaunch, 'prelaunch accepts Launcher UE and requires Build.bat instead of RunUBT.bat')
req("$InstalledBuild = Test-Path" in validation and "Compile Dedicated Server' 'SKIP'" in validation, 'Launcher UE path is explicitly supported')
req("$BuildBat=Join-Path" in preflight and 'RunUBT.bat' not in preflight, 'toolchain preflight uses Build.bat on installed UE')
req('R13 CONTENT + GAMEPLAY PASS' in start and 'RUN_R13_LISTEN_TEST.cmd' in start and 'RUN_R11_LISTEN_TEST.cmd' not in start,
    'START_HERE exposes the current R13 listen-server gameplay path')
req('-Frontend' in quick and '-NoFrontend' not in quick and '?listen?Mode=Conquest' in quick and '-game' in quick and 'R13Gameplay=1' in quick,
    'quick launch enters the current player-facing R13 listen-server frontend')
req('CREATE_RELEASE_MAP.py' in quick and 'OsterConflict_Runtime.umap' in quick and 'UnrealEditor-Cmd.exe' in quick, 'fresh quick launch bootstraps generated runtime map')

# Local generated folders are expected after compiling. Only tracked generated artifacts violate the source archive contract.
try:
    tracked = subprocess.run(['git','ls-files'], cwd=ROOT, check=True, capture_output=True, text=True).stdout.splitlines()
except (OSError, subprocess.CalledProcessError) as exc:
    raise SystemExit('R11 VERIFY FAIL: unable to inspect tracked generated folders: ' + str(exc))
tracked = [path.replace('\\','/') for path in tracked]
for bad in ['Binaries','Intermediate','Saved','DerivedDataCache']:
    prefix=f'OsterConflict/{bad}/'
    req(not any(path.startswith(prefix) for path in tracked), f'archive does not track generated {bad}')

print(f'R11 VISUAL FOUNDATION verifier: PASS ({len(checks)} checks)')
