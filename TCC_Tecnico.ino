
// ========================================================================================================
// --- Includes ---
#include "DHT.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TimerOne.h>

// ========================================================================================================
// --- Mapeamento de Hardware ---
#define RELE_PELTIER   13    //controle do relé de acionamento da pastilha Peltier
#define RELE_VALVE     10    //controle do relé de acionamento da valvula
#define DHT_PIN        A0    // Pino sensor DHT
#define HIGRO_PIN      A1    // Pino sensor de umidade do solo

#define DHT_TYPE       DHT11 // Tipo de DHT

#define LIMITE_INFERIOR_TEMP   23 //Minima temperatura para desligar a pastilha Peltier.
#define LIMITE_SUPERIOR_TEMP   25 //Minima temperatura para ligar a pastilha Peltier.
#define LIMITE_INFERIOR_SOLO   74 //Minima umidade do solo para comecar a regar o solo, 74% de umidade.
#define LIMITE_SUPERIOR_SOLO   95 //Maxima umidade do solo para parar de regar o solo, 95% de umidade.

DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);  //Seta o endereco do LCD para 0x3F para um LCD 16x2

// ========================================================================================================

// ========================================================================================================
// --- Variaveis Globais ---
int umidade_solo = 100;         //armazena umidade do solo
float umidade_ar = 70.0;       //armazena umidade do ar
float temperatura = 20.0;      //armazena temperatura em Graus Celsius
//Flags de controle
bool valve_status = false;
bool peltier_status = false;
bool flag_ler_sensores = false;
bool flag_atualizar_monitor = false;
bool flag_atualizou_umidade_solo = false;
bool flag_atualizou_umidade_ar = false;
bool flag_atualizou_temperatura = false;

// ========================================================================================================
// --- Configurações Iniciais ---
void setup()
{
  //Configura pinos como entrada ou saida 
  pinMode(RELE_PELTIER, OUTPUT); //Configura o pino de saida da pastilha como SAIDA/OUTPUT
  pinMode(RELE_VALVE, OUTPUT); //Configura o pino de saida da valvula como SAIDA/OUTPUT
  pinMode(DHT_PIN, INPUT); //Configura o pino de entrada do sensor de temperatura e umidade DHT como ENTRADA/INPUT
  pinMode(HIGRO_PIN, INPUT); //Configura o pino de entrada do sensor de umidade do solo como ENTRADA/INPUT

  //Seta valores padroes para os pinos de saida
  digitalWrite(RELE_PELTIER, HIGH); //Desliga relé de controle da pastilha. O modulo rele tem logica invertida por isso do HIGH 
  digitalWrite(RELE_VALVE, HIGH); //Desliga relé de controle da valvula. O modulo rele tem logica invertida por isso do HIGH 
  
  //Configura monitor serial
  Serial.begin(9600); //Inicializa serial
  Serial.println(F("Inicio do monitor do TCC")); 

  //Inicializa tudo
  dht.begin();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.println("TCC");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Umidade: "));
  lcd.setCursor(0, 1);
  lcd.print(F("Temp: "));

  //Configura timer
  Timer1.initialize(1000000); //Inicia um timer de duracao de 1000000 microsegundos (ou 1s - ou 1Hz)
  Timer1.attachInterrupt( funcaoTimer ); //Chama a funcao adquireValoresDosSensores a cada 1s. 
}

//end setup

// ========================================================================================================
// --- Loop ---

