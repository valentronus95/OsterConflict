from pathlib import Path

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
quick = read(ROOT/'RUN_R11_LISTEN_TEST.cmd')

req('UDirectionalLightComponent' in env_h and 'USkyAtmosphereComponent' in env_h, 'runtime daylight rig declared')
req('bReplicates = true' in env_cpp and 'bAlwaysRelevant = true' in env_cpp, 'daylight rig reaches network clients')
# Pass 14 keeps the fixed daytime atmosphere but stops continuous SkyLight recapture. The sun does not
# animate during a match, so realtime cubemap capture is render cost without a visual benefit.
req('SetAtmosphereSunLight(true)' in env_cpp and 'SetRealTimeCaptureEnabled(false)' in env_cpp and
    'SetDynamicShadowCascades(4)' in env_cpp and 'SetDynamicShadowDistanceMovableLight(18000.0f)' in env_cpp,
    'sun/sky runtime lighting configured with fixed-daylight performance budget')
# Current art-QA direction intentionally removes height-fog colour grading. Atmospheric depth now comes
# from the explicit Rayleigh/Mie SkyAtmosphere coefficients, while fog density/opacity stay at zero.
req('SetRayleighScatteringScale(1.0f)' in env_cpp and 'SetMieScatteringScale(1.0f)' in env_cpp and
    'SetFogDensity(0.0f)' in env_cpp and 'SetFogMaxOpacity(0.0f)' in env_cpp and
    'SetVolumetricFog(false)' in env_cpp, 'neutral sky atmosphere configured without legacy haze')
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

req('FPProxyArmL"), Cylinder' in char and 'FPProxyHandL"), Sphere' in char, 'first-person proxy arms/hands no longer rectangular blocks')
req('GetComponents<UStaticMeshComponent>(MeshComponents)' in vehicle and 'CivilianPalette' in vehicle and 'MilitaryBody' in vehicle, 'vehicle source proxies receive readable palettes')

req("$BuildBat=Resolve-Required" in validation and 'RunUBT.bat' not in validation, 'validation no longer requires missing RunUBT.bat')
req('[string[]]$ArgumentList' in validation and '& $Exe @ArgumentList' in validation and '[string[]]$Args' not in validation, 'native child-process arguments do not collide with PowerShell automatic $Args')
req("Launcher/installed UE 5.8 detected; source-only RunUBT.bat is not required." in prelaunch and "Engine\\Build\\BatchFiles\\Build.bat" in prelaunch, 'prelaunch accepts Launcher UE and requires Build.bat instead of RunUBT.bat')
req("$InstalledBuild = Test-Path" in validation and "Compile Dedicated Server' 'SKIP'" in validation, 'Launcher UE path is explicitly supported')
req("$BuildBat=Join-Path" in preflight and 'RunUBT.bat' not in preflight, 'toolchain preflight uses Build.bat on installed UE')
# Current single-launcher contract: normal play is R14 current gameplay, while option 2 is the
# Pass 14/29 playflow+performance wrapper. Focused Pass 15 and landmark Pass 21 stay internal.
req('RUN_R14_CURRENT_GAMEPLAY.cmd' in start and 'RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd' in start and
    'ЗВИЧАЙНА ГРА' in start and 'ПОВНИЙ RUNTIME-ТЕСТ' in start and '-d3d11' in start and
    'RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd' not in start and
    'RUN_R21_LANDMARK_OWNERSHIP_RUNTIME_ACCEPTANCE.cmd' not in start and
    'RUN_R14_MAIN_SANDBOX_TEST.cmd' not in start,
    'START_HERE exposes canonical normal/full-test routes on the safe D3D11 renderer')
req('-NoFrontend' in quick and '?listen?Mode=Conquest' in quick and '-game' in quick, 'quick launch enters visible listen-server gameplay directly')
req('CREATE_RELEASE_MAP.py' in quick and 'OsterConflict_Runtime.umap' in quick and 'UnrealEditor-Cmd.exe' in quick, 'fresh R11 quick launch bootstraps generated runtime map')

for bad in ['Binaries','Intermediate','Saved','DerivedDataCache']:
    req(not (P/bad).exists(), f'archive excludes generated {bad}')

print(f'R11 VISUAL FOUNDATION verifier: PASS ({len(checks)} checks)')
