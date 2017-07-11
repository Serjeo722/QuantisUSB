#include "Quantis.h"
#include "pgmstrings.h"

#define EXTRADEBUG
#define PRINTREPORT

QUANTIS::QUANTIS(USB *p) :
pUsb(p), // pointer to USB class instance - mandatory
bAddress(0), // device address - mandatory
bPollEnable(false) { // don't start polling before dongle is connected

	for(uint8_t i = 0; i < QUANTIS_PIPES_COUNT; i++) {
		epInfo[i].epAddr      = 0;
		epInfo[i].maxPktSize  = EP_MAXPKTSIZE;
		epInfo[i].bmSndToggle = 0;
		epInfo[i].bmRcvToggle = 0;
		epInfo[i].bmNakPower = (i) ? USB_NAK_NOWAIT : USB_NAK_MAX_POWER;
	}

	if(pUsb)
	pUsb->RegisterDeviceClass(this);
}

uint8_t QUANTIS::getstrdescr( uint8_t addr, uint8_t idx )
{
	uint8_t buf[ 256 ];
	uint8_t rcode;
	uint8_t length;
	uint8_t i;
	uint16_t langid;
	rcode = pUsb->getStrDescr( addr, 0, 1, 0, 0, buf );  //get language table length
	if ( rcode ) {
		Serial.println("Error retrieving LangID table length");
		return ( rcode );
	}
	length = buf[ 0 ];      //length is the first byte
	rcode = pUsb->getStrDescr( addr, 0, length, 0, 0, buf );  //get language table
	if ( rcode ) {
		Serial.print("Error retrieving LangID table ");
		return ( rcode );
	}
	langid = (buf[3] << 8) | buf[2];
	rcode = pUsb->getStrDescr( addr, 0, 1, idx, langid, buf );
	if ( rcode ) {
		Serial.print("Error retrieving string length ");
		return ( rcode );
	}
	length = buf[ 0 ];
	rcode = pUsb->getStrDescr( addr, 0, length, idx, langid, buf );
	if ( rcode ) {
		Serial.print("Error retrieving string ");
		return ( rcode );
	}
	for ( i = 2; i < length; i += 2 ) {   //string is UTF-16LE encoded
		Serial.print((char) buf[i]);
	}
	return ( rcode );
}	

