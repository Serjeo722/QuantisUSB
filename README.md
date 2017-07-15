# QuantisUSB
This driver used to connect Quantis USB TRN Generator to Arduino-like platforms.

# Hardware

For this project used:
1. Andruino like platform: [Teensy 3.2](https://www.pjrc.com/store/teensy32.html) <br>
Your platform should has at least one SPI interface to work with USB shield.

2. USB Shield: [DuinoFun UHS mini v2.0 (Aug. 16, 2014)](https://www.circuitsathome.com/mcu/arduino-usb-host-mini-initial-revision/)<br>
This shield version:<br>
a. provides only 3.3 volts (Quantis USB requires 5). To resolve it use USB hub with external power or another one USB Shield which has 5 volts support.<br> 
b. has mistaken pinouts (see my [post](https://forum.pjrc.com/threads/43357-Teensy-with-mini-USB-host-shield-(chineese))), so be aware with SPI connection.<br>

3. USB Hub: [DEXP BT7-01](http://www.dns-shop.ru/product/76ec67e92f783361/usb-razvetvitel-dexp-bt7-01/)<br>
It is first one adapter that I see in the store, so you can use any other one, it just dependes on that how much devices you want to connect to one USB Host and does it provide required power for it.<br>
(If you are using mini shield version with 3.3 volts and USB Hub with external power, you have to screen power contact from USB Hub. I'm using just adhansive tape on USB connector to screen [pin 1](https://commons.wikimedia.org/wiki/File:USB.svg), you can unsolder it or use [5V mini shield modification](https://www.circuitsathome.com/usb-host-shield-hardware-manual/)).

4. TRN Generator: [Quantis USB](http://www.idquantique.com/random-number-generation/quantis-random-number-generator/)<br>
Driver has VID & PID values in [Quantis.h](https://github.com/Serjeo722/QuantisUSB/blob/master/Quantis.h) file, so if you need to use another generator, just change it to required values. Also another generator could have another UANTIS_INPUT_PIPE_ADDR, so you also have to change it.

# Software
5. [Arduino IDE](https://www.arduino.cc/en/main/software) v1.6.9 or higher<br> 
6. [Teensyduino driver](https://www.pjrc.com/teensy/td_download.html)
7. [USB Host Shield Library](https://github.com/felis/USB_Host_Shield_2.0) v2.0

# Examples

[GenerateTRN.ino](https://github.com/Serjeo722/QuantisUSB/blob/master/examples/GenerateTRN/GenerateTRN.ino)<br>
Waits till device will be connected, shows it parameters (name,serial...), obtains every second new 192 bytes block from Quantis USB and shows it on the screen.
![alt tag](https://github.com/Serjeo722/QuantisUSB/blob/master/examples/GenerateTRN/ScreenShot.png?raw=true)


[MeasureTRNGSpeed.ino](https://github.com/Serjeo722/QuantisUSB/blob/master/examples/MeasureTRNGSpeed/MeasureTRNGSpeed.ino)<br>
Waits available QuantisUSB devices and every second count how many bytes has been received from them.
![alt tag](https://github.com/Serjeo722/QuantisUSB/blob/master/examples/MeasureTRNGSpeed/ScreenShot.png?raw=true)

