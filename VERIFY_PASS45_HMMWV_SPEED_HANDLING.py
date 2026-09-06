from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"

hmmwv_h = (SRC / "Public" / "OCHMMWVGunTruck.h").read_text(encoding="utf-8", errors="replace")
speed_cpp = (SRC / "Private" / "OCVehicleSpeedRuntimeSubsystem.cpp").read_text(encoding="utf-8", errors="replace")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit("PASS45 HMMWV SPEED/HANDLING VERIFY FAIL: " + message)
    print("PASS:", message)


def value(pattern: str, text: str, label: str) -> float:
    match = re.search(pattern, text)
    require(match is not None, f"{label} is explicitly configured")
    return float(match.group(1))


require("class OSTERCONFLICT_API AOCHMMWVGunTruck : public AOCPickupGunTruck" in hmmwv_h,
        "HMMWV remains a distinct pickup-derived gameplay identity")

max_speed = value(r"MaxForwardSpeedKmh\s*=\s*([0-9.]+)f", hmmwv_h, "HMMWV declared max speed")
steering_torque = value(r"SteeringTorque\s*=\s*([0-9.]+)f", hmmwv_h, "HMMWV steering torque")
lateral_grip = value(r"LateralGrip\s*=\s*([0-9.]+)f", hmmwv_h, "HMMWV lateral grip")

require(max_speed >= 80.0, "HMMWV declared gameplay speed is >= 80 km/h")
require(max_speed <= 118.0, "HMMWV declared gameplay speed stays below the old pickup-like 120 km/h contract")
require(steering_torque <= 70000000.0, "HMMWV steering torque is reduced for high-speed stability")
require(lateral_grip >= 9500.0, "HMMWV lateral grip is increased for high-speed stability")

require('#include "OCHMMWVGunTruck.h"' in speed_cpp,
        "runtime speed subsystem knows the dedicated HMMWV class")
require("Cast<AOCHMMWVGunTruck>(Vehicle)" in speed_cpp,
        "runtime speed subsystem separates HMMWV from ordinary pickup")

hmmwv_runtime = re.search(
    r"ApplySpeedContract\(\*HMMWV,\s*([0-9.]+)f,\s*([0-9.]+)f\)", speed_cpp)
require(hmmwv_runtime is not None, "HMMWV has an explicit runtime speed/assist contract")
runtime_target = float(hmmwv_runtime.group(1))
assist = float(hmmwv_runtime.group(2))
require(runtime_target >= 80.0, "HMMWV runtime target is >= 80 km/h")
require(abs(runtime_target - max_speed) < 0.01, "HMMWV declared and runtime speed targets match")
require(assist > 0.0 and assist < 550.0, "HMMWV assist is positive but softer than ordinary pickup")

require("ApplySpeedContract(*Vehicle, 120.0f, 550.0f);" in speed_cpp,
        "ordinary pickup retains its 120 km/h contract")
require("ApplySpeedContract(*Vehicle, 90.0f, 320.0f);" in speed_cpp,
        "BTR retains its 90 km/h contract")

print(
    "PASS45 HMMWV SPEED/HANDLING VERIFY PASS "
    f"target={runtime_target:.0f}km/h steering={steering_torque:.0f} grip={lateral_grip:.0f} assist={assist:.0f}"
)
