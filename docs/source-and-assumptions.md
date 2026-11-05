# What came from the source, and what did not

This project is built from the description in aaedmusa's Instructables
write-up. None of the author's files are redistributed here. The CAD and the
firmware were written from scratch against the published description, which
matters both for licensing and for knowing which numbers you can trust.

Source: <https://www.instructables.com/Ball-Balancer/>, published 26 April
2023, licensed CC BY-NC-SA 4.0.

## Specified by the source

These are stated outright in the write-up and are reproduced faithfully.

| Item | Value |
|---|---|
| Mechanism | 3RRS parallel manipulator, 3 degrees of freedom |
| Actuators | 3 x NEMA 17 bipolar stepper, 59 Ncm |
| Drivers | 3 x TMC2208 |
| Microstepping | 1/16, giving 3200 steps/rev and 0.1125 deg per step |
| Controller | Teensy 4.1 |
| Sensing | 8.4 inch 4 wire resistive touch panel |
| Ball | 1 inch steel bearing ball |
| Supply | 24 V, with a regulator dropping it to 5 V for the Teensy |
| Teensy 5 V trace | cut, so USB and the bench supply are not tied together |
| Decoupling | 100 uF 35 V across each driver |
| Pin map | 0 = ENA, 1/2 = A step/dir, 3/4 = B, 5/6 = C, A0 to A3 = panel |
| Control | two PID loops, X and Y, reading the panel every 20 ms |
| Motion smoothing | stepper speed and acceleration scale with distance to target |
| Homing | none, the platform is pushed fully down before power on |
| Tie rod | M3, 22 mm |
| Inserts | M3 x 6 heat set |
| Standoffs | M3 x 5, two filling the gap either side of each tie rod |
| Print settings | PLA, 20% infill, auto supports, about 310 g |
| Part list | base_stand, base_plate, spacer x3, link_1 x3, link_2 x3, platform_frame, retainer_clip x4, two optional slot inserts |
| Fastener sizes | M3 x 5, x8, x10, x35; M4 x 20, x25; M3 and M4 nylon locknuts |

## Chosen here, because the source does not say

Every dimension below is a decision made for this build. The source gives no
link lengths, no radii and no heights, so these were picked to give a sensible
working envelope and then checked. `cad/parameters.md` lists them all with
the reasoning.

| Item | Value | Why |
|---|---|---|
| `base_motor_r` | 78 mm | fits three NEMA 17s at 120 degrees without the bodies touching |
| `platform_joint_r` | 95 mm | inside the panel footprint, with ears reaching slightly past the short edge |
| `crank_len` | 55 mm | gives 85 mm of heave across the crank travel |
| `coupler_len` | 95 mm | 73 mm link plus the 22 mm tie rod the source specifies |
| `motor_shaft_z` | 35 mm | clears the base plate through the whole crank sweep |
| `crank_home_angle` | 20 deg | mid travel, so a tilt has room in both directions |
| Crank travel | -20 to +60 deg | keeps the coupler out of the base plate at one end and the platform at the other |
| Motor mounting | three vertical flanges, part of the base plate | the source part list has no separate bracket, so the plate has to carry the motors |
| Platform ear | clevis with a radial slot | matches the M3 x 35 plus two standoffs the source describes |
| Panel size | 191 x 145 x 1.8 mm | typical 8.4 inch 4:3, measure yours |
| Panel active area | 170.4 x 127.8 mm | typical, and it sets the units on the PID gains |
| Running clearances | 2.0 mm at the knee, 1.5 mm at the crank hub | enough to stop a rotating face rubbing a fixed one |
| PID gains | kp 0.085, ki 0.0012, kd 0.040 | plausible starting values in deg per mm, never tuned on hardware |
| Tilt limit | 12 deg | inside the 15 deg the mechanism can reach |
| Touch calibration | placeholder ADC span | must be measured per panel, see `docs/calibration.md` |

## Where this corrects the source

**TMC2208 supply pins.** The write-up says to wire each driver's VIO to the
24 V supply and VM to the Teensy's 3.3 V. On a TMC2208 those are the other
way round: VM is the motor supply and VIO is the logic reference. Following
the text as written puts 24 V into a 3.3 V logic pin. This project wires them
per the datasheet and `electronics/wiring.md` explains it.

**Home position.** An early version of this design parked the crank at 0
degrees with the platform level, which sits near the bottom of the travel. A
10 degree tilt then drove one leg to -23 degrees, past its stop.
`tools/verify_kinematics.py` caught it. Home moved to 20 degrees, near mid
travel.

## Licensing

The source is CC BY-NC-SA 4.0, which is share-alike and non-commercial. The
CAD and documentation here are derived from its description and carry the
same licence. The firmware was written from scratch and is MIT, which is
compatible for anyone combining the two.

The author's own GitHub repository for this project carries no licence file,
which under GitHub's terms means all rights reserved. None of that code is
used or reproduced here. The firmware in this repository implements the same
documented approach, two PID loops feeding inverse kinematics, from the
written description only.
