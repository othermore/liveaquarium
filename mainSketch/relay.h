#ifndef relay_h
#define relay_h

class Relay
{
public:
	enum relayMode { OFF, ON, AND, OR };
	int previousValue;
	int pin;
	relayMode mode;
	int numChannels;
	bool *channels;
	Relay(int pin);	
	Relay(int pin, relayMode mode, int numChannels);
	~Relay();

	void setOn()
	{
		mode = ON;
	}

	void setOff()
	{
		mode = OFF;
	}

	void setMode(relayMode _mode)
	{
		mode = _mode;
	}

	void set(bool value)
	{
		if (value)
			mode = ON;
		else
			mode = OFF;
	}

	void setChannel(int channel, bool value)
	{
		if (numChannels > channel)
			channels[channel] = value;
	}

	bool getChannel(int channel)
	{
		if (numChannels > channel)
			return channels[channel];
		return false;
	}

	bool get()
	{
		if (mode == ON)
			return true;
		if (mode == OFF)
			return false;
		if (mode == AND)
		{
			for (int i = 0; i< numChannels; i++)
			{
				if (!channels[i])
					return false;
			}
			return true;
		}
		if (mode == OR)
		{
			for (int i = 0; i< numChannels; i++)
			{
				if (channels[i])
					return true;
			}
			return false;
		}
		return false;
	}

	void setup()
	{
		pinMode(pin, OUTPUT);
	}

	void loop()
	{
		bool value = get();
		if (previousValue != (int) value) 
		{
			previousValue = (int)value;
			if(value)
				digitalWrite(pin, HIGH );
			else
				digitalWrite(pin, LOW );
		}
	}

private:


};

Relay::Relay(int pin):pin(pin)
{
	previousValue = -1;
	numChannels = 0;
	channels = 0;
	mode = OFF;
}

Relay::Relay(int pin,relayMode mode, int numChannels):
	pin(pin), mode(mode), numChannels(numChannels) 
{
	previousValue = -1;
	channels = (bool*)malloc(sizeof(bool)*numChannels);
	for(int i = 0; i< numChannels;i++)
		channels[i] = false;
}

Relay::~Relay()
{
	free(channels);
}


#endif