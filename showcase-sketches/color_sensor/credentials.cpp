/**
 * Containing all kinds of credentials
 *
 * @author leonmutschke
 */

#include <Arduino.h>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

/**
 * Get the wifi credentials
 *
 * @return std::vector<const char *>
 */
std::vector<const char *> getWifiCredentials() {
  
  // ------------------------------------------
  // ENTER YOUR WIFI CREDENTIALS BELOW
  // ------------------------------------------
  const char * SSID = "";
  const char * PASSWORD = "";


  // ------------------------------------------
  // DO NOT TOUCH CODE BELOW
  // ------------------------------------------
  std:vector<const char *> credentials;
  credentials.push_back(SSID);
  credentials.push_back(PASSWORD);
  return credentials;
}

/**
 * Get the Losant credentials
 *
 * @return std::vector<const char *>
 */
std::vector<const char *> getLosantCredentials() {
  
  // ------------------------------------------
  // ENTER YOUR LOSANT CREDENTIALS BELOW
  // ------------------------------------------
  const char* LOSANT_DEVICE_ID = "";
  const char* LOSANT_ACCESS_KEY = "";
  const char* LOSANT_ACCESS_SECRET = "";

  // ------------------------------------------
  // DO NOT TOUCH CODE BELOW
  // ------------------------------------------
  std:vector<const char *> credentials;

  credentials.push_back(LOSANT_DEVICE_ID);
  credentials.push_back(LOSANT_ACCESS_KEY);
  credentials.push_back(LOSANT_ACCESS_SECRET);
  return credentials;
}
