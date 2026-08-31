#ifndef LED_RGB_H
#define LED_RGB_H

#include <Arduino.h>

static constexpr int LED_RGB_LEDC_RESOLUTION = 8;  // 8 bits

class LED_RGB {
public:
    LED_RGB(int pinRed, int pinGreen, int pinBlue)
        : _pinR(pinRed), 
          _pinG(pinGreen),
          _pinB(pinBlue),
          _blinkInterval(0),
          _lastToggle(0),
          _blinkEnabled(false),
          _state(false),
          _targetR(0),
          _targetG(0),
          _targetB(0) { }

    void begin(){
        ledcAttach(_pinR, 5000, LED_RGB_LEDC_RESOLUTION);
        ledcAttach(_pinG, 5000, LED_RGB_LEDC_RESOLUTION);
        ledcAttach(_pinB, 5000, LED_RGB_LEDC_RESOLUTION);
    }

    void setColor(uint8_t red, uint8_t green, uint8_t blue){
        _targetR = red;
        _targetG = green;
        _targetB = blue;

        if (!_blinkEnabled && !_forceMode) {
            _writePWM(_pinR, _targetR);
            _writePWM(_pinG, _targetG);
            _writePWM(_pinB, _targetB);
            _state = true;
        }
    }

    void setBlinkInterval(unsigned long intervalMs){
        _blinkInterval = intervalMs;
    }

    void update(){
        if (!_blinkEnabled || _forceMode) return;

        unsigned long currentMillis = millis();

        if (currentMillis - _lastToggle >= _blinkInterval) { // TODO: Talvez mudar pra Timer.h
            _lastToggle = currentMillis;
            _state = !_state; 

            _writePWM(_pinR, (_state) ? _targetR : 0);
            _writePWM(_pinG, (_state) ? _targetG : 0);
            _writePWM(_pinB, (_state) ? _targetB : 0);
        }
    }

    void setBlinkEnabled(bool enabled){
        _blinkEnabled = enabled;

        if (!enabled && !_forceMode) {
            _writePWM(_pinR, _targetR);
            _writePWM(_pinG, _targetG);
            _writePWM(_pinB, _targetB);
            _state = true;
        }
    }

    // TODO: Rever funcionamento do forceColor para a ativação do SKIB
    void forceColor(bool force, uint8_t r, uint8_t g, uint8_t b){ 
        _forceMode = force;

        if (_forceMode) {
            _forcedR = r;
            _forcedG = g;
            _forcedB = b;

            _writePWM(_pinR, _forcedR);
            _writePWM(_pinG, _forcedG);
            _writePWM(_pinB, _forcedB);
            _state = true;
        }
    }

private:
    int _pinR, _pinG, _pinB;
    int _channelR, _channelG, _channelB; // TODO: Talvez remover variáveis (Core 3.x não usa channel)
    unsigned long _blinkInterval;
    unsigned long _lastToggle;
    bool _blinkEnabled;
    bool _state;
    uint8_t _targetR, _targetG, _targetB;
    uint8_t _forcedR, _forcedG, _forcedB;
    bool _forceMode;

    void _writePWM(int pin, uint8_t value){
        ledcWrite(pin, value);
    }
};

#endif