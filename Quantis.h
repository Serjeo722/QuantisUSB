#ifndef _quantis_h_
#define _quantis_h_

#define QUANTIS_VID 0x0ABA
#define QUANTIS_PID 0x0102
#define QUANTIS_MAXPKTSIZE 1024

#define EP_INTERVAL 0x1

#define EP_MAXPKTSIZE 64
#define EP_INPUT_BUFFER_SIZE 512

#define QUANTIS_CONTROL_PIPE 0
#define QUANTIS_CONTROL_PIPE_ADDR 0x00

#define QUANTIS_INPUT_PIPE   1
#define QUANTIS_INPUT_PIPE_ADDR   0x6

#define QUANTIS_PIPES_COUNT  2

#include "Usb.h"

class QUANTIS : public USBDeviceConfig {
public:
		/**
         * Constructor for the QuantisUSB class.
         * @param  pUsb   Pointer to USB class instance.
         */
        QUANTIS(USB *pUsb);
        
		/**
         * Initialize the Xbox wireless receiver.
         * @param  parent   Hub number.
         * @param  port     Port number on the hub.
         * @param  lowspeed Speed of the device.
         * @return          0 on success.
         */        
		uint8_t Init(uint8_t parent, uint8_t port, bool lowspeed);

        /**
         * Release the USB device.
         * @return 0 on success.
         */
		uint8_t Release();
        
		/**
         * Poll the USB Input endpoins and run the state machines.
         * @return 0 on success.
         */
        
		uint8_t Poll();

        /**
         * Get the device address.
         * @return The device address.
         */
        virtual uint8_t GetAddress() {
                return bAddress;
        };

        /**
         * Used to check if the controller has been initialized.
         * @return True if it's ready.
         */
        virtual bool isReady() {
                return bPollEnable;
        };

        /**
         * Used by the USB core to check what this driver support.
         * @param  vid The device's VID.
         * @param  pid The device's PID.
         * @return     Returns true if the device's VID and PID matches this driver.
         */
        virtual bool VIDPIDOK(uint16_t vid, uint16_t pid) {
                return ((vid == QUANTIS_VID) && (pid == QUANTIS_PID));
        };
        /**@}*/

    
        /**
         * Used to disconnect any of the controllers.
         * @param controller The controller to disconnect. Default to 0.
         */
        void disconnect(uint8_t controller = 0);
		
		/** True if a wireless receiver is connected. */
		bool connected;
		
		
protected:
		/** Pointer to USB class instance. */
        USB *pUsb;
        /** Device address. */
        uint8_t bAddress;
		/** Endpoint info structure. */
        EpInfo epInfo[QUANTIS_PIPES_COUNT];
		
private:
		void readReport(uint8_t controller);
        void printReport(uint8_t controller, uint8_t nBytes);
		uint8_t getstrdescr( uint8_t addr, uint8_t idx );
		
		bool bPollEnable;
		uint8_t readBuf[EP_INPUT_BUFFER_SIZE];
		uint32_t checkStatusTimer;
		uint8_t state[1];
};
#endif
