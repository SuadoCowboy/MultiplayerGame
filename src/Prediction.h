#pragma once

#include <enet/enet.h>

namespace rl {
    #include <raylib.h>
}

struct PredictionData {
    enet_uint8 dir;
    rl::Vector2 position;
};

constexpr enet_uint16 predictedDirBufferSize = 512;
constexpr enet_uint16 predictedDirBufferMask = predictedDirBufferSize - 1;
extern PredictionData predictedData[predictedDirBufferSize];
extern enet_uint16 nextPredictionId;

void recordPrediction(enet_uint8 dir, float x, float y);