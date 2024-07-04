#include "TimeSystem.h"

#include <chrono>
#include <thread>
#include <math.h>

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

// https://blat-blatnik.github.io/computerBear/making-accurate-sleep-function
void preciseSleep(double seconds) {
    using namespace std;
    using namespace std::chrono;

    static double estimate = 5e-3;
    static double mean = 5e-3;
    static double m2 = 0;
    static int64_t count = 1;

    while (seconds > estimate) {
        auto start = std::chrono::high_resolution_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        auto end = std::chrono::high_resolution_clock::now();

        double observed = (end - start).count() / 1e9;
        seconds -= observed;

        ++count;
        double delta = observed - mean;
        mean += delta / count;
        m2   += delta * (observed - mean);
        double stddev = sqrt(m2 / (count - 1));
        estimate = mean + stddev;
    }

    // spin lock
    auto start = high_resolution_clock::now();
    while ((high_resolution_clock::now() - start).count() / 1e9 < seconds);
}