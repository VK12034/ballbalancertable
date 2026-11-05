# Tuning the PID

The gains in `Config.h` have never been run on hardware. They are a starting
point worked out from the geometry, not a tuned set. Expect to change them.

```cpp
constexpr float KP = 0.085f;   // deg of tilt per mm of error
constexpr float KI = 0.0012f;  // deg per mm-second
constexpr float KD = 0.040f;   // deg per mm/s
```

Gains act on millimetres and produce degrees, so they carry real units. A kp
of 0.085 means a ball 50 mm off centre asks for about 4.3 degrees of tilt.
That makes the numbers easy to reason about instead of being arbitrary.

Calibrate the panel first. Tuning against a miscalibrated panel means tuning
against the wrong distances, and you will have to start over.

## Setting up

Connect the serial console at 115200. You can change gains live:

```
kp 0.09
ki 0
kd 0.05
status
```

Changing `ki` clears the integral, so you will not inherit a wound up term
from the previous value.

Turn on `telemetry` to watch `x, y, errorX, errorY, tiltX, tiltY` stream past.
Paste it into a spreadsheet if you want to look at the shape of the response.

## Order of work

Start with P only. Set `ki 0` and `kd 0`.

Raise `kp` until the ball oscillates steadily around the centre without
settling. Too low and it drifts off without ever really coming back. Too high
and it slams from side to side and leaves the panel.

Now add D. Raise `kd` until the oscillation damps out and the ball settles.
This is the term that does the real work. Derivative acts on ball speed, so
it starts pushing back before the ball reaches the centre. Most of the
difference between a machine that balances and one that does not is here.

Too much `kd` makes the platform twitchy and noisy, because it is amplifying
panel noise. If the steppers start buzzing with the ball sitting still, back
it off.

Add I last, if at all. The ball will likely settle a few millimetres off
centre. A small `ki` walks it in. Values around 0.001 to 0.002 are the right
order. Too much and you get a slow wallow that builds over several seconds.

The source's author notes the machine balances about as well on PD alone, with
the integral term only pulling the ball exactly to the middle. Worth
remembering if I is giving you trouble: you can leave it at zero.

## What good looks like

- Ball placed anywhere settles within two or three seconds
- No standing oscillation once settled
- Platform quiet when the ball is still
- A nudge gets corrected without the ball crossing the centre more than once
  or twice

## Common shapes

| Behaviour | Likely cause |
|---|---|
| Oscillates and never settles | `kd` too low for the `kp` |
| Ball leaves the platform immediately | `kp` far too high, or an axis reversed |
| Drifts off slowly, no real correction | `kp` too low |
| Settles off centre and stays there | needs a little `ki`, or the offsets are wrong |
| Slow wallow over several seconds | `ki` too high |
| Platform buzzes with the ball still | `kd` too high, amplifying panel noise |
| Corrects well then suddenly goes flat | losing the touch reading, not a gain problem |

That last one is worth separating out. If the platform drops to level
mid-correction, the firmware has decided the ball is gone. Check
`TOUCH_PRESSURE_MIN` and the wiring before touching any gain.

## Loop rate

The control loop runs at 50 Hz, set by `LOOP_INTERVAL_US`. The source uses
the same 20 ms interval.

Faster is not automatically better here. A resistive panel is noisy, and
shortening the interval means the derivative term works on a smaller real
change against the same noise, so the signal to noise ratio gets worse. If
you do change it, retune `kd`, since the derivative scales with it.

## Tilt limit

`TILT_LIMIT_DEG` is 12 degrees. The mechanism reaches 15 in any direction,
verified in the CAD, so there is margin.

Raising it toward 15 gives faster correction on a ball that is already moving
quickly, at the cost of getting closer to the travel stops. The inverse
kinematics refuse any pose that would drive a crank past its limit, so an
over-ambitious tilt makes the platform ignore the command rather than damage
itself. That shows up as a correction that just does not happen.
