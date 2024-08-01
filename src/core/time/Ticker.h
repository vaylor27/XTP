
#ifndef TICKER_H
#define TICKER_H

#include <iostream>

class Ticker {
public:
    long interval;
    std::function<void()> func;
    long timeIntervalStarted;

    Ticker(const std::chrono::nanoseconds period, const std::function<void()>& function) {
        interval = period.count();
        func = function;
        timeIntervalStarted = getNanoTime().count();
    }

    static std::chrono::nanoseconds getNanoTime() {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch());
    }

    void tryExecute() {
        if (!func) {
            return;
        }

        if (getNanoTime().count() - timeIntervalStarted >= interval) {
            func();
            timeIntervalStarted = getNanoTime().count();
        }
    }

    void resetTime() {
        timeIntervalStarted = getNanoTime().count();
    }
};



#endif //TICKER_H
