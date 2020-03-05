/**
   Handling the connection to the TH02 sensor
   Getting the temperature and humidity

   @author leonmutschke
*/

#include <Arduino.h>
#include <math.h>
#include <TH02_dev.h>
#include <Wire.h>

using namespace std;

/**
   Initialize the sensor
*/
void setupTh02Sensor() {
  TH02.begin();
}

/**
   Checking if TH02 sensor is attached and running

   @return boolean
*/
boolean isTh02SensorRunning() {
  return !isnan(TH02.ReadTemperature()) && !isnan(TH02.ReadHumidity());
}

/**
   Print the temperature and humidity in the console
*/
void runTh02Sensor() {
  if (!isTh02SensorRunning()) {
    Serial.println("No DHT Sensor attached!");
    return;
  }

  float t = TH02.ReadTemperature();
  float h = TH02.ReadHumidity();

  // Display data
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C ");

  delay(2000);
}

/**
   Getting the temperature

   @return float
*/
float getTh02Temperature() {
  return TH02.ReadTemperature();
}

/**
   Getting the humidity

   @return float
*/
float getTh02Humidity() {
  return TH02.ReadHumidity();
}
