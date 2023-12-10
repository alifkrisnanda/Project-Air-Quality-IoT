#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "DHTesp.h"
#include <ArduinoJson.h>
#include "ThingSpeak.h"

const char* ssid = "Wokwi-GUEST";
const char* password = "";
// koneksi thingspeak
const char* myApiKey = "your api key";
const int myChannelNumber = your channel number;


// Telegram BOT
#define BOTtoken "insert your bot token"
// #define CHAT_ID "" 


#define DHT_PIN 12 
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

WiFiClientSecure teleclient;
WiFiClient client;
UniversalTelegramBot bot(BOTtoken, teleclient);

int botRequestDelay = 1000;
unsigned long lastTimeBotRan;




void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  float temperature = random(300, 360) / 10.0;
  float humidity = random(500, 600) / 10.0;
  float debu = random(1400, 1500) / 10.0;
  float co = random(1200, 1300) / 10.0;

  for (int i=0; i<numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);

    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String control = "Selamat Datang, " + from_name + ".\n";
      control += "Gunakan Commands Di Bawah Untuk Monitoring\n\n";
      control += "/temperature Untuk Monitoring Suhu \n";
      control += "/humidity Untuk Monitoring Kelembapan \n";
      control += "/debu Untuk Monitoring Debu \n";
      control += "/karbonmonoksida Untuk Monitoring Gas Carbon";
      bot.sendMessage(chat_id, control, "");
    }

    if (text == "/temperature") {
      int t= temperature;
      if (isnan(t)) {
          Serial.println(F("Gagal membaca data sensor"));
          return;
      }
      String suhu = "Status Suhu ";
      suhu += t;
      suhu +="⁰C\n";
      bot.sendMessage(chat_id, suhu, "");
    }
    
    if (text == "/humidity") {
      int h= humidity;
      if (isnan(h)) {
          Serial.println(F("Gagal membaca data sensor"));
          return;
      }
      String lembab = "Status Kelembapan ";
      lembab += h;
      lembab +="%\n";
      bot.sendMessage(chat_id, lembab, "");
    }
    if (text == "/debu") {
      int d= debu;
      if (isnan(d)) {
          Serial.println(F("Gagal membaca data sensor"));
          return;
      }
      String debu = "Status Debu ";
      debu += d;
      debu +="PPB\n";
      bot.sendMessage(chat_id, debu, "");
    }
    if (text == "/karbonmonoksida") {
      int c= co;
      if (isnan(c)) {
          Serial.println(F("Gagal membaca data sensor"));
          return;
      }
      String co = "Status Karbon Monoksida ";
      co += c;
      co +="PPM\n";
      bot.sendMessage(chat_id, co, "");
    }
  }
}
void setup() {
  Serial.begin(115200);
  dht.begin();
  
  // Koneksi Ke Wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  #ifdef ESP32
    teleclient.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("WiFi connected !");
  Serial.println("Local IP: " + String(WiFi.localIP()));

  // Mulai koneksi ke ThingSpeak
  ThingSpeak.begin(client);
}

void loop() {
  float temperature = random(300, 360) / 10.0;
  float humidity = random(500, 600) / 10.0;
  float debu = random(1400, 1500) / 10.0;
  float co = random(1200, 1300) / 10.0;

  // Set nilai field di ThingSpeak
  ThingSpeak.setField(1, temperature);
  ThingSpeak.setField(2, humidity);
  ThingSpeak.setField(3, debu);
  ThingSpeak.setField(4, co);

  // Kirim data ke ThingSpeak
  int x = ThingSpeak.writeFields(myChannelNumber, myApiKey);

  // Tampilkan data di Serial Monitor
  Serial.println("Temperature: " + String(temperature, 1) + "°C");
  Serial.println("Humidity: " + String(humidity, 2) + "%");
  Serial.println("Debu: " + String(debu, 3) + "PPB");
  Serial.println("Karbon Monoksida: " + String(co, 4) + "PPM");


  // Cek status pengiriman data ke ThingSpeak
  if(x == 200){
    Serial.println("Data pushed successfully");
  } else {
    Serial.println("Failed to push  HTTP status code: " + String(x));
  }
  Serial.println("---");

  delay(10000);

  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}
