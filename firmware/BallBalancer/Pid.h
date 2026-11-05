// One PID axis. The balancer runs two of these, X and Y.
//
// Error is in millimetres from the setpoint, output is in degrees of platform
// tilt, so the gains carry real units and you can reason about them. The
// derivative term works on ball speed, which is the part that does most of
// the stabilising, and it runs through a first order filter because a
// resistive touch panel is noisy enough to make a raw difference useless.

#pragma once

class Pid {
 public:
  Pid(float kp, float ki, float kd);

  // dt in seconds. Returns degrees of tilt, already clamped.
  float update(float error, float dt);

  void reset();

  // Gains are public so the serial console can retune on the fly without a
  // reflash. docs/tuning.md explains the procedure.
  void setGains(float kp, float ki, float kd);

  float kp() const { return kp_; }
  float ki() const { return ki_; }
  float kd() const { return kd_; }

  float lastP() const { return p_; }
  float lastI() const { return i_; }
  float lastD() const { return d_; }

 private:
  float kp_, ki_, kd_;
  float integral_;
  float previousError_;
  float velocity_;
  bool primed_;
  float p_, i_, d_;
};
