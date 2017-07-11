#include "Quantis.h"

#define EXTRADEBUG
#define PRINTREPORT

QUANTIS::QUANTIS(USB *p) :
pUsb(p),            // pointer to USB class instance - mandatory
bAddress(0),        // device address - mandatory
bPollEnable(false)  // don't start polling before dongle is connected
{ 

	for(uint8_t i = 0; i < QUANTIS_PIPES_COUNT; i++)
	{
		epInfo[i].epAddr      = 0;
		epInfo[i].maxPktSize  = EP_MAXPKTSIZE;
		epInfo[i].bmSndToggle = 0;
		epInfo[i].bmRcvToggle = 0;
		epInfo[i].bmNakPower = (i) ? USB_NAK_NOWAIT : USB_NAK_MAX_POWER;
	}

	if(pUsb)
	{
		pUsb->RegisterDeviceClass(this);
	}
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

int exception(byte rcode)
{
#ifdef DEBUG_USB_HOST
	switch (rcode)  
	{  
	case USB_ERROR_CLASS_INSTANCE_ALREADY_IN_USE:  
		Notify(PSTR("\r\nAddress in use"), 0x80);
		break;  
	case USB_ERROR_ADDRESS_NOT_FOUND_IN_POOL:
		Notify(PSTR("\r\nAddress not found"), 0x80);
		break;  
	case USB_ERROR_EPINFO_IS_NULL:
		Notify(PSTR("\r\nEndpoint Info is null"), 0x80);
		break;  
	case USB_ERROR_OUT_OF_ADDRESS_SPACE_IN_POOL:
		Notify(PSTR("\r\nUnknown device"), 0x80);
		break; 
	case USB_DEV_CONFIG_ERROR_DEVICE_NOT_SUPPORTED:
		Notify(PSTR("\r\nOut of address space"), 0x80);
		break; 
	default:  
		Notify(PSTR("\r\nFailed, error code: "), 0x80);
		NotifyFail(rcode);
	}
#endif
	return rcode;
}

uint8_t QUANTIS::Init(uint8_t parent, uint8_t port, bool lowspeed) {
	uint8_t descriptor[sizeof (USB_DEVICE_DESCRIPTOR)];
	USB_DEVICE_DESCRIPTOR * udd = reinterpret_cast<USB_DEVICE_DESCRIPTOR*>(descriptor);
	uint8_t rcode;
	UsbDevice *p = NULL;
	EpInfo *oldep_ptr = NULL;

	AddressPool &addrPool = pUsb->GetAddressPool(); // Get memory address of USB device address pool

	#ifdef EXTRADEBUG
	Notify(PSTR("\r\nQuantis Init"), 0x80);
	#endif

	if(bAddress)
	{
		return exception(USB_ERROR_CLASS_INSTANCE_ALREADY_IN_USE);
	} 
	else 
	{
		p=addrPool.GetUsbDevicePtr(0);
		if(!p)
		{
			return exception(USB_ERROR_ADDRESS_NOT_FOUND_IN_POOL);
		} 
		else
		{
			if(!p->epinfo)
			{
				return exception(USB_ERROR_EPINFO_IS_NULL);
			} 
			else
			{
				oldep_ptr = p->epinfo; // Save old pointer to EP_RECORD of address 0
				p->epinfo = epInfo; // Temporary assign new pointer to epInfo to p->epinfo in order to avoid toggle inconsistence
				p->lowspeed = lowspeed;

				// Get device descriptor
				rcode = pUsb->getDevDescr(0, 0, sizeof (USB_DEVICE_DESCRIPTOR), (uint8_t*)descriptor);
				// Restore p->epinfo
				p->epinfo = oldep_ptr;

				if(rcode)
				{
					#ifdef DEBUG_USB_HOST
					NotifyFailGetDevDescr();
					#endif

					Release();
					return exception(rcode);
				}
				else
				{
					if(!VIDPIDOK(udd->idVendor, udd->idProduct))
					{
						#ifdef DEBUG_USB_HOST
						NotifyFailUnknownDevice(udd->idVendor, udd->idProduct);
						#endif
						
						Release();
						return exception(USB_DEV_CONFIG_ERROR_DEVICE_NOT_SUPPORTED);
					}
					else
					{
						#ifdef EXTRADEBUG
						Notify(PSTR("\r\nManufacturer: "), 0x80);
						getstrdescr(0, udd->iManufacturer);
						
						Notify(PSTR("\r\nProduct: "), 0x80);
						getstrdescr(0, udd->iProduct);

						Notify(PSTR("\r\nSerial: "), 0x80);
						getstrdescr(0, udd->iSerialNumber);
						#endif

						bAddress = addrPool.AllocAddress(parent, false, port); // Allocate new address according to device class

						if(!bAddress)
						{
							return exception(USB_ERROR_OUT_OF_ADDRESS_SPACE_IN_POOL);
						} 
						else
						{
							epInfo[0].maxPktSize = udd->bMaxPacketSize0;      // Extract Max Packet Size from device descriptor
							rcode = pUsb->setAddr(0, 0, bAddress);            // Assign new address to the device
							if(rcode)
							{
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
							
							delay(300); // Spec says you should wait at least 200ms

							p->lowspeed = false;
							
							rcode = pUsb->setEpInfoEntry(bAddress, 1, epInfo); // Assign epInfo to epinfo pointer - only EP0 is known
							if(rcode)
							{
								#ifdef DEBUG_USB_HOST
								NotifyFailSetDevTblEntry();
								#endif
								return exception(rcode);
							}
							else
							{
								epInfo[QUANTIS_CONTROL_PIPE].epAddr    = 0; 
								epInfo[QUANTIS_CONTROL_PIPE].epAttribs = USB_TRANSFER_TYPE_CONTROL;

								epInfo[QUANTIS_INPUT_PIPE].epAddr      = QUANTIS_INPUT_PIPE_ADDR; //!!! 
								epInfo[QUANTIS_INPUT_PIPE].epAttribs   = USB_TRANSFER_TYPE_BULK;
								epInfo[QUANTIS_INPUT_PIPE].bmSndToggle = bmSNDTOG0; //!!!
								epInfo[QUANTIS_INPUT_PIPE].bmRcvToggle = bmRCVTOG0; //!!!
								//epInfo[QUANTIS_INPUT_PIPE].maxPktSize  = udd->bMaxPacketSize0;  //!!!!     // Extract Max Packet Size from device descriptor

								rcode = pUsb->setEpInfoEntry(bAddress, QUANTIS_PIPES_COUNT, epInfo);
								if(rcode)
								{
									#ifdef DEBUG_USB_HOST
									NotifyFailSetDevTblEntry();
									#endif
									return exception(rcode);
								} 
								else
								{
									rcode = pUsb->setConf(bAddress, epInfo[QUANTIS_CONTROL_PIPE].epAddr, 1);
									if(rcode)
									{
										#ifdef DEBUG_USB_HOST
										NotifyFailSetConfDescr();
										#endif
										return exception(rcode);
									}
									else
									{
										#ifdef DEBUG_USB_HOST
										Notify(PSTR("\r\nQuantis USB Connected"), 0x80);
										#endif

										bPollEnable = true;                  // ready to use input pipe

										return 0;                            // Successful configuration
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

int32_t QUANTIS::getTRNGBytes(uint16_t bufferSize, uint8_t* buffer)
{
	byte rcode;
	
	if(!bPollEnable)
	{
		return 0;
	}
	else
	{
		uint16_t requestedBytes = bufferSize;
		rcode = pUsb->inTransfer(bAddress, epInfo[QUANTIS_INPUT_PIPE].epAddr, &requestedBytes, buffer);
		if(rcode)
		{
			#ifdef EXTRADEBUG
			Notify(PSTR("\r\nFailed polling: "), 0x80);
			D_PrintHex<uint8_t > (rcode, 0x80);
			#endif
			return (-1 * rcode);
		} else {
			return requestedBytes;
		}
	}
}

uint8_t QUANTIS::Release()
{
	pUsb->GetAddressPool().FreeAddress(bAddress);
	
	bAddress = 0;
	bPollEnable = false;
	
	#ifdef DEBUG_USB_HOST
	Notify(PSTR("\r\nQuantis disconnected\r\n\r\n"), 0x80);
	#endif
	
	return 0;
}