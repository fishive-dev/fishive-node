#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Servo.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"

// For the DS18B20
#include <OneWire.h>
#include <DallasTemperature.h>

//Servo pwm pin: D5
static const int servoPin = 5;

Servo servo1;

// For the RGB WS2812B
#include <Adafruit_NeoPixel.h>

// For OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

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

// OLED Screen Stuff
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// NeoPixel
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(60, rgb13, NEO_GRB + NEO_KHZ800);

// GLOBAL VARIABLES
float temp[2];
int ambientLight = 0;
bool wifiConnected = false;

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

void servomotor() {
    for(int posDegrees = 0; posDegrees <= 180; posDegrees++) {
        servo1.write(posDegrees);
        Serial.println(posDegrees);
        delay(20);
    }

    for(int posDegrees = 180; posDegrees >= 0; posDegrees--) {
        servo1.write(posDegrees);
        Serial.println(posDegrees);
        delay(20);
    }
}

void wifiInitlizer()
{

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    wifiConnected = false;
    Serial.printf("WiFi Connection Failed!\n");
    Serial.println(String(ssid) + " not found!");
  }
  wifiConnected = true;
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
      for (size_t i = 0; i < 60; i++)
        pixels.setPixelColor(i, pixels.Color(data["r"], data["g"], data["b"]));
      
      pixels.show();
    }
    else if(data["state"] == "off")
    {
      Serial.println("INFO: RGB is off");
      Serial.println("GPIO: 13 off");
      pixels.setBrightness(0);
      pixels.show();
    }
    else
      Serial.println("WARN: RGB is offline");

    String response;
    serializeJson(data, response);
    request->send(200, "application/json", response); });
}

void pizeoHandler()
{
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

void displayUpdate()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.cp437(true);
  display.println("~ Fisive Node v0.2 ~");
  if (wifiConnected)
    display.println("IP: " + WiFi.localIP().toString());
  else
    display.println(" WiFi not connected ");

  display.println("SSID:" + WiFi.SSID());
  display.println("Temp 1: " + String(temp[0]) + " *C");
  display.println("Temp 2: " + String(temp[1]) + " *C");
  display.println("Ambient Light: " + String(ambientLight));
  display.println("RGB:" + String(pixels.getPixelColor(0)));
  display.display();
}

void displayInitialize()
{
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  delay(1000);
  display.clearDisplay();
  display.display();
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.cp437(true);
}

void setup()
{
  Serial.begin(115200);
  displayInitialize();
  wifiInitlizer();
  pinInitializer();
  emptyHandler();
  sensorHandler();
  pumpHandler();
  rgbHandler();
  pizeoHandler();
  
  //Servo motor
  servo1.attach(servoPin);

  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");
  server.onNotFound(notFound);
  server.begin();
}
void loop()
{
  
  //Servo motor
  servomotor();
  
  // Sesnor state updates
  updateSensorData();
  displayUpdate();
  sleep(1);
}
