#include "settings.h"
#include <SPI.h>
#include <WiFi.h>
#include <Wire.h>
#include <SD.h>

#include "RealTimeClockDS1307.h"
#include "webServer.h"

char settingsFileName[] = "settings.txt";
WebServer webServer(true, 60000,10);
#include "webServerActions.h"

long prev_millis;
char formatted[] = "00-00-00 00:00:00x";
void setup() {

  Serial.begin(9600);
  if (!SD.begin(4)) {
      Serial.println("SD initialization failed!");
      return;
  }
  Serial.println("SD initialization done.");

  Settings::loadSettings(settingsFileName);
  Settings::printSettings();
  bool changed = false;
  if(!Settings::getStringSetting("wifi_ssid"))
  {
    changed = true;
    Settings::setStringSetting("wifi_ssid", "************");
  }
  if(!Settings::getStringSetting("wifi_pass"))
  {
    changed = true;
    Settings::setStringSetting("wifi_pass", "*************");
  }
  if(changed)
  {
    Settings::saveSettings(settingsFileName);
    Settings::loadSettings(settingsFileName);
    Serial.println("===Settings Changed===");
    Settings::printSettings();
  }

	webServer.setup();
	Serial.println("Web server started.");

	webServer.registerFunction(wsa_test_keyword, wsa_test);
  webServer.registerFunction(wsa_clock_keyword, wsa_clock);
  webServer.registerFunction(wsa_settings_keyword, wsa_settings);
  webServer.registerFunction(wsa_settings_update_keyword, wsa_settings_update);
  Serial.println("Web server actions registered.");
	prev_millis = millis();      
}

void loop() {
	// put your main code here, to run repeatedly: 
	webServer.loop();
	if(millis() - prev_millis > 10000)
	{
                Serial.print("*");
		RTC.readClock();
		RTC.getFormatted(formatted);
		prev_millis = millis();
                Serial.print("-");
		Serial.print(formatted);
		Serial.println();
  	}
}
