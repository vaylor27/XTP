
#include "TimeManager.h"

#include <chrono>

long TimeManager::lastFrameTime;
double TimeManager::deltaTimeMilis;

void TimeManager::endFrame() {
    const long currentFrameTime = getCurrentTimeMicro();
    deltaTimeMilis = static_cast<double>(currentFrameTime - lastFrameTime) / 1000;
    lastFrameTime = currentFrameTime;
}

double TimeManager::getDeltaTimeMilis() {
    return deltaTimeMilis;
}

float TimeManager::getFPS() {
    const double frameTimeMilis = getDeltaTimeMilis();
    return static_cast<float>(1000 / frameTimeMilis);
}

void TimeManager::startup() {
    lastFrameTime = getCurrentTimeMicro();
}

long TimeManager::getCurrentTimeMicro() {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}