void loop()
{
  if(flag_ler_sensores == true) {
    //Adquires os valores dos sensores
    adquireValoresDosSensores();
    flag_ler_sensores = false;
  }

  if(flag_atualizou_umidade_ar == true) {
    //Mostra dados no LCD
    atualizaUmidadeNoLCD();
    flag_atualizou_umidade_ar = false;
  }

  if(flag_atualizou_temperatura == true) {
    //Mostra dados no LCD
    atualizaTemperaturaNoLCD();

    //Controle do Peltier
    if((round(temperatura) > LIMITE_SUPERIOR_TEMP) && (peltier_status == false)){
      digitalWrite(RELE_PELTIER, LOW); //O modulo rele tem logica invertida, entao LOW vai ativar o rele.
      peltier_status = true;
    } else if((round(temperatura) < LIMITE_INFERIOR_TEMP) && (peltier_status == true)) {
      digitalWrite(RELE_PELTIER, HIGH); //O modulo rele tem logica invertida, entao HIGH vai desativar o rele.
      peltier_status = false;
    }

    flag_atualizou_temperatura = false;
  }

  if(flag_atualizou_umidade_solo == true) {
    //Controle do irrigador/valvula
    if((round(umidade_solo) < LIMITE_INFERIOR_SOLO) && (valve_status == false)){
      digitalWrite(RELE_VALVE, LOW); //O modulo rele tem logica invertida, entao LOW vai ativar o rele.
      valve_status = true;
    } else if((round(umidade_solo) > LIMITE_SUPERIOR_SOLO) && (valve_status == true)) {
      digitalWrite(RELE_VALVE, HIGH); //O modulo rele tem logica invertida, entao HIGH vai desativar o rele.
      valve_status = false;
    }

    flag_atualizou_umidade_solo = false;
  }

  if(flag_atualizar_monitor == true) {
    //Mostra dados no monitor do Arduino
    mostrarNoMonitor();
    flag_atualizar_monitor = false;
  }
}
//end loop

void funcaoTimer() {
  flag_ler_sensores = true;
  flag_atualizar_monitor = true;
}

void adquireValoresDosSensores() {
  //DHT
  //A leitura da temperatura ou umidade leva cerca de 250 milissegundos!
  //PS.: O sensor pode ter um atraso de até 2 segundos para a leitura
  float nova_umidade_ar = dht.readHumidity(); //Le a umidade
  float nova_temperatura = dht.readTemperature(); //Le a temperatura
  //Sensor de umidade do solo
  int nova_umidade_solo = analogRead(HIGRO_PIN); //Faz a leitura do sensor de umidade do solo
  nova_umidade_solo = map(nova_umidade_solo, 1023, 0, 0, 100); //Converte a variação do sensor de 0 a 1023 do ADC para 0 a 100

  //Atualiza flags de controle
  if(isnan(nova_umidade_ar) == false){
    if(nova_umidade_ar != umidade_ar) {
      flag_atualizou_umidade_ar = true;
      umidade_ar = nova_umidade_ar;
    }
  }

  if(isnan(nova_temperatura) == false){
    if(nova_temperatura != temperatura) {
      flag_atualizou_temperatura = true;
      temperatura = nova_temperatura;
    }
  }

  if(nova_umidade_solo != umidade_solo) {
    flag_atualizou_umidade_solo = true;
    umidade_solo = nova_umidade_solo;
  }
}

void atualizaUmidadeNoLCD() {
  lcd.setCursor(9, 0);
  lcd.print(round(umidade_ar));
  lcd.setCursor(11, 0);
  lcd.print(F(" %    "));
}

void atualizaTemperaturaNoLCD() {
  lcd.setCursor(6, 1);
  lcd.print(round(temperatura));
  lcd.setCursor(8, 1);
  lcd.write(32);  // Caracter espaço
  lcd.write(223); // Caracter °
  lcd.print(F("C    "));
}

void mostrarNoMonitor() {
  Serial.print(F("Umidade do solo: "));
  Serial.print(umidade_solo);
  Serial.print(F("%, Umidade do ar: "));
  Serial.print(umidade_ar);
  Serial.print(F("%, Temperatura: "));
  Serial.print(temperatura);
  Serial.print(F("°C ("));
  Serial.print(round(temperatura));
  Serial.print(F("°C)\n"));
  Serial.print(F("Status do regador: "));
  Serial.print(valve_status);
  Serial.print(F(", Status do peltier: "));
  Serial.print(peltier_status);
  Serial.print(F("\n"));
}
