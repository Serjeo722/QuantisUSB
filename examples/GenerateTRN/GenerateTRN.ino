#include <usbhub.h>

#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif

#include <Quantis.h>

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

void printCurrentBuffer(uint16_t limit){
	for(uint8_t row=0; row * PRINT_NUMBER_OF_BYTES_TO_ONE_LINE < limit; row++)
	{
		Notify(PSTR("\r\n"), 0x80);
		for(uint8_t column=0; column < PRINT_NUMBER_OF_BYTES_TO_ONE_LINE; column++)
		{
			if(row * PRINT_NUMBER_OF_BYTES_TO_ONE_LINE + column < limit)
			{
				D_PrintHex<uint8_t > (TRNG_ReadBuffer[row * PRINT_NUMBER_OF_BYTES_TO_ONE_LINE + column], 0x80);
				Notify(PSTR(" "), 0x80);
			}
		}
	}
	Notify(PSTR("\r\n"), 0x80);
}

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

void loop()
{
	for (int i=0;i<10;i++)
	{
		delay(100);
		Usb.Task();
	}

	for(int i=0;i<MAXIMUM_SUPPORTED_GENERATORS;i++)
	{

		Serial.print("Slot:");
		Serial.print(i);

		if(Generators[i].isReady())
		{
			int32_t receivedBytes = Generators[i].getTRNGBytes(sizeof(TRNG_ReadBuffer), TRNG_ReadBuffer);
			if (receivedBytes > 0)
			{
				Serial.print(" received bytes:");
				Serial.print(receivedBytes);

				printCurrentBuffer(receivedBytes);
			}
		} 
		else 
		{
			Serial.println(" NOT CONNECTED");
		}
	}
	
	Serial.println();
}