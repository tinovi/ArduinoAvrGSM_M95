/*
This file is part of the GSM3 communications library for Arduino
-- Multi-transport communications platform
-- Fully asynchronous
-- Includes code for the Arduino-Telefonica GSM/GPRS Shield V1
-- Voice calls
-- SMS
-- TCP/IP connections
-- HTTP basic clients

This library has been developed by Telef�nica Digital - PDI -
- Physical Internet Lab, as part as its collaboration with
Arduino and the Open Hardware Community. 

September-December 2012

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

The latest version of this library can always be found at
https://github.com/BlueVia/Official-Arduino
*/
#include "GSM3SoftSerial.h"
#include <Arduino/Arduino.h>
#include "debug.h"

#define __XON__ 0x11
#define __XOFF__ 0x13

#define _GSMSOFTSERIALFLAGS_ESCAPED_ 0x01
#define _GSMSOFTSERIALFLAGS_SENTXOFF_ 0x02


GSM3SoftSerial* GSM3SoftSerial::_activeObject=0;

GSM3SoftSerial::GSM3SoftSerial():
	cb(this)
{
	firstByte = true;
}

bool GSM3SoftSerial::keepThisChar(uint8_t* c)
{
	// Horrible things for Quectel XON/XOFF
	// 255 is the answer to a XOFF
	// It comes just once
	if((*c==255)&&(_flags & _GSMSOFTSERIALFLAGS_SENTXOFF_))
	{
		_flags ^= _GSMSOFTSERIALFLAGS_SENTXOFF_;
		return false;
	}

	// 0x77, w, is the escape character
	if(*c==0x77)
	{
		_flags |= _GSMSOFTSERIALFLAGS_ESCAPED_;
		return false;
	}

	// and these are the escaped codes
	if(_flags & _GSMSOFTSERIALFLAGS_ESCAPED_)
	{
		if(*c==0xEE)
			*c=0x11;
		else if(*c==0xEC)
			*c=0x13;
		else if(*c==0x88)
			*c=0x77;

		_flags ^= _GSMSOFTSERIALFLAGS_ESCAPED_;
		return true;
	}

	return true;
}

//void GSM3SoftSerial::printRX(uint8_t c)
//{
//	Tr('|');
//	if((c>31)&&(c<127))
//		Tr((char)c);
//	else
//	{
//		//Tr('%');
//		Tr(c);
//		//Tr('%');
//	}
//}

void GSM3SoftSerial::handleRx( uint8_t c )
{
	if(_activeObject){
		//if(_activeObject->keepThisChar(&c)){
			_activeObject->cb.write(c);
			if(!_activeObject->transparent){
				if(_activeObject->firstByte){
					_activeObject->firstByte=false;
					//_activeObject->startTime = millis();
					_activeObject->thisHead=_activeObject->cb.getTail();
				}
				if((c == 10) || _activeObject->cb.availableBytes()<6 ){ //|| millis() - _activeObject->startTime > 400  && activeObject->lastByte =  == 10
					_activeObject->firstByte = true;
					//_activeObject->lastByte = 0;
					if(_activeObject->mgr && (_activeObject->cb.getTail() - _activeObject->thisHead)>1){
						_activeObject->mgr->manageMsg(_activeObject->thisHead, _activeObject->cb.getTail());
					}
				}
				//_activeObject->lastByte = c;
			}else{
			}
		}
	//}
}

int GSM3SoftSerial::begin(long speed)
{
	if(_activeObject){
		return 1;
	}
	_activeObject=this;
	NeoSerial1.attachInterrupt( GSM3SoftSerial::handleRx );
	NeoSerial1.begin( speed );
	return 0;
}

void GSM3SoftSerial::close()
 {
	NeoSerial1.end();
	setTransparent(false);
	_activeObject=0;
 }

size_t GSM3SoftSerial::write(uint8_t c)
{
	printTX(c);
	return NeoSerial1.write(c);

	// Characters to be escaped under XON/XOFF control with Quectel
//	if(c==0x11)
//	{
//		this->finalWrite(0x77);
//		return this->finalWrite(0xEE);
//	}
//
//	if(c==0x13)
//	{
//		this->finalWrite(0x77);
//		return this->finalWrite(0xEC);
//	}
//
//	if(c==0x77)
//	{
//		this->finalWrite(0x77);
//		return this->finalWrite(0x88);
//	}
//
//	return this->finalWrite(c);
}

void GSM3SoftSerial::printTX(uint8_t c)
{
	if((c>31)&&(c<127)){
#if DEBUG
		Tr((char)c);
#endif
	}else
	{
#if DEBUG
		Tr('\'');
		Tr(c);
#endif
	}
}

size_t GSM3SoftSerial::finalWrite(uint8_t c)
{
	printTX(c);
	return NeoSerial1.write(c);
}


/*inline*/ void GSM3SoftSerial::tunedDelay(uint16_t delay) { 
  uint8_t tmp=0;

  asm volatile("sbiw    %0, 0x01 \n\t"
    "ldi %1, 0xFF \n\t"
    "cpi %A0, 0xFF \n\t"
    "cpc %B0, %1 \n\t"
    "brne .-10 \n\t"
    : "+r" (delay), "+a" (tmp)
    : "0" (delay)
    );
}

void GSM3SoftSerial::setTransparent(bool transp){
	if(_activeObject && _activeObject->transparent != transp){
		_activeObject->transparent = transp;
		_activeObject->cb.flush();
		_activeObject->firstByte = true;
	}
}


void GSM3SoftSerial::spaceAvailable()
{
	// If there is spaceAvailable in the buffer, lets send a XON
	//finalWrite((byte)__XON__);
}


// This is here to avoid problems with Arduino compiler
void GSM3SoftSerialMgr::manageMsg(byte from, byte to){};
