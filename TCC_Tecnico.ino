
// ========================================================================================================
// --- Includes ---
#include "DHT.h"
#include <Wire.h>
#include <LiquidCrystal.h>

// ========================================================================================================
// --- Mapeamento de Hardware ---
#define RELE_PELTIER   13    //controle do relé de acionamento da pastilha Peltier
#define DHTPIN         A0     // Pino digital sensor DHT
#define DHTTYPE DHT11        // DHT 11
#define LCD_PIN_RS 12
#define LCD_PIN_E  11
#define LCD_PIN_D4 5
#define LCD_PIN_D5 4
#define LCD_PIN_D6 3
#define LCD_PIN_D7 2

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal lcd(LCD_PIN_RS,LCD_PIN_E,LCD_PIN_D4,LCD_PIN_D5,LCD_PIN_D6,LCD_PIN_D7);

// ========================================================================================================

// ========================================================================================================
// --- Variaveis Globais ---

float humi;           //armazena humidade
float tempC;          //armazena temperatura em Graus Celsius


// ========================================================================================================
// --- Configurações Iniciais ---
void setup()
{
  Serial.begin(9600);    //inicializa serial
  
  Serial.println(F("DHTxx teste!")); 
  
  dht.begin();
  lcd.begin(16,2);

  delay(1000);
  lcd.setCursor(0,0);
  lcd.println("TCC");

  delay(1000);
  lcd.clear();
 
  pinMode(RELE_PELTIER , OUTPUT); //saída para controle da pastilha
  digitalWrite(RELE_PELTIER , LOW); //desliga relé de controle da pastilha
  pinMode(DHTPIN,     INPUT); //entrada para sensor de temperatura e umidade

}

//end setup

// ========================================================================================================
// --- Configurações Iniciais ---

void loop()
{
   // Aguarde alguns segundos entre as medições.
  delay(2000);

  // A leitura da temperatura ou umidade leva cerca de 250 milissegundos!
  // O sensor pode ter um atraso de até 2 segundos para a leitura
  humi = dht.readHumidity(); 
  // Temperature em Celsius (default)
  tempC  = dht.readTemperature();
  
  // Verifique se alguma leitura falhou e tenta novamente.
  if (isnan(humi) || isnan(tempC)) {
    Serial.println(F("Falha de leitura do sensor DHT!"));
    return;
  }

  //Display
  lcd.setCursor(0, 0);
  lcd.print(F("Humidade: "));
  lcd.setCursor(10, 0);
  lcd.print(round(humi));
  lcd.setCursor(12, 0);
  lcd.print(F(" %"));
  
  lcd.setCursor(0, 1);
  lcd.print(F("Tempo: "));
  lcd.setCursor(7, 1);
  lcd.print(round(tempC));
  lcd.setCursor(9, 1);
  lcd.write(32);  // Caracter espaço
  lcd.write(223); // Caracter °
  lcd.print(F("C"));

   // Monitor display
  Serial.print(F("Umidade: "));
  Serial.print(humi);
  Serial.print(F("%  Temperatura: "));
  Serial.print(tempC);
  Serial.print(F("°C "));
  
  //Conntrole do Peltier
  temperatura();
  if(tempC > 25.0) digitalWrite(RELE_PELTIER , HIGH);
  else digitalWrite(RELE_PELTIER , LOW);
}


 //end loop
