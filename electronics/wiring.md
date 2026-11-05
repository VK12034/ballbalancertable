# Wiring

Three TMC2208 drivers, one Teensy 4.1, one 24 V supply. Everything shares a
single ground.

## Read this before you power anything

The Instructables write-up says to connect each driver's **VIO** pin to the
positive terminal of the 24 V supply and its **VM** pin to the Teensy's 3.3 V
rail. On a TMC2208 StepStick those two pins are the other way round. VM is the
motor supply and takes 24 V. VIO is the logic reference and takes 3.3 V.
Wiring it as written puts 24 V into a 3.3 V logic pin and will destroy the
driver, and quite possibly the Teensy with it.

This repository wires them per the TMC2208 datasheet:

| Driver pin | Goes to | Why |
|---|---|---|
| VM (VMOT) | +24 V | motor supply, 4.75 V to 36 V |
| GND (both) | supply negative | one of these is the motor ground, one the logic ground, tie both |
| VIO | Teensy 3.3 V | logic reference, sets the step/dir threshold |

If your breakout silkscreen disagrees, trust the silkscreen on your own board
and check it against the datasheet before applying power. Boards from
different sellers do vary.

## Power

| Item | Value |
|---|---|
| Supply | 24 V DC, 4 A or better |
| Per motor, running | about 0.9 A peak, far less when holding still |
| Teensy 4.1 | 5 V, under 150 mA |
| Driver logic | 3.3 V from the Teensy, a few mA each |

A bench supply set to 24 V is the easiest option while you tune. 24 V at 4 A
leaves plenty of headroom for three motors that rarely move together at full
speed.

### Getting 5 V for the Teensy

A buck regulator steps the 24 V down. Set it before connecting the Teensy:
wire 24 V to IN+ and GND, turn the trim pot until a meter across VO+ and GND
reads 5.0 V, then power down and connect it.

Feed that 5 V to the Teensy's VIN pad.

### Cut the Teensy 5 V trace

The Teensy 4.1 has a small trace on the underside joining USB 5 V to the
board's 5 V rail. Cut it. Otherwise your bench supply and your laptop's USB
port end up connected to each other through the Teensy, and one of them will
win. The pads either side of the cut can be bridged with solder later if you
ever want to go back to USB power.

With the trace cut you can still plug USB in for the serial console while the
bench supply runs the board.

## Pin assignments

Taken from the source project, which keeps the drivers on consecutive pins.

| Teensy 4.1 pin | Connects to | Note |
|---|---|---|
| 0 | ENA on all three drivers | active low, one pin enables everything |
| 1 | STEP, driver A | |
| 2 | DIR, driver A | |
| 3 | STEP, driver B | |
| 4 | DIR, driver B | |
| 5 | STEP, driver C | |
| 6 | DIR, driver C | |
| 3.3 V | VIO and MS1 and MS2 on all drivers | logic rail and 1/16 microstepping |
| A0 | touch panel X+ | |
| A1 | touch panel Y+ | |
| A2 | touch panel X- | |
| A3 | touch panel Y- | |
| GND | common ground | |

Driver A drives leg A, which sits on the +X axis. Legs B and C follow
anticlockwise at 120 degrees. Mark the motors as you wire them. Getting them
in the wrong order makes the platform tilt the wrong way and is tedious to
find later.

## Microstepping

Tie MS1 and MS2 high, to 3.3 V, on every driver. That selects 1/16 on the
TMC2208, so a 1.8 degree motor gives 3200 steps per revolution and a step
angle of 0.1125 degrees.

At the home pose one microstep moves a ball joint 0.140 mm, which tilts the
platform by 0.085 degrees. That number comes out of the kinematics and is the
finest correction the machine can make.

## Capacitors

Put a 100 uF 35 V electrolytic across VM and GND at each driver, as close to
the driver as you can get it. Stepper coils dump current back into the supply
when they switch, and without local bulk capacitance those spikes go straight
through the driver's rating. Watch the polarity.

## Motor coils

Each motor's four wires go to M1A, M1B, M2A, M2B. The pairing matters, the
order within a pair only sets direction. Find the pairs with a multimeter:
two wires that read a couple of ohms between them are one coil, and read open
against the other pair.

Common NEMA 17 colours, worth checking rather than trusting:

| Coil | Typical colours |
|---|---|
| 1 | black and green |
| 2 | red and blue |

Run `firmware/StepperTest` once everything is connected. It lifts all three
cranks together. Any motor that goes the other way has its coil pair
reversed, and the quickest fix is flipping that leg's flag in `MOTOR_INVERT`
in `Config.h`.

## Current limit

Set each driver's Vref before the motors carry any load. With a 59 Ncm NEMA 17
rated around 1.2 A per phase, aim for roughly 0.9 A RMS, which is a Vref near
0.9 V on most TMC2208 boards. Measure between the trim pot wiper and ground
with the driver powered and the motor idle.

Lower is quieter and cooler. If a motor loses position under load, raise it a
little. If the drivers get too hot to touch, lower it.

## Ground

Every ground has to meet: supply negative, both driver GND pins, the
regulator GND, the Teensy GND. A touch panel reading nonsense is very often a
ground that is only connected one way round a loop.
