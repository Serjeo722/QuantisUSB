#include <usbhub.h>

#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif

#include <Quantis.h>

#define SHOW_STAT_EVERY_MICROS 1000*1000 // 1 sec


#define PRINT_NUMBER_OF_BYTES_TO_ONE_LINE 32

USB     Usb;
USBHub  Hub1(&Usb);
USBHub  Hub2(&Usb);
USBHub  Hub3(&Usb);
USBHub  Hub4(&Usb);

#define MAXIMUM_SUPPORTED_GENERATORS 4
QUANTIS Generators[] =
{
	QUANTIS(&Usb),
	QUANTIS(&Usb),
	QUANTIS(&Usb),
	QUANTIS(&Usb)
};

#define BUFFER_SIZE (3 * 64)
uint8_t TRNG_ReadBuffer[BUFFER_SIZE];

void setup()
{
	Serial.begin(115200);
	#if !defined(__MIPSEL__)
	while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
	#endif
	
	Serial.println("Start");

	while (Usb.Init() == -1)
	{
		Serial.println("OSC did not start.");
		delay(1000);
	}
}

unsigned long lastMeasureTime = micros();
unsigned long receivedBytes = 0;
	
void loop()
{
	int connectedDevices = 0;
	
	Usb.Task();
	
	for(int i=0;i<MAXIMUM_SUPPORTED_GENERATORS;i++)
	{
		if(Generators[i].isReady())
		{
			receivedBytes += Generators[i].getTRNGBytes(sizeof(TRNG_ReadBuffer), TRNG_ReadBuffer);
			connectedDevices++;
		} 
	}
	
	unsigned long current = micros();
	if (current > lastMeasureTime+SHOW_STAT_EVERY_MICROS) {
		
		Serial.print("devices connected: ");
		Serial.print(connectedDevices);
		Serial.print(" recieved bytes: ");
		Serial.println(receivedBytes);
		
		receivedBytes = 0;
		lastMeasureTime = micros();  // not error. do not calculate time to serial port send
	}	
	
}