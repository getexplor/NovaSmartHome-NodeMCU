#include <Arduino.h>
//arduino json
#include <ArduinoJson.h>
//remote
#include <IRremoteESP8266.h>
#include <IRsend.h>

//esp
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
ESP8266WiFiMulti WiFiMulti;

char ssid[] = "samsung S8";
char password[] = "c14no12a";

String payload ;

const char* stat;

int StatLama, StatBaru;

String DeviceCategory = "televisi";
String DeviceNumber = "1";

String url = "http://192.168.1.14/novasmarthome/api/deviceapi?device_category=" + DeviceCategory + "&number=" + DeviceNumber;

int LedPin = 2;
const uint16_t RemoteTV = 4;  // ESP8266 GPIO pin to use. Recommended: 4 (D2).

IRsend irsend(RemoteTV);  // Set the GPIO to be used to sending the message.

void setup() {
  Serial.begin(115200);
  irsend.begin();
  pinMode(LedPin, OUTPUT);

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);
}

void loop() {
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    WiFiClient client;
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    if (http.begin(client, url)) {
      Serial.print("[HTTP] GET...\n");
      int httpCode = http.GET();

      if (httpCode > 0) {
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          payload = http.getString();

          const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(6) + 100;
          DynamicJsonDocument doc(capacity);
          deserializeJson(doc, payload);

          bool status = doc["status"];
          JsonObject data = doc["data"][0];

          const char* id = data["id"];
          const char* iduser = data["iduser"];
          const char* device_category = data["device_category"];
          const char* number = data["number"];
          stat = data["status"];

          Serial.println(payload);

          Televisi(stat);
        }

      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
  }
  delay(100);
}
//fungsi meyalakan dan mematikan televisi
void Televisi(const char* stat) {
  StatBaru = atoi((char*) stat);
  if (StatBaru != StatLama) {
    if (StatBaru == 1) {
      digitalWrite(LedPin, LOW);
      //power
      irsend.sendNEC(0x20DF10EF, 32);
      delay(100);
      irsend.sendNEC(0xFF02FD , 32);
      delay(50000);
      //kanan kanan ok
      irsend.sendNEC(0xFF52AD , 32);
      delay(300);
      irsend.sendNEC(0xFF52AD , 32);
      delay(300);
      irsend.sendNEC(0xFF926D , 32);
      delay(2000);
      //bawah bawah bawah ok
      irsend.sendNEC(0xFFD22D , 32);
      delay(300);
      irsend.sendNEC(0xFFD22D , 32);
      delay(300);
      irsend.sendNEC(0xFFD22D , 32);
      delay(300);
      irsend.sendNEC(0xFF52AD , 32);
      delay(100);
      irsend.sendNEC(0xFF926D , 32);
      StatLama = StatBaru;
    } else {
      digitalWrite(LedPin, HIGH);
      //power
      irsend.sendNEC(0x20DF10EF, 32);
      delay(100);
      irsend.sendNEC(0xFF02FD , 32);
      StatLama = StatBaru;
    }
  }
}
