/*
This file is part of the Tinovi.io GSM communications library for Arduino

This library has been developed by Tinovi.io based on Arduino GSM module library: https://github.com/arduino/Arduino/tree/master/libraries/GSM

2016

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
#include "AGSMSSL.h"
#include "GSM3MobileClientProvider.h"
#include <Arduino/Arduino.h>
#include <util/delay.h>

// While there is only a shield (ShieldV1) we will include it by default
#include "AGSMSSLProvider.h"
#if USE_SSL
	GSMSSLProvider theShieldV1MultiClientProvider;
#endif


#define GSM3MOBILECLIENTSERVICE_CLIENT 0x01 // 1: This side is Client. 0: This side is Server
#define GSM3MOBILECLIENTSERVICE_WRITING 0x02 // 1: TRUE 0: FALSE
#define GSM3MOBILECLIENTSERVICE_SYNCH 0x04 // 1: TRUE, compatible with other clients 0: FALSE

#define __TOUTBEGINWRITE__ 20000


GSMSSL::GSMSSL(bool synch)
{
	flags = GSM3MOBILECLIENTSERVICE_CLIENT;
	if(synch)
		flags |= GSM3MOBILECLIENTSERVICE_SYNCH;
	mySocket=255;
}

GSMSSL::GSMSSL(int socket, bool synch)
{
	// We are creating a socket on an existing, occupied one.
	flags=0;
	if(synch)
		flags |= GSM3MOBILECLIENTSERVICE_SYNCH;
	mySocket=socket;
	theGSM3MobileClientProvider->getSocket(socket);
	
}

// Returns 0 if last command is still executing
// 1 if success
// >1 if error 
int GSMSSL::ready(char c)
{	
	return theGSM3MobileClientProvider->ready(c);
}

int GSMSSL::connect(IPAddress add, uint16_t port)
{

	if(theGSM3MobileClientProvider==0)
		return 2;

	if(connected()){
		stop();
	}


	// TODO: ask for the socket id
	mySocket=theGSM3MobileClientProvider->getSocket();

	if(mySocket<0)
		return 2;
	
	int res=theGSM3MobileClientProvider->connectTCPClient(add, port, mySocket);
	if(flags & GSM3MOBILECLIENTSERVICE_SYNCH)
		res=waitForAnswer('c');
	
	return res;
};

int GSMSSL::connect(const char *host, uint16_t port)
{

	if(theGSM3MobileClientProvider==0)
		return 2;		

	if(connected()){
		stop();
	}
// TODO: ask for the socket id
	mySocket=theGSM3MobileClientProvider->getSocket();

	if(mySocket<0)
		return 2;
	
	int res=theGSM3MobileClientProvider->connectTCPClient(host, port, mySocket);
	if(flags & GSM3MOBILECLIENTSERVICE_SYNCH)
		res=waitForAnswer('c');
		
	return res;
}

int GSMSSL::waitForAnswer(char c)
{
	unsigned long m;
	m=millis();
	int res;
	
	while(((millis()-m)< __TOUTBEGINWRITE__ )&&(ready(c)==0))
		_delay_ms(200);
	
	res=ready(c);

	// If we get something different from a 1, we are having a problem
	if(res!=1)
		res=0;

	return res;
}

void GSMSSL::beginWrite(bool sync)
{
	flags |= GSM3MOBILECLIENTSERVICE_WRITING;
	theGSM3MobileClientProvider->beginWriteSocket(flags & GSM3MOBILECLIENTSERVICE_CLIENT, mySocket);
	if(sync)
		waitForAnswer('W');
}

size_t GSMSSL::write(uint8_t c)
{	
	if(!(flags & GSM3MOBILECLIENTSERVICE_WRITING))
		beginWrite(true);
	theGSM3MobileClientProvider->writeSocket(c);
	return 1;
}

size_t GSMSSL::write(const uint8_t* buf)
{
	if(!(flags & GSM3MOBILECLIENTSERVICE_WRITING))
		beginWrite(true);
	theGSM3MobileClientProvider->writeSocket((const char*)(buf));
	return strlen((const char*)buf);
}

size_t GSMSSL::write(const uint8_t* buf, size_t sz)
{
	if(!(flags & GSM3MOBILECLIENTSERVICE_WRITING))
		beginWrite(true);
	for(int i=0;i<sz;i++)
		theGSM3MobileClientProvider->writeSocket(buf[i]);
	return sz;
}

void GSMSSL::endWrite(bool sync)
{
	flags ^= GSM3MOBILECLIENTSERVICE_WRITING;
	theGSM3MobileClientProvider->endWriteSocket();
	if(sync)
		waitForAnswer('w');
}

uint8_t GSMSSL::connected()
{
	if(mySocket==255)
		return 0;
	return theGSM3MobileClientProvider->getStatusSocketClient(mySocket);	 
}

GSMSSL::operator bool()
{
	return connected()==1;
};

int GSMSSL::available()
{
	int res;

	// Even if not connected, we are looking for available data
	
	if(flags & GSM3MOBILECLIENTSERVICE_WRITING)
		endWrite(true);

	res=theGSM3MobileClientProvider->availableSocket(flags & GSM3MOBILECLIENTSERVICE_CLIENT,mySocket);
	if(res==-1){
		stop();
		return -1;
	}
	if(flags & GSM3MOBILECLIENTSERVICE_SYNCH && res==0)
		res=waitForAnswer('a');

	return res;
}

int GSMSSL::read(uint8_t *buf, size_t size)
{
	int i;
	uint8_t c;
	
	for(i=0;i<size;i++)
	{
		c=read();
		if(c==-1)
			break;
		buf[i]=c;
	}
	
	return i;
/* This is the old implementation, testing a simpler one
	int res;//==0
	// If we were writing, just stop doing it.
	if(flags & GSM3MOBILECLIENTSERVICE_WRITING)
		endWrite(true);
	res=theGSM3MobileClientProvider->readSocket(flags & GSM3MOBILECLIENTSERVICE_CLIENT, (char *)(buf), size, mySocket);

	return res;
*/
}

int GSMSSL::read()
{
	if(flags & GSM3MOBILECLIENTSERVICE_WRITING)
		endWrite(true);
	return theGSM3MobileClientProvider->readSocket();;
}

int GSMSSL::peek()
{
	if(flags & GSM3MOBILECLIENTSERVICE_WRITING)
		endWrite(true);
	return theGSM3MobileClientProvider->peekSocket(/*mySocket, false*/);
}

void GSMSSL::flush()
{
	if(flags & GSM3MOBILECLIENTSERVICE_WRITING)
		endWrite(true);
	theGSM3MobileClientProvider->flushSocket(/*mySocket*/);
	if(flags & GSM3MOBILECLIENTSERVICE_SYNCH)
		waitForAnswer('f');

}

void GSMSSL::stop()
{
	if(flags & GSM3MOBILECLIENTSERVICE_WRITING)
		endWrite(true);
	theGSM3MobileClientProvider->disconnectTCP(flags & GSM3MOBILECLIENTSERVICE_CLIENT, mySocket);
	theGSM3MobileClientProvider->releaseSocket(mySocket);
	mySocket = 0;
	if(flags & GSM3MOBILECLIENTSERVICE_SYNCH)
		waitForAnswer('t');
}

