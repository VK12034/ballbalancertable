# Firmware

Two sketches for the Teensy 4.1.

| Sketch | What it does |
|---|---|
| `BallBalancer/` | the control loop, flash this to run the machine |
| `StepperTest/` | lifts all three cranks together, for checking motor direction |

Run `StepperTest` first. Sorting out a reversed motor is much easier before
the platform is on.

## Dependencies

| Component | Version used | Where from |
|---|---|---|
| arduino-cli | 1.4.1 | bundled with Arduino IDE 2, or <https://arduino.github.io/arduino-cli/> |
| Teensy core (`teensy:avr`) | 1.62.0 | <https://www.pjrc.com/teensy/package_teensy_index.json> |
| AccelStepper | 1.64.0 | Library Manager |
| TouchScreen | bundled with the Teensy core | no separate install needed |

The Teensy core ships its own TouchScreen library, so the Adafruit one the
source recommends is not required when building for a Teensy. If you install
it anyway, arduino-cli will pick the core's copy and say so. Both are the
same library.

## Building

With the Arduino IDE: add the PJRC board manager URL under Preferences,
install the Teensy boards, install AccelStepper, pick Teensy 4.1, open
`BallBalancer/BallBalancer.ino`, upload.

From the command line:

```bash
arduino-cli core update-index --additional-urls https://www.pjrc.com/teensy/package_teensy_index.json
```

```bash
arduino-cli core install teensy:avr@1.62.0 --additional-urls https://www.pjrc.com/teensy/package_teensy_index.json
```

```bash
arduino-cli lib install AccelStepper@1.64.0
```

```bash
arduino-cli compile --fqbn teensy:avr:teensy41 firmware/BallBalancer --warnings all
```

Both sketches compile with no warnings at `--warnings all`.

| Sketch | Flash | RAM1 variables |
|---|---|---|
| `BallBalancer` | 49,232 B | 10,432 B |
| `StepperTest` | 14,072 B | 6,144 B |

On Windows, keep the toolchain path short. Installing the Teensy core
somewhere deep makes GCC fail to find its own headers, with
`bits/c++config.h: No such file or directory`. That is a path length problem,
not a broken install.

## Layout

| File | Contents |
|---|---|
| `Config.h` | all the geometry, pins, gains and limits |
| `Kinematics.h/.cpp` | the 3RRS inverse kinematics |
| `Pid.h/.cpp` | one PID axis, with a filtered derivative and clamped integral |
| `Patterns.h/.cpp` | setpoint generators for the ball paths |
| `BallBalancer.ino` | setup, the control loop, the serial console |

`Config.h` is duplicated in `StepperTest/` because the Arduino build only
compiles files inside the sketch folder. If you change one, change both.

## The geometry has to match three places

`Config.h`, `cad/parameters.md` and `tools/verify_kinematics.py` all carry
the same link lengths and radii. They have to agree, or the firmware will
confidently aim the platform somewhere the mechanism cannot reach.

After changing any of them, run:

```bash
python tools/verify_kinematics.py
```

It checks the inverse kinematics against forward kinematics over the whole
envelope and exits non-zero if anything is off.

## Serial console

115200 baud. Type `help`.

| Command | Effect |
|---|---|
| `kp <v>`, `ki <v>`, `kd <v>` | set a gain on both axes |
| `offset <x> <y>` | trim the panel centre, in mm |
| `pattern <name>` | `hold`, `line`, `circle`, `ellipse`, `triangle`, `rectangle`, `figure8` |
| `level` | go flat and clear the PIDs |
| `telemetry` | toggle the data stream |
| `status` | print the current state |

Telemetry streams `x, y, errorX, errorY, tiltX, tiltY` as comma separated
values, which paste straight into a spreadsheet.

## Startup behaviour

There are no encoders and no endstops. Push the platform all the way down
before power on, until the cranks hit their lower stop, and hold it flat.
That position is taken as `CRANK_MIN_DEG` and everything else is counted from
it. The firmware then lifts to level on its own.

Get this wrong and every angle is off by however far out you started.

## A caution on the gains

`KP`, `KI` and `KD` have never run on hardware. They are starting values
reasoned from the geometry. See `docs/tuning.md` and `docs/validation.md`.
