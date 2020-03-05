/**
 * Handling the accelerometer sensor
 *
 * @author leonmutschke
 */

#include <Arduino.h>
#include <list>
#include <vector>
#include <Wire.h>
#include "MMA7660.h"

using namespace std;

MMA7660 accelemeter;

/**
 * Set up the accelerometer sensor
 */
void setupAccelerometerSensor() {
  accelemeter.init(); 
}

/**
 * Checking if accelerometer sensor is attached and running
 *
 * @return boolean
 */
boolean isAccelerometerSensor() {
  int8_t x = 0;
  int8_t y = 0;
  int8_t z = 0;
  accelemeter.getXYZ(&x,&y,&z);
  Serial.println(x);
  Serial.println(isnan(x));
  return x != 0 && y != 0 && z != 0;
}

/**
 * Get the x, y, z position
 * 
 * @return std::vector<int8_t>
 */
std::vector<int8_t> getXYZPosition() {
  std::vector<int8_t> vector;
  int8_t x, y, z;
  accelemeter.getXYZ(&x, &y, &z);

  vector.push_back(x);
  vector.push_back(y);
  vector.push_back(z);
  return vector;
}
