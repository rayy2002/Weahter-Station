#include "DHT.h"
#include <MQ135.h>
#include <Arduino.h>
#include <Wire.h>
#include <ESP8266WiFi.h>

#define DHTPIN 2 
#define I2C_ADDRESS 0x77

#define DHTTYPE DHT22
#define PIN_MQ135 A0

DHT dht(DHTPIN, DHTTYPE);
MQ135 mq135_sensor(PIN_MQ135);
BMP180I2C bmp180(I2C_ADDRESS);

String apiKey="API KEY"; //Enter your api key
const char *ssid = "SSID"; // Enter your WiFi Name
const char *pass = "PASSWORD"; // Enter your WiFi Password
const char* server = "api.thingspeak.com";

WiFiClient client;

void setup() {
  Serial.begin(9600);
  dht.begin();
  Wire.begin();
  
//  ----------------------BMP 180--------------------------- // 
  if (!bmp180.begin())
  {
    Serial.println("begin() failed. check your BMP180 Interface and I2C Address.");
    while (1);
  }

  //reset sensor to default parameters.
  bmp180.resetToDefaults();

  //enable ultra high resolution mode for pressure measurements
  bmp180.setSamplingMode(BMP180MI::MODE_UHR);

// ------------------WiFi--------------------------------- //
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP Address: ");
  delay(5000);
  Serial.println(WiFi.localIP());
  delay(5000);
 
  
}

void loop() {
  
  delay(4000);
  //Read values
  
  float p = bmp180.getPressure();
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float ppm = mq135_sensor.getPPM(); 
  float f = dht.readTemperature(true);
  if (isnan(h) || isnan(t) || isnan(f)|| isnan(p)|| isnan(ppm)) {
    Serial.println(F("Failed to read from sensors!"));
    return;
  }
  if (client.connect(server,80))   //   "184.106.153.149" or api.thingspeak.com
                      {  
                            
                             String postStr = apiKey;
                             postStr +="&field1=";
                             postStr += String(t);
                             postStr +="&field2=";
                             postStr += String(h);
                             postStr +="&field3=";
                             postStr += String(p);
                             postStr +="&field4=";
                             postStr += String(ppm);
                             postStr += "\r\n\r\n";
 
                             client.print("POST /update HTTP/1.1\n");
                             client.print("Host: api.thingspeak.com\n");
                             client.print("Connection: close\n");
                             client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
                             client.print("Content-Type: application/x-www-form-urlencoded\n");
                             client.print("Content-Length: ");
                             client.print(postStr.length());
                             client.print("\n\n");
                             client.print(postStr);
                      }

  //Print data on serial terminal                    
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%\nTemperature: "));
  Serial.print(t);
  Serial.print(F("°C "));

  //MQ135--------
  Serial.print("\nPPM: ");
  Serial.print(ppm);  
  Serial.println("ppm");
  delay(100);
  
 //BMP180----------
  delay(100);
  if (!bmp180.measurePressure())
  {
    Serial.println("could not start perssure measurement, is a measurement already running?");
    return;
  }
  do
  {
    delay(100);
  } while (!bmp180.hasValue());
  Serial.print("Pressure: "); 
  Serial.print(p);
  Serial.println(" Pa");

}
