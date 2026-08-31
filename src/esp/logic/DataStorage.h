#ifndef DATA_STORAGE_H
#define DATA_STORAGE_H

#include <Arduino.h>
#include <EEPROM.h>
#include "../utils/CircularQueue.h"
#include "../hardwareParts/Accelerometer.h"
#include "../config/Constants.h"

class DataStorage {
public:
    DataStorage();
    void begin();   // EEPROM.begin(1000) — chamar ANTES de qualquer leitura

    // Fila pré-voo: altitude + aceleração (ESP32 grava ambas)
    void savePreFlightPoints(
        const CircularQueue<double, PRE_FLIGHT_QUEUE_SIZE>& altQueue,
        const CircularQueue<AccelData, PRE_FLIGHT_QUEUE_SIZE>& accelQueue
    );

    void saveAltitude(double altitude, unsigned int index);
    double readAltitude(unsigned int index);
    void saveMaxAltitude(double maxAltitude);
    double readMaxAltitude();
    void saveBaseline(double baseline);
    double readBaseline();
    unsigned int getRecordCount() const;
    void setRecordCount(unsigned int count);
    void incrementRecordCount();

private:
    unsigned int _recordCount;
    int _getAltitudeAddress(unsigned int index) const;
};

#endif