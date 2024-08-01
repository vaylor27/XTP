
#include "TimeManager.h"

#include <chrono>

long TimeManager::lastFrameTime;
long TimeManager::deltaTimeNanos;
long TimeManager::timeAtProgramStart;

void TimeManager::endFrame() {
    const long currentFrameTime = getCurrentTimeNano();
    deltaTimeNanos = currentFrameTime - lastFrameTime;
    lastFrameTime = currentFrameTime;
}

long TimeManager::getDeltaTimeMilis() {
    return deltaTimeNanos;
}

long TimeManager::getTimeSinceProgramStart() {
    return getCurrentTimeNano() - timeAtProgramStart;
}

void TimeManager::startup() {
    const long curTime = getCurrentTimeNano();
    lastFrameTime = curTime;
    timeAtProgramStart = curTime;
}

long TimeManager::getCurrentTimeNano() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}
