# Assembly

Read `electronics/wiring.md` before you connect anything to power. There is a
wiring error in the original write-up that will destroy a driver.

Rough order: print, inserts, electronics on the bench, base, links, platform.
Test the motors before the platform goes on, because reaching the cranks
afterwards is awkward.

## 1. Print the parts

STLs are in `cad/mesh/`. PLA, 20% infill, 0.2 mm layers.

| Part | Qty | Orientation |
|---|---|---|
| `base_plate` | 1 | flanges up, supports on |
| `base_stand` | 1 | open end down |
| `platform_frame` | 1 | recess up |
| `link_1_crank` | 3 | flat |
| `link_2_coupler` | 3 | flat |
| `motor_spacer` | 3 | flat |
| `retainer_clip` | 4 | flat |

Check the links before you take them off the bed. The M4 holes want to be
round and the clamp slot on each crank wants to be open, not bridged over. A
slot that has fused shut will not clamp.

## 2. Heat set inserts

Seven M3 x 6 brass inserts:

- Four in the base stand, around the inside of the top deck, for the
  protoboards
- Three in the base plate underside, on a 70 mm bolt circle

Set the iron to about 200 C. Rest the insert in the hole, push straight down
until the top sits flush with the surface, then come away without twisting.
Going in crooked is the usual failure, and a crooked insert in the base plate
means the plate will not sit flat on the stand.

Let them cool before you put any load on them.

## 3. Build the circuit

Full detail in `electronics/wiring.md`. Short version:

1. Cut the 5 V trace on the Teensy underside.
2. Set the buck regulator to 5.0 V with nothing else connected, then power
   down.
3. Build the driver board: three TMC2208 on female headers, VM to 24 V, VIO
   to 3.3 V, GND to supply negative, a 100 uF capacitor across VM and GND at
   each driver.
4. Tie MS1 and MS2 to 3.3 V on all three drivers for 1/16 microstepping.
5. Run step and dir to Teensy pins 1 through 6, and ENA from pin 0 to all
   three drivers.
6. Bring the four panel wires to A0 through A3.
7. Check every ground is actually common before applying power.

Set each driver's current limit now, before the motors are under load.
Roughly 0.9 A RMS, which is a Vref near 0.9 V on most boards.

## 4. Assemble the base

1. Put a spacer on each motor's face, lining up the bolt pattern.
2. Bolt each motor to a flange on the base plate with four M3 x 10, through
   the spacer. The shaft pokes through the flange toward the middle of the
   machine.
3. Label the motors A, B and C and connect them to the matching drivers.
   Getting these in the wrong order is the single most annoying mistake to
   find later. Leg A sits on the +X axis, B and C follow anticlockwise.
4. Mount the two protoboards to the base stand's inserts with M3 x 5.
5. Leave the base plate off the stand for now. It is easier to reach the
   wiring.

## 5. Fit the cranks

Slide a `link_1_crank` onto each motor shaft, hub first, arm pointing inward
toward the middle of the machine. Close the clamp with an M4 x 20 and a
nylon locknut.

Do not fully tighten yet. The cranks need to be set to a common angle, and
the easiest time to do that is after the first motor test.

## 6. Test the motors

Flash `firmware/StepperTest` and run it. All three cranks should lift
together, pause, and come back down.

If one goes the other way, its coil pair is reversed. Flip that leg's entry
in `MOTOR_INVERT` in `Config.h` and reflash. Rewiring the motor works too and
amounts to the same thing.

Nothing moving at all usually means ENA. It is active low, so the driver is
disabled when pin 0 is high.

## 7. Finish the legs

For each of the three legs:

1. Screw a tie rod into the far end of a `link_2_coupler` with an M3 x 8.
2. Drop the tie rod's other ball end into one of the platform frame's clevis
   slots. Slide an M3 x 5 standoff into the gap either side of the ball, then
   run an M3 x 35 down through the whole stack and fit a nylon locknut. Snug,
   not tight. The ball has to swivel freely or the platform will bind.
3. Join the coupler's other end to the crank's knee with an M4 x 25 and a
   nylon locknut. Again snug, not tight.

Check leg A's coupler reaches the ear nearest the +X axis. A leg connected to
the wrong corner makes the platform tilt in a direction the firmware does not
expect, and the ball will run straight off.

## 8. Close up the base

Lower the base plate onto the stand, feeding the motor and panel wiring
through the middle hole as you go. Three M3 x 8 go up from inside the stand
into the plate's inserts.

Route the panel's ribbon so it has slack through the whole range of travel.
The platform moves about 85 mm vertically and tilts both ways, and a ribbon
pulled tight at full extension will tear at the tail. A dab of hot glue where
it leaves the panel takes the strain off the connector.

## 9. Fit the panel

Set the touch panel into the recess in the platform frame, active side up.
Clip it down with the four retainer clips.

The clips should hold it without force. If the panel is bowed, the readings
go strange near the edges.

## 10. First power up

1. Push the platform all the way down, until the cranks hit their lower stop,
   and hold it flat.
2. Power on.
3. The platform lifts to level on its own.
4. Place the ball.

The push-down step is not optional. There are no encoders and no endstops, so
that bottom position is the only reference the firmware has. Skip it and
every angle is wrong by however far off you started.

Then go to `docs/calibration.md`, because the panel needs calibrating before
any of this balances.
