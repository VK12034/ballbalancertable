// Setpoint generators.
//
// Balancing already drives the ball to a target point. Move the target and
// the ball follows, so a pattern is just a target that changes with time.
// Anything you can write as x(t), y(t) works. Keep the speeds low: the ball
// has to actually catch up, and it only accelerates as fast as the tilt
// limit allows.

#pragma once

enum class Pattern {
  Hold,       // centre, the default
  Line,
  Circle,
  Ellipse,
  Triangle,
  Rectangle,
  FigureEight,
};

struct Setpoint {
  float x;  // mm from panel centre
  float y;
};

namespace patterns {

// seconds is time since the pattern started.
Setpoint evaluate(Pattern pattern, float seconds);

const char *name(Pattern pattern);

// Parses a pattern name from the serial console. Returns false if unknown.
bool fromName(const char *text, Pattern &out);

}  // namespace patterns
