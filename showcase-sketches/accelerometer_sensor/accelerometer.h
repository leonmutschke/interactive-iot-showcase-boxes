/**
 * Handling the accelerometer sensor
 *
 * @author leonmutschke
 */

#include <Arduino.h>

using namespace std;

void setupAccelerometerSensor();

boolean isAccelerometerSensor();

std::vector<int8_t> getXYZPosition();
