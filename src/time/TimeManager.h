
#ifndef TIMEMANAGER_H
#define TIMEMANAGER_H


class TimeManager {
public:
    static long lastFrameTime;
    static double deltaTimeMilis;

    static void endFrame();
    static double getDeltaTimeMilis();
    static float getFPS();
    static void startup();
    static long getCurrentTimeMicro();
};


#endif //TIMEMANAGER_H
