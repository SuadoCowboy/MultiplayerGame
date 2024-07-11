#pragma once

#include <chrono>

// @brief Server-side tick handler using std::chrono
class TickHandler {
public:
    TickHandler() {}
    TickHandler(const double& tickInterval) : tickInterval(tickInterval) {}

    /// @brief sets timeBegin to the current time
    /// @warning should be called before shouldTick
    void start() {
        timeBegin = std::chrono::system_clock::now();
        counter = 0.0;
    }
    
    /// @return amount of times that should tick
    unsigned short shouldTick();
    
    double tickInterval = 1000; // example: 1s/20(ticks) -> 20 ticks per second = 0.05s = 50ms tickInterval
private:
    double counter = 0.0; // ms; dt + dt + ... until tickInterval
    std::chrono::system_clock::time_point timeBegin;
};

void preciseSleep(double seconds);
void timerSleep(double seconds);