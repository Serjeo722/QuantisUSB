#include <usbhub.h>
#include "pgmstrings.h"

#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif

#include "Quantis.h"

USB     Usb;
USBHub  Hub1(&Usb);
USBHub  Hub2(&Usb);
USBHub  Hub3(&Usb);
USBHub  Hub4(&Usb);

QUANTIS Quantis(&Usb);

uint32_t next_time;

void setup()
{
	Serial.begin( 115200 );
#if !defined(__MIPSEL__)
	while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
	Serial.println("Start");

	if (Usb.Init() == -1)
	Serial.println("OSC did not start.");
	delay( 200 );

	next_time = (uint32_t)millis() + 10000;
}

void loop()
{
	Usb.Task();

	if(Quantis.connected){
		delay(200);
	}
}