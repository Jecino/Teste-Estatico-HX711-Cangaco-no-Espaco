#include <Wire.h>
#include <HX711.h>

#define loadcell_data 7
#define loadcell_clk 6
//#define calibrate_btn 3


// Massa do objeto de referência (em Kg)
#define PESO_REFERENCIA 2.016
// Fator de escala em Newtons
#define FATOR_ESCALA PESO_REFERENCIA / 9.81

#define RAW_VALUE 40868.80

#define REFERENCIA_IS_SET true

HX711 scale;

bool calibrado = false;
unsigned long last_time = 0;
unsigned long init_time = 0;

void setup() {
  //pinMode(calibrate_btn, INPUT);

  Serial.begin(9600);

  // Tempo para estabilizar o HX711 ao ligar
  delay(1000);

  scale.begin(loadcell_data, loadcell_clk);

  Serial.println("Iniciando calibração");

  while(!calibrado) {
    calibrar();
  }

  init_time = millis();
}

void loop() {

  Serial.print(scale.get_units(), 1);
  Serial.print(",");

  last_time = millis();
  Serial.println(last_time - init_time);
  }

void calibrar (){
  if (scale.is_ready()){

    scale.set_scale();
    
    Serial.println("Fazendo o tare");
    delay(5000);
    
    scale.tare();

    float leitura = -1.0;
    if(!REFERENCIA_IS_SET){
      Serial.println("Iniciando a leitura, deixe o peso de referencia no banco estático");
      delay(5000);

      leitura = scale.get_value(1000);
      Serial.print("leitura: ");
      Serial.print(leitura);
      Serial.print(", fator de escala: ");
      Serial.println(FATOR_ESCALA);
      
      Serial.println("Leitura realizada, resete o arduino para o valor coletado");
      while(1);
    } else{
      Serial.print("Leitura já definida: ");
      leitura = 41926.91;
      Serial.println(leitura);
    }

    scale.set_scale((leitura/FATOR_ESCALA));

    Serial.print("Escala: ");
    Serial.println(leitura/FATOR_ESCALA);

    calibrado = true;
  }
  else{
    Serial.println("HX771 não encontrado (calibrar)");
  }
}
