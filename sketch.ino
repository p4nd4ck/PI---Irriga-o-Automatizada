#include <WiFi.h>
#include <DHT.h>
#include <RTClib.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <PubSubClient.h>
#include "config.h"

// Pinos
#define DHTPIN 4
#define DHTTYPE DHT22
#define SOIL_MOISTURE_PIN 34
#define RELAY_PIN 18
#define LED_PIN 2

// Tópicos MQTT
const char* topic_comando = "vitor/milho/comando";
const char* topic_status = "vitor/milho/status";
const char* topic_solo = "vitor/milho/solo";      // Umidade da Terra
const char* topic_temp = "vitor/milho/temp";      // Temperatura do Ar
const char* topic_hum_ar = "vitor/milho/hum_ar";  // Umidade do Ar
const char* topic_consumo = "vitor/milho/consumo";
const char* topic_uptime = "vitor/milho/uptime";

// Configurações de Fluxo e Irrigação
const float litrosPorMinuto = 10.0; 
float totalLitrosConsumidos = 0;
unsigned long tempoInicioIrrigacao = 0;
bool bombaEstavaLigada = false;

const int moistureThreshold = 2000;
const int startHour = 21;
const int endHour = 11;

bool modoManual = false; 
unsigned long lastMsg = 0;

DHT dht(DHTPIN, DHTTYPE);
RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) message += (char)payload[i];
  
  if (String(topic) == topic_comando) {
    if (message == "1") { digitalWrite(RELAY_PIN, LOW); modoManual = true; }
    else if (message == "0") { digitalWrite(RELAY_PIN, HIGH); modoManual = true; }
    else if (message == "auto") { modoManual = false; }
    else if (message == "reset") { totalLitrosConsumidos = 0; }
  }
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32_Vitor_Irriga_Final")) {
      client.subscribe(topic_comando);
    } else { delay(5000); }
  }
}

void setup() {
  dht.begin();
  rtc.begin(); 
  lcd.init();
  lcd.backlight();
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
  WiFi.begin(ssid, pass);
  client.setServer("broker.hivemq.com", 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // Leituras dos Sensores
  int soilMoisture = analogRead(SOIL_MOISTURE_PIN);
  float temp = dht.readTemperature();
  float humAr = dht.readHumidity();
  bool estadoRelé = digitalRead(RELAY_PIN);

  // Lógica de Automação
  if (!modoManual) {
    DateTime now = rtc.now();
    bool diaDeIrrigar = (now.day() % 2 == 0);
    bool naJanela = (now.hour() >= startHour || now.hour() < endHour);
    if (diaDeIrrigar && naJanela && soilMoisture < moistureThreshold) {
      digitalWrite(RELAY_PIN, LOW);
      digitalWrite(LED_PIN, HIGH);
    } else {
      digitalWrite(RELAY_PIN, HIGH);
      digitalWrite(LED_PIN, LOW);
    }
  }

  // Cálculo de Consumo
  if (estadoRelé == LOW) {
    if (!bombaEstavaLigada) { tempoInicioIrrigacao = millis(); bombaEstavaLigada = true; }
  } else {
    if (bombaEstavaLigada) {
      unsigned long tempoDecorrido = millis() - tempoInicioIrrigacao;
      totalLitrosConsumidos += (tempoDecorrido / 60000.0) * litrosPorMinuto;
      bombaEstavaLigada = false;
    }
  }

  // Monitor Serial (Para depuração)
  if (millis() - lastMsg > 10000) {
    lastMsg = millis();
    
    // Envio para o MyMQTT
    client.publish(topic_status, (estadoRelé == LOW) ? "LIGADO" : "PARADO");
    client.publish(topic_solo, String(soilMoisture).c_str());
    client.publish(topic_temp, String(temp, 1).c_str());
    client.publish(topic_hum_ar, String(humAr, 1).c_str());
    client.publish(topic_consumo, String(totalLitrosConsumidos, 1).c_str());
    client.publish(topic_uptime, String(millis() / 60000).c_str());

    // Atualiza LCD
    lcd.setCursor(0, 0);
    lcd.print("T:"); lcd.print((int)temp);
    lcd.print("C H:"); lcd.print((int)humAr); lcd.print("%  ");
    lcd.setCursor(0, 1);
    lcd.print("Solo: "); lcd.print(soilMoisture);
  }
}