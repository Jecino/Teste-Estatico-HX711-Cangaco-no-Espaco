#ifndef LORA_RADIO_H
#define LORA_RADIO_H

#include <Arduino.h>
#include <EBYTE.h>

// Estrutura de dados do payload do LoRa
struct LoRaPayload {
    String message;
    unsigned long time;
};

class LoRaRadio {
public:
    LoRaRadio(int rxPin, int txPin, int m0Pin, int m1Pin, int auxPin)
        : _rxPin(rxPin),
          _txPin(txPin),
          _m0Pin(m0Pin),
          _m1Pin(m1Pin),
          _auxPin(auxPin),
          _lora(&Serial1, m0Pin, m1Pin, auxPin) {}

    // Inicializa e configura o módulo LoRa com parâmetros específicos para comunicação;
    // Exige um "baudrate" para configurar as portas RX e TX
    void begin(int baudrate) {
        Serial2.begin(baudrate, SERIAL_8N1, _rxPin, _txPin);

        _initialized = _lora.init();
        _lora.SetAirDataRate(ADR_2K);
        // TODO: Constantes devem ser implementadas em Constantes.h
        _lora.SetAddressH(1);
        _lora.SetAddressL(1);
        _lora.SetChannel(23);

        _lora.SaveParameters(TEMPORARY);
        
        //setModeNormal();
    }

    void setModeNormal() {
        //_lora.SetMode(MODE_NORMAL); 
    }
    
    // Eniva o payload de dados da mensagem
    bool transmit(const LoRaPayload& payload) {
        _lora.SendStruct(&payload, sizeof(payload));
    }

    // Verifica se o LoRa está inicializado
    bool isReady() const {
        return _initialized;
    }

private:
    int _rxPin, _txPin, _m0Pin, _m1Pin, _auxPin;
    EBYTE _lora;
    bool _initialized;

    // void _setMode(uint8_t m0, uint8_t m1) {}
};

#endif