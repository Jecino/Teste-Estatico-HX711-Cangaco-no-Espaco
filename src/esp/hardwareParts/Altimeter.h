//TODO: documentar o código direito

#ifndef ALTIMETER_H
#define ALTIMETER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP280.h> 

static constexpr int WINDOW_SIZE = 5;

class Altimeter {
public:

    Altimeter() 
        : _connected(false), 
          _baseline(1013.25), 
          _indiceFila(0), 
          _leiturasTotais(0) 
    {
        
        for (int i = 0; i < WINDOW_SIZE; i++) {
            _fila[i] = 0.0;
            _alturasSuavizadas[i] = 0.0;
        }
    }

    bool begin() {
        _connected = _sensor.begin(); 
        
        if (_connected) {
            _sensor.setSampling(Adafruit_BMP280::MODE_NORMAL,     
                                Adafruit_BMP280::SAMPLING_X2,     // Oversampling Temp
                                Adafruit_BMP280::SAMPLING_X16,    // Oversampling Pressão (máx precisão)
                                Adafruit_BMP280::FILTER_X16,      // Filtro anti-turbulência no hardware
                                Adafruit_BMP280::STANDBY_MS_1);   // Leituras super rápidas
            resetBaseline(); 
        }
        return _connected;
    }

    bool isConnected() const {
        return _connected;
    }

    double readPressure() {
        if (!_connected) return -1.0;
        
        double p = _sensor.readPressure() / 100.0;
        return (p > 0) ? p : -1.0;
    }

    double calculateAltitude(double pressure, double baseline) {
        if (pressure <= 0) return 0.0;
        
        return 44330.0 * (1.0 - pow(pressure / baseline, 0.190295));
    }

    double updateSmoothedAltitude(double rawAltitude) {
        _fila[_indiceFila] = rawAltitude;
        _indiceFila = (_indiceFila + 1) % WINDOW_SIZE;
        
        if (_leiturasTotais < WINDOW_SIZE) {
            _leiturasTotais++;
        }

        double soma = 0;
        for (int i = 0; i < _leiturasTotais; i++) {
            soma += _fila[i];
        }
        double altitudeSuavizada = soma / _leiturasTotais;

        for (int i = WINDOW_SIZE - 1; i > 0; i--) {
            _alturasSuavizadas[i] = _alturasSuavizadas[i - 1];
        }
        _alturasSuavizadas[0] = altitudeSuavizada;

        return altitudeSuavizada;
    }

    void resetBaseline() {
        double p = readPressure();
        if (p > 0) {
            _baseline = p;
        }
    }

    double getBaseline() const {
        return _baseline;
    }

    double getHistoricalAltitude(int index) const {
        if (index >= 0 && index < WINDOW_SIZE) {
            return _alturasSuavizadas[index];
        }
        return 0.0;
    }

private:
    Adafruit_BMP280 _sensor;
    bool _connected;
    double _baseline;

    double _fila[WINDOW_SIZE];
    int _indiceFila;
    int _leiturasTotais;
    double _alturasSuavizadas[WINDOW_SIZE];
};

#endif
