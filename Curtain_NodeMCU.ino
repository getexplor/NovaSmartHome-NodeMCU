#include <Arduino.h>
//arduino json
#include <ArduinoJson.h>
//stepper
#include <Stepper.h>

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

int StatLama, StatBaru, count = 0;

String DeviceCategory = "tirai";
String DeviceNumber = "1";

String url = "http://192.168.1.5/novasmarthome/api/deviceapi?device_category=" + DeviceCategory + "&number=" + DeviceNumber;

//pin stepper
const int Step = 0; //GPIO0---D3 of Nodemcu--Step of stepper motor driver
const int Dir  = 2; //GPIO2---D4 of Nodemcu--Direction of stepper motor driver

void setup()
{
  pinMode(Step, OUTPUT); //Step pin as output
  pinMode(Dir,  OUTPUT); //Direcction pin as output
  
  Serial.begin(115200);

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

void loop() 
{
  
  if(StatBaru != StatLama){
    Tirai(StatBaru);
  }
  else{
    stat = GETstatus();
  }
}

char* GETstatus(){
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
          
          const char* id = data["id"];
          const char* iduser = data["iduser"];
          const char* device_category = data["device_category"];
          const char* number = data["number"];
          stat = data["status"];
          
          StatBaru =  atoi((char*) stat);
          Serial.println(payload);

          Serial.println(id);
          Serial.println(iduser);
          Serial.println(device_category);
          Serial.println(number);
          Serial.println(stat);
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
void Tirai(int StatBaru){
  int i;
  if(StatBaru == 1){
      digitalWrite(Dir, LOW); //Rotate stepper motor in clock wise direction
      for( i=1;i<=1800;i++){
        yield();
        digitalWrite(Step, HIGH);
        delayMicroseconds(500);
        digitalWrite(Step, LOW);
        delayMicroseconds(500);
      }
  }
  if(StatBaru == 0){
    digitalWrite(Dir, HIGH); //Rotate stepper motor in anti clock wise direction
    for( i=1;i<=2200;i++){
      yield();
      digitalWrite(Step, HIGH);
      delayMicroseconds(500);
      digitalWrite(Step, LOW);
      delayMicroseconds(500);
     }
  }
  StatLama = StatBaru;
}
