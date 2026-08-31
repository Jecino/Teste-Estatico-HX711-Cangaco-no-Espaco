#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <Arduino.h>
#include "../hardwareParts/LoRaRadio.h"
#include "../hardwareParts/GPS.h"
#include "../logic/FlightStateMachine.h"
#include "../utils/Timer.h"
#include "../utils/FlightStateUtils.h"

class Telemetry {
public:
    Telemetry(LoRaRadio* radio, GPSSensor* gps);
    void begin();
    void update(unsigned long flightTimeMs, double altitudeBaro, FlightState state);
    void setEnabled(bool enabled);

private:
    LoRaRadio* _radio;
    GPSSensor* _gps;
    bool _enabled;
    Timer _timer;

    String _buildPacket(unsigned long flightTimeMs, double altitudeBaro, FlightState state, const GPSData& gpsData);
};

#endif