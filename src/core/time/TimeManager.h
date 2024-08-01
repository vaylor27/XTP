
#ifndef TIMEMANAGER_H
#define TIMEMANAGER_H


class TimeManager {
public:
    static long lastFrameTime;
    static long deltaTimeNanos;
    static long timeAtProgramStart;

    static void endFrame();
    static long getDeltaTimeMilis();

    static long getTimeSinceProgramStart();

    static void startup();
    static long getCurrentTimeNano();
};


#endif //TIMEMANAGER_H
