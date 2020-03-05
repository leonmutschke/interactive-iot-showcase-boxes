/**
 * Sketch for accelerometer sensor on ESP32 controller
 *
 * @author leonmutschke
 */
 
#include <Arduino.h>
#include <analogWrite.h>
#include <Losant.h>
#include <math.h>
#include <WiFiClientSecure.h>
#include "accelerometer.h"
#include "connect_wifi.h"
#include "credentials.h"
#include "WiFi.h"

#define MQTT_MAX_PACKET_SIZE 4096

using namespace std;

WiFiClientSecure wifiClient;
LosantDevice device(getLosantCredentials().at(0));

const int rPin = 14;
const int gPin = 12;
const int bPin = 13;

boolean isPositionControl = false;
int amountTilted = 0;
int warningCounterLimit = 0;
boolean moved = false;
boolean warningSent = false;
boolean sendWarning = false;
int rVal, gVal, bVal;
int x = 1;
int y = 1;
int z = 1;


/**
 * Setting up ESP32 controller
 */
void setup() {
  Serial.begin(9600);  

  connectToWifi();
  device.onCommand(&handleCommand);
  setupLed();
  setupAccelerometerSensor();
}

/**
 * Main loop method
 */
void loop() { 
  delay(30);

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wifi Disconnected");
    Serial.println("Trying to reconnect...");
    connectToWifi();
    return;
  }

  if (WiFi.status() == WL_CONNECTED && !device.connected()) {
    connectToLosant();
  }

  std::vector<int8_t> pos = getXYZPosition();
  x = round((x + (int)pos.at(0)) / 2);
  y = round((y + (int)pos.at(1)) / 2);
  z = round((z + (int)pos.at(2)) / 2);

  if (!isPositionControl) {

    changeRed((x*x)/1.6);
    changeGreen((y*y)/1.6);
    changeBlue((z*z)/1.6);

  } else {
    changeBlue(0);    

    if ((x*x) <= 25 && (y*y) <= 25 && z > 0) {
      changeGreen(255);
      changeRed(0);
      moved = true;
    } else if (((x*x) <= 225 && (y*y) <= 225) && z > 0) {
      int tmp = x;
      if (x*x < y*y) {
        tmp = y;
      }
      changeGreen(255 - (tmp*tmp)/1.6);
      changeRed(tmp*tmp); 
    } else {
      changeRed(255);
      changeGreen(0);

      if (moved) {
        moved = false;
        amountTilted++;
        if(warningCounterLimit > 0 && !warningSent && amountTilted >= warningCounterLimit) {
          warningSent = true;
          sendWarning = true;
        }
      }
    }
  }

  static unsigned long start_time = millis();
  if(millis() - start_time >= 2000) {
    reportToLosant(sendWarning);
    if (sendWarning) {
      sendWarning = !sendWarning;
    }
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
 * @param warning
 */ 
void reportToLosant(boolean warning) {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  
  root["isPositionControl"] = isPositionControl;
  if (isPositionControl) {
    root["warning"] = warning;
    root["amountTilted"] = amountTilted;
  } else {
    root["r"] = rVal;
    root["g"] = gVal;
    root["b"] = bVal;
  }
  //Serial.println("Sending acceleration data...");
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
 * Change red LED
 * 
 * @param boolean
 */
void changeRed(int red) {

  rVal = red > 255 ? 255 : red;

  analogWrite(rPin, rVal);
}

/**
 * Change green LED
 * 
 * @param boolean
 */
void changeGreen(int green) {

  gVal = green > 255 ? 255 : green;

  analogWrite(gPin, gVal);
}

/**
 * Change blue LED
 * 
 * @param boolean
 */
void changeBlue(int blue) {

  bVal = blue > 255 ? 255 : blue;

  analogWrite(bPin, bVal);
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

  if (strcmp(command->name, "posCon") == 0) {
    if (payload["pos"].is<boolean>()) {
      isPositionControl = (boolean)payload["pos"];
    }
  }

  if (strcmp(command->name, "warn") == 0) {
    if (payload["step"].is<int>()) {
      warningCounterLimit = (int)payload["step"];
      amountTilted = 0;
      warningSent = false;
    }
  }

  if (strcmp(command->name, "reset") == 0) {
    if (payload["res"].is<boolean>()) {
      if((boolean)payload["res"]) {
        amountTilted = 0;
        warningSent = false;
      }
    }
  }
}
