#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>

class Timer {
public:
    Timer(unsigned long intervalMs) {
        _interval = intervalMs;
        _lastTick = millis();
    }

    void setInterval(unsigned long intervalMs) {
        _interval = intervalMs;
    }

    // Retorna true se o intervalo passou; reinicia automaticamente
    bool isReady() {
        unsigned long currentTime = millis();

        if (currentTime - _lastTick >= _interval) {
            _lastTick = currentTime;
            return true;
        }

        return false;
    }           

    // Retorna true apenas uma vez; não reinicia até reset()
    bool isReadyOnce() {
        return millis() - _lastTick >= _interval;
    }
    
    void reset() {
        _lastTick = millis();
    }

    unsigned long elapsed() const{
        return millis() - _lastTick;
    }

private:
    unsigned long _interval;
    unsigned long _lastTick;
};

#endif