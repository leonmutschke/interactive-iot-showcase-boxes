/**
 * Handling the connection to the TH02 sensor
 * Getting the temperature and humidity
 *
 * @author leonmutschke
 */
 
#include <Arduino.h>

using namespace std;

void setupTh02Sensor();

void runTh02Sensor();

boolean isTh02SensorRunning();

float getTh02Temperature();

float getTh02Humidity();
