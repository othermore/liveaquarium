#ifndef webServerActions_h
#define webServerActions_h

char* wsa_test_keyword="GET /test HTTP/1.1";
void wsa_test(WiFiClient* client)
{
    WebServer::sendHtmlHeaders(client);
    WebServer::finishHeaders(client);
    client->println("TEST OK");
}

char* wsa_clock_keyword="GET /clock HTTP/1.1";
void wsa_clock(WiFiClient* client)
{
    WebServer::sendHtmlHeaders(client);
    WebServer::finishHeaders(client);

    client->print("<html><head><title>Clock</title></head><body>");
    DateTime now = RTC.now();
    client->print(now.year(), DEC);
    client->print('/');
    client->print(now.month(), DEC);
    client->print('/');
    client->print(now.day(), DEC);
    client->print(' ');
    client->print(now.hour(), DEC);
    client->print(':');
    client->print(now.minute(), DEC);
    client->print(':');
    client->print(now.second(), DEC);


    client->print("</body></html>");
}


char* wsa_settings_keyword="GET /settings HTTP/1.1";
void wsa_settings(WiFiClient* client)
{
    WebServer::sendPlainHeaders(client);
    WebServer::finishHeaders(client);
    Settings::printSettings(client);
}

char* wsa_settings_update_keyword="PUT /settings HTTP/1.1";
void wsa_settings_update(WiFiClient* client)
{
    WebServer::discardRemainingHeaders(client);
    //Ahora podemos empezar a leer el fichero de settings.
    Settings::loadSettings(client);
    Settings::saveSettings(settingsFileName);
    wsa_settings(client);
}

char* wsa_dim_mode_keyword="PUT /dim HTTP/1.1";
void wsa_dim_mode(WiFiClient* client)
{
    WebServer::discardRemainingHeaders(client);
    // Leemos el comando que nos mandan
    char comando [20];
    int leido = 0;
    while(client->available() && leido < 10)
    {
        comando[leido] = client->read();
        leido++;
        Serial.write(comando[leido]);
    }
    comando[leido] = '\0';
    int cmd = atoi(comando);

    Serial.print("Leido: ");
    Serial.print(leido);
    Serial.print(" Comando: ");

    Serial.print(comando);
    Serial.print(" --> ");
    Serial.println(cmd);

    if(cmd == 0)
    {
        dim1.setOff();
        Serial.println("Set OFF");
    }
    if(cmd == 1)
    {
        dim1.setOn();
        Serial.println("Set ON");
    }
    if(cmd == 2)
    {
        dim1.setOffWithDimming();
        Serial.println("Dim to OFF");
    }
    if(cmd == 3)
    {
        dim1.setOnWithDimming();
        Serial.println("Dim to ON");
    }
    if(cmd == 4)
    {
        dim1.setOff();
        Serial.println("Set OFF");
    }
    if(cmd == 5)
    {
        dim1.setOn();
        Serial.println("Set ON");
    }
    if(cmd == 6)
    {
        dim1.startDimmingOffAndAuto();
        Serial.println("Dim to OFF and AUTO");
    }
    if(cmd == 7)
    {
        dim1.startDimmingOnAndAuto();
        Serial.println("Dim to ON and AUTO");
    }
    if(cmd == 8)
    {
        dim1.setAuto();
        DateTime now = RTC.now();
        dim1.day_auto_off = (now.hour()*60+now.minute())*60+now.second()+20;
        dim1.day_auto_on = (now.hour()*60+now.minute())*60+now.second()+80;
        Serial.println("Auto to OFF in 20 secconds and ON in 80");
    }
    if(cmd == 9)
    {
        dim1.setAuto();
        DateTime now = RTC.now();
        dim1.day_auto_on = (now.hour()*60+now.minute())*60+now.second()+20;
        dim1.day_auto_off = (now.hour()*60+now.minute())*60+now.second()+80;
        Serial.println("Auto to ON in 20 secconds and OFF in 80");
    }


    
}


#endif
