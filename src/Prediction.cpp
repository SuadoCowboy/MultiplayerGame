#include "Prediction.h"

PredictionData predictedData[predictedDirBufferSize];

enet_uint16 nextPredictionId;

void recordPrediction(enet_uint8 dir, float x, float y) {
    predictedData[nextPredictionId].dir = dir;
    predictedData[nextPredictionId].position = {x, y};

    nextPredictionId = (nextPredictionId + 1) & predictedDirBufferMask;
}