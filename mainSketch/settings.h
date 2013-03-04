#ifndef settings_h
#define settings_h

#include <SD.h>
/********************
 *  SETTINGS WIFI   *
 ********************/

#define MAX_SETTINGS 50
#define SETTINGS_KEY_LENGTH 25
#define BUFFER_SIZE 100

 char settings_buffer[BUFFER_SIZE];
 int settings_buffer_pos;
 class Settings
 {
 public:
 	enum settingType { INT, FLOAT, STRING, UNKNOWN };
 	struct DynamicSettings{
 		char keys[MAX_SETTINGS][SETTINGS_KEY_LENGTH];
 		settingType types[MAX_SETTINGS];
 		void* values[MAX_SETTINGS];
 		int used;
 	};
 	static DynamicSettings d;

 	static void loadSettings(char* file)
 	{
        File settingsFile;
        //Ponemos a 0 todos los valores
        if (!SD.exists(file))
        {
            settingsFile = SD.open(file, FILE_WRITE);
            settingsFile.close();
        }
        settingsFile = SD.open(file, FILE_READ);
        loadSettings(&settingsFile);
        settingsFile.close();
        return;
    }

    static void loadSettings(Stream* stream)
    {
 		d.used=0;
    	//Hacemos free de todos los valores
 		for(int i = 0; i< MAX_SETTINGS;i++)
 		{
 			if(d.values[i] != 0)
 			{
 				free(d.values[i]);
 				d.values[i] = 0;
 			}
 		}

 		int modo = 0;
 		settings_buffer_pos = 0;
 		while (stream->available()) {
 			char c = stream->read();
        	//Vamos leyendo tipo, nombre y valor
        	//Los dos primeros se separan por un espacio y el último es hasta el intro (/r o /n)
        	//Leemos hasta el primer espacio
 			settings_buffer[settings_buffer_pos] = c;
 			settings_buffer_pos++;
        	if (modo ==0) //Esperamos el tipo
        	{
        		if(c==' '||c=='\t')
        		{
        			settings_buffer_pos--;
        			settings_buffer[settings_buffer_pos]='\0';
        			modo++;
        			//Procesamos el tipo
        			if(strcmp(settings_buffer,"INT")==0)
        			{
        				d.types[d.used] = INT;
        			}
        			else if(strcmp(settings_buffer,"FLOAT")==0)
        			{
        				d.types[d.used] = FLOAT;
        			}
        			else
        			{
        				d.types[d.used] = STRING;
        			}
        			//Eliminamos todos los espacios o tabuladores
        			while(stream->available() && (stream->peek() == ' ' || stream->peek() == '\t'))
        				stream->read();
        			//Reseteamos el buffer
        			settings_buffer_pos = 0;
        		}
        	}
        	else if (modo == 1) // Esperamos el nombre
        	{
        		if(c==' '||c=='\t')
        		{
        			settings_buffer_pos--;
        			settings_buffer[settings_buffer_pos]='\0';
        			modo++;
                    //Buscamos el nombre por si ya existe. 
                    for(int i = 0;i<d.used;i++)
                    {
                        if(strcmp(d.keys[i],settings_buffer)==0)
                        {
                            //Existe ya, así que borramos la que existe y movemos todas
                            free (d.values[i]);
                            for(int j = i; j<d.used;j++)
                            {
                                strcpy(d.keys[j],d.keys[j+1]);
                                d.values[j] = d.values[j+1];
                                d.types[j] = d.types[j+1];
                            }
                            d.values[d.used] = 0;
                            d.used--;
                            break;
                        }
                    }
        			//Guardamos el nombre
        			strcpy(d.keys[d.used], settings_buffer);
        			//Eliminamos todos los espacios o tabuladores
        			while(stream->available() && (stream->peek() == ' ' || stream->peek() == '\t'))
        				stream->read();
        			//Reseteamos el buffer
        			settings_buffer_pos = 0;
        		}
        	}
        	else if (modo == 2) //Esperamos el valor
        	{
        		if(c=='\n'||c=='\r')
        		{
        			settings_buffer_pos--;
        			settings_buffer[settings_buffer_pos]='\0';
        			modo=0;
        			//Guardamos el valor
        			if(d.types[d.used] == INT)
        			{
                        d.values[d.used] = (void*)malloc(sizeof(int));
        				*((int*)d.values[d.used]) = atoi(settings_buffer);
        			}
        			else if(d.types[d.used] == FLOAT)
        			{
                        d.values[d.used] = (void*)malloc(sizeof(float));
        				*((float*)d.values[d.used]) = atof(settings_buffer);
        			}
        			else
        			{
        				d.values[d.used] = (void*)malloc(sizeof(char)*(settings_buffer_pos+2));
        				strcpy((char*)(d.values[d.used]), settings_buffer);
        			}
        			//Eliminamos todos los intros
        			while(stream->available() && (stream->peek() == '\r' || stream->peek() == '\n'))
        				stream->read();
        			//Reseteamos el buffer
        			settings_buffer_pos = 0;
        			d.used++;
        		}
        	}
        }
        return;
    }

    static void printSettings()
    {
    	printSettings(&Serial);
    }

    static void printSettings(Stream *s)
    {
        for(int i = 0; i<d.used; i++)
        {
            if(d.types[i]== INT)
                s->print("INT");
            else if (d.types[i]==FLOAT)
                s->print("FLOAT");
            else
                s->print("STRING");
            s->print('\t');
            s->print(d.keys[i]);
            s->print('\t');
            if(d.types[i]== INT)
                s->println(*(int*)d.values[i]);
            else if (d.types[i]==FLOAT)
                s->println(*(float*)d.values[i]);
            else
                s->println((char*)d.values[i]);
        }
    }

    static void saveSettings(char* file)
    {
        File settingsFile;
    	if (SD.exists(file))
 		{
 			SD.remove(file);
 		}
 		settingsFile = SD.open(file, FILE_WRITE);

    	for(int i = 0; i<d.used; i++)
    	{
    		if(d.types[i]== INT)
    			settingsFile.print("INT");
    		else if (d.types[i]==FLOAT)
    			settingsFile.print("FLOAT");
    		else
    			settingsFile.print("STRING");
    		settingsFile.print('\t');
    		settingsFile.print(d.keys[i]);
    		settingsFile.print('\t');
    		if(d.types[i]== INT)
    			settingsFile.println(*(int*)d.values[i]);
    		else if (d.types[i]==FLOAT)
    			settingsFile.println(*(float*)d.values[i]);
    		else
    			settingsFile.println((char*)d.values[i]);
    	}
    	settingsFile.close();
    }
    
    static void initSettings()
    {
    	d.used = 0;
    	for (int i = 0; i<d.used;i++)
    		d.values[i] = 0;
    }

    static int getIntSetting(char* name)
    {
    	for (int i = 0; i<d.used;i++)
    	{
    		if(strcmp(d.keys[i], name)==0 && d.types[i] == INT)
    			return *((int*)d.values[i]);
    	}
    	return 0;
    }

    static float getFloatSetting(char* name)
    {
    	for (int i = 0; i<d.used;i++)
    	{
    		if(strcmp(d.keys[i], name)==0 && d.types[i] == FLOAT)
    			return *((float*)d.values[i]);
    	}
    	return 0;
    }

    static char* getStringSetting(char* name)
    {
    	for (int i = 0; i<d.used;i++)
    	{
    		if(strcmp(d.keys[i], name)==0 && d.types[i] == STRING)
    			return (char*)d.values[i];
    	}
        return 0;
    }

    static settingType getSettingType(char* name)
    {
    	for (int i = 0; i<d.used;i++)
    	{
    		if(strcmp(d.keys[i], name)==0 )
    			return d.types[i];
    	}
    	return UNKNOWN;
    }

    static void setIntSetting(char* name, int value )
    {
    	for (int i = 0; i<d.used;i++)
    	{
    		if(strcmp(d.keys[i], name)==0 && d.types[i] == INT)
    		{
    			*((int*)d.values[i]) = value;
    			return;
    		}
    	}
    	d.values[d.used] = (void*)malloc(sizeof(int));
    	*(int*)d.values[d.used] = value;
    	d.types[d.used] = INT;
    	strcpy(d.keys[d.used],name);
    	d.used++;
    }

    static void setFloatSetting(char* name, float value )
    {
    	for (int i = 0; i<d.used;i++)
    	{
    		if(strcmp(d.keys[i], name)==0 && d.types[i] == FLOAT)
    		{
    			*((float*)d.values[i]) = value;
    			return;
    		}
    	}
    	d.values[d.used] = (void*)malloc(sizeof(float));
    	*(float*)d.values[d.used] = value;
    	d.types[d.used] = FLOAT;
    	strcpy(d.keys[d.used],name);
    	d.used++;
    }

    static void setStringSetting(char* name, char* value )
    {
    	for (int i = 0; i<d.used;i++)
    	{
    		if(strcmp(d.keys[i], name)==0 && d.types[i] == FLOAT)
    		{
    			free(d.values[i]);
    			d.values[i] = (void*)malloc(sizeof(char)*(strlen(value)+1));
    			strcpy((char*)(d.values[i]), value);
    			return;
    		}
    	}
    	d.values[d.used] = (void*)malloc(sizeof(char)*(strlen(value)+1));
    	strcpy((char*)(d.values[d.used]), value);
    	d.types[d.used] = STRING;
    	strcpy(d.keys[d.used],name);
    	d.used++;
    }
};

Settings::DynamicSettings Settings::d;

#endif
