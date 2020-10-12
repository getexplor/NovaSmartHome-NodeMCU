#include <Arduino.h>

//arduinojson
#include <ArduinoJson.h>

//esp8266httpclient
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
ESP8266WiFiMulti WiFiMulti;

//dht22 Module
#include "DHT.h"
#define DHTPIN 2 
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

//RTC Module
#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS3231.h>
RtcDS3231<TwoWire> Rtc(Wire);
char datestring[20];
#define countof(a) (sizeof(a) / sizeof(a[0]))

//ssid dan password
char ssid[] = "samsung S8";
char password[] = "c14no12a";

//http request
String url;
String DeviceCategory = "suhu";
String DeviceNumber = "1";
String urlGET = "http://192.168.1.5/novasmarthome/api/deviceapi?device_category=" + DeviceCategory + "&number=" + DeviceNumber;
String payload ;

//device info
String device_category = "suhu";
int number = 1;
String times = "";
float hum;
float temp;
const char* stat;

void setup()
{
//  RTC setup
  Serial.print("compiled: ");
  Serial.print(__DATE__);
  Serial.println(__TIME__);

  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  printDateTime(compiled);
  Serial.println();

  if(!Rtc.IsDateTimeValid()){
    if(Rtc.LastError() != 0){
      Serial.print("RTC communications error = ");
      Serial.println(Rtc.LastError());
    }else{
      Serial.println("RTC lost confidence in the DateTime!");
      Rtc.SetDateTime(compiled);
    }
  }
  if(!Rtc.GetIsRunning()){
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }
  RtcDateTime now = Rtc.GetDateTime();
  if(now < compiled){
    Serial.println("RTC is older than compile time!  (Updating DateTime)");
    Rtc.SetDateTime(compiled);
  }
  else if(now > compiled){
    Serial.println("RTC is newer than compile time. (this is expected)");
  }
  else if(now == compiled){
    Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }
  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
  
//  dht setup
  dht.begin();
  Serial.begin(115200);

  Serial.println();
  Serial.println();
  Serial.println();
  
//esp8266 setup
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
//  rttc
  if(!Rtc.IsDateTimeValid()){
    if(Rtc.LastError() != 0){
      Serial.print("RTC communications error = ");
      Serial.println(Rtc.LastError());
    }else{
      Serial.println("RTC lost confidence in the DateTime!");
    }
  }
  RtcDateTime now = Rtc.GetDateTime();
  printDateTime(now);
  Serial.println();

  delay(1000);
  
//  dht
  hum = dht.readHumidity();
  temp= dht.readTemperature();
  
//  httppost
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    WiFiClient client;
    HTTPClient http;
    Serial.print("[HTTP] begin...\n");
    if (http.begin(client,urlGET)) {  // HTTP
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
          
          stat = data["status"];
          Serial.println(stat);

          postData(stat);
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    } 
  }
  delay(1000);
}
//post data ke database
void postData(const char* stat){
  int CurrentStatus = atoi((char*) stat);
  if( CurrentStatus != 0){
    WiFiClient client;
    HTTPClient http;
    url = "http://192.168.1.5/novasmarthome/api/deviceapi";
    http.begin(url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String DataSensor = "temp=";
    DataSensor += temp;
    DataSensor += "&hum=";
    DataSensor += hum;
    DataSensor += "&time=";
    DataSensor += datestring;
    DataSensor += "&device_category=";
    DataSensor += device_category;
    DataSensor += "&number=";
    DataSensor += number;
    Serial.println(DataSensor);
    int httpResponseCode = http.POST(DataSensor);

    if(httpResponseCode > 0){
      String response = http.getString();                       //Get the response to the request
      Serial.println(httpResponseCode);   //Print return code
      Serial.println(response);
    }else{
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }
    http.end(); //Free the resources
   }
}
//fungsi menampilkan RTC dalam bentuk HH:MM:SS
void printDateTime(const RtcDateTime& dt)
{
    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u:%02u:%02u"),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    Serial.print(datestring);
}
