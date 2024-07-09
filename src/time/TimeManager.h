
#ifndef TIMEMANAGER_H
#define TIMEMANAGER_H


class TimeManager {
public:
    static long lastFrameTime;
    static double deltaTimeMilis;
    static double additiveFPS;
    static int averageFPSSampleCount;
    static long timeAtProgramStart;

    static void computeAverageFPS();
    static void endFrame();
    static double getDeltaTimeMilis();
    static float getFPS();

    static long getTimeSinceProgramStart();

    static double getAverageFPS();
    static void startup();
    static long getCurrentTimeMicro();
private:
    static long firstAverageSampleTime;
};


#endif //TIMEMANAGER_H
