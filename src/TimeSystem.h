#pragma once

#include <chrono>

// @brief Server-side tick handler using std::chrono
class TickHandler {
public:
    TickHandler() {}
    TickHandler(const double& tickInterval)
        : tickInterval(tickInterval) {}

    /// @brief sets lastTick to the current time
    /// @warning should be called before shouldTick
    void start() {
        lastTick = std::chrono::system_clock::now();
    }
    
    bool shouldTick();
    
    // example: 1s/20(ticks) -> 20 ticks per second = 0.05s = 50ms tickInterval
    uint16_t tickInterval = 16;
private:
    std::chrono::system_clock::time_point lastTick;
};

void preciseSleep(double seconds);
void timerSleep(double seconds);