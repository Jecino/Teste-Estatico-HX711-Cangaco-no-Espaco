#include <Wire.h>
#include <HX711.h>

#define loadcell_data 7
#define loadcell_clk 6
//#define calibrate_btn 3


// Massa do objeto de referência (em Kg)
#define PESO_REFERENCIA 2.016
// Fator de escala em Newtons
#define FATOR_ESCALA PESO_REFERENCIA * 9.81

HX711 scale;

bool calibrado = false;
unsigned long last_time = 0;
unsigned long init_time = 0;

void setup() {
  //pinMode(calibrate_btn, INPUT);

  Serial.begin(115200);

  // Tempo para estabilizar o HX711 ao ligar
  delay(500);

  scale.begin(loadcell_data, loadcell_clk);
}

void loop() {
  if (!calibrado /*|| digitalRead(calibrate_btn)*/){
    Serial.println("Iniciando calibração");

    delay(200);
    
    calibrar();

    init_time = millis();
  }

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

    Serial.println("Iniciando a leitura");
    delay(5000);

    float leitura = scale.get_value(10);
    Serial.print("leitura: ");
    Serial.println(leitura);

    scale.set_scale((leitura/FATOR_ESCALA));

    Serial.print("Escala: ");
    Serial.println(leitura/FATOR_ESCALA);

    calibrado = true;
  }

  else{
    Serial.println("HX771 não encontrado (calibrar)");
  }
}
