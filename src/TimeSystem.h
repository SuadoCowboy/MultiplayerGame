#pragma once

struct TickHandler {
    TickHandler() {};
    TickHandler(const float& tickInterval);

    void update(const float& dt);
    bool shouldTick();
    
    float tickInterval = 1.0f; // example: 1.0f(a second) / 20(t) -> 20 ticks will be drawn in a second evenly
    float delta = 0.0f; // time passed since last tick
};

void preciseSleep(double seconds);