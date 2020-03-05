/**
 * Sketch for Digital Light Sensor on ESP32 controller
 *
 * @author leonmutschke
 */
 
#include <Arduino.h>
#include <analogWrite.h>
#include <Digital_Light_TSL2561.h>
#include <Losant.h>
#include <math.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include "connect_wifi.h"
#include "credentials.h"

#define MQTT_MAX_PACKET_SIZE 2048

using namespace std;

WiFiClientSecure wifiClient;

std::vector<const char *> chars = getLosantCredentials();

const char* LOSANT_DEVICE_ID = chars.at(0);
const char* LOSANT_ACCESS_KEY = chars.at(1);
const char* LOSANT_ACCESS_SECRET = chars.at(2);

const int ledPin = 13;

long val;
long str;

LosantDevice device(LOSANT_DEVICE_ID);

/**
 * Setting up ESP32 controller
 */
void setup() {
  Wire.begin();
  Serial.begin(9600);
  delay(2000);

  TSL2561.init();

  connectToWifi();
  device.onCommand(&handleCommand);
  setupLed();
}

/**
 * Main loop method
 */
void loop() { 
  delay(200);
  int led, val;

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wifi Disconnected");
    Serial.println("Trying to reconnect...");
    connectToWifi();
    return;
  }

  if (WiFi.status() == WL_CONNECTED && !device.connected()) {
    connectToLosant();
  }  

  signed long lux;
  
  lux = TSL2561.readVisibleLux();
  
  //Serial.print("The Light value is: ");
  //Serial.println(lux);

  if (!isnan(lux)) {

    int max = 3000;
    lux = lux > max ? max : lux;

    led = round((1-((float)lux / max)) * 255);
    toggleLed(led);

    static unsigned long start_time = millis();
    if(millis() - start_time >= 2*1000) {  
      int ledP = round(((float)led / 255) * 100);
      reportToLosant(lux, ledP);
      start_time = millis();
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
 * @param lux
 */
void reportToLosant(int lux, int ledP) {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["lux"] = lux;
  root["led"] = ledP;
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
}

/**
 * Setup LED
 */
void setupLed() {
  pinMode(ledPin, OUTPUT);
}

/**
 * Toggle LED
 * 
 * @param led
 */
void toggleLed(int led) {

  if(led > 255){
    led = 255;
  }

  analogWrite(ledPin, led);
}
