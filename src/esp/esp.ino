#include <WiFi.h>
#include <ESPmDNS.h>
#include <HX711.h>
#include <Wire.h>

#include "./include/webCnE.h"

// ==========================================
//                Constantes
// ==========================================

#define ESPERANDO 0
#define GRAVANDO 1
#define TAREANDO 2
#define CALIBRANDO 3

#define loadcell_data 2
#define loadcell_clk 4

// Massa do objeto de referência (em Kg)
#define PESO_REFERENCIA 4.616
// Fator de escala em Newtons
#define FATOR_ESCALA PESO_REFERENCIA / 9.81

#define RAW_VALUE 98838.58

const char *ap_ssid = "Banco Estático CnE";
const char *ap_pass = "Cangas168!";
const char *mdns_name = "bancoestatico";

WebServer server(80);

HX711 scale;

// ==========================================
//                Globais
// ==========================================

unsigned long last_time = 0;
unsigned long init_time = 0;
float last_reading = 0;

unsigned long gravacao_init_time = 0;
unsigned long gravacao_last_time = 0;

bool pedido_tare = false;
bool pedido_calibrar = false;
bool is_tareado = false;
bool is_referencia_set = false;

int estado_atual = 0;

SemaphoreHandle_t mutex_estado = NULL;


// ==========================================
//              Funções Aux
// ==========================================

void updateSensor(){
  last_reading = scale.get_units();
  last_time = millis();
}

String getEstado(){
  if (estado_atual == ESPERANDO)
    return "Esperando";
  else if (estado_atual == GRAVANDO)
    return "Gravando";
  else if (estado_atual == TAREANDO)
    return "Tareando";
  else
    return "Calibrando";
}

String getData(){
  char b[100];
  snprintf(
    b, 
    sizeof(b), 
    "{\"forca\": %.4f, \"duracao\": \"%.3lu\", \"estado\": \"%s\"}", 
    last_reading, 
    (gravacao_last_time - gravacao_init_time), 
    getEstado());

  return String(b);
}

void endpointCalibrar (){
  int estado_temp;

  xSemaphoreTake(mutex_estado, portMAX_DELAY);
  estado_temp = estado_atual;
  xSemaphoreGive(mutex_estado);

  if (estado_temp != GRAVANDO)
    pedido_calibrar = true;
}

void endpointTare (){
  int estado_temp;

  xSemaphoreTake(mutex_estado, portMAX_DELAY);
  estado_temp = estado_atual;
  xSemaphoreGive(mutex_estado);

  if (estado_temp != GRAVANDO)
    pedido_tare = true;
}

void endpointGravar(){
  int estado_temp;

  xSemaphoreTake(mutex_estado, portMAX_DELAY);
  estado_temp = estado_atual;
  xSemaphoreGive(mutex_estado);

  if (estado_temp == ESPERANDO){
    gravacao_init_time = millis();
    mudarEstado(GRAVANDO);
  }
  else if (estado_temp == GRAVANDO) {
    gravacao_last_time = millis();
    mudarEstado(ESPERANDO);
  }
}

void tarear(){
  Serial.println("Fazendo o tare");
  mudarEstado(TAREANDO);

  //Bip para o inicio
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  
  scale.tare();
  is_tareado = true;
  //Bip para o fim

  Serial.println("Tare finalizado");
  mudarEstado(ESPERANDO);
}

void calibrar(){
  if (scale.is_ready()){
    mudarEstado(CALIBRANDO);

    scale.set_scale();
    
    if(!is_tareado){
      Serial.println("tare não realizado, iniciando tare");
      tarear();
      Serial.println("voltando para a calibragem");
      mudarEstado(CALIBRANDO);
    }

    double leitura = -1.0;

    if(!is_referencia_set){
      Serial.println("Iniciando a leitura, deixe o peso de referencia no banco estático");
      //2 bips para o inicio
      vTaskDelay(5000 / portTICK_PERIOD_MS);

      leitura = scale.get_value(100);
      Serial.print("leitura: ");
      Serial.print(leitura);
      Serial.print(", fator de escala: ");
      Serial.println(FATOR_ESCALA);

      //2 bips para o fim
    } else{
      Serial.print("Leitura já definida: ");
      leitura = RAW_VALUE;
      Serial.println(leitura);
    }

    double escala = leitura/FATOR_ESCALA;
    if (escala == 0)
      scale.set_scale(1);
    else
      scale.set_scale((leitura/FATOR_ESCALA));

    Serial.print("Escala: ");
    Serial.println(leitura/FATOR_ESCALA);
    Serial.println("Calibragem finalizada");
  }
  else{
    Serial.println("HX771 não encontrado (calibrar)");
  }

  mudarEstado(ESPERANDO);
}

void mudarEstado(int estado){
  xSemaphoreTake(mutex_estado, portMAX_DELAY);
  estado_atual = estado;
  xSemaphoreGive(mutex_estado);
}

void taskHandleClient(void* pvParameters){
  while(true){
    server.handleClient();

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void taskRotinas(void* pvParameters){
  while(true){
    if(pedido_calibrar){
      calibrar();
      pedido_calibrar = false;
    }

    else if(pedido_tare){
      tarear();
      pedido_tare = false;
    }

    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

// ==========================================
//            Setup e Main Loop
// ==========================================

void setup() {
  Serial.begin(115200);

  delay(2000);

  // Configura o ponto de acesso
  Serial.println("Configurando o access point");
  WiFi.mode(WIFI_AP);
  
  WiFi.softAP(ap_ssid, ap_pass);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("Meu IP: ");
  Serial.println(myIP);

  // Inicia o DNS
  if (!MDNS.begin(mdns_name)) {
    Serial.println("mDNS falhou");
  } else {
    Serial.println("mDNS: http://bancoestatico.local/");
  }

  // Inicia as células de carga
  scale.begin(loadcell_data, loadcell_clk);

  // Define as rotas acessíveis
  server.on("/",handleRoot);
  server.on("/calibrar",endpointCalibrar);
  server.on("/tare",endpointTare);
  server.on("/iniciar_gravar",endpointGravar);
  server.on("/get_data",[](){server.send(200,"application/json",getData());});
  server.begin();

  mutex_estado = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(taskHandleClient, "handleClient", 10000, NULL, 0, NULL, 0);
  xTaskCreatePinnedToCore(taskRotinas, "Rotinas", 5000, NULL, 0, NULL, 0);
}

void loop() {
  int estado_temp;

  xSemaphoreTake(mutex_estado, portMAX_DELAY);
  estado_temp = estado_atual;
  xSemaphoreGive(mutex_estado);

  // Não faz nada se estiver tareando ou calibrando
  if (estado_temp == TAREANDO || estado_temp == CALIBRANDO){

  }
  else if (estado_temp == GRAVANDO){
    gravacao_last_time = millis();
    updateSensor();
  }
  else{
    updateSensor();
  }
}