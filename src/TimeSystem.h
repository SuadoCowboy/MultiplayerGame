#pragma once

#include <chrono>

// @brief Server-side tick handler using std::chrono
class TickHandler {
public:
    TickHandler() {}
    TickHandler(const std::chrono::duration<int64_t, std::milli>& tickInterval)
        : tickInterval(tickInterval) {}

    /// @brief sets lastTick to the current time
    /// @warning should be called before shouldTick
    void start() {
        lastTick = std::chrono::system_clock::now();
    }
    
    bool shouldTick();
    
    // example: 1s/20(ticks) -> 20 ticks per second = 0.05s = 50ms tickInterval
    std::chrono::duration<int64_t, std::milli> tickInterval = std::chrono::milliseconds(16);
private:
    std::chrono::system_clock::time_point lastTick;
};

/// @brief Client-side tick handler using deltaTime specially the raylib one
struct DeltaTimedTickHandler {
    bool shouldTick(const float& dt);

    float tickInterval = 1.0f;
    float counter = 0.0f;
};

void preciseSleep(double seconds);
void timerSleep(double seconds);