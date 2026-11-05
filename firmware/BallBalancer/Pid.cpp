#include "Pid.h"

#include "Config.h"

#include <math.h>

Pid::Pid(float kp, float ki, float kd)
    : kp_(kp), ki_(ki), kd_(kd), integral_(0.0f), previousError_(0.0f),
      velocity_(0.0f), primed_(false), p_(0.0f), i_(0.0f), d_(0.0f) {}

void Pid::reset() {
  integral_ = 0.0f;
  previousError_ = 0.0f;
  velocity_ = 0.0f;
  primed_ = false;
  p_ = i_ = d_ = 0.0f;
}

void Pid::setGains(float kp, float ki, float kd) {
  kp_ = kp;
  ki_ = ki;
  kd_ = kd;
}

float Pid::update(float error, float dt) {
  if (dt <= 0.0f) {
    return p_ + i_ + d_;
  }

  p_ = kp_ * error;

  // The first sample after the ball lands has no meaningful previous error,
  // so skip the derivative that one time. Otherwise the platform kicks.
  float rawVelocity = 0.0f;
  if (primed_) {
    rawVelocity = (error - previousError_) / dt;
  } else {
    primed_ = true;
  }
  velocity_ += D_FILTER * (rawVelocity - velocity_);
  d_ = kd_ * velocity_;

  integral_ += error * dt;

  // Clamp the integral by its contribution, not by the raw accumulator, so
  // the limit means the same thing whatever ki happens to be.
  if (ki_ > 1e-9f) {
    const float limit = INTEGRAL_LIMIT / ki_;
    if (integral_ > limit) {
      integral_ = limit;
    } else if (integral_ < -limit) {
      integral_ = -limit;
    }
  } else {
    integral_ = 0.0f;
  }
  i_ = ki_ * integral_;

  previousError_ = error;

  float output = p_ + i_ + d_;
  if (output > TILT_LIMIT_DEG) {
    output = TILT_LIMIT_DEG;
  } else if (output < -TILT_LIMIT_DEG) {
    output = -TILT_LIMIT_DEG;
  }
  return output;
}
