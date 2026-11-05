# CAD parameters

Every number here is a user parameter in the Fusion design and a constant in
`cad/scripts/BallBalancer/BallBalancer.py`. The same values appear again in
`firmware/BallBalancer/Config.h` and `tools/verify_kinematics.py`. Change one
and you have to change all three, or the firmware will aim the platform
somewhere the mechanism cannot go.

## How parametric this actually is

The design is script driven. Extrude depths, hole diameters and link lengths
come from named user parameters, and you can open the design in Fusion and
read them in the parameters dialog. In-plane sketch positions were generated
from those same values by the build script rather than by dimensional
constraints, so changing a link length means editing the parameter block at
the top of `BallBalancer.py` and running it again, not dragging a dimension
and letting the timeline recompute. The script is the source of truth. It
rebuilds the whole machine from scratch in about a minute.

## Kinematic skeleton

| Parameter | Value | Meaning |
|---|---|---|
| `base_motor_r` | 78 mm | machine axis to crank pivot axis |
| `platform_joint_r` | 95 mm | platform centre to tie rod ball centre |
| `crank_len` | 55 mm | crank pivot to knee, link_1 |
| `link2_len` | 73 mm | knee to tie rod thread, link_2 |
| `tie_rod_len` | 22 mm | tie rod, between ball centres |
| `coupler_len` | 95 mm | derived, `link2_len + tie_rod_len` |
| `motor_shaft_z` | 35 mm | crank pivot height above the base plate top face |
| `crank_home_angle` | 20 deg | crank angle with the platform level |
| `crank_min_angle` | -20 deg | lower travel limit |
| `crank_max_angle` | 60 deg | upper travel limit |

The crank points inward at 0 degrees and lifts as the angle grows.

## What that geometry gives you

| Quantity | Value |
|---|---|
| Platform height, level | 119.44 mm |
| Ball joint travel | 81.82 mm to 166.56 mm |
| Heave | 84.74 mm |
| Tilt envelope | plus or minus 15 degrees in any direction |
| Resolution, one microstep | 0.140 mm of joint lift |
| Resolution as platform tilt | 0.085 degrees |

Home sits at 20 degrees rather than 0 on purpose. An earlier version parked
the crank at 0, which is close to the bottom stop, and a 10 degree tilt then
drove one leg to -23 degrees and past its limit. Parking near mid travel
leaves room in both directions whichever way the platform tips.

## Touch panel

| Parameter | Value |
|---|---|
| `panel_w` | 191 mm |
| `panel_h` | 145 mm |
| `panel_t` | 1.8 mm |
| Active area | 170.4 by 127.8 mm |

The source specifies an 8.4 inch 4 wire resistive panel and gives no
dimensions. These are typical for that size in 4:3. Measure yours. If it
differs, change `panel_w`, `panel_h` and `panel_t` and rerun the script: the
platform frame, its recess and the clip slots all follow from them.

## Platform

| Parameter | Value |
|---|---|
| `frame_border` | 9 mm |
| `frame_t` | 6 mm |
| `panel_recess` | 2 mm |
| `ear_r` | 11 mm |
| Frame outer | 209 by 163 mm, ears reaching 187 mm across |

Each ear is a clevis. A radial slot splits it in two, the tie rod's ball end
sits in the slot, and an M3 x 35 passes through both prongs with an M3 x 5
standoff filling the gap either side. That matches what the source describes
for this joint.

## Base

| Parameter | Value |
|---|---|
| `base_plate_r` | 106 mm |
| `base_plate_t` | 6 mm |
| `stand_r` | 95 mm |
| `stand_h` | 55 mm |
| `stand_wall` | 3 mm |
| `stand_deck_t` | 4 mm |
| `stand_bolt_r` | 70 mm |

The base plate carries three vertical flanges, each 50 mm wide by 62 mm tall
by 6 mm thick. The motors bolt to those with their axes horizontal and
tangential, which is what makes each crank sweep in its own radial vertical
plane. The source's part list has no separate motor bracket, so the flanges
have to be part of the plate.

## Motor mounting stack

Distances along the motor axis, measured from the leg's radial centre plane.
Negative is away from the platform side.

| Feature | Span |
|---|---|
| Coupler | -4 to +4 |
| Crank hub | -16 to -6 |
| Motor flange | -23.5 to -17.5 |
| Spacer | -28.5 to -23.5 |
| Motor front face | -28.5 |

Two running clearances fall out of that: 2.0 mm between the coupler and the
crank arm, and 1.5 mm between the crank hub and the flange. The first build
had both at exactly zero, with a rotating face flush against a fixed one.
`check_fastener_access()` in the build script measures them on every run so
that cannot come back quietly.

The NEMA 17 shaft has to span 22.5 mm to reach across the hub. A standard
24 mm shaft clears it by 1.5 mm.

## Fasteners and fits

| Parameter | Value |
|---|---|
| `m3_clear` | 3.4 mm |
| `m4_clear` | 4.5 mm |
| `m3_insert_d` | 4.0 mm |
| `m3_insert_depth` | 6 mm |
| `nema_bolt` | 31 mm square pattern |
| `nema_pilot` | 22 mm |

The insert pilot suits a common M3 x 6 brass heat set insert. Brands vary by a
few tenths. Press one into a test coupon before you commit to the real parts.

## Printed mass

Volumes come from the Fusion model. Fusion's stock material library has no
PLA, so the build script reports mass from volume at 1.24 g/cc rather than
from whatever material matched.

| Part | Qty | Solid | At 20% infill |
|---|---|---|---|
| base_plate | 1 | 315.5 g | 119.9 g |
| base_stand | 1 | 248.7 g | 94.5 g |
| platform_frame | 1 | 83.3 g | 31.6 g |
| link_1_crank | 3 | 10.4 g | 3.9 g |
| link_2_coupler | 3 | 11.4 g | 4.3 g |
| motor_spacer | 3 | 8.3 g | 3.2 g |
| retainer_clip | 4 | 1.2 g | 0.5 g |
| **Total** | | **743 g** | **282 g** |

The infill figure uses a 0.38 factor for walls plus 20% infill. The source
quotes about 310 g of filament for its version, so 282 g for this one is in
the right territory and suggests the two designs are a similar size.
