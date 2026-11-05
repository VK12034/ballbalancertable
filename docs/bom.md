# Bill of materials

Quantities are for one machine. Fastener counts come from the assembly order
in `docs/assembly.md`, so they should add up exactly. Buy a few spares of the
M3 hardware anyway.

## Printed parts

All PLA. 20% infill, 0.2 mm layers, supports on. STLs are in `cad/mesh/`.

| Part | Qty | Mass at 20% infill | Notes |
|---|---|---|---|
| `base_plate` | 1 | 120 g | print flange side up, needs supports |
| `base_stand` | 1 | 95 g | print open end down, no supports needed |
| `platform_frame` | 1 | 32 g | flat, recess up |
| `link_1_crank` | 3 | 4 g each | flat |
| `link_2_coupler` | 3 | 4 g each | flat |
| `motor_spacer` | 3 | 3 g each | flat |
| `retainer_clip` | 4 | 0.5 g each | flat, lip up |

About 282 g of filament in total. Expect somewhere around 20 to 25 hours of
print time depending on your machine.

The base plate is the awkward one. Three vertical flanges stand off a flat
disc, so it either needs supports under the flange overhangs or a careful
orientation. Printing it flange side up with auto supports works.

## Electronics

| Item | Qty | Notes |
|---|---|---|
| Teensy 4.1 | 1 | the 5 V trace gets cut, see `electronics/wiring.md` |
| NEMA 17 stepper, 59 Ncm, bipolar | 3 | 42 mm frame, 5 mm shaft, 24 mm shaft length |
| TMC2208 stepper driver | 3 | StepStick form factor |
| 100 uF 35 V electrolytic capacitor | 3 | one per driver, across VM and GND |
| Buck regulator, 24 V to 5 V, adjustable | 1 | 1 A is plenty |
| Mini protoboard | 2 | one for the Teensy, one for the drivers |
| Screw terminals | 2 | for the supply input |
| Male and female header pins | 1 set | makes the Teensy and drivers removable |
| 22 AWG stranded wire | a few metres | |
| 24 V DC supply, 4 A or better | 1 | a bench supply is ideal while tuning |

Socketing the Teensy and the drivers on header pins is worth the extra
effort. You will want to pull a driver at some point.

## Mechanical

| Item | Qty | Notes |
|---|---|---|
| 8.4 inch 4 wire resistive touch panel | 1 | roughly 191 by 145 mm outer, see note below |
| 1 inch steel bearing ball | 1 | plain carbon or stainless, both work |
| M3 tie rod, 22 mm between ball centres | 3 | ball ends, 8 mm ball |
| M3 x 6 heat set insert, brass | 7 | 4 in the base stand, 3 in the base plate |
| M3 x 5 standoff | 6 | two per platform clevis |

## Fasteners

| Size | Qty | Where |
|---|---|---|
| M3 x 5 | 4 | protoboards to the base stand |
| M3 x 8 | 6 | 3 base plate to stand, 3 tie rod to coupler |
| M3 x 10 | 12 | motors to the flanges, 4 per motor |
| M3 x 35 | 3 | through each platform clevis |
| M3 nylon locknut | 3 | on the M3 x 35 |
| M4 x 20 | 3 | crank clamp onto the motor shaft |
| M4 x 25 | 3 | knee pivot, crank to coupler |
| M4 nylon locknut | 6 | 3 on each of the M4 sizes |

Nylon locknuts rather than plain nuts on anything that rotates. The knee and
the clamp both see constant reversing load and a plain nut walks off.

## Tools

- Soldering iron, for the heat set inserts as well as the wiring
- Needle nose pliers
- Wire cutters and strippers
- 2.5 mm hex key
- Craft knife, for the Teensy 5 V trace
- Multimeter, for setting the regulator and the driver current limits
- Hot glue gun, for strain relief on the panel ribbon

A multimeter is not optional here. You need it to set the 5 V rail before the
Teensy sees it, and to set the driver current limits.

## A note on the touch panel

The source specifies an 8.4 inch 4 wire resistive panel and gives no
dimensions. Panels sold at this size vary. The CAD assumes 191 by 145 mm
outer with a 1.8 mm thickness and a 170.4 by 127.8 mm active area.

Measure yours before printing the platform frame. If it differs, change
`panel_w`, `panel_h` and `panel_t` at the top of
`cad/scripts/BallBalancer/BallBalancer.py` and run the script again. The
recess, the clip slots and the frame outline all follow from those three
numbers.

Panels usually arrive with a small controller board and a USB connector
attached. You do not need that. The four wires from the panel's flex tail go
straight to the Teensy's analog pins. Which of the four is which varies by
supplier, and `docs/calibration.md` covers working it out.
