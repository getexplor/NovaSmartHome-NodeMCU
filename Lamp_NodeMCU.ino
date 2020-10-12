#include <Arduino.h>

//arduinojson
#include <ArduinoJson.h>

//esp
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
ESP8266WiFiMulti WiFiMulti;

char ssid[] = "samsung S8";
char password[] = "c14no12a";

String payload ;

const char* id;
const char* iduser;
const char* device_category;
const char* number;
const char* stat;

String DeviceCategory = "lampu";
String DeviceNumber = "1";

String url = "http://192.168.1.14/novasmarthome/api/deviceapi?device_category=" + DeviceCategory + "&number=" + DeviceNumber;

int relay = 2;

void setup() {

  pinMode(relay, OUTPUT);
  
  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);
}

void loop() {
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    WiFiClient client;
    HTTPClient http;
    
    Serial.print("[HTTP] begin...\n");
    if (http.begin(client,url)) {  // HTTP
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
          
          id = data["id"];
          iduser = data["iduser"];
          device_category = data["device_category"];
          number = data["number"];
          stat = data["status"];
          
          Serial.println(payload);

          Serial.println(id);
          Serial.println(iduser);
          Serial.println(device_category);
          Serial.println(number);
          Serial.println(stat);
          
          Lampu(stat);
          
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
//fungsi menyalakan lampu
void Lampu(const char* stat){
  int nyala = atoi((char*) stat);
  Serial.print("ini status dari char ke int : ");
  Serial.println(nyala);
          
  if(nyala == 0 ){
    digitalWrite(relay, LOW);
  }
  else{
    digitalWrite(relay, HIGH);
  }
}
