#include "Kinematics.h"

#include <math.h>

namespace {

// Leg geometry, pulled out once so the solver reads like the derivation.
constexpr float L1 = CRANK_LEN;
constexpr float L2 = COUPLER_LEN;
constexpr float RB = BASE_MOTOR_R;
constexpr float ZB = MOTOR_SHAFT_Z;

constexpr float DEG2RAD = 3.14159265358979f / 180.0f;
constexpr float RAD2DEG = 180.0f / 3.14159265358979f;

}  // namespace

namespace kinematics {

bool crankAngle(float rMm, float zMm, float &outRad) {
  // The crank points inward at angle zero and lifts as the angle grows, so
  // the knee sits at (RB - L1*cos, ZB + L1*sin). Putting that into
  // |P - K| = L2 and collecting terms gives
  //
  //     A*cos(t) + B*sin(t) = C
  //
  // which solves as t = atan2(B, A) + acos(C / hypot(A, B)).
  const float a = rMm - RB;
  const float b = zMm - ZB;

  const float A = 2.0f * a * L1;
  const float B = -2.0f * b * L1;
  const float C = L2 * L2 - L1 * L1 - a * a - b * b;

  const float r = sqrtf(A * A + B * B);
  if (r < 1e-6f) {
    return false;
  }

  const float ratio = C / r;
  if (ratio < -1.0f || ratio > 1.0f) {
    return false;  // out of reach, no real solution
  }

  // The plus branch is the elbow-up assembly. The minus branch folds the
  // coupler back down through the base plate and cannot be built.
  outRad = atan2f(B, A) + acosf(ratio);
  return true;
}

float ballHeight(float crankRad) {
  const float kneeR = RB - L1 * cosf(crankRad);
  const float kneeZ = ZB + L1 * sinf(crankRad);
  const float dr = PLATFORM_JOINT_R - kneeR;
  const float disc = L2 * L2 - dr * dr;
  if (disc < 0.0f) {
    return kneeZ;
  }
  return kneeZ + sqrtf(disc);
}

LegSolution solve(const PlatformPose &pose) {
  LegSolution out;
  out.valid = true;

  const float tanPitch = tanf(pose.pitchRad);
  const float tanRoll = tanf(pose.rollRad);

  for (int leg = 0; leg < 3; leg++) {
    const float phi = LEG_PHI_DEG[leg] * DEG2RAD;
    const float x = PLATFORM_JOINT_R * cosf(phi);
    const float y = PLATFORM_JOINT_R * sinf(phi);

    // Plane through the platform centre with the commanded tilt. The joints
    // also creep inward a little as the platform tips, but at the tilts this
    // machine uses that shift is smaller than one microstep, so it is left
    // out on purpose.
    const float z = pose.heightMm - x * tanPitch + y * tanRoll;

    float angle = 0.0f;
    if (!crankAngle(PLATFORM_JOINT_R, z, angle)) {
      out.valid = false;
      out.crankRad[leg] = CRANK_HOME_DEG * DEG2RAD;
      continue;
    }

    const float deg = angle * RAD2DEG;
    if (deg < CRANK_MIN_DEG || deg > CRANK_MAX_DEG) {
      out.valid = false;
    }
    out.crankRad[leg] = angle;
  }

  return out;
}

long crankRadToSteps(int leg, float crankRad) {
  // Steps are counted from the startup position, which is the bottom stop.
  const float deg = crankRad * RAD2DEG;
  const float fromStart = deg - STARTUP_CRANK_DEG;
  long steps = (long)lroundf(fromStart / DEG_PER_STEP);
  if (MOTOR_INVERT[leg]) {
    steps = -steps;
  }
  return steps;
}

}  // namespace kinematics
