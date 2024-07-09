
#include "TimeManager.h"

#include <chrono>

long TimeManager::lastFrameTime;
double TimeManager::deltaTimeMilis;
double TimeManager::additiveFPS;
long TimeManager::firstAverageSampleTime;
long TimeManager::timeAtProgramStart;
int TimeManager::averageFPSSampleCount;

void TimeManager::computeAverageFPS() {
    long curTime = getCurrentTimeMicro();
    // if (firstAverageSampleTime - curTime >= 10000000) {
        // additiveFPS = 0;
        // firstAverageSampleTime = curTime;
        // averageFPSSampleCount = 0;
    // }

    additiveFPS += getFPS();
    averageFPSSampleCount += 1;
}

void TimeManager::endFrame() {
    const long currentFrameTime = getCurrentTimeMicro();
    deltaTimeMilis = static_cast<double>(currentFrameTime - lastFrameTime) / 1000;
    computeAverageFPS();
    lastFrameTime = currentFrameTime;
}

double TimeManager::getDeltaTimeMilis() {
    return deltaTimeMilis;
}

float TimeManager::getFPS() {
    const double frameTimeMilis = getDeltaTimeMilis();
    return static_cast<float>(1000 / frameTimeMilis);
}

long TimeManager::getTimeSinceProgramStart() {
    return getCurrentTimeMicro() - timeAtProgramStart;
}

double TimeManager::getAverageFPS() {
    return additiveFPS / averageFPSSampleCount;
}

void TimeManager::startup() {
    long curTime = getCurrentTimeMicro();
    lastFrameTime = curTime;
    timeAtProgramStart = curTime;
}

long TimeManager::getCurrentTimeMicro() {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}
