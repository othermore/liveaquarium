#ifndef dimmer_h
#define dimmer_h

class Dimmer
{
public:
	int currentValue;
	int pin;
	int startValue;
	int endValue;
	int dimmingDuration;
	uint32_t manual_end;
	int manual_initial;
	int day_auto_on;
	int day_auto_off;

	int onRelayChannel;
	Relay *onRelay;
	bool onRelayReversed;

	int offRelayChannel;
	Relay *offRelay;
	bool offRelayReversed;



	enum dimmingMode { OFF, ON, AUTO, MANUAL_DIMMING_ON, MANUAL_DIMMING_OFF };
	dimmingMode mode;
	dimmingMode manual_next_mode;
	dimmingMode fixed_mode;


	Dimmer(int pin, int startValue, int endValue, int dimmingDuration, int day_auto_off, int day_auto_on);

	void setup()
	{
		pinMode(pin, OUTPUT);
		manual_end = 0;
	}

	void loop()
	{
		DateTime now;
		int previousValue = currentValue;
		int currentSecond;
		dimmingMode currentMode;
		switch(mode)
		{
			case OFF:
				if(onRelay)
					onRelay->setChannel(onRelayChannel, onRelayReversed);
				if(offRelay)
					offRelay->setChannel(offRelayChannel, !offRelayReversed);
				currentValue = 0; 
				manual_end = 0;
				fixed_mode = AUTO;
			break;
			case ON:
				if(onRelay)
					onRelay->setChannel(onRelayChannel, !onRelayReversed);
				if(offRelay)
					offRelay->setChannel(offRelayChannel, offRelayReversed);
				currentValue = 255; 
				manual_end = 0;
				fixed_mode = AUTO;
			break;
			case AUTO:
				manual_end = 0;
				now = RTC.now();
				//Si estamos en modo auto, y currentValue no encaja con lo programado, inciamos un dimming manual
				//que luego nos vuelva a poner en modo AUTO
				
				//Vemos el segundo del día en el que estamos
				currentSecond = (now.hour()*60+now.minute())*60+now.second();
				currentMode = getCurrentMode(currentSecond);				
				if (currentMode == ON && currentValue == 0 && fixed_mode != OFF)
				{
					//Si tenemos que pasar a ON y estamos en OFF
					mode = MANUAL_DIMMING_ON;
					manual_next_mode = AUTO;
					fixed_mode = AUTO;
					Serial.println("Mode switch to MANUAL_ON in AUTO");
				}
				else if(currentMode == OFF && currentValue == 255 && fixed_mode != ON)
				{
					mode = MANUAL_DIMMING_OFF;
					manual_next_mode = AUTO;
					fixed_mode = AUTO;
					Serial.println("Mode switch to MANUAL_OFF in AUTO");
				}
				else if(currentMode == ON && currentValue == 255 && fixed_mode == ON)
				{
					Serial.println("Removed fixed mode ON");
					fixed_mode = AUTO;
				}
				else if(currentMode == OFF && currentValue == 0 && fixed_mode == OFF)
				{
					Serial.println("Removed fixed mode OFF");
					fixed_mode = AUTO;
				}
				//Si no se da ningún caso, estamos en el estado correcto, seguimos así.
				if(currentValue == 255)
				{
					if(onRelay)
						onRelay->setChannel(onRelayChannel, !onRelayReversed);
					if(offRelay)
						offRelay->setChannel(offRelayChannel, offRelayReversed);
				}
				if(currentValue == 0)
				{
					if(onRelay)
						onRelay->setChannel(onRelayChannel, onRelayReversed);
					if(offRelay)
						offRelay->setChannel(offRelayChannel, !offRelayReversed);
				}

			break;
			case MANUAL_DIMMING_ON:
				now = RTC.now();
				if(onRelay)
					onRelay->setChannel(onRelayChannel, onRelayReversed);
				if(offRelay)
					offRelay->setChannel(offRelayChannel, offRelayReversed);
				if(manual_end == 0) //Acabamos de empezar el dimming manual
				{
					manual_end = now.unixtime() + dimmingDuration;
					manual_initial = currentValue;
					if (manual_initial < startValue)
					{
						manual_initial = startValue;
						currentValue = startValue;
						Serial.print("current value set to startValue in MANUAL_ON: ");
						Serial.println(startValue);
					}

				}
				else
				{
					if(manual_end < now.unixtime() || currentValue > endValue)
					{
						// Hemos terminado el dimming, así que pasamos a modo ON o AUTO
						if (manual_next_mode == AUTO)
						{
							mode = AUTO;
							Serial.println("Mode switch to AUTO in MANUAL_ON");
						}
						else
						{
							mode = ON;
							Serial.println("Mode switch to ON in MANUAL_ON");
						}
						currentValue = 255;
						manual_end = 0;
					}
					else
					{
						//Estamos haciendo un dimming manual hacia arriba, así que recalculamos currentValue
						uint32_t remaining_secconds = manual_end-now.unixtime();
						float done_dimming_percentage = 1.0 - (1.0*remaining_secconds/dimmingDuration);
						uint32_t dimming_length = endValue - manual_initial;
						currentValue = manual_initial + done_dimming_percentage*dimming_length; 
					}
				}
			break;
			case MANUAL_DIMMING_OFF:
				now = RTC.now();
				if(onRelay)
					onRelay->setChannel(onRelayChannel, onRelayReversed);
				if(offRelay)
					offRelay->setChannel(offRelayChannel, offRelayReversed);
				if(manual_end == 0) //Acabamos de empezar el dimming manual
				{
					manual_end = now.unixtime() + dimmingDuration;
					manual_initial = currentValue;
					if (manual_initial > endValue)
					{
						manual_initial = endValue;
						currentValue = endValue;
						Serial.print("current value set to endValue in MANUAL_OFF: ");
						Serial.println(endValue);
					}
				}
				else
				{
					if(manual_end < now.unixtime() || currentValue < startValue)
					{
						// Hemos terminado el dimming, así que pasamos a modo OFF o AUTO
						if (manual_next_mode == AUTO)
						{
							mode = AUTO;
							Serial.println("Mode switch to AUTO in MANUAL_OFF");
						}
						else
						{
							mode = OFF;
							Serial.print("Current: ");
							Serial.print(currentValue);
							Serial.print(", Start: ");
							Serial.print(startValue);
							Serial.print(" - Manual end:");
							Serial.print(manual_end);
							Serial.print(", now:");
							Serial.print(now.unixtime());
							Serial.println("Mode switch to OFF in MANUAL_OFF");
						}
						currentValue = 0;
						manual_end = 0;
					}
					else
					{
						//Estamos haciendo un dimming manual hacia abajo, así que recalculamos currentValue
						uint32_t remaining_secconds = manual_end-now.unixtime();
						float done_dimming_percentage = 1.0 - (1.0*remaining_secconds/dimmingDuration);
						uint32_t dimming_length = manual_initial-startValue;
						currentValue = manual_initial - done_dimming_percentage*dimming_length; 
					}
				}
			break;
		}
		if(previousValue!=currentValue)
			analogWrite(pin, currentValue);   
	}

