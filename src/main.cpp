#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"

// For the DS18B20
#include <OneWire.h>
#include <DallasTemperature.h>

// For the RGB WS2812B
#include <Adafruit_NeoPixel.h>

// Start webserver at port 80
AsyncWebServer server(80);

// WIFI Credentials
const char *ssid = "Wu-Tang Lan_2.4";
const char *password = "bluesmoonshot";

// Global GPIO Pin Setup
const int relay12 = 12;
const int ambientLight4 = 4;
const int rgb13 = 13;
const int pizeo26 = 26;

// One wire sensors
const int oneWireBus = 2;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

// NeoPixel
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(60, rgb13, NEO_GRB + NEO_KHZ800);

// GLOBAL VARIABLES
float temp[2];
int ambientLight = 0;

void pinInitializer()
{
  pixels.begin();
  pinMode(relay12, OUTPUT);
  pinMode(ambientLight4, INPUT);
  pinMode(pizeo26, OUTPUT);
  // Set outputs to LOW
  
  digitalWrite(relay12, LOW);
  digitalWrite(pizeo26, LOW);
}

void wifiInitlizer()
{

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.printf("WiFi Connection Failed!\n");
    Serial.println(String(ssid) + " not found!");
  }
  Serial.println("Connection Succesful!");
  Serial.println("IP Address: ");
  Serial.print(WiFi.localIP());
}

void updateTempData()
{
  sensors.requestTemperatures();
  temp[0] = sensors.getTempCByIndex(0);
  temp[1] = sensors.getTempCByIndex(1);
  Serial.println("INFO: Fetching Temperature Data");
  Serial.println("DATA: Temp_1: " + String(temp[0]) + " *C; Temp_2: " + String(temp[1]) + " *C");
}

void updateLightData()
{
  ambientLight = digitalRead(ambientLight4);
  Serial.println("INFO: Fetching Light Data");
  Serial.println("DATA: Ambient Light: " + String(ambientLight));
}

void updateSensorData()
{
  updateTempData();
  updateLightData();
}

void emptyHandler()
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    StaticJsonDocument<100> data;
    data["message"] = "Welcome to your fisive node";
    String response;
    serializeJson(data, response);
    request->send(200, "application/json", response); });
}

void sensorHandler()
{
  server.on("/get-sensors", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    StaticJsonDocument<100> data;
    data["temp_1"] = temp[0];
    data["temp_2"] = temp[1];
    data["ambient_light"] = ambientLight;
    String response;
    serializeJson(data, response);
    request->send(200, "application/json", response); });
}

void pumpHandler()
{
  server.on("/pump-state", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    StaticJsonDocument<100> data;
    if (request->hasParam("state"))
      data["state"] = request->getParam("state")->value();
    else
      data["message"] = "No message parameter";


    if(data["state"] == "on")
    {
      Serial.println("INFO: Pump is on");
      Serial.println("GPIO: 12 ON");
      digitalWrite(relay12, HIGH);

    }
    else if(data["state"] == "off")
    {
      Serial.println("INFO: Pump is off");
      Serial.println("GPIO: 12 off");
      digitalWrite(relay12, LOW);
    }
    else
      Serial.println("WARN: Pump is offline");

    String response;
    serializeJson(data, response);
    request->send(200, "application/json", response); });
}

void rgbHandler()
{
  server.on("/rgb-state", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    StaticJsonDocument<100> data;
    if (request->hasParam("state")){
      data["state"] = request->getParam("state")->value();
      data["r"] = request->getParam("r")->value();
      data["g"] = request->getParam("g")->value();
      data["b"] = request->getParam("b")->value();
      data["brightness"] = request->getParam("brightness")->value();
    }
    else
      data["message"] = "No message parameter";

    if(data["state"] == "on")
    {
      Serial.println("INFO: RGB is on");
      Serial.println("GPIO: 13 ON");
      pixels.setBrightness(data["brightness"]);
      pixels.setPixelColor(0, pixels.Color(data["r"], data["g"], data["b"]));
      pixels.show();
    }
    else if(data["state"] == "off")
    {
      Serial.println("INFO: RGB is off");
      Serial.println("GPIO: 13 off");
      pixels.setBrightness(0);
      pixels.setPixelColor(0, pixels.Color(0, 0, 0));
      pixels.show();
    }
    else
      Serial.println("WARN: RGB is offline");

    String response;
    serializeJson(data, response);
    request->send(200, "application/json", response); });
}

void pizeoHandler(){
  server.on("/pizeo-state", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    StaticJsonDocument<100> data;
    if (request->hasParam("state"))
      data["state"] = request->getParam("state")->value();
    else
      data["message"] = "No message parameter";


    if(data["state"] == "on")
    {
      Serial.println("INFO: Pizeo is on");
      Serial.println("GPIO: 26 ON");
      digitalWrite(pizeo26, HIGH);

    }
    else if(data["state"] == "off")
    {
      Serial.println("INFO: Pizeo is off");
      Serial.println("GPIO: 26 off");
      digitalWrite(pizeo26, LOW);
    }
    else
      Serial.println("WARN: Pizeo is offline");

    String response;
    serializeJson(data, response);
    request->send(200, "application/json", response); });
}

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "application/json", "{\"status\":\"404\"}");
}

void setup()
{
  Serial.begin(115200);
  wifiInitlizer();
  pinInitializer();
  emptyHandler();
  sensorHandler();
  pumpHandler();
  rgbHandler();
  pizeoHandler();
  server.onNotFound(notFound);
  server.begin();
}
void loop()
{
  //Sesnor state updates
  updateSensorData();
  sleep(1);
}