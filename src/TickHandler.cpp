#include "TickHandler.h"

TickHandler::TickHandler(const float& tickInterval) : tickInterval(tickInterval) {}

void TickHandler::update(const float& dt) {
    delta += dt;
}

bool TickHandler::shouldTick() {
    if (delta >= tickInterval) {
        delta -= tickInterval;
        return true;
    }
    return false;
}