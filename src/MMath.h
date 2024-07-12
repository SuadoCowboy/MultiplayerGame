#pragma once

namespace rl {
    #include <raylib.h>
}

float lerp(const float& start, const float& end, const float& amount);
rl::Vector2 lerpVec2(const rl::Vector2& start, const rl::Vector2& end, const float& amount);

double distance2D(const double& x1, const double& y1, const double& x2, const double& y2);

/// @return x*x + y*y
float squaredVec2(const rl::Vector2& vector); 