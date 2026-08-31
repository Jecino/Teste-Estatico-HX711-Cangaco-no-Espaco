#ifndef SDCARD_H
#define SDCARD_H

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

// Tamanho máximo para o nome do arquivo LOG_XX.CSV
static constexpr int SD_MAX_LOG_FILES = 100;  // LOG_00 a LOG_99

class SDCard {
public:
    SDCard(int csPin)
        : _csPin(csPin),
          _ready(false),
          _header(""),
          _currentFileName("") {}

    // Inicializa SPI e SD Card, cria arquivo LOG_XX.CSV com auto-incremento
    // Retorna true se SD pronto para gravação
    bool begin() {
        if (!SD.begin(_csPin)) {
            Serial.println(F("[SDCard] Falha na inicialização do SD Card"));
            _ready = false;
            return false;
        }

        Serial.println(F("[SDCard] SD Card inicializado com sucesso"));
        _ready = true;

        // Criar arquivo de log com auto-incremento
        createNewLogFile();

        return _ready;
    }

    // Retorna true se SD está pronto para gravação
    bool isReady() const {
        return _ready;
    }

    // Grava linha no arquivo, faz flush imediato (segurança contra perda de energia)
    // PRD RF-05: flush forçado a cada gravação para evitar perda de dados
    bool writeLine(const String& line) {
        if (!_ready) {
            return false;
        }

        // Abordagem segura: open→write→flush→close a cada gravação (~3-10ms)
        // Possível mudança futura: manter arquivo aberto e fazer apenas flush (~1-5ms)
        // _logFile.println(line); _logFile.flush(); — sem open/close por chamada
        _logFile = SD.open(_currentFileName, FILE_APPEND);
        if (!_logFile) {
            Serial.println(F("[SDCard] Erro ao abrir arquivo para escrita"));
            return false;
        }

        _logFile.println(line);
        _logFile.flush();
        _logFile.close();

        return true;
    }

    // Força flush do buffer do arquivo
    void flush() {
        if (_logFile) {
            _logFile.flush();
        }
    }

    // Define cabeçalho CSV — será escrito na primeira linha de cada novo arquivo
    // Exemplo ESP32: "Timestamp,Altitude,AccelX,AccelY,AccelZ,State"
    void setHeader(const String& header) {
        _header = header;
    }

    // Cria novo arquivo de log (auto-incremento LOG_00.CSV a LOG_99.CSV)
    // Escreve cabeçalho se definido
    void createNewLogFile() {
        if (!_ready) {
            return;
        }

        _currentFileName = _findNextFileName();

        if (_currentFileName.length() == 0) {
            Serial.println(F("[SDCard] Limite de 100 arquivos de log atingido"));
            _ready = false;
            return;
        }

        // Criar arquivo e escrever cabeçalho
        _logFile = SD.open(_currentFileName, FILE_WRITE);
        if (!_logFile) {
            Serial.println(F("[SDCard] Erro ao criar arquivo de log"));
            _ready = false;
            return;
        }

        // Escrever cabeçalho CSV se definido
        if (_header.length() > 0) {
            _logFile.println(_header);
        }

        _logFile.flush();
        _logFile.close();

        Serial.print(F("[SDCard] Arquivo criado: "));
        Serial.println(_currentFileName);
    }

private:
    int _csPin;
    File _logFile;
    bool _ready;
    String _header;
    String _currentFileName;

    // Procura próximo nome de arquivo disponível: LOG_00.CSV, LOG_01.CSV, ..., LOG_99.CSV
    // Retorna String vazia se todos os 100 slots estão ocupados
    String _findNextFileName() {
        char filename[13];  // "/LOG_XX.CSV\0" = 12 chars + null

        for (int i = 0; i < SD_MAX_LOG_FILES; i++) {
            snprintf(filename, sizeof(filename), "/LOG_%02d.CSV", i);

            if (!SD.exists(filename)) {
                return String(filename);
            }
        }

        return String("");  // Todos os slots ocupados
    }
};

#endif