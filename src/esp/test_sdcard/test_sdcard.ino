// ============================================================================
// TEST SKETCH — Teste Isolado do Módulo SDCard
// ============================================================================
// Este sketch testa APENAS o módulo SDCard.h com dados mockados de voo.
//
// Conexões necessárias (SPI padrão ESP32):
//   SD_CS   → GPIO 5
//   MOSI    → GPIO 23
//   MISO    → GPIO 19
//   SCK     → GPIO 18
//   VCC     → 3.3V
//   GND     → GND
//
// Comandos via Serial Monitor (115200 baud, NL+CR):
//   w  → Escreve 1 linha de dados mockados
//   b  → Escreve um batch de 20 linhas (simula rajada de dados)
//   n  → Cria novo arquivo de log (testa auto-incremento)
//   s  → Mostra status do SD Card
//   l  → Lista arquivos na raiz do SD
//   r  → Lê e imprime o conteúdo do arquivo de log atual
//   h  → Mostra menu de ajuda
// ============================================================================

#include "../hardwareParts/SDCard.h"
#include <SD.h>

// ===== CONFIGURAÇÃO =====
static constexpr int TEST_SD_CS_PIN = 5;  // GPIO 5 — CS do SD Card

// ===== INSTÂNCIA =====
SDCard sdCard(TEST_SD_CS_PIN);

// ===== DADOS MOCKADOS =====
// Simula um perfil de voo: pré-voo → subida → apogeu → descida → recuperação
struct MockFlightPoint {
    float altitude;
    float accelX;
    float accelY;
    float accelZ;
    const char* state;
};

static const MockFlightPoint MOCK_FLIGHT[] = {
    {   0.0,  0.00,  0.00,  9.81, "PRE_FLIGHT"  },
    {   5.2,  0.10,  0.05, 15.30, "ASCENT"       },
    {  45.8,  0.08,  0.03, 18.50, "ASCENT"       },
    { 120.3,  0.05,  0.02, 12.10, "ASCENT"       },
    { 250.7,  0.03,  0.01,  5.40, "ASCENT"       },
    { 380.1, -0.02, -0.01,  2.10, "ASCENT"       },
    { 490.5, -0.05, -0.03,  0.50, "ASCENT"       },
    { 510.2, -0.08, -0.04, -0.30, "ASCENT"       },
    { 512.0,  0.00,  0.00, -1.20, "APOGEE"       },
    { 508.3,  0.02,  0.01, -3.50, "DESCENT"      },
    { 480.1,  0.05,  0.03, -5.80, "DESCENT"      },
    { 420.6,  0.08,  0.04, -7.20, "DESCENT"      },
    { 350.2,  0.10,  0.05, -8.10, "DESCENT"      },
    { 270.9,  0.12,  0.06, -6.50, "DESCENT"      },
    { 180.4,  0.08,  0.04, -4.20, "DESCENT"      },
    { 100.1,  0.05,  0.03, -3.80, "DESCENT"      },
    {  40.5,  0.03,  0.02, -3.50, "DESCENT"      },
    {  10.2,  0.01,  0.01, -2.00, "DESCENT"      },
    {   2.1,  0.00,  0.00, -0.50, "RECOVERY"     },
    {   0.0,  0.00,  0.00,  9.81, "RECOVERY"     },
};

static constexpr int MOCK_FLIGHT_SIZE = sizeof(MOCK_FLIGHT) / sizeof(MOCK_FLIGHT[0]);

// Contador de linhas escritas (para timestamp incremental)
static unsigned long lineCounter = 0;

// Nome do arquivo atual (para leitura)
static String currentLogName = "";

// ===== FUNÇÕES AUXILIARES =====

// Monta uma linha CSV a partir de um ponto mockado
String buildCSVLine(const MockFlightPoint& pt) {
    // Formato: Timestamp,Altitude,AccelX,AccelY,AccelZ,State
    String line = "";
    line += String(millis());
    line += ",";
    line += String(pt.altitude, 1);
    line += ",";
    line += String(pt.accelX, 2);
    line += ",";
    line += String(pt.accelY, 2);
    line += ",";
    line += String(pt.accelZ, 2);
    line += ",";
    line += pt.state;
    return line;
}

