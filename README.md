# QuantisUSB
This driver used to connect Quantis USB TRN Generator to Arduino-like platforms.

# Hardware

For this project used:
1. Andruino like platform: [Teensy 3.2](https://www.pjrc.com/store/teensy32.html) <br>
Your platform should has at least one SPI interface to work with USB shield.

2. USB Shield: [DuinoFun UHS mini v2.0 (Aug. 16, 2014)](https://www.circuitsathome.com/mcu/arduino-usb-host-mini-initial-revision/)<br>
This shield version:<br>
a. provides only 3 volts (Quantis USB requires 5). To resolve it use USB hub with external power or another one USB Shield which has 5 Volts support.<br> 
b. has mistaken pinouts (see my [post](https://forum.pjrc.com/threads/43357-Teensy-with-mini-USB-host-shield-(chineese))), so be aware with SPI connection.<br>

3. USB Hub: [DEXP BT7-01](http://www.dns-shop.ru/product/76ec67e92f783361/usb-razvetvitel-dexp-bt7-01/)<br>
It is first one adapter that I see in the store, so you can use any other one, it just dependes on that how much devices you want to connect to one USB Host and does it provide required power for it.

4. TRN Generator: [Quantis USB](http://www.idquantique.com/random-number-generation/quantis-random-number-generator/)<br>
This driver has VID & PID values, so if you need to use another generator, just change it to required values. Also another generator could have another UANTIS_INPUT_PIPE_ADDR, so you have to change it.

# Software

# Results
