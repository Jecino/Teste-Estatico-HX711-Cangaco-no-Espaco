#ifndef RECOVERY_SYSTEM_H
#define RECOVERY_SYSTEM_H

#include <Arduino.h>
#include "../hardwareParts/Buzzer.h"

class RecoverySystem {
public:
    RecoverySystem(int skibPin, Buzzer* buzzer);
    void begin();                      // Primeira instrução do setup() — GPIO LOW antes de tudo
    void activate(double maxAltitude);
    void update();
    bool isActivated() const;
    bool isSkibActive() const;

private:
    int _skibPin;
    Buzzer* _buzzer;
    bool _activated;
    bool _skibActive;
    unsigned long _skibActivationTime;
    double _maxAltitude;

    void _activateSkib();
    void _deactivateSkib();
};

#endif