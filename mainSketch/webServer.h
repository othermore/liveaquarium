#ifndef webServer_h
#define webServer_h
#include "settings.h"

class WebServer
{
public:
    WiFiServer server;
    bool useSerial;
    long timeout;
    bool startSerial;
    int status;
    int registerCount;
    int maxRegister;
    void (**registeredFunctions)(WiFiClient*);
    char** registeredKeywords;

    WebServer(bool _startSerial, bool _useSerial, long _timeout, int _maxRegister);
    ~WebServer();


    bool setup()
    {
        if(Settings::getStringSetting("wifi_ssid") == 0 || Settings::getStringSetting("wifi_pass") == 0)
        {
            Serial.print("Missing wifi settings");
            return false;
        }
        long init_millis = millis();
        status = WL_IDLE_STATUS;
        int spent = 0;
        // attempt to connect to Wifi network:
        while (status != WL_CONNECTED && spent < timeout)
        {
            if (useSerial)
            {
                Serial.print("Attempting to connect to SSID: ");
                Serial.println(Settings::getStringSetting("wifi_ssid"));
            }
            status = WiFi.begin(Settings::getStringSetting("wifi_ssid"), Settings::getStringSetting("wifi_pass"));
            // wait 10 seconds for connection:
            delay(10000);
            spent = millis() - init_millis;
        }
        if (status != WL_CONNECTED)
        {
            if (useSerial)
            {
                Serial.print("Timeout: ");
                Serial.println(spent);
            }
            return false;
        }
        else
        {
            if (useSerial)
            {
                Serial.print("Conected: ");
                Serial.println(spent);
            }
        }
        server.begin();
        // you're connected now, so print out the status:
        if (useSerial)
            printWifiStatus();
        return true;
    }

    void printWifiStatus()
    {
        if (!useSerial)
            return;
        // print the SSID of the network you're attached to:
        Serial.print("SSID: ");
        Serial.println(WiFi.SSID());

        // print your WiFi shield's IP address:
        IPAddress ip = WiFi.localIP();
        Serial.print("IP Address: ");
        Serial.println(ip);

        // print the received signal strength:
        long rssi = WiFi.RSSI();
        Serial.print("signal strength (RSSI):");
        Serial.print(rssi);
        Serial.println(" dBm");
    }

    bool registerFunction(char* keyword, void (*function)(WiFiClient*))
    {
        if(registerCount>=maxRegister)
        {
            return false;
        }
        registeredKeywords[registerCount] = keyword;
        registeredFunctions[registerCount] = function;
        registerCount++;
        return true;
    }

    static void sendHtmlHeaders(WiFiClient* client)
    {
        client->println("HTTP/1.1 200 OK");
        client->println("Content-Type: text/html");
        client->println("Connnection: close");
    }

    static void finishHeaders(WiFiClient* client)
    {
        client->println();
    }


    static void sendPlainHeaders(WiFiClient* client)
    {
        client->println("HTTP/1.1 200 OK");
        client->println("Content-Type: text/plain");
        client->println("Connnection: close");
    }

    static void discardRemainingHeaders(WiFiClient* client)
    {
        //Tenemos que leer hasta que lleguemos a una línea en blanco
        //Asumimos que estamos al principio de una línea
        bool blankline = true;
        while(client->available())
        {
            char c = client->read();
            if(c=='\r' && client->available())
            {
                c = client->read();
                if (c == '\n')
                {
                    //Tenemos una linea nueva. Si antes teníamos otra, no hay nada
                    if(blankline)
                    {
                        return;
                    }
                    else
                    {
                        blankline = true;
                    }
                }
                else
                {
                    //Si tenemos un \r pero luego no un \n, es algo inesperado. No lo consideramos nueva línea
                    blankline = false;
                }
            }
            else
            {
                blankline = false;
            }
        }
        //Si no hay nada, no quedan headers
        return;
    }

    void loop()
    {
        // listen for incoming clients
        WiFiClient client = server.available();
        if (client)
        {
            if (useSerial)
                Serial.println("new client");
            if (client.connected())
            {
                int usedbytes = 0;
                char buffer[256];
                bool end = false;
                long start_millis = millis();
                while (!end)
                {
                    if(client.available())
                      buffer[usedbytes] = client.read(); 
                    if(useSerial)
                      Serial.write(buffer[usedbytes]);
                    if (buffer[usedbytes] == '\n' || buffer[usedbytes] == '\r' ||  usedbytes>=256 || millis()-start_millis > 3000)
                    {
                        //Si hemos recibido un \r y lo siguiente es un \n, lo quitamos para quedarnos al principio de la linea
                        if(buffer[usedbytes] == '\r' && client.peek()=='\n')
                            client.read();
                        end = true;
                        buffer[usedbytes] = '\0';
                    }
                    usedbytes++;
                }
                //Tenemos la primera linea en buffer. Hay que buscar si coincide con algo registrado
                for(int i =0;i<registerCount;i++)
                {
                    if(strcmp(registeredKeywords[i], buffer)==0)
                    {
                        if(useSerial)
                            Serial.println("Runing registered function");
                        registeredFunctions[i](&client);
                    }
                }
                // give the web browser time to receive the data
                delay(1);
                client.flush();
                // close the connection:
                client.stop();
                if (useSerial)
                    Serial.println("client disonnected");
            }
        }
    }
};

WebServer::WebServer(bool _startSerial, bool _useSerial, long _timeout, int _maxRegister):server(80)
{
    startSerial = _startSerial;
    useSerial = _useSerial;
    timeout = _timeout;
    status = WL_IDLE_STATUS;
    registerCount = 0;
    maxRegister = _maxRegister;
    registeredFunctions = (void (**)(WiFiClient*)) malloc(
          maxRegister*sizeof(
              void*
          ));
    registeredKeywords = (char**) malloc(maxRegister*sizeof(char*));

};

WebServer::~WebServer(){
    free(registeredFunctions);
    free(registeredKeywords);
}

#endif
