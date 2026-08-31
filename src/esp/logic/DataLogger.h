#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include <Arduino.h>
#include "../hardwareParts/SDCard.h"
#include "../hardwareParts/Accelerometer.h"
#include "../hardwareParts/GPS.h"
#include "../logic/FlightStateMachine.h"
#include "../utils/Timer.h"
#include "../utils/FlightStateUtils.h"
#include "../config/Constants.h"

class DataLogger {
public:
    DataLogger(SDCard* sdCard);
    void begin();

    /**
     * update() — Grava uma linha CSV no SD Card se o timer estiver pronto.
     *
     * @param flightTimeMs  Tempo de voo em milissegundos (fonte: FlightStateMachine)
     * @param altitude      Altitude barométrica suavizada (metros)
     * @param pressure      Pressão bruta do BMP280 (hPa) — usada apenas em FULL_DATA_LOGGING
     * @param accel         Dados do MPU6050 (aceleração + giroscópio + temperatura)
     * @param gps           Dados do GPS NEO-M6N (lat, lon, alt, speed, fix)
     * @param state         Estado atual da máquina de estados de voo
     */
    void update(unsigned long flightTimeMs, double altitude, double pressure,
                const AccelData& accel, const GPSData& gps, FlightState state);

    void setEnabled(bool enabled);

private:
    SDCard* _sdCard;
    bool _enabled;
    Timer _timer;

    String _formatLine(unsigned long flightTimeMs, double altitude, double pressure,
                       const AccelData& accel, const GPSData& gps, FlightState state);
};

#endif
