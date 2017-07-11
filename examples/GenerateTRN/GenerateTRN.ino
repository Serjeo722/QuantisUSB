#include <usbhub.h>

#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif

#include <Quantis.h>

#define BUFFER_SIZE 512
#define PRINT_NUMBER_OF_BYTES_TO_ONE_LINE 32

USB     Usb;
USBHub  Hub1(&Usb);
USBHub  Hub2(&Usb);
USBHub  Hub3(&Usb);
USBHub  Hub4(&Usb);

QUANTIS Quantis1(&Usb);
QUANTIS Quantis2(&Usb);

uint8_t TRNG_ReadBuffer[BUFFER_SIZE];

void printCurrentBuffer(uint16_t limit){
	for(uint8_t row=0; row < BUFFER_SIZE / PRINT_NUMBER_OF_BYTES_TO_ONE_LINE; row++)
	{
		Notify(PSTR("\r\n"), 0x80);
		for(uint8_t column=0; column <PRINT_NUMBER_OF_BYTES_TO_ONE_LINE; column ++)
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
	Usb.Task();

	if(Quantis1.isReady())
	{
		int32_t receivedBytes = Quantis1.getTRNGBytes(sizeof(TRNG_ReadBuffer), TRNG_ReadBuffer);
		if (receivedBytes > 0)
		{
			printCurrentBuffer(receivedBytes);
		}
	}

	if(Quantis2.isReady())
	{
		int32_t receivedBytes = Quantis2.getTRNGBytes(sizeof(TRNG_ReadBuffer), TRNG_ReadBuffer);
		if (receivedBytes > 0)
		{
			printCurrentBuffer(receivedBytes);
		}
	}
	
	delay(2000);
}