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
#ifndef __GSM3_SOFTSERIAL__
#define __GSM3_SOFTSERIAL__

#include "GSM3CircularBuffer.h"
#include <avr/pgmspace.h>


class GSM3SoftSerialMgr
{
	public:
	
		/** Manages soft serial message
			@param from			Initial byte
			@param to			Final byte
		 */
		virtual void manageMsg(byte from, byte to);
};

// This class manages software serial communications
// Changing it so it doesn't know about modems or whatever HardwareSerial

class GSM3SoftSerial : public GSM3CircularBufferManager
{
	private:
	
		static GSM3SoftSerial* _activeObject;
		GSM3SoftSerialMgr* mgr;


	  public:
		//static void printRX( uint8_t c );
		void printTX( uint8_t c );
		static void handleRx( uint8_t c );
	  
		/** Tuned delay in microcontroller
			@param delay		Time to delay
		 */
		static /*inline */void tunedDelay(uint16_t delay);
		
		GSM3CircularBuffer cb; // Circular buffer
		byte thisHead;
		bool firstByte;
		byte lastByte;
		unsigned long startTime;
		uint8_t _flags;
		volatile bool transparent;
		
		/** Register serial manager
			@param manager		Serial manager
		 */
		inline void registerMgr(GSM3SoftSerialMgr* manager){mgr=manager;};
		
		/** If there is spaceAvailable in the buffer, lets send a XON
		 */
		void spaceAvailable();
		
		/** Write a character in serial connection
			@param c			Character
			@return	1 if succesful, 0 if transmission delay = 0
		 */
		virtual size_t write(uint8_t);
		virtual size_t finalWrite(uint8_t);
		
		void setTransparent(bool transp);
		/** Decide, attending to escapes, if the received character should we
				    kept, forgotten, or changed
					@param c			Character, may be changed
					@return	1 if shall be kept, 0 if forgotten
				 */
				bool keepThisChar(uint8_t* c);

		/** Constructor */
		GSM3SoftSerial();
		
		/** Establish serial connection
			@param speed		Baudrate
			@return
		 */
		int begin(long speed);
		

		/** Close serial connection
		 */
		void close();
};

#endif
