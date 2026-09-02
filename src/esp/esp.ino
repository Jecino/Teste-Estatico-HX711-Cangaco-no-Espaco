#include <WiFi.h>
#include <ESPmDNS.h>
#include <HX711.h>
#include <Wire.h>

#include "./include/webCnE.h"

#define ESPERANDO 0
#define GRAVANDO 1
#define TAREANDO 2
#define CALIBRANDO 3

// Massa do objeto de referência (em Kg)
#define PESO_REFERENCIA 4.616
// Fator de escala em Newtons
#define FATOR_ESCALA PESO_REFERENCIA / 9.81

#define RAW_VALUE 98838.58

/* ----------------- Constantes ----------------- */
const char *ap_ssid = "Banco Estático CnE";
const char *ap_pass = "Cangas168!";
const char *mdns_name = "bancoestatico";

bool is_tareado = false;
bool is_referencia_set = false;

int estado_atual = 0;

WebServer server(80);

HX711 scale;

float last = 0;
unsigned long last_time = millis();

void updateSensor(){
  //valor de teste, trocar para atualização dos sensores reais
  if (millis() - last_time >= 300){
    last_time = millis();
    last = rand() % 100;
  }
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
    "{\"forca\": %.3f, \"forca_string\": \"%.3f N\", \"estado\": \"%s\"}", 
    last, 
    last, 
    getEstado());

  return String(b);
}

void setup() {
  Serial.begin(115200);

  delay(2000);

  Serial.println("Configurando o access point");
  WiFi.mode(WIFI_AP);
  
  WiFi.softAP(ap_ssid, ap_pass);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("Meu IP: ");
  Serial.println(myIP);

  if (!MDNS.begin(mdns_name)) {
    Serial.println("mDNS falhou");
  } else {
    Serial.println("mDNS: http://bancoestatico.local/");
  }

  server.on("/",handleRoot);
  server.on("/get_data",[](){server.send(200,"application/json",getData());});
  server.begin();
}

void tarear (){
  Serial.println("Fazendo o tare");
    //Bip para o inicio
    delay(5000);
    
    scale.tare();
    is_tareado = true;
    //Bip para o fim
}

void calibrar (){
  if (scale.is_ready()){

    scale.set_scale();
    
    if(!is_tareado){
      tarear();
    }

    float leitura = -1.0;

    if(!is_referencia_set){
      Serial.println("Iniciando a leitura, deixe o peso de referencia no banco estático");
      //2 bips para o inicio
      delay(5000);

      leitura = scale.get_value(1000);
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

    scale.set_scale((leitura/FATOR_ESCALA));

    Serial.print("Escala: ");
    Serial.println(leitura/FATOR_ESCALA);
  }
  else{
    Serial.println("HX771 não encontrado (calibrar)");
  }
}

void loop() {
  updateSensor();
  server.handleClient();
}