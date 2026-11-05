# Validation status

What has actually been checked, and what has not. Nothing here has been built
in the physical world.

## Verified

### Kinematics

`tools/verify_kinematics.py`, run with Python 3.13.5, exits 0.

| Check | Result |
|---|---|
| Inverse then forward kinematics round trip, 401 points across the heave range | worst error 2.8e-14 mm |
| Home pose returns the home crank angle | +20.000000 deg, platform at 119.444 mm |
| Tilt envelope, 384 poses from 0 to 15 deg in every direction | 0 poses outside crank travel |

The same solver is implemented three times, in the Python checker, in the
Fusion build script and in `firmware/BallBalancer/Kinematics.cpp`. The Python
version is checked against forward kinematics, which is what gives any
confidence the other two are right.

This check earned its place. An earlier geometry parked the crank at 0
degrees with the platform level, and the envelope test found that a 10 degree
tilt drove one leg to -23 degrees, past its -20 degree stop. Home moved to
mid travel.

### CAD

Built by `cad/scripts/BallBalancer/BallBalancer.py` running inside Fusion
2706.1.17 through the Fusion MCP connector. 30 of 30 build steps pass with 0
errors and 0 warnings. The log is `cad/build_log.txt` and the structured
report is `cad/build_report.json`.

| Check | Result |
|---|---|
| Interference, all occurrences, platform fully down at crank -20 deg | 0 pairs |
| Interference, home at +20 deg | 0 pairs |
| Interference, fully up at +60 deg | 0 pairs |
| Running clearance, coupler to crank arm | 2.00 mm |
| Running clearance, crank hub to motor flange | 1.50 mm |
| NEMA 17 shaft engagement across the crank hub | 1.50 mm spare on a 24 mm shaft |
| Crank bore to motor shaft | 0.075 mm radial, a 0.15 mm diametral fit |
| Tie rod ball end engagement into the platform clevis | 5.66 mm into a 6 mm frame |

Interference analysis is Fusion's own, run across every occurrence at each
pose, not a visual check.

Two real problems were found and fixed this way. The first build had zero
running clearance at both rotating joints, with the crank hub flush against
the motor flange and the coupler flush against the crank arm. The second had
the tie rod stopping exactly at the platform's underside, so nothing sat
inside the clevis for the M3 x 35 to pass through.

### Exports

Every file in `cad/` was written by the build script and is non-empty.

| Format | Files |
|---|---|
| Fusion archive | `cad/source/BallBalancer.f3d` |
| STEP | `cad/step/BallBalancer.step` |
| STL | 7, one per printed part |
| DXF | 4 flat profiles |
| PNG | 5 viewport captures |

### Firmware

Compiled with arduino-cli 1.4.1 against teensy:avr 1.62.0, targeting
`teensy:avr:teensy41`, with `--warnings all`.

| Sketch | Flash | RAM1 variables | Warnings |
|---|---|---|---|
| `BallBalancer` | 49,232 B | 10,432 B | none |
| `StepperTest` | 14,072 B | 6,144 B | none |

Libraries: AccelStepper 1.64.0, and the TouchScreen library bundled with the
Teensy core.

The compile found a real bug: `Config.h` used `A0` through `A3` without
including `Arduino.h`, which the `.ino` gets implicitly but the `.cpp` files
do not.

## Not verified

**Nothing has been built.** No part has been printed, no board soldered, no
motor turned. Every statement about how the machine behaves is derived from
the model and the arithmetic.

Specifically untested:

- **The PID gains.** `KP`, `KI` and `KD` are starting values reasoned from
  the geometry. They have never balanced a ball. Follow `docs/tuning.md`.
- **Touch panel calibration.** The raw ADC limits in `Config.h` are
  placeholders. They must be measured per panel.
- **Panel dimensions.** 191 x 145 x 1.8 mm is typical for 8.4 inch 4:3, not a
  measured figure. Measure yours before printing the platform frame.
- **Printability.** The STLs have not been through a slicer. Nothing checks
  overhang angles, bridging or whether the base plate's flanges print
  cleanly.
- **STEP interchange.** The STEP file has not been opened in another CAD
  package.
- **Motor sizing.** No torque calculation was done. 59 Ncm is what the source
  specifies and it is assumed adequate.
- **Panel ribbon routing.** The CAD has no cable model, so clearance for the
  ribbon through the full range of travel is unchecked.
- **Thermal behaviour.** Driver and motor temperatures under continuous
  balancing are unknown.

## Known limitations

**No dimensioned drawings.** Fusion's drawing API does not expose enough to
generate dimensioned sheets from a script, so none were produced.
`cad/parameters.md` carries every driving dimension in a table and the DXF
profiles can be measured directly. If you want drawings, open the F3D and
make them by hand.

**The Fusion design is saved under an Education licence.** Exports from an
Education seat carry an educational use flag. That is fine for a personal
build and is worth knowing if you intended anything commercial, which the
CC BY-NC-SA licence rules out anyway.

**Parametric, with an asterisk.** Extrude depths, hole diameters and link
lengths are driven by named user parameters. In-plane sketch positions were
generated from the same values by the build script rather than by dimensional
constraints, so changing geometry means editing the parameter block and
rerunning the script. `cad/parameters.md` explains this.
