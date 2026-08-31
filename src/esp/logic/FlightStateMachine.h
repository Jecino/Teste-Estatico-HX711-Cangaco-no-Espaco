#ifndef FLIGHT_STATE_MACHINE_H
#define FLIGHT_STATE_MACHINE_H

#include <Arduino.h>
#include "../config/Constants.h"

// Enum de domínio: pertence à lógica de voo, não a Constants.h
enum class FlightState : uint8_t {
    PRE_FLIGHT = 0,
    ASCENT     = 1,
    RECOVERY   = 2
};

class FlightStateMachine {
public:
    FlightStateMachine();

    // altitudeHistory: array circular com ALTITUDE_HISTORY_SIZE elementos
    // gerenciado internamente — não mais exposto no main.ino
    void update(double currentAltitude, double maxAltitude);

    FlightState getCurrentState() const;
    void forceState(FlightState newState);
    bool wasApogeeDetected() const;
    unsigned long getFlightStartTime() const;   // millis() da transição para ASCENT

    // Acesso ao histórico para DataStorage/DataLogger se necessário
    double getHistory(int index) const;

private:
    FlightState _state;
    bool _apogeeDetected;
    bool _armed;
    unsigned long _flightStartTime;

    // Histórico de altitudes suavizadas — gerenciado internamente
    double _altitudeHistory[ALTITUDE_HISTORY_SIZE];

    void _pushHistory(double altitude);
    bool _shouldTransitionToAscent(double altitude) const;
    bool _shouldTransitionToRecovery(double currentAlt, double maxAlt) const;
};

#endif