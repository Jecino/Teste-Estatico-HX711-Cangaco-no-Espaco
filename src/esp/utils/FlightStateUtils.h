#ifndef FLIGHT_STATE_UTILS_H
#define FLIGHT_STATE_UTILS_H

#include "../logic/FlightStateMachine.h"

namespace FlightStateUtils {
    // Para CSV do SD Card: "PRE_FLIGHT", "ASCENT", "RECOVERY"
    inline const char* toString(FlightState state) {
        switch(state) {
            case FlightState::PRE_FLIGHT: return "PRE_FLIGHT";
            case FlightState::ASCENT:     return "ASCENT";
            case FlightState::RECOVERY:   return "RECOVERY";
            default:                      return "UNKNOWN";
        }
    }

    // Para campo numérico do pacote LoRa: 0, 1, 2
    inline uint8_t toInt(FlightState state) {
        return static_cast<uint8_t>(state);
    }
}

#endif