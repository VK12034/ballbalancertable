"""Numerical check of the 3RRS inverse kinematics used by the firmware.

The firmware solves, for each leg, a planar two-link problem in that leg's
radial-vertical plane. This script re-derives the same closed form in Python,
then checks it against forward kinematics over the whole working envelope.
A round-trip error larger than a micrometre means the firmware's solver and
this reference disagree, which has always meant one of them is wrong.

Run:  python tools/verify_kinematics.py
Exit code 0 means every check passed.

No third-party packages. Standard library only.
"""

import math
import sys

# ---------------------------------------------------------------------------
# Machine geometry. These are the same numbers as cad/parameters.md and
# firmware/BallBalancer/config.h. Keep the three in step.
# ---------------------------------------------------------------------------

BASE_MOTOR_R = 78.0       # mm, centre to crank pivot axis
PLATFORM_JOINT_R = 95.0   # mm, platform centre to tie-rod ball centre
CRANK_LEN = 55.0          # mm, pivot to knee
COUPLER_LEN = 95.0        # mm, knee to tie-rod ball centre (link_2 + tie rod)
MOTOR_SHAFT_Z = 35.0      # mm, crank pivot height above base plate top face

CRANK_MIN_DEG = -20.0     # mechanical travel limits
CRANK_MAX_DEG = 60.0
CRANK_HOME_DEG = 20.0     # level platform parks near mid-travel, so a tilt in
                          # any direction has room in both directions

LEG_PHI_DEG = (0.0, 120.0, 240.0)

TOL_MM = 1e-6


# ---------------------------------------------------------------------------
# Inverse kinematics, one leg
# ---------------------------------------------------------------------------

def crank_angle(r_p, z_p,
                base_r=BASE_MOTOR_R, base_z=MOTOR_SHAFT_Z,
                l1=CRANK_LEN, l2=COUPLER_LEN):
    """Crank angle, in radians, that puts this leg's ball joint at (r_p, z_p).

    r_p is measured from the machine axis, z_p above the base plate top face,
    both in the leg's own radial-vertical plane.

    Sign convention: the crank points inward at angle 0 and lifts as the angle
    increases, so the knee sits at

        (base_r - l1*cos(theta),  base_z + l1*sin(theta))

    Substituting that into |P - K| = l2 and collecting terms gives

        A*cos(theta) + B*sin(theta) = C

    which has the standard solution theta = atan2(B, A) + acos(C / hypot(A, B)).
    The plus branch is the elbow-up assembly; the minus branch folds the
    coupler back through the base plate and is not reachable.

    Returns None when the point is outside the leg's reach.
    """
    a = r_p - base_r
    b = z_p - base_z

    A = 2.0 * a * l1
    B = -2.0 * b * l1
    C = l2 * l2 - l1 * l1 - a * a - b * b

    r = math.hypot(A, B)
    if r < 1e-12:
        return None

    ratio = C / r
    if ratio < -1.0 or ratio > 1.0:
        return None  # unreachable: no real solution

    return math.atan2(B, A) + math.acos(ratio)


def knee_point(theta,
               base_r=BASE_MOTOR_R, base_z=MOTOR_SHAFT_Z, l1=CRANK_LEN):
    """Knee position (r, z) in the leg plane for a given crank angle."""
    return (base_r - l1 * math.cos(theta), base_z + l1 * math.sin(theta))


def ball_z_from_crank(theta, r_p,
                      base_r=BASE_MOTOR_R, base_z=MOTOR_SHAFT_Z,
                      l1=CRANK_LEN, l2=COUPLER_LEN):
    """Forward kinematics: ball-joint height for a crank angle.

    Assumes the ball joint stays at radius r_p, which is what the platform
    geometry enforces at small tilt. Returns the upper intersection.
    """
    kr, kz = knee_point(theta, base_r, base_z, l1)
    dr = r_p - kr
    disc = l2 * l2 - dr * dr
    if disc < 0.0:
        return None
    return kz + math.sqrt(disc)


# ---------------------------------------------------------------------------
# Platform pose -> three ball-joint targets
# ---------------------------------------------------------------------------

def platform_joint_targets(height, roll_rad, pitch_rad,
                           joint_r=PLATFORM_JOINT_R):
    """Ball-joint (r, z) targets for a platform at `height` with a given tilt.

    The platform is a rigid triangle of ball joints on a circle of radius
    joint_r. Tilting it about the X axis (roll) and Y axis (pitch) moves each
    joint vertically by the plane equation, which to the accuracy this machine
    works at is

        z_i = height - x_i*tan(pitch) + y_i*tan(roll)

    The joints also sweep very slightly inward as the platform tilts. That
    second-order term is below the resolution of a 0.1125 deg stepper and the
    firmware ignores it, so this reference ignores it too.
    """
    out = []
    for phi_deg in LEG_PHI_DEG:
        phi = math.radians(phi_deg)
        x = joint_r * math.cos(phi)
        y = joint_r * math.sin(phi)
        z = height - x * math.tan(pitch_rad) + y * math.tan(roll_rad)
        out.append((joint_r, z))
    return out


# ---------------------------------------------------------------------------
# Checks
# ---------------------------------------------------------------------------

def check_round_trip():
    """IK then FK must return the height we asked for."""
    worst = 0.0
    tested = 0
    unreachable = 0

    r_p = PLATFORM_JOINT_R
    z_lo = ball_z_from_crank(math.radians(CRANK_MIN_DEG), r_p)
    z_hi = ball_z_from_crank(math.radians(CRANK_MAX_DEG), r_p)

    steps = 400
    for i in range(steps + 1):
        z_target = z_lo + (z_hi - z_lo) * i / steps
        theta = crank_angle(r_p, z_target)
        if theta is None:
            unreachable += 1
            continue
        z_back = ball_z_from_crank(theta, r_p)
        if z_back is None:
            unreachable += 1
            continue
        worst = max(worst, abs(z_back - z_target))
        tested += 1

    ok = tested > 0 and unreachable == 0 and worst < TOL_MM
    print("round trip over heave range   : {0} points, worst error {1:.3e} mm, "
          "{2} unreachable -> {3}".format(tested, worst, unreachable,
                                          "pass" if ok else "FAIL"))
    return ok


def home_height():
    """Platform height, level, with every crank at the home angle."""
    return ball_z_from_crank(math.radians(CRANK_HOME_DEG), PLATFORM_JOINT_R)


def check_home_pose():
    """IK at the home height must return the home crank angle."""
    h_home = home_height()
    theta = crank_angle(PLATFORM_JOINT_R, h_home)
    deg = math.degrees(theta)
    ok = abs(deg - CRANK_HOME_DEG) < 1e-6
    print("home pose                      : platform z = {0:.3f} mm, crank = "
          "{1:+.6f} deg (want {2:+.1f}) -> {3}".format(
              h_home, deg, CRANK_HOME_DEG, "pass" if ok else "FAIL"))
    return ok, h_home


def check_envelope(h_home):
    """Every pose the controller can command must be inside crank travel."""
    max_tilt_deg = 15.0
    bad = []
    rows = 0

    for tilt in range(0, int(max_tilt_deg) + 1):
        for direction in range(0, 360, 15):
            roll = math.radians(tilt) * math.sin(math.radians(direction))
            pitch = math.radians(tilt) * math.cos(math.radians(direction))
            targets = platform_joint_targets(h_home, roll, pitch)
            rows += 1
            for leg, (r_p, z_p) in enumerate(targets):
                theta = crank_angle(r_p, z_p)
                if theta is None:
                    bad.append((tilt, direction, leg, "unreachable"))
                    continue
                deg = math.degrees(theta)
                if deg < CRANK_MIN_DEG or deg > CRANK_MAX_DEG:
                    bad.append((tilt, direction, leg, "{0:+.2f} deg".format(deg)))

    ok = not bad
    print("tilt envelope to +/-{0:.0f} deg      : {1} poses -> {2}".format(
        max_tilt_deg, rows, "pass" if ok else "FAIL"))
    for entry in bad[:8]:
        print("    tilt {0} deg, dir {1} deg, leg {2}: {3}".format(*entry))
    return ok


def report_travel():
    """Numbers the documentation quotes, printed so they can be checked."""
    r_p = PLATFORM_JOINT_R
    z_lo = ball_z_from_crank(math.radians(CRANK_MIN_DEG), r_p)
    z_hi = ball_z_from_crank(math.radians(CRANK_MAX_DEG), r_p)
    print("")
    print("derived figures")
    print("  crank travel                 : {0:+.1f} to {1:+.1f} deg".format(
        CRANK_MIN_DEG, CRANK_MAX_DEG))
    print("  ball joint height range      : {0:.2f} to {1:.2f} mm "
          "({2:.2f} mm of heave)".format(z_lo, z_hi, z_hi - z_lo))
    steps_per_rev = 3200.0
    deg_per_step = 360.0 / steps_per_rev
    print("  stepper resolution           : {0:.4f} deg/step at 1/16 "
          "microstepping".format(deg_per_step))

    # Height change for one microstep at the home pose, the practical
    # resolution limit on platform tilt.
    h_home = home_height()
    print("  level platform parks at      : {0:.2f} mm, crank {1:+.1f} deg"
          .format(h_home, CRANK_HOME_DEG))
    z_a = ball_z_from_crank(math.radians(CRANK_HOME_DEG), r_p)
    z_b = ball_z_from_crank(math.radians(CRANK_HOME_DEG + deg_per_step), r_p)
    print("  one microstep at home        : {0:.4f} mm of joint lift".format(
        abs(z_b - z_a)))
    print("  that is a platform tilt of   : {0:.5f} deg".format(
        math.degrees(math.atan2(abs(z_b - z_a), PLATFORM_JOINT_R))))
    return h_home


def main():
    print("3RRS inverse kinematics check")
    print("geometry: base_r={0} mm  joint_r={1} mm  crank={2} mm  "
          "coupler={3} mm  shaft_z={4} mm".format(
              BASE_MOTOR_R, PLATFORM_JOINT_R, CRANK_LEN, COUPLER_LEN,
              MOTOR_SHAFT_Z))
    print("")

    results = []
    home_ok, h_home = check_home_pose()
    results.append(home_ok)
    results.append(check_round_trip())
    results.append(check_envelope(h_home))
    report_travel()

    print("")
    if all(results):
        print("all checks passed")
        return 0
    print("CHECKS FAILED")
    return 1


if __name__ == "__main__":
    sys.exit(main())
