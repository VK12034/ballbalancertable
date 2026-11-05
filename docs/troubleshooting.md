# Troubleshooting

Work down from power, to motors, to sensing, to control. Most faults that
look like a control problem are a wiring problem.

## Nothing moves at all

**Check ENA first.** Pin 0 drives ENA on all three drivers and it is active
low. The driver is disabled when the pin is high. The firmware sets it high
in `setup()` and drops it low after a short delay, so if the pin never goes
low nothing will ever turn.

**Check the 5 V rail.** Measure at the Teensy's VIN pad. If the regulator was
never set, it could be anywhere. It wants 5.0 V.

**Check the Teensy is running.** Open the serial console. You should see
`ball balancer ready` and the help text on reset. Silence means the board is
not powered, or the 5 V trace was cut without wiring up an external supply.

## One motor runs the wrong way

Its coil pair is reversed. Set that leg's entry in `MOTOR_INVERT` in
`Config.h` to `true` and reflash. Swapping one coil pair at the driver does
the same thing physically.

Run `firmware/StepperTest` to confirm. All three cranks should rise together.

## Motors buzz but do not turn

Usually a coil pairing problem. The four motor wires form two pairs, and the
pairing matters even though the order within a pair only sets direction.
Measure with a meter: two wires reading a couple of ohms are one coil, and
read open against the other pair.

Also possible: the current limit is set too low. Check Vref.

## Motors get very hot

Current limit too high. Drop Vref. Around 0.9 V suits a 59 Ncm NEMA 17 at
roughly 0.9 A RMS.

Steppers run warm in normal use. Too hot to keep a finger on is too hot.

## Drivers get hot or fail

**Check VM and VIO before anything else.** The original write-up has these
reversed, and wiring VIO to 24 V will destroy a TMC2208. VM takes the 24 V,
VIO takes 3.3 V. See `electronics/wiring.md`.

Missing decoupling is the other common cause. Each driver needs its 100 uF
across VM and GND, close to the driver.

## The platform lifts crooked

One crank is clamped at a different angle from the others. There is no
homing, so each crank's angle is whatever it was when you clamped it.

Power down, push the platform fully flat and down, loosen the offending
leg's M4 x 20, set the crank to match the other two, retighten.

## The platform goes flat on its own, mid balance

The firmware has decided the ball is gone. It drops to level after
`BALL_LOST_MS`, which is 400 ms without a usable reading.

- Pressure reading below `TOUCH_PRESSURE_MIN`. A 1 inch steel ball is heavy
  enough, so this usually means a wiring or ground problem rather than a
  genuinely light touch.
- A loose panel wire that drops out when the platform tilts.
- The ribbon pulling tight at the extremes of travel. Give it slack.

Turn on `telemetry` and watch for gaps in the stream.

## The ball runs straight off

**An axis is reversed.** The platform tilts the wrong way and the error
grows instead of shrinking. Follow step 1 of `docs/calibration.md`.

**A leg is on the wrong corner.** Leg A's coupler must reach the ear nearest
the +X axis, with B and C anticlockwise from it. Wrong order makes the
platform tilt along an axis the firmware does not expect.

**`kp` far too high.** Drop it by a factor of five and see if the behaviour
changes character.

## The ball settles off centre

Normal before trimming. The panel's electrical centre is not the platform's
mechanical centre.

Use `offset <x> <y>` over serial to nudge it, then write the value you settle
on into `TOUCH_OFFSET_X` and `TOUCH_OFFSET_Y` in `Config.h`.

If no offset fixes it, check the platform is mechanically level first.

## Readings are noisy or jump about

- Grounds not common. Every ground has to meet: supply negative, both driver
  GND pins, the regulator, the Teensy.
- Panel ribbon run alongside motor wires. Motor cables are the noisiest
  things in the machine. Separate them.
- A cracked or bowed panel. Readings go strange near the edges.

## The platform judders when the ball is still

`kd` is too high and amplifying panel noise. Back it off.

If it persists at low `kd`, check `SPEED_MIN` and `ACCEL_MIN` in `Config.h`.
Those set the floor for the distance-proportional motion, and if the floor is
too high every tiny correction becomes a jolt.

## A correction just does not happen

The inverse kinematics refuse any pose that would drive a crank past its
stop. `commandPose()` returns without moving anything rather than damaging
the mechanism.

This shows up as the platform ignoring a large correction while handling
small ones. Either the tilt limit is set too close to the mechanism's 15
degree envelope, or the platform is not where the firmware thinks it is,
which comes back to the push-down-before-power-on step.

## Serial console shows nothing

115200 baud. The Teensy presents a USB serial port regardless of whether the
5 V trace is cut, so you can have a working console on a board with no power
to run the motors. `status` with no motor response is a clue the 5 V rail is
missing.
