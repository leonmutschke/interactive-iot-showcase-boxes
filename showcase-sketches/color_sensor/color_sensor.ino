/**
 * Sketch for color sensor on ESP32 controller
 *
 * @author leonmutschke
 */
 
#include <Arduino.h>
#include <analogWrite.h>
#include <Losant.h>
#include <WiFiClientSecure.h>
#include "Adafruit_TCS34725.h"
#include "connect_wifi.h"
#include "credentials.h"
#include "WiFi.h"

#define MQTT_MAX_PACKET_SIZE 2048

using namespace std;

WiFiClientSecure wifiClient;
LosantDevice device(getLosantCredentials().at(0));

const int rPin = 14;
const int gPin = 12;
const int bPin = 13;

const String ANY_MODE = "any";
const String RED_ONLY = "red";
const String GREEN_ONLY = "green";
const String BLUE_ONLY = "blue";

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_60X);

boolean isRedOnly;
boolean isGreenOnly;
boolean isBlueOnly;
boolean isAllColors = true;
String activeMode = ANY_MODE;

/**
 * Setting up ESP32 controller
 */
void setup() {
  Serial.begin(9600);  

  connectToWifi();
  device.onCommand(&handleCommand);
  setupLed();
}

/**
 * Main loop method
 */
void loop() { 
  delay(100);

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wifi Disconnected");
    Serial.println("Trying to reconnect...");
    connectToWifi();
    return;
  }

  if (WiFi.status() == WL_CONNECTED && !device.connected()) {
    connectToLosant();
  }

  if (!tcs.begin()) {
    return;
  }

  uint16_t clear, red, green, blue;
  tcs.setInterrupt(false);
  tcs.getRawData(&red, &green, &blue, &clear);
  tcs.setInterrupt(true);

  uint32_t sum = clear;
  float r, g, b;
  r = (float)red / (float)sum;
  g = (float)green / (float)sum;
  b = (float)blue / (float)sum;

  float r_amp;
  float g_amp;
  float b_amp;

  r_amp = r*r*r*r;
  g_amp = g*g*g*g;
  b_amp = b*b*b*b;
  
  float amp_factor = 4000;
 
  r_amp *= amp_factor;
  g_amp *= amp_factor;
  b_amp *= amp_factor;
  
  if (r_amp > 255){
    r_amp = 255;
  }
  if (g_amp > 255){
    g_amp = 255;
  }
  if (b_amp > 255){
    b_amp = 255;
  }

  float amp = 60;

  if (isRedOnly) {
    activeMode = RED_ONLY;
    int rPinCol = 0;
    if (r_amp > g_amp + amp && r_amp > b_amp + amp) {
      rPinCol = 255;
    }
    analogWrite(rPin, rPinCol);
    analogWrite(gPin, 0);
    analogWrite(bPin, 0);
  }

  if (isGreenOnly) {
    activeMode = GREEN_ONLY;
    int gPinCol = 0;
    if (g_amp > r_amp + amp && g_amp > b_amp + amp) {
      gPinCol = 255;
    }
    analogWrite(rPin, 0);
    analogWrite(gPin, gPinCol);
    analogWrite(bPin, 0);
  }
  
  if (isBlueOnly) {
    activeMode = BLUE_ONLY;
    int bPinCol = 0;
    if (b_amp > g_amp + amp && b_amp > r_amp + amp) {
      bPinCol = 255;
    }
    analogWrite(rPin, 0);
    analogWrite(gPin, 0);
    analogWrite(bPin, bPinCol);
  }

  if (isAllColors) {
    activeMode = ANY_MODE;
    analogWrite(rPin, (int)r_amp);
    analogWrite(gPin, (int)g_amp);
    analogWrite(bPin, (int)b_amp);
  }

  static unsigned long start_time = millis();
  if(millis() - start_time >= 1000) {  
    reportToLosant((int) r_amp, (int) g_amp, (int) b_amp, activeMode);
    start_time = millis();
  }
    
  device.loop();
}

/**
 * Connect to Losant
 */
void connectToLosant() {

  std::vector<const char *> chars = getLosantCredentials();

  const char* LOSANT_ACCESS_KEY = chars.at(1);
  const char* LOSANT_ACCESS_SECRET = chars.at(2);

  Serial.println("Connecting to Losant...");
  device.connectSecure(wifiClient, LOSANT_ACCESS_KEY, LOSANT_ACCESS_SECRET);
  
  if(device.connected()) {
    Serial.println("Connection to Losant established!");
  } else {
    Serial.print("MQTT State: ");
    Serial.println(device.mqttClient.state());
    connectToLosant();
  }
}

/**
 * Report the data to Losant
 * 
 * @param r
 * @param g
 * @param b
 */
void reportToLosant(int r, int g, int b, String activeMode) {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["r"] = r;
  root["g"] = g;
  root["b"] = b;
  root["activeMode"] = activeMode;
  Serial.println("Sending RGB data...");
  device.sendState(root);
}

/**
 * Setup LED
 */
void setupLed() {
  pinMode(rPin, OUTPUT);
  pinMode(gPin, OUTPUT);
  pinMode(bPin, OUTPUT);
}

/**
 * Handle callbacks
 * 
 * @param *command
 */
void handleCommand(LosantCommand *command) {
  Serial.print("Command received: ");
  Serial.println(command->name);

  JsonObject& payload = *command->payload;
  payload.printTo(Serial); 
  boolean state;

  if (strcmp(command->name, "red") == 0) {
    if (payload["red"].is<boolean>()) {
      state = (boolean)payload["red"];
      
      isRedOnly = state;
      isGreenOnly = !state;
      isBlueOnly = !state;
      isAllColors = !state;
    }
  }
  
  if (strcmp(command->name, "green") == 0) {
    if (payload["green"].is<boolean>()) {
      state = (boolean)payload["green"];
      Serial.println(state);
      
      isRedOnly = !state;
      isGreenOnly = state;
      isBlueOnly = !state;
      isAllColors = !state;
    }
  }

  if (strcmp(command->name, "blue") == 0) {
    if (payload["blue"].is<boolean>()) {
      state = (boolean)payload["blue"];
      
      isRedOnly = !state;
      isGreenOnly = !state;
      isBlueOnly = state;
      isAllColors = !state;
    }
  }
  if (strcmp(command->name, "any") == 0) {
    if (payload["any"].is<boolean>()) {
      state = (boolean)payload["any"];
      
      isRedOnly = !state;
      isGreenOnly = !state;
      isBlueOnly = !state;
      isAllColors = state;
    }
  }
}