	void startDimmingOffAndAuto()
	{
		manual_end = 0;
		mode = MANUAL_DIMMING_OFF;
		manual_next_mode = AUTO;
		fixed_mode = OFF;
	}

	void startDimmingOnAndAuto()
	{
		manual_end = 0;
		mode = MANUAL_DIMMING_ON;
		manual_next_mode = AUTO;
		fixed_mode = ON;
	}

	void setOffWithDimming()
	{
		manual_end = 0;
		mode = MANUAL_DIMMING_OFF;
		manual_next_mode = OFF;
	}

	void setOnWithDimming()
	{
		manual_end = 0;
		mode = MANUAL_DIMMING_ON;
		manual_next_mode = ON;
	}

	void setOff()
	{
		mode = OFF;
	}

	void setOn()
	{
		mode = ON;
	}

	void setAuto()
	{
		mode = AUTO;
	}

	void setOnRelay(Relay *relay, int channel, bool reversed)
	{
		onRelay = relay;
		onRelayChannel = channel;
		onRelayReversed = reversed;
	}

	void clearOnRelay()
	{
		onRelay = 0;
		onRelayChannel = 0;
		onRelayReversed = false;
	}

	void setOffRelay(Relay *relay, int channel, bool reversed)
	{
		offRelay = relay;
		offRelayChannel = channel;
		offRelayReversed = reversed;
	}

	void clearOffRelay()
	{
		offRelay = 0;
		offRelayChannel = 0;
		offRelayReversed = false;
	}

private:

	dimmingMode getCurrentMode(int currentSecond){
		//Vemos cual es el modo siguiente
		if(currentSecond <= day_auto_on && currentSecond <= day_auto_off ) 
		{
			//Estamos antes del on y del off
			//El más pequeño es el siguiente
			if(day_auto_on < day_auto_off)
			{
				//El siguiente es ON, ahora OFF
				return OFF;
			}
			else
			{
				//El siguiente es OFF, ahora ON
				return ON;
			}
		}
		else if(currentSecond <= day_auto_on)
		{
			//Estamos después del off y antes del on: estamos en OFF
			return OFF;
		}
		else if(currentSecond <= day_auto_off)
		{
			//Estamos después del on y antes del off: estamos en ON
			return ON;
		}
		else
		{
			//Estamos después del on y después del off
			//El más pequeño es el siguiente
			if(day_auto_on < day_auto_off)
			{
				//Estamos en OFF
				return OFF;
			}
			else
			{
				//Estamos en ON
				return ON;
			}
		}
	}	
};

Dimmer::Dimmer( int pin, int startValue, int endValue, int dimmingDuration, int day_auto_off, int day_auto_on):
	pin(pin), 
	startValue(startValue), 
	endValue(endValue), 
	dimmingDuration(dimmingDuration), 
	day_auto_off(day_auto_off), 
	day_auto_on(day_auto_on)
{
	clearOnRelay();
	clearOffRelay();
	currentValue = 0;
	manual_end = 0;
	manual_initial = 0;
	mode = AUTO;
	manual_next_mode = AUTO;
	fixed_mode = AUTO;
}




#endif
