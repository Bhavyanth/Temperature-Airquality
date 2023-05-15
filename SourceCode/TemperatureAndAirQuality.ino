#include "heltec.h"
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#define BLYNK_TEMPLATE_ID "TMPLI2oDQqir"
#define BLYNK_TEMPLATE_NAME "iot group 4"
#define BLYNK_AUTH_TOKEN "7xFpQtW7R_zGC9XQeKJ5x5wmBqqF53EK"

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#define DHT_SENSOR_PIN 22
#define DHT_SENSOR_TYPE DHT22
int air_input = 32;

//String GoogleSheetId = "AKfycbx7xsd5IUua2FuwwTosn1Badvm3m9ZJ-_sNtvaoWkU_sFS4dEgtVL7-6ON2yePar04M"; // Zone 1
//String GoogleSheetId ="AKfycbxKZW9Y1dpFNciFs4yjvjgAfDbxKMLjyuC-cOUvlHJSiCCcgA-8NgR-DpIr4JaOq8lT"; // Zone 2
String GoogleSheetId = "AKfycbxvE2yZI-tI0gRO5wzducNoWQlV20t4H08z5icXj11R6NLt6VQ04Nin6KWHig29vmaoJg"; //Zone 3

char auth[] = BLYNK_AUTH_TOKEN;

char ssid[] = "Goldfish Evian";
char pass[] = "lazywater505";

BlynkTimer timer;
DHT dht(DHT_SENSOR_PIN, DHT_SENSOR_TYPE); 
const int sendInt = 50;

WiFiClientSecure client;
void sendSensor(){
  
  int analog_data = dht.readHumidity();
  int temp_data = dht.readTemperature(true);
  int air_data = analogRead(air_input);
  delay(200);

Blynk.virtualWrite(V0,temp_data);
Blynk.virtualWrite(V1,air_data);
Blynk.virtualWrite(V2,analog_data);
delay(300);
}

int airQual = 0;
float currTempF = 0.0;
float currHumidity = 0.0;

void displayValuesOled(){
  String tempFarenheit = "Temperature : " + (String)currTempF + "*F";
  String humidtyDisplay = "Humidity : " + (String)currHumidity + "%";
  String airQuality = "AirQuality : " + (String)airQual;

  Heltec.display->clear();
  Heltec.display->drawString(0,0,tempFarenheit);
  Heltec.display->drawString(0,12,humidtyDisplay);
  Heltec.display->drawString(0,24,airQuality);
  Heltec.display->display();
}

void setup(){
  Serial.begin(9600);
  dht.begin();
  Blynk.begin(auth,ssid,pass);
  timer.setInterval(100L, sendSensor);
   Heltec.begin(true /*DisplayEnable Enable*/,false /*LoRa Enable*/,true /*Serial Enable*/);
  
  currTempF = dht.readTemperature(true);
  currHumidity = dht.readHumidity();
  airQual = analogRead(air_input);
  displayValuesOled();
  delay(1000);
    WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,pass);

  Serial.print("Connecting to WiFI");
  while(WiFi.status() != WL_CONNECTED){
    digitalWrite(2,LOW);
    delay(2000);
    Serial.print("*");
    digitalWrite(2,HIGH);
    delay(2000);
  }
  Serial.print("OKAY");
}

void loop(){
  Blynk.run();
  timer.run();
  delay(2000);
  currTempF = dht.readTemperature(true);
  currHumidity = dht.readHumidity();
  airQual = analogRead(air_input);
  write_google_sheet("sensor="+String(currHumidity)+"&sensor1="+String(currTempF)+"&sensor2="+String(airQual));
}

void write_google_sheet(String params){
HTTPClient http;
String url ="https://script.google.com/macros/s/"+GoogleSheetId+"/exec?"+params;
Serial.println(url);
Serial.println("Updating sensor status");
http.begin(url.c_str());
http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
int httpCode = http.GET();
Serial.print("Http status code: ");
Serial.println(httpCode);
String payLoad;
if(httpCode > 0){
  payLoad = http.getString();

  Serial.println("Payload:  "+ payLoad);
}
http.end();
}