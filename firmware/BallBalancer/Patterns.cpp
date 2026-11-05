#include "Patterns.h"

#include <math.h>
#include <string.h>

namespace {

constexpr float PI_F = 3.14159265358979f;

// Keep the path well inside the panel. The ball overshoots on every corner,
// and running the setpoint out to the edge is how you lose it.
constexpr float RADIUS_MM = 45.0f;
constexpr float PERIOD_S = 12.0f;

// Walks a closed polygon at constant speed. Corners are where patterns go
// wrong, so each leg gets equal time rather than equal length.
Setpoint polygon(const float *xs, const float *ys, int count, float seconds) {
  const float t = fmodf(seconds / PERIOD_S, 1.0f) * (float)count;
  const int leg = (int)t;
  const float f = t - (float)leg;
  const int a = leg % count;
  const int b = (leg + 1) % count;
  Setpoint out;
  out.x = xs[a] + (xs[b] - xs[a]) * f;
  out.y = ys[a] + (ys[b] - ys[a]) * f;
  return out;
}

}  // namespace

namespace patterns {

Setpoint evaluate(Pattern pattern, float seconds) {
  Setpoint out = {0.0f, 0.0f};
  const float w = 2.0f * PI_F / PERIOD_S;

  switch (pattern) {
    case Pattern::Hold:
      break;

    case Pattern::Line:
      out.x = RADIUS_MM * sinf(w * seconds);
      out.y = 0.0f;
      break;

    case Pattern::Circle:
      out.x = RADIUS_MM * cosf(w * seconds);
      out.y = RADIUS_MM * sinf(w * seconds);
      break;

    case Pattern::Ellipse:
      out.x = RADIUS_MM * cosf(w * seconds);
      out.y = RADIUS_MM * 0.55f * sinf(w * seconds);
      break;

    case Pattern::Triangle: {
      static const float xs[3] = {0.0f, -38.9f, 38.9f};
      static const float ys[3] = {RADIUS_MM, -22.5f, -22.5f};
      out = polygon(xs, ys, 3, seconds);
      break;
    }

    case Pattern::Rectangle: {
      static const float xs[4] = {-50.0f, 50.0f, 50.0f, -50.0f};
      static const float ys[4] = {-35.0f, -35.0f, 35.0f, 35.0f};
      out = polygon(xs, ys, 4, seconds);
      break;
    }

    case Pattern::FigureEight:
      // Lemniscate of Gerono. Stays smooth through the crossing, which the
      // polar form does not.
      out.x = RADIUS_MM * sinf(w * seconds);
      out.y = RADIUS_MM * 0.6f * sinf(2.0f * w * seconds);
      break;
  }

  return out;
}

const char *name(Pattern pattern) {
  switch (pattern) {
    case Pattern::Hold: return "hold";
    case Pattern::Line: return "line";
    case Pattern::Circle: return "circle";
    case Pattern::Ellipse: return "ellipse";
    case Pattern::Triangle: return "triangle";
    case Pattern::Rectangle: return "rectangle";
    case Pattern::FigureEight: return "figure8";
  }
  return "hold";
}

bool fromName(const char *text, Pattern &out) {
  struct Entry { const char *name; Pattern value; };
  static const Entry table[] = {
    {"hold", Pattern::Hold},
    {"line", Pattern::Line},
    {"circle", Pattern::Circle},
    {"ellipse", Pattern::Ellipse},
    {"triangle", Pattern::Triangle},
    {"rectangle", Pattern::Rectangle},
    {"figure8", Pattern::FigureEight},
  };

  for (unsigned i = 0; i < sizeof(table) / sizeof(table[0]); i++) {
    if (strcmp(text, table[i].name) == 0) {
      out = table[i].value;
      return true;
    }
  }
  return false;
}

}  // namespace patterns
