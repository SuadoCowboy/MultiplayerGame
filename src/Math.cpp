#include <bits/stdc++.h>
#include "Math.h"

float lerp(const float& start, const float& end, const float& amount) {
    return start + (end - start) * amount;
}

rl::Vector2 lerpVec2(const rl::Vector2& start, const rl::Vector2& end, const float& amount) {
    return (rl::Vector2){lerp(start.x, end.x, amount), lerp(start.y, end.y, amount)};
}

double distance2D(const double& x1, const double& y1, const double& x2, const double& y2) {
    return std::sqrt(std::pow((x2 - x1), 2) + std::pow((y2 - y1), 2));
}