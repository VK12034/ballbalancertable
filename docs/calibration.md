# Calibration

Three things need setting before the machine will balance: which panel wire
is which, what raw numbers the panel actually gives, and where its electrical
centre sits.

The values shipped in `Config.h` are placeholders. They have not been
measured against a real panel, and no two panels agree closely enough to
share them.

## 1. Sort out the panel wires

A 4 wire resistive panel has X+, Y+, X- and Y-, and suppliers order the tail
differently. The firmware expects:

| Teensy pin | Signal |
|---|---|
| A0 | X+ |
| A1 | Y+ |
| A2 | X- |
| A3 | Y- |

Connect them in whatever order you like to start with, then flash
`BallBalancer.ino`, open the serial console at 115200 and type `telemetry`.

Put the ball on and push it around by hand. The stream is
`x, y, errorX, errorY, tiltX, tiltY`.

- Moving the ball along the panel's long edge should change the first number
  and leave the second roughly alone. If it is the other way round, swap the
  A0/A2 pair with the A1/A3 pair.
- If a number runs backwards, moving negative when the ball goes positive,
  swap that axis's two wires. X+ with X-, or Y+ with Y-.

Change the pin constants in `Config.h` rather than rewiring. It is the same
result and you will not disturb the ribbon.

## 2. Measure the raw span

With telemetry still running, roll the ball slowly into each corner of the
active area and hold it there for a moment.

You want the raw ADC readings, not the millimetre output, so temporarily
change the telemetry line in `control()` to print `raw.x` and `raw.y`, or add
a print inside `readBall`.

Note the smallest and largest value you see on each axis, then put them in
`Config.h`:

```cpp
constexpr int TOUCH_RAW_X_MIN = 120;
constexpr int TOUCH_RAW_X_MAX = 910;
constexpr int TOUCH_RAW_Y_MIN = 130;
constexpr int TOUCH_RAW_Y_MAX = 900;
```

Readings within about 30 counts of the extremes tend to be unreliable, so
pull each limit in slightly from the absolute figure you saw. Losing a
millimetre of usable area at the edge is a fair trade for not chasing a ghost
reading.

Also check `TOUCH_ACTIVE_W` and `TOUCH_ACTIVE_H` against your panel's active
area, measured with calipers. Those two numbers set the scale from counts to
millimetres, and they are what make the PID gains mean something physical. If
they are wrong, the gains are wrong by the same factor.

## 3. Level the platform mechanically

Before touching the software offsets, get the mechanics square.

Power down, push the platform fully down, power up and let it lift to level.
Put a spirit level on the panel, or just set the ball down in the middle and
watch which way it drifts.

If it sits level but the ball still wanders, skip to step 4. If the platform
itself is visibly tilted, one crank is clamped at a different angle from the
others. Loosen that leg's M4 x 20, set the crank by eye to match the other
two, and retighten. Repeat until the platform looks flat.

This is worth doing properly. Software offsets can hide a crooked platform,
but the leg that is out will run closer to its travel limit than the others.

## 4. Trim the centre

The panel's electrical centre and the platform's mechanical centre are never
quite the same point. With a ball balancing, it will settle slightly off
middle.

Measure how far off, in millimetres, and put the correction in `Config.h`:

```cpp
constexpr float TOUCH_OFFSET_X = 0.0f;   // mm
constexpr float TOUCH_OFFSET_Y = 0.0f;   // mm
```

The sign is whichever moves the resting point toward the middle. Try a few
millimetres, reflash, watch, adjust. Two or three rounds normally does it.

The serial console also takes `offset <x> <y>` so you can try values without
reflashing, then write the one that works into `Config.h`.

## 5. Check the travel

With the ball off the platform, type `pattern circle` and watch it run
through a full cycle with nothing on it.

The platform should move smoothly all the way round. Any stall, click or
visible hesitation means a leg is reaching its limit or a joint is binding.
Check the M3 x 35 at each clevis is snug rather than tight, since over
tightening that bolt clamps the ball end and stops it swivelling.

Type `level` to stop.

## What the numbers should look like

Once calibrated, with the ball sitting still in the middle:

- `x` and `y` within a millimetre or two of zero
- `tiltX` and `tiltY` small and wandering slightly, not pinned at the 12
  degree limit
- No reading dropouts, which show up as the platform suddenly going flat

A tilt figure sitting at the limit means the loop thinks the ball is a long
way off and cannot fix it. That is usually an axis mapped backwards, which
sends the platform the wrong way and makes the error grow.
