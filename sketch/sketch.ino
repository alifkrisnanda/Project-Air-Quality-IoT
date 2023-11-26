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
const char* myApiKey = "api key";
const int myChannelNumber = channelnumber;


// Telegram BOT
#define BOTtoken "BOTtoken"


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
  float temperature = random(200, 300) / 10.0; // Nilai acak antara 20.0 dan 30.0
  float humidity = random(400, 600) / 10.0;

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
      bot.sendMessage(chat_id, control, "");
    }

    if (text == "/temperature") {
      int t= temperature;
      if (isnan(t)) {
          Serial.println(F("Failed to read from DHT sensor!"));
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
          Serial.println(F("Failed to read from DHT sensor!"));
          return;
      }
      String lembab = "Status Kelembapan ";
      lembab += h;
      lembab +="%\n";
      bot.sendMessage(chat_id, lembab, "");
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
  float temperature = random(200, 300) / 10.0; // Nilai acak antara 20.0 dan 30.0
  float humidity = random(400, 600) / 10.0; 
  
  // Set nilai field di ThingSpeak
  ThingSpeak.setField(1, temperature);
  ThingSpeak.setField(2, humidity);

  // Kirim data ke ThingSpeak
  int x = ThingSpeak.writeFields(myChannelNumber, myApiKey);

  // Tampilkan data di Serial Monitor
  Serial.println("Temperature: " + String(temperature, 2) + "°C");
  Serial.println("Humidity: " + String(humidity, 1) + "%");

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

