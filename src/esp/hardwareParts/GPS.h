#ifndef GPS_SENSOR_H
#define GPS_SENSOR_H

#include <Arduino.h>
#include <TinyGPS++.h>

#include <HardwareSerial.h>

struct GPSData {
    double latitude;
    double longitude;
    double altitude;   // metros
    double speed;      // km/h
    uint8_t fixQuality;
    bool isValid;
};

class GPSSensor {
public:
    GPSSensor(int rxPin, int txPin)
        : _rxPin(rxPin),
          _txPin(txPin),
          _gpsSerial(2),
          _fixQualityGPGGA(_gps, "GPGGA", 6),
          _fixQualityGNGGA(_gps, "GNGGA", 6),
          _lastValidTime(0)
    {
        _latestData.latitude = 0.0;
        _latestData.longitude = 0.0;
        _latestData.altitude = 0.0;
        _latestData.speed = 0.0;
        _latestData.fixQuality = 0;
        _latestData.isValid = false;
    }

    void begin(unsigned long baudRate = 9600) {
        _gpsSerial.begin(baudRate, SERIAL_8N1, _rxPin, _txPin);

        _latestData.latitude = 0.0;
        _latestData.longitude = 0.0;
        _latestData.altitude = 0.0;
        _latestData.speed = 0.0;
        _latestData.fixQuality = 0;
        _latestData.isValid = false;

        _lastValidTime = 0;
    }

    void update() {
        while (_gpsSerial.available() > 0) {
            char c = _gpsSerial.read();

            if (_gps.encode(c)) {
                _parseData();
            }
        }

        // Se passar muito tempo sem localização válida, considera sem fix
        if (_lastValidTime > 0 && millis() - _lastValidTime > 5000) {
            _latestData.isValid = false;
            _latestData.fixQuality = 0;
        }
    }

    GPSData getLatestData() const {
        return _latestData;
    }

    bool hasFix() const {
        return _latestData.isValid;
    }

private:
    int _rxPin;
    int _txPin;
    
    HardwareSerial _gpsSerial;

    TinyGPSPlus _gps;

    TinyGPSCustom _fixQualityGPGGA;
    TinyGPSCustom _fixQualityGNGGA;

    GPSData _latestData;
    unsigned long _lastValidTime;

    void _parseData() {
        if (_fixQualityGPGGA.isValid()) {
            _latestData.fixQuality = atoi(_fixQualityGPGGA.value());
        } else if (_fixQualityGNGGA.isValid()) {
            _latestData.fixQuality = atoi(_fixQualityGNGGA.value());
        }

        if (_gps.location.isValid()) {
            _latestData.latitude = _gps.location.lat();
            _latestData.longitude = _gps.location.lng();
        }

        if (_gps.altitude.isValid()) {
            _latestData.altitude = _gps.altitude.meters();
        }

        if (_gps.speed.isValid()) {
            _latestData.speed = _gps.speed.kmph();
        }

        bool locationOk = _gps.location.isValid() && _gps.location.age() < 3000;
        bool fixOk = _latestData.fixQuality > 0;

        if (locationOk && fixOk) {
            _latestData.isValid = true;
            _lastValidTime = millis();
        } else {
            _latestData.isValid = false;
        }
    }
};

#endif
