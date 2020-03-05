/**
 * Sketch for TH02 Sensor on ESP32 controller
 *
 * @author leonmutschke
 */
 
#include <Arduino.h>
#include <analogWrite.h>
#include <Losant.h>
#include <WiFiClientSecure.h>
#include "connect_wifi.h"
#include "credentials.h"
#include "temperature.h"

#define MQTT_MAX_PACKET_SIZE 2048

using namespace std;

WiFiClientSecure wifiClient;

std::vector<const char *> chars = getLosantCredentials();

const char* LOSANT_DEVICE_ID = chars.at(0);
const char* LOSANT_ACCESS_KEY = chars.at(1);
const char* LOSANT_ACCESS_SECRET = chars.at(2);

const int rPin = 14;
const int gPin = 12;
const int bPin = 13;

int maxTemp = 27;
int minTemp = 19;

LosantDevice device(LOSANT_DEVICE_ID);

/**
 * Setting up ESP32 controller
 */
void setup() {
  Serial.begin(9600);
  delay(2000);

  connectToWifi();
  enableLed(maxTemp);
  device.onCommand(&handleCommand);
  setupTh02Sensor();
  setupLed();
}

/**
 * Main loop method
 */
void loop() { 
  delay(1000);

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wifi Disconnected");
    Serial.println("Trying to reconnect...");
    connectToWifi();
    return;
  }

  if (WiFi.status() == WL_CONNECTED && !device.connected()) {
    connectToLosant();
  }  

  if (isTh02SensorRunning()) {
    runTh02Sensor();
    float temperature = getTh02Temperature();
    float humidity = getTh02Humidity();

    if (!isnan(temperature) && !isnan(humidity) && device.connected() && humidity > 0) {
      reportToLosant(temperature, humidity);
      enableLed(temperature);
    }
  }
  device.loop();
}

/**
 * Connect to Losant
 */
void connectToLosant() {

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
 * @param temperature
 * @param humidity
 */
void reportToLosant(float temperature, float humidity) {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["temperature"] = temperature;
  root["humidity"] = humidity;
  root["maxTemp"] = maxTemp;
  root["minTemp"] = minTemp;  
  device.sendState(root);
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

  if (strcmp(command->name, "tempMax") == 0) {
    if (payload["max"].is<int>()) {
      changeMaxTemp((int)payload["max"]);
    }
  }

  if (strcmp(command->name, "tempMin") == 0) {
    if (payload["min"].is<int>()) {
      changeMinTemp((int)payload["min"]);
    }
  }
}

/**
 * Setup LED
 */
void setupLed() {
  pinMode(rPin, OUTPUT);
  pinMode(gPin, OUTPUT);
  pinMode(bPin, OUTPUT);
}

void changeMaxTemp(int max) {
  maxTemp = max;
}

void changeMinTemp(int min) {
  minTemp = min;
}

/**
 * Enable LED
 * 
 * @param temperature
 */
void enableLed(int temperature) {

  int rVal,bVal = 0;
  int gVal = 255;

  if (maxTemp < minTemp || maxTemp == minTemp) {
    maxTemp = minTemp + 2;
  }

  // Middle between maxTemp and minTemp
  int middleTemp = (int)(maxTemp + minTemp) / 2;

  // Middele between middleTemp and maxTemp
  int midMidMax = (int)(maxTemp - minTemp) * 0.75 + minTemp;

  // Middle between middleTemp and minTemp
  int midMidMin = (int)(maxTemp - minTemp) * 0.25 + minTemp;

  if (temperature > middleTemp && temperature <= midMidMax) {
    rVal = 255 - (255 / (midMidMax - middleTemp)) * (midMidMax - temperature);
    gVal = 255;
    bVal = 0;
  }

  if (temperature > midMidMax) {
    rVal = 255;
    gVal = (255 / (maxTemp - midMidMax)) * (maxTemp - temperature);
    bVal = 0;
  }

  if (temperature >= maxTemp) {
    rVal = 255;
    gVal = 0;
    bVal = 0;
  }

  if (temperature < middleTemp && temperature >= midMidMin) {
    rVal = 0;
    gVal = 255;
    bVal = 255 - (255 / (middleTemp - midMidMin)) * (temperature - midMidMin);
  }
  
  if (temperature < midMidMin) {
    rVal = 0;
    gVal = (255 / (midMidMin - minTemp)) * (temperature - minTemp);;
    bVal = 255;
  } 
  
  if (temperature <= minTemp) { 
    rVal = 0;
    gVal = 0;
    bVal = 255;
  }

  analogWrite(rPin, rVal);
  analogWrite(gPin, gVal);
  analogWrite(bPin, bVal);
}
