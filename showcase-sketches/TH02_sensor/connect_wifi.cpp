/**
 * Connecting ESP32 controller to wifi
 *
 * @author leonmutschke
 */

#include <fstream>
#include <iostream>
#include <vector>
#include "credentials.h"
#include "WiFi.h"

using namespace std;

/**
 * Connecting to wifi
 */
void connectToWifi() {

  const int MAX_CONNECTION_ATTEMPTS = 10;

  std::vector<const char *> chars = getWifiCredentials();

  const char* ssid = chars.at(0);
  const char* password = chars.at(1);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.println("\n");

  WiFi.begin(ssid, password);
  Serial.print("Connecting to: ");
  Serial.print(ssid);
  Serial.print("\n");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED && i < MAX_CONNECTION_ATTEMPTS) {
    delay(500);
    Serial.print('.');
    i++;
    if (i == MAX_CONNECTION_ATTEMPTS) {
      Serial.print("\n");
      Serial.print("Unable to connect to wifi!");
      Serial.print("\n");
      return;
    }
  }

  Serial.println("Connection established!");
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
}