// Escreve uma única linha mockada
void writeSingleMockLine() {
    int idx = lineCounter % MOCK_FLIGHT_SIZE;
    String csvLine = buildCSVLine(MOCK_FLIGHT[idx]);

    Serial.print(F("[TEST] Escrevendo linha #"));
    Serial.print(lineCounter);
    Serial.print(F(" → "));
    Serial.println(csvLine);

    unsigned long t0 = micros();
    bool ok = sdCard.writeLine(csvLine);
    unsigned long elapsed = micros() - t0;

    if (ok) {
        Serial.print(F("[TEST] ✓ Gravado com sucesso em "));
        Serial.print(elapsed);
        Serial.println(F(" µs"));
    } else {
        Serial.println(F("[TEST] ✗ FALHA na gravação!"));
    }

    lineCounter++;
}

// Escreve batch de N linhas (simula rajada de dados de voo)
void writeBatch(int count) {
    Serial.print(F("\n[TEST] === BATCH: Escrevendo "));
    Serial.print(count);
    Serial.println(F(" linhas ==="));

    unsigned long t0 = millis();
    int successCount = 0;

    for (int i = 0; i < count; i++) {
        int idx = lineCounter % MOCK_FLIGHT_SIZE;
        String csvLine = buildCSVLine(MOCK_FLIGHT[idx]);
        if (sdCard.writeLine(csvLine)) {
            successCount++;
        }
        lineCounter++;
    }

    unsigned long elapsed = millis() - t0;

    Serial.print(F("[TEST] Batch completo: "));
    Serial.print(successCount);
    Serial.print(F("/"));
    Serial.print(count);
    Serial.print(F(" linhas em "));
    Serial.print(elapsed);
    Serial.print(F(" ms (média: "));
    Serial.print(elapsed / count);
    Serial.println(F(" ms/linha)"));
}

// Lista arquivos na raiz do SD
void listFiles() {
    Serial.println(F("\n[TEST] === Arquivos no SD Card ==="));
    File root = SD.open("/");
    if (!root) {
        Serial.println(F("[TEST] Erro ao abrir diretório raiz"));
        return;
    }

    int fileCount = 0;
    File entry = root.openNextFile();
    while (entry) {
        Serial.print(F("  "));
        Serial.print(entry.name());
        if (entry.isDirectory()) {
            Serial.println(F("/"));
        } else {
            Serial.print(F("  ("));
            Serial.print(entry.size());
            Serial.println(F(" bytes)"));
        }
        fileCount++;
        entry = root.openNextFile();
    }
    root.close();

    Serial.print(F("[TEST] Total: "));
    Serial.print(fileCount);
    Serial.println(F(" arquivo(s)"));
}

// Lê e imprime o conteúdo do arquivo de log atual
void readCurrentLog() {
    Serial.println(F("\n[TEST] === Conteúdo do Log Atual ==="));

    // Procura o arquivo de log mais recente (último LOG_XX.CSV existente)
    char filename[13];
    String lastFile = "";
    for (int i = 0; i < SD_MAX_LOG_FILES; i++) {
        snprintf(filename, sizeof(filename), "/LOG_%02d.CSV", i);
        if (SD.exists(filename)) {
            lastFile = String(filename);
        } else {
            break;  // Parar no primeiro que não existe
        }
    }

    if (lastFile.length() == 0) {
        Serial.println(F("[TEST] Nenhum arquivo de log encontrado"));
        return;
    }

    Serial.print(F("[TEST] Lendo: "));
    Serial.println(lastFile);

    File f = SD.open(lastFile, FILE_READ);
    if (!f) {
        Serial.println(F("[TEST] Erro ao abrir arquivo"));
        return;
    }

    int lineNum = 0;
    while (f.available()) {
        String line = f.readStringUntil('\n');
        Serial.print(F("  ["));
        Serial.print(lineNum);
        Serial.print(F("] "));
        Serial.println(line);
        lineNum++;

        // Limitar impressão para não travar o Serial
        if (lineNum > 50) {
            Serial.println(F("  ... (truncado após 50 linhas)"));
            break;
        }
    }

    Serial.print(F("[TEST] Tamanho: "));
    Serial.print(f.size());
    Serial.println(F(" bytes"));

    f.close();
}

// Mostra status do SD Card
void showStatus() {
    Serial.println(F("\n[TEST] === Status do SD Card ==="));
    Serial.print(F("  SD pronto:        "));
    Serial.println(sdCard.isReady() ? "SIM" : "NÃO");
    Serial.print(F("  Linhas escritas:  "));
    Serial.println(lineCounter);
    Serial.print(F("  Uptime:           "));
    Serial.print(millis() / 1000);
    Serial.println(F(" s"));

    // Info do cartão
    uint8_t cardType = SD.cardType();
    Serial.print(F("  Tipo do cartão:   "));
    switch (cardType) {
        case CARD_NONE:  Serial.println(F("Nenhum"));   break;
        case CARD_MMC:   Serial.println(F("MMC"));      break;
        case CARD_SD:    Serial.println(F("SD"));        break;
        case CARD_SDHC:  Serial.println(F("SDHC"));     break;
        default:         Serial.println(F("Desconhecido")); break;
    }

    uint64_t totalBytes = SD.totalBytes();
    uint64_t usedBytes  = SD.usedBytes();
    Serial.print(F("  Espaço total:     "));
    Serial.print((uint32_t)(totalBytes / (1024 * 1024)));
    Serial.println(F(" MB"));
    Serial.print(F("  Espaço usado:     "));
    Serial.print((uint32_t)(usedBytes / 1024));
    Serial.println(F(" KB"));
}

// Mostra menu de ajuda
void showHelp() {
    Serial.println(F("\n╔══════════════════════════════════════════════╗"));
    Serial.println(F("║     TESTE SD CARD — Cangaço no Espaço       ║"));
    Serial.println(F("╠══════════════════════════════════════════════╣"));
    Serial.println(F("║  w  → Escreve 1 linha de dados mockados     ║"));
    Serial.println(F("║  b  → Escreve batch de 20 linhas            ║"));
    Serial.println(F("║  n  → Cria novo arquivo de log              ║"));
    Serial.println(F("║  s  → Mostra status do SD Card              ║"));
    Serial.println(F("║  l  → Lista arquivos no SD                  ║"));
    Serial.println(F("║  r  → Lê conteúdo do log atual              ║"));
    Serial.println(F("║  h  → Mostra este menu                      ║"));
    Serial.println(F("╚══════════════════════════════════════════════╝"));
}

// ===== SETUP =====
void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println(F("\n"));
    Serial.println(F("================================================"));
    Serial.println(F("   TESTE ISOLADO — Módulo SD Card               "));
    Serial.println(F("   Computador de Bordo — Cangaço no Espaço      "));
    Serial.println(F("================================================"));
    Serial.println(F(""));

    // Definir cabeçalho CSV antes de inicializar
    sdCard.setHeader("Timestamp,Altitude,AccelX,AccelY,AccelZ,State");

    // Tentar inicializar SD Card
    Serial.println(F("[INIT] Inicializando SD Card..."));
    bool ok = sdCard.begin();

    if (ok) {
        Serial.println(F("[INIT] ✓ SD Card inicializado com sucesso!"));
        showStatus();
    } else {
        Serial.println(F("[INIT] ✗ SD Card FALHOU!"));
        Serial.println(F("[INIT] Verifique:"));
        Serial.println(F("  1. Cartão SD está inserido no módulo?"));
        Serial.println(F("  2. Cartão está formatado em FAT32?"));
        Serial.println(F("  3. Conexões SPI estão corretas?"));
        Serial.println(F("     CS=GPIO5, MOSI=GPIO23, MISO=GPIO19, SCK=GPIO18"));
        Serial.println(F("  4. Alimentação 3.3V está estável?"));
    }

    showHelp();
    Serial.println(F("\nAguardando comando... (digite 'h' para ajuda)\n"));
}

// ===== LOOP =====
void loop() {
    if (Serial.available()) {
        char cmd = Serial.read();

        // Ignorar caracteres de controle (newline, carriage return)
        if (cmd == '\n' || cmd == '\r') {
            return;
        }

        switch (cmd) {
            case 'w':
            case 'W':
                writeSingleMockLine();
                break;

            case 'b':
            case 'B':
                writeBatch(20);
                break;

            case 'n':
            case 'N':
                Serial.println(F("\n[TEST] Criando novo arquivo de log..."));
                sdCard.createNewLogFile();
                lineCounter = 0;  // Reset do contador
                break;

            case 's':
            case 'S':
                showStatus();
                break;

            case 'l':
            case 'L':
                listFiles();
                break;

            case 'r':
            case 'R':
                readCurrentLog();
                break;

            case 'h':
            case 'H':
                showHelp();
                break;

            default:
                Serial.print(F("[TEST] Comando desconhecido: '"));
                Serial.print(cmd);
                Serial.println(F("' — digite 'h' para ajuda"));
                break;
        }

        Serial.println();
    }
}
