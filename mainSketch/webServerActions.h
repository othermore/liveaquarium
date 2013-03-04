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
    char time[] = "00-00-00 00:00:00x";
    RTC.readClock();
    RTC.getFormatted(time);
    client->print("<html><head><title>Clock</title></head><body>");
    client->print(time);
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


#endif
