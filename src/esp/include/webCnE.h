#pragma once

#include <WebServer.h>
#include <LittleFS.h>

#define ESPERANDO 0
#define GRAVANDO 1
#define TAREANDO 2
#define CALIBRANDO 3

extern WebServer server;
extern SemaphoreHandle_t mutex_estado;
extern SemaphoreHandle_t mutex_arquivo;

extern int estado_atual;
extern bool pedido_calibrar;
extern bool pedido_tare;
extern float last_reading;

extern unsigned long gravacao_init_time;
extern unsigned long gravacao_last_time;

extern File arquivo;
extern String nome_arquivo_atual;

extern String getEstado();
extern void mudarEstado(int estado);

void handleRoot();
void handleArquivos();

void endpointCalibrar();

void endpointTare();

void endpointGravar();

void endpointListarArquivos();

void getData();
