/**
 * FlightStateMachine.cpp
 *
 * Implementação da máquina de estados de voo do Computador de Bordo ESP32.
 * Baseado no PRD RF-02 (seção 4.2) e no Projeto de Refatoração (seção 13).
 *
 * Máquina de 3 estados com transições unidirecionais:
 *   PRE_FLIGHT → ASCENT → RECOVERY
 *
 * Transições:
 *   PRE_FLIGHT → ASCENT:   altitude suavizada ≥ ASCENT_THRESHOLD (5.0m)
 *   ASCENT → RECOVERY:     altitude ≤ (maxAltitude - DESCENT_DETECTION_THRESHOLD) E armado == true
 *
 * O histórico de altitudes suavizadas (ALTITUDE_HISTORY_SIZE = 5) é gerenciado
 * internamente com shift à direita: índice [0] = mais recente, [4] = mais antigo.
 */

#include "FlightStateMachine.h"

// ============================================================================
// Construtor
// ============================================================================

FlightStateMachine::FlightStateMachine()
    : _state(FlightState::PRE_FLIGHT),
      _apogeeDetected(false),
      _armed(true),
      _flightStartTime(0)
{
    // Inicializa histórico de altitudes com zero
    for (int i = 0; i < ALTITUDE_HISTORY_SIZE; i++) {
        _altitudeHistory[i] = 0.0;
    }
}

// ============================================================================
// Atualização principal — chamada a cada ciclo de leitura do sensor
// ============================================================================

void FlightStateMachine::update(double currentAltitude, double maxAltitude) {
    // Atualiza o histórico interno de altitudes suavizadas
    _pushHistory(currentAltitude);

    switch (_state) {

        // ------------------------------------------------------------------
        // PRE_FLIGHT: Monitorando altitude, aguardando decolagem
        //   - Não grava EEPROM nem SD
        //   - Armazena na fila circular pré-voo (gerenciada externamente)
        //   - Transição: altitude ≥ ASCENT_THRESHOLD (5.0m)
        // ------------------------------------------------------------------
        case FlightState::PRE_FLIGHT:
            if (_shouldTransitionToAscent(currentAltitude)) {
                _state = FlightState::ASCENT;
                _flightStartTime = millis();

                Serial.println(F("[FSM] Transição: PRE_FLIGHT → ASCENT"));
                Serial.print(F("[FSM] Altitude de decolagem: "));
                Serial.print(currentAltitude, 1);
                Serial.println(F(" m"));
            }
            break;

        // ------------------------------------------------------------------
        // ASCENT: Capturando dados de voo em alta resolução
        //   - Grava EEPROM a cada 50ms (20Hz)
        //   - Grava SD Card a cada 10ms (100Hz)
        //   - Atualiza altitude máxima (gerenciada externamente)
        //   - Transição: altitude ≤ (maxAltitude - 5.0m) E armado == true
        // ------------------------------------------------------------------
        case FlightState::ASCENT:
            if (_shouldTransitionToRecovery(currentAltitude, maxAltitude)) {
                _state = FlightState::RECOVERY;
                _apogeeDetected = true;
                _armed = false;  // Impede reativação — PRD RF-03 item 7

                Serial.println(F("[FSM] Transição: ASCENT → RECOVERY (apogeu detectado)"));
                Serial.print(F("[FSM] Altitude máxima: "));
                Serial.print(maxAltitude, 1);
                Serial.println(F(" m"));
                Serial.print(F("[FSM] Altitude atual: "));
                Serial.print(currentAltitude, 1);
                Serial.println(F(" m"));
            }
            break;

        // ------------------------------------------------------------------
        // RECOVERY: Estado terminal — recuperação em andamento
        //   - SKIB ativado (gerenciado pelo RecoverySystem)
        //   - Grava EEPROM a cada 200ms (5Hz)
        //   - Grava SD Card a cada 10ms (100Hz)
        //   - Sem transição de saída
        // ------------------------------------------------------------------
        case FlightState::RECOVERY:
            // Estado terminal — nenhuma transição possível
            break;
    }
}

// ============================================================================
// Getters públicos
// ============================================================================

FlightState FlightStateMachine::getCurrentState() const {
    return _state;
}

bool FlightStateMachine::wasApogeeDetected() const {
    return _apogeeDetected;
}

unsigned long FlightStateMachine::getFlightStartTime() const {
    return _flightStartTime;
}

double FlightStateMachine::getHistory(int index) const {
    if (index >= 0 && index < ALTITUDE_HISTORY_SIZE) {
        return _altitudeHistory[index];
    }
    return 0.0;
}

// ============================================================================
// Forçar estado (ex: reset após pouso, testes)
// ============================================================================

void FlightStateMachine::forceState(FlightState newState) {
    _state = newState;

    Serial.print(F("[FSM] Estado forçado para: "));
    Serial.println(static_cast<uint8_t>(newState));
}

// ============================================================================
// Métodos privados
// ============================================================================

/**
 * _pushHistory — Atualiza o histórico de altitudes suavizadas.
 *
 * Implementa shift à direita conforme PRD seção 4.4.4:
 *   [0] = mais recente → [ALTITUDE_HISTORY_SIZE-1] = mais antigo
 *
 * A cada nova altitude, todos os valores são deslocados uma posição
 * para a direita e o novo valor é inserido na posição [0].
 */
void FlightStateMachine::_pushHistory(double altitude) {
    // Shift à direita: move [0] para [1], [1] para [2], etc.
    for (int i = ALTITUDE_HISTORY_SIZE - 1; i > 0; i--) {
        _altitudeHistory[i] = _altitudeHistory[i - 1];
    }
    // Insere novo valor na posição mais recente
    _altitudeHistory[0] = altitude;
}

/**
 * _shouldTransitionToAscent — Verifica condição de decolagem.
 *
 * PRD RF-02, seção 4.2.1:
 *   Transição quando smoothedAltitude ≥ ASCENT_THRESHOLD (5.0m)
 *
 * Usa a altitude suavizada mais recente (já filtrada pelo MovingAverage
 * antes de ser passada ao update()).
 */
bool FlightStateMachine::_shouldTransitionToAscent(double altitude) const {
    return altitude >= ASCENT_THRESHOLD;
}

/**
 * _shouldTransitionToRecovery — Verifica condição de apogeu.
 *
 * PRD RF-02, seção 4.2.2:
 *   Transição quando:
 *     1. altitudeHistory[0] ≤ maxAltitude - DESCENT_DETECTION_THRESHOLD (5.0m)
 *     2. _armed == true (impede reativação múltipla — PRD RF-03 item 7/8)
 *
 * A verificação de _armed garante que, uma vez desarmado, o SKIB
 * nunca seja reativado no mesmo voo.
 */
bool FlightStateMachine::_shouldTransitionToRecovery(double currentAlt, double maxAlt) const {
    if (!_armed) {
        return false;
    }
    return currentAlt <= (maxAlt - DESCENT_DETECTION_THRESHOLD);
}
