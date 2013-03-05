#include "settings.h"
#include <SPI.h>
#include <WiFi.h>
#include <Wire.h>
#include <SD.h>

#include "RTClib.h"
RTC_DS1307 RTC;

#include "dimmer.h"
Dimmer dim1(13, 10, 240, 20, 9*60*60, 21*60*60 );

#include "webServer.h"
char settingsFileName[] = "settings.txt";
WebServer webServer(true, 60000,10);
#include "webServerActions.h"

long prev_millis;


void setup() {
  
  Serial.begin(57600);
  Wire.begin();
  RTC.begin(); 
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
  }
  if (!SD.begin(4)) {
      Serial.println("SD initialization failed!");
      return;
  }
  Serial.println("SD initialization done.");

  dim1.setup();

  Settings::loadSettings(settingsFileName);
  Settings::printSettings();
  bool changed = false;
  if(Settings::getSettingType("ledLevel") == Settings::UNKNOWN)
  {
    Settings::setIntSetting("ledLevel", 0);
  }
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
  webServer.registerFunction(wsa_dim_mode_keyword, wsa_dim_mode);
  Serial.println("Web server actions registered.");
	prev_millis = millis();  
  
}

void loop() {
	// put your main code here, to run repeatedly: 
	webServer.loop();
  dim1.loop();
	if(millis() - prev_millis > 10000)
	{
    Serial.print("*");
		DateTime now = RTC.now();
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    
    Serial.print(" since midnight 1/1/1970 = ");
    Serial.print(now.unixtime());
    Serial.print("s = ");
    Serial.print(now.unixtime() / 86400L);
    Serial.println("d");
		prev_millis = millis();
    Serial.print("-");
		Serial.println();
  	}
}