uint8_t QUANTIS::Init(uint8_t parent, uint8_t port, bool lowspeed) {
	uint8_t buf[sizeof (USB_DEVICE_DESCRIPTOR)];
	USB_DEVICE_DESCRIPTOR * udd = reinterpret_cast<USB_DEVICE_DESCRIPTOR*>(buf);
	uint8_t rcode;
	UsbDevice *p = NULL;
	EpInfo *oldep_ptr = NULL;
	uint16_t PID;
	uint16_t VID;


	AddressPool &addrPool = pUsb->GetAddressPool(); // Get memory address of USB device address pool
	
	
#ifdef EXTRADEBUG
	Notify(PSTR("\r\nQUANTIS Init"), 0x80);
#endif

	if(bAddress) { // Check if address has already been assigned to an instance
#ifdef DEBUG_USB_HOST
		Notify(PSTR("\r\nAddress in use"), 0x80);
#endif
		return USB_ERROR_CLASS_INSTANCE_ALREADY_IN_USE;
	}

	p = addrPool.GetUsbDevicePtr(0); // Get pointer to pseudo device with address 0 assigned

	if(!p) {
#ifdef DEBUG_USB_HOST
		Notify(PSTR("\r\nAddress not found"), 0x80);
#endif
		return USB_ERROR_ADDRESS_NOT_FOUND_IN_POOL;
	}

	if(!p->epinfo) {
#ifdef DEBUG_USB_HOST
		Notify(PSTR("\r\nepinfo is null"), 0x80);
#endif
		return USB_ERROR_EPINFO_IS_NULL;
	}

	oldep_ptr = p->epinfo; // Save old pointer to EP_RECORD of address 0
	p->epinfo = epInfo; // Temporary assign new pointer to epInfo to p->epinfo in order to avoid toggle inconsistence
	p->lowspeed = lowspeed;

	// Get device descriptor
	rcode = pUsb->getDevDescr(0, 0, sizeof (USB_DEVICE_DESCRIPTOR), (uint8_t*)buf); // Get device descriptor - addr, ep, nbytes, data
	// Restore p->epinfo
	p->epinfo = oldep_ptr; // Restore p->epinfo

	if(rcode)
	goto FailGetDevDescr;

	VID = udd->idVendor;
	PID = udd->idProduct;

	if(!VIDPIDOK(VID, PID)) // Check VID
	goto FailUnknownDevice;

#ifdef EXTRADEBUG
	Notify(PSTR("\r\nManufacturer: "), 0x80);
	getstrdescr(0, udd->iManufacturer);
	
	Notify(PSTR("\r\nProduct: "), 0x80);
	getstrdescr(0, udd->iProduct);

	Notify(PSTR("\r\nSerial: "), 0x80);
	getstrdescr(0, udd->iSerialNumber);
#endif

	bAddress = addrPool.AllocAddress(parent, false, port); // Allocate new address according to device class

	if(!bAddress) {
#ifdef DEBUG_USB_HOST
		Notify(PSTR("\r\nOut of address space"), 0x80);
#endif
		return USB_ERROR_OUT_OF_ADDRESS_SPACE_IN_POOL;
	}

	epInfo[0].maxPktSize = udd->bMaxPacketSize0; // Extract Max Packet Size from device descriptor


	// Assign new address to the device
	rcode = pUsb->setAddr(0, 0, bAddress);
	if(rcode) {
		p->lowspeed = false;
		addrPool.FreeAddress(bAddress);
		bAddress = 0;
#ifdef DEBUG_USB_HOST
		Notify(PSTR("\r\nsetAddr: "), 0x80);
		D_PrintHex<uint8_t > (rcode, 0x80);
#endif
		return rcode;
	}
#ifdef EXTRADEBUG
	Notify(PSTR("\r\nAddr: "), 0x80);
	D_PrintHex<uint8_t > (bAddress, 0x80);
#endif
	delay(200);
	
	p->lowspeed = false;
	
	// Assign epInfo to epinfo pointer - only EP0 is known
	rcode = pUsb->setEpInfoEntry(bAddress, 1, epInfo);
	if(rcode)
	goto FailSetDevTblEntry;

	epInfo[QUANTIS_CONTROL_PIPE].epAddr    = QUANTIS_CONTROL_PIPE_ADDR; 
	epInfo[QUANTIS_CONTROL_PIPE].epAttribs = USB_TRANSFER_TYPE_CONTROL;

	epInfo[QUANTIS_INPUT_PIPE].epAddr      = QUANTIS_INPUT_PIPE_ADDR; 
	epInfo[QUANTIS_INPUT_PIPE].epAttribs   = USB_TRANSFER_TYPE_BULK;
	epInfo[QUANTIS_INPUT_PIPE].bmSndToggle = bmSNDTOG0;
	epInfo[QUANTIS_INPUT_PIPE].bmRcvToggle = bmRCVTOG0;

	//epInfo[QUANTIS_INPUT_PIPE].bmRcvToggle = 1;
	

	//epInfo[ QUANTIS_INPUT_PIPE ].bmNakPower = USB_NAK_NOWAIT;
	
	rcode = pUsb->setEpInfoEntry(bAddress, QUANTIS_PIPES_COUNT, epInfo);
	if(rcode)
	goto FailSetDevTblEntry;

	delay(300); //Give time for address change

	rcode = pUsb->setConf(bAddress, epInfo[QUANTIS_CONTROL_PIPE].epAddr, 1);
	if(rcode)
		goto FailSetConfDescr;

	
#ifdef DEBUG_USB_HOST
	Notify(PSTR("\r\nQuantis USB Connected\r\n"), 0x80);
#endif
	connected = true;
	bPollEnable = true;
	checkStatusTimer = 0; // Reset timer
	return 0; // Successful configuration

FailGetDevDescr:
#ifdef DEBUG_USB_HOST
	NotifyFailGetDevDescr();
	goto Fail;
#endif

FailSetDevTblEntry:
#ifdef DEBUG_USB_HOST
	NotifyFailSetDevTblEntry();
	goto Fail;
#endif

FailSetConfDescr:
#ifdef DEBUG_USB_HOST
	NotifyFailSetConfDescr();
#endif
	goto Fail;

FailUnknownDevice:
#ifdef DEBUG_USB_HOST
	NotifyFailUnknownDevice(VID, PID);
#endif
	rcode = USB_DEV_CONFIG_ERROR_DEVICE_NOT_SUPPORTED;

Fail:
#ifdef DEBUG_USB_HOST
	Notify(PSTR("\r\nQuantis Init Failed, error code: "), 0x80);
	NotifyFail(rcode);
#endif
	Release();
	return rcode;
}

/* Performs a cleanup after failed Init() attempt */
uint8_t QUANTIS::Release() {
	connected = false;
	pUsb->GetAddressPool().FreeAddress(bAddress);
	bAddress = 0;
	bPollEnable = false;
#ifdef DEBUG_USB_HOST
	Notify(PSTR("\r\nQuantis disconnected"), 0x80);
#endif
	return 0;
}
/*
void QUANTIS::checkStatus() {
		if(!bPollEnable)
				return;
}			
*/

void QUANTIS::printReport(uint8_t showFirstNBytes, uint8_t nBytes) {
#ifdef PRINTREPORT
	for(uint8_t i = 0; i < showFirstNBytes; i++) {
		D_PrintHex<uint8_t > (readBuf[i], 0x80);
		Notify(PSTR(" "), 0x80);
	}
#endif
}

uint8_t QUANTIS::Poll() {
	byte rcode = 0;
	
	if(!bPollEnable)
	return 0;
	/*
		if(!checkStatusTimer || ((int32_t)((uint32_t)millis() - checkStatusTimer) > 3000)) { // Run checkStatus every 3 seconds
				checkStatusTimer = (uint32_t)millis();
				checkStatus();
		}*/

	uint16_t bufferSize = 512;
	
	rcode = pUsb->inTransfer(bAddress, epInfo[QUANTIS_INPUT_PIPE].epAddr, &bufferSize, readBuf);

	if(rcode){
#ifdef EXTRADEBUG
		Notify(PSTR("Failed polling: "), 0x80);
		D_PrintHex<uint8_t > (rcode, 0x80);
#endif	
	} else {
#ifdef PRINTREPORT
		D_PrintHex<uint16_t > (bufferSize, 0x80);	
		Notify(PSTR(": "), 0x80);
		if(bufferSize > 0) {
			printReport(32, bufferSize);
		}
#endif	
	}
#ifdef PRINTREPORT
	Notify(PSTR("\r\n"), 0x80);
#endif	
	
	return 0;
}