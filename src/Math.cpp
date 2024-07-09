#include "Math.h"

float lerp(float start, float end, float amount) {
    return start + (end - start) * amount;
}