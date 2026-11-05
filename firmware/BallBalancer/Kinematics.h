// Inverse kinematics for the 3RRS platform.
//
// Each leg is solved on its own, as a flat two-link problem in the vertical
// plane that contains that leg's crank. The platform pose fixes where each
// ball joint has to be, and this file works out the crank angle that puts it
// there. Closed form, no iteration, so the loop time is predictable.
//
// tools/verify_kinematics.py holds the same solver in Python and checks it
// against forward kinematics across the whole working envelope. If you touch
// the maths here, run that script too.

#pragma once

#include "Config.h"

struct PlatformPose {
  float heightMm;   // platform centre above the base plate top face
  float rollRad;    // rotation about X
  float pitchRad;   // rotation about Y
};

struct LegSolution {
  float crankRad[3];
  bool valid;       // false if any leg was out of reach or past a stop
};

namespace kinematics {

// Crank angle that puts this leg's ball joint at (rMm, zMm) in the leg plane.
// Returns false when the point is out of reach.
bool crankAngle(float rMm, float zMm, float &outRad);

// Ball joint height for a crank angle. The inverse of the above, used for
// homing and for checking the solver.
float ballHeight(float crankRad);

// Full pose to three crank angles. Clamps nothing: check .valid.
LegSolution solve(const PlatformPose &pose);

// Crank angle to stepper position in microsteps, including per motor
// direction inversion from Config.h.
long crankRadToSteps(int leg, float crankRad);

}  // namespace kinematics
