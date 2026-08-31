#ifndef CONSTANTS_H
#define CONSTANTS_H

// ========== PARÂMETROS DE VOO ==========
constexpr float ASCENT_THRESHOLD            = 5.0f;   // Altitude para detectar decolagem (m)
constexpr float DESCENT_DETECTION_THRESHOLD = 5.0f;   // Queda para detectar apogeu (m)
constexpr int   WINDOW_SIZE                 = 5;      // Janela da média móvel
constexpr int   ALTITUDE_HISTORY_SIZE       = 5;      // Tamanho do histórico de altitudes (detecção de apogeu)

// ========== PARÂMETROS DE TEMPORIZAÇÃO ==========
constexpr unsigned long SENSOR_READ_INTERVAL_MS      = 50;   // 20 Hz
constexpr unsigned long RECORD_INTERVAL_ASCENT_MS    = 50;   // 20 Hz na subida
constexpr unsigned long RECORD_INTERVAL_DESCENT_MS   = 200;  // 5 Hz na descida
constexpr unsigned long RECORD_INTERVAL_SD_MS        = 10;   // 100 Hz no SD
constexpr unsigned long LORA_TX_INTERVAL_MS          = 1000; // 1 Hz LoRa

// ========== PARÂMETROS DO SKIB ==========
constexpr unsigned long SKIB_DEACTIVATION_TIME_MS = 2000;  // Tempo ativo antes de desligar (ms)
constexpr unsigned long SKIB_BUZZER_DURATION_MS   = 4000;  // Buzzer emergência (ms)

// ========== PARÂMETROS DE FEEDBACK ==========
// LED blink intervals (ms)
constexpr unsigned long LED_BLINK_PRE_FLIGHT_MS = 1000;
constexpr unsigned long LED_BLINK_ASCENT_MS     = 250;
constexpr unsigned long LED_BLINK_RECOVERY_MS   = 1500;

// Buzzer frequencies (Hz)
constexpr unsigned int BEEP_FREQ_PRE_FLIGHT = 1500;
constexpr unsigned int BEEP_FREQ_ASCENT     = 2093;
constexpr unsigned int BEEP_FREQ_RECOVERY   = 3136;
constexpr unsigned int BEEP_FREQ_SKIB       = 2637;

// Buzzer timing (ms)
constexpr unsigned long BEEP_PERIOD_PRE_FLIGHT_MS = 3000;
constexpr unsigned long BEEP_PERIOD_ASCENT_MS     = 1000;
constexpr unsigned long BEEP_PERIOD_RECOVERY_MS   = 500;
constexpr unsigned long BEEP_DURATION_MS          = 400;

// ========== PARÂMETROS DE ARMAZENAMENTO ==========
constexpr int   MAX_FLIGHT_DATA_POINTS  = 497;   // Pontos na EEPROM
constexpr int   PRE_FLIGHT_QUEUE_SIZE   = 4;     // Pontos pré-voo (altitude E aceleração)
constexpr float ALTITUDE_SCALE_FACTOR  = 10.0f;  // Para conversão short int

// ========== COMUNICAÇÃO SERIAL ==========
constexpr unsigned long GPS_UART_BAUD  = 9600;   // Baudrate UART GPS NEO-M6N
constexpr unsigned long LORA_UART_BAUD = 9600;   // Baudrate UART E220-900T22D

// ========== PINAGEM (ESP32 DevKit V1) ==========
// SKIB e Feedback
constexpr int PIN_SKIB      = 4;
constexpr int PIN_BUZZER    = 13;
constexpr int PIN_LED_RED   = 26;
constexpr int PIN_LED_GREEN = 25;
constexpr int PIN_LED_BLUE  = 27;

// SD Card (SPI)
constexpr int PIN_SD_CS    = 5;
constexpr int PIN_SPI_MOSI = 23;
constexpr int PIN_SPI_MISO = 19;
constexpr int PIN_SPI_SCK  = 18;

// I2C (BMP180/280 + MPU6050)
constexpr int PIN_I2C_SDA = 21;
constexpr int PIN_I2C_SCL = 22;

// GPS (UART2)
constexpr int PIN_GPS_RX = 16; // TODO: Rever pino do GPS
constexpr int PIN_GPS_TX = 17; // TODO: Rever pino do GPS

// LoRa E220 (UART1)
constexpr int PIN_LORA_RX = 16;   // TX do módulo → RX do ESP
constexpr int PIN_LORA_TX = 17;   // RX do módulo → TX do ESP
constexpr int PIN_LORA_M0 = 32;
constexpr int PIN_LORA_M1 = 33;
constexpr int PIN_LORA_AUX = 34;

#endif
