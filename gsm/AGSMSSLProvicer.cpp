/*
This file is part of the GSM3 communications library for Arduino
-- Multi-transport communications platform
-- Fully asynchronous
-- Includes code for the Arduino-Telefonica GSM/GPRS Shield V1
-- Voice calls
-- SMS
-- TCP/IP connections
-- HTTP basic clients

This library has been developed by Telefónica Digital - PDI -
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
#include "debug.h"
#include "AGSMSSLProvider.h"
#include "GSM3ShieldV1ModemCore.h"
#include <NeoHWSerial/NeoHWSerial.h>
#include <stdio.h>

//const char _command_MultiQISRVC[] PROGMEM = "AT+QISRVC=";

#define __TOUTFLUSH__ 10000

GSMSSLProvider::GSMSSLProvider()
{
	theGSM3MobileClientProvider=this;
	theGSM3ShieldV1ModemCore.registerUMProvider(this);
};

//Response management.
void GSMSSLProvider::manageResponse(byte from, byte to)
{
	switch(theGSM3ShieldV1ModemCore.getOngoingCommand())
	{
//		case XON:
//			if (flagReadingSocket)
//				{
////					flagReadingSocket = 0;
//					fullBufferSocket = (theGSM3ShieldV1ModemCore.theBuffer().storedBytes()<3);
//				}
//			else theGSM3ShieldV1ModemCore.setOngoingCommand(NONE);
//			break;
		case NONE:
			theGSM3ShieldV1ModemCore.gss.cb.deleteToTheEnd(from);
			break;
		case CONNECTTCPCLIENT:
			connectTCPClientContinue();
			break;
		case DISCONNECTTCP:
			disconnectTCPContinue();
			break;	
	 	case BEGINWRITESOCKET:
			beginWriteSocketContinue();
			break;
		case ENDWRITESOCKET:
			endWriteSocketContinue();
			break;
		case AVAILABLESOCKET:
			availableSocketContinue();
			break;
		case WAITURC:
			isUrc();
			break;
		case FLUSHSOCKET:
			//fullBufferSocket = (theGSM3ShieldV1ModemCore.theBuffer().availableBytes()<3);
			flushSocketContinue();
			break;
	}
}


//Connect TCP main function.
int GSMSSLProvider::connectTCPClient(const char* server, int port, int id_socket)
{
	if(theGSM3ShieldV1ModemCore.getOngoingCommand()==CONNECTTCPCLIENT &&  theGSM3ShieldV1ModemCore.getCommandCounter()>1){
		connectTCPClientContinue();
		return theGSM3ShieldV1ModemCore.getCommandError();
	}

	theGSM3ShieldV1ModemCore.setPort(port);
	idSocket = id_socket;

	theGSM3ShieldV1ModemCore.setPhoneNumber((char*)server);
	theGSM3ShieldV1ModemCore.openCommand(this,CONNECTTCPCLIENT);
	theGSM3ShieldV1ModemCore.registerUMProvider(this);
	if(theGSM3ShieldV1ModemCore.getStatus()==SSL_INIT_DONE){
		theGSM3ShieldV1ModemCore.setCommandCounter(10);
	}
	connectTCPClientContinue();
	return theGSM3ShieldV1ModemCore.getCommandError();
}

int GSMSSLProvider::connectTCPClient(IPAddress add, int port, int id_socket)
{
	remoteIP=add;
	theGSM3ShieldV1ModemCore.setPhoneNumber(0);
	return connectTCPClient(0, port, id_socket);
}

//Connect TCP continue function.
void GSMSSLProvider::connectTCPClientContinue()
{
	bool resp;
	// 0: Dot or DNS notation activation
	// 1: Disable SW flow control
	// 2: Waiting for IFC OK
	// 3: Start-up TCP connection "AT+QIOPEN"
	// 4: Wait for connection OK
	// 5: Wait for CONNECT
	char _ok [4];
	prepareAuxLocate(PSTR("OK\r"), _ok);

	switch (theGSM3ShieldV1ModemCore.getCommandCounter()) {

	case 1:
//		if(theGSM3ShieldV1ModemCore.genericParse_rsp(resp))
//	    {
//			if(resp)
//			{
				// AT+QIMUX=1 for multisocket
//		theGSM3ShieldV1ModemCore.theBuffer().chopUntil(_ok, true);
				theGSM3ShieldV1ModemCore.genericCommand_rq(PSTR("AT+QIMUX=1"));
				theGSM3ShieldV1ModemCore.setCommandCounter(2);
//			}
//			else theGSM3ShieldV1ModemCore.closeCommand(3);
//		}
		break;
	case 2:
		if(theGSM3ShieldV1ModemCore.genericParse_rsp(resp))
	    {
			if(resp)
			{
				//AT+QIMODE=0  NON - transparent
		//theGSM3ShieldV1ModemCore.theBuffer().chopUntil(_ok, true);
				theGSM3ShieldV1ModemCore.genericCommand_rq(PSTR("AT+QIMODE=0"));
				theGSM3ShieldV1ModemCore.setCommandCounter(3);
			}
			else theGSM3ShieldV1ModemCore.closeCommand(3);
		}
		break;
	case 3:

		//SSL config
		if(theGSM3ShieldV1ModemCore.genericParse_rsp(resp))
	    {
			if(resp)
			{
//		theGSM3ShieldV1ModemCore.theBuffer().chopUntil(_ok, true);
				theGSM3ShieldV1ModemCore.genericCommand_rq(PSTR("AT+QSSLCFG=\"sslversion\",0,3"));
				theGSM3ShieldV1ModemCore.setCommandCounter(4);
			}
			else theGSM3ShieldV1ModemCore.closeCommand(3);
		}
		break;
	case 4:
		//SSL config
		if(theGSM3ShieldV1ModemCore.genericParse_rsp(resp))
	    {
			if(resp)
			{
//		theGSM3ShieldV1ModemCore.theBuffer().chopUntil(_ok, true);
				theGSM3ShieldV1ModemCore.genericCommand_rq(PSTR("AT+QSSLCFG=\"seclevel\",0,0"));
				theGSM3ShieldV1ModemCore.setCommandCounter(5);
			}
			else theGSM3ShieldV1ModemCore.closeCommand(3);
		}
		break;
	case 5:
		//SSL config
		if(theGSM3ShieldV1ModemCore.genericParse_rsp(resp))
	    {
			if(resp)
			{
//		theGSM3ShieldV1ModemCore.theBuffer().chopUntil(_ok, true);
//			theGSM3ShieldV1ModemCore.genericCommand_rq(PSTR("AT+QSSLCFG=\"ciphersuite\",0,\"0X0035\""));
				theGSM3ShieldV1ModemCore.genericCommand_rq(PSTR("AT+QSSLCFG=\"ciphersuite\",0,\"0XFFFF\""));
				theGSM3ShieldV1ModemCore.setCommandCounter(7);
			}
			else theGSM3ShieldV1ModemCore.closeCommand(3);
	    }
		break;
//	case 6:
//		if(theGSM3ShieldV1ModemCore.genericParse_rsp(resp))
//	    {
//			if(resp)
//			{
//				theGSM3ShieldV1ModemCore.genericCommand_rq(PSTR("AT+QSSLCFG=\"cacert\",0,\"NVRAM:CA0\""));
//				theGSM3ShieldV1ModemCore.setCommandCounter(7);
//			}
//			else theGSM3ShieldV1ModemCore.closeCommand(3);
//	    }
//		break;
	case 7:
		if(theGSM3ShieldV1ModemCore.genericParse_rsp(resp))
	    {
			if(resp)
			{
				theGSM3ShieldV1ModemCore.genericCommand_rq(PSTR("AT+QIDNSIP="), false);
				if (theGSM3ShieldV1ModemCore.getPhoneNumber()!=0)
				{
					theGSM3ShieldV1ModemCore.print('1');
					theGSM3ShieldV1ModemCore.print('\r');
				}
				else
				{
					theGSM3ShieldV1ModemCore.print('0');
					theGSM3ShieldV1ModemCore.print('\r');
				}
				theGSM3ShieldV1ModemCore.setCommandCounter(8);
			}
			else theGSM3ShieldV1ModemCore.closeCommand(3);
	    }
		break;
	case 8: // Register the TCP/IP stack.
		if(theGSM3ShieldV1ModemCore.genericParse_rsp(resp))
		{
			if(resp)
			{
				// AT+QIREGAPP
				theGSM3ShieldV1ModemCore.genericCommand_rq(PSTR("AT+QIREGAPP"));
				theGSM3ShieldV1ModemCore.setCommandCounter(9);
			}
			else theGSM3ShieldV1ModemCore.closeCommand(3);
		}
		break;
	case 9: //Activate FGCNT
		if(theGSM3ShieldV1ModemCore.genericParse_rsp(resp))
		{
			if(resp)
			{
				// AT+QIACT
				theGSM3ShieldV1ModemCore.genericCommand_rq(PSTR("AT+QIACT"));
				theGSM3ShieldV1ModemCore.setCommandCounter(10);
			}
			else theGSM3ShieldV1ModemCore.closeCommand(3);
		}
		break;
	case 10:
		resp = true;
		if(theGSM3ShieldV1ModemCore.getStatus()!=SSL_INIT_DONE){
			if(!theGSM3ShieldV1ModemCore.genericParse_rsp(resp)){
				return;
			}
		}
		if(resp)
		{
			theGSM3ShieldV1ModemCore.setStatus(SSL_INIT_DONE);
			// AT+QIOPEN
			theGSM3ShieldV1ModemCore.genericCommand_rq(PSTR("AT+QSSLOPEN="),false);
			theGSM3ShieldV1ModemCore.print(idSocket);
			theGSM3ShieldV1ModemCore.print(",0,\"");
			if(theGSM3ShieldV1ModemCore.getPhoneNumber()!=0)
			{
				theGSM3ShieldV1ModemCore.print(theGSM3ShieldV1ModemCore.getPhoneNumber());
			}
			else
			{
				remoteIP.printTo(theGSM3ShieldV1ModemCore);
			}
			theGSM3ShieldV1ModemCore.print('"');
			theGSM3ShieldV1ModemCore.print(',');
			theGSM3ShieldV1ModemCore.print(theGSM3ShieldV1ModemCore.getPort());
			theGSM3ShieldV1ModemCore.print(',');
			theGSM3ShieldV1ModemCore.print('0');
			theGSM3ShieldV1ModemCore.print('\r');
			theGSM3ShieldV1ModemCore.setCommandCounter(11);
		}
		else theGSM3ShieldV1ModemCore.closeCommand(3);
		break;

	case 11:
		if(theGSM3ShieldV1ModemCore.theBuffer().chopUntil(_ok, true))
	    {
				// OK Received
				// Great. Go for the next step
				theGSM3ShieldV1ModemCore.setCommandCounter(12);
		}
		else theGSM3ShieldV1ModemCore.closeCommand(3);
		break;
	case 12:
		char auxLocate [11];
		prepareAuxLocate(PSTR("+QSSLOPEN"), auxLocate);
		if(theGSM3ShieldV1ModemCore.theBuffer().chopUntil(auxLocate, true))
	    {
				// Received CONNECT OK
				// Great. We're done
				theGSM3ShieldV1ModemCore.closeCommand(1);
				//theGSM3ShieldV1ModemCore.openCommand(this,WAITURC);
		}
		else
			theGSM3ShieldV1ModemCore.closeCommand(3);
		break;

	}
}

//Disconnect TCP main function.
int GSMSSLProvider::disconnectTCP(bool client1Server0, int id_socket)
{
	idSocket = id_socket;

	// First of all, we will flush the socket synchronously
	unsigned long m;
	m=millis();
	flushSocket();
	while(((millis()-m)< __TOUTFLUSH__ )&&(ready('T')==0))
		_delay_ms(10);

	// Could not flush the communications... strange
	if(ready('T')==0)
	{
		theGSM3ShieldV1ModemCore.setCommandError(2);
		return theGSM3ShieldV1ModemCore.getCommandError();
	}

	// Set up the command
	toRead=0;
	theGSM3ShieldV1ModemCore.openCommand(this,DISCONNECTTCP);
	disconnectTCPContinue();
	return theGSM3ShieldV1ModemCore.getCommandError();
}

//Disconnect TCP continue function
void GSMSSLProvider::disconnectTCPContinue()
{
	bool resp;
	// 1: Send AT+QISRVC
	// 2: "AT+QICLOSE"
	// 3: Wait for OK

	switch (theGSM3ShieldV1ModemCore.getCommandCounter()) {
	case 1:
		// Parse response to QISRVC
		theGSM3ShieldV1ModemCore.genericParse_rsp(resp);
		if(resp)
		{
			// Send QICLOSE command
			theGSM3ShieldV1ModemCore.genericCommand_rq(PSTR("AT+QSSLCLOSE="),false);
			theGSM3ShieldV1ModemCore.print(idSocket);
			theGSM3ShieldV1ModemCore.print('\r');
			theGSM3ShieldV1ModemCore.setCommandCounter(3);
		}
		else
			theGSM3ShieldV1ModemCore.closeCommand(3);
		break;
	case 3:
		if(theGSM3ShieldV1ModemCore.genericParse_rsp(resp))
	    {
			theGSM3ShieldV1ModemCore.setCommandCounter(0);
			if (resp)
				theGSM3ShieldV1ModemCore.closeCommand(1);
			else
				theGSM3ShieldV1ModemCore.closeCommand(3);
			theGSM3ShieldV1ModemCore.unRegisterUMProvider(this);
		}
		break;
	}
}

//Write socket first chain main function.
void GSMSSLProvider::beginWriteSocket(bool client1Server0, int id_socket)
{
	toRead=0;
	flagReadingSocket = false;
	idSocket = id_socket;
	theGSM3ShieldV1ModemCore.openCommand(this,BEGINWRITESOCKET);
	beginWriteSocketContinue();
}

//Write socket first chain continue function.
void GSMSSLProvider::beginWriteSocketContinue()
{
	bool resp;
	// 1: Send AT+QISRVC
	// 2: Send AT+QISEND
	// 3: wait for > and Write text
	switch (theGSM3ShieldV1ModemCore.getCommandCounter()) {
    case 1:
		// AT+QISEND
		theGSM3ShieldV1ModemCore.genericCommand_rq(PSTR("AT+QSSLSEND="), false);
		theGSM3ShieldV1ModemCore.print(idSocket);
		theGSM3ShieldV1ModemCore.print('\r');
		theGSM3ShieldV1ModemCore.setCommandCounter(3);
		break;
	case 3:
		char aux[2];
		aux[0]='>';
		aux[1]=0;
		if(theGSM3ShieldV1ModemCore.genericParse_rsp(resp, aux))
		{
			if(resp)
			{
				// Received ">"
				theGSM3ShieldV1ModemCore.closeCommand(1);
			}
			else
			{
				theGSM3ShieldV1ModemCore.closeCommand(3);
			}
		}
		break;
	}
}

//Write socket next chain function.
void GSMSSLProvider::writeSocket(const char* buf)
{
	theGSM3ShieldV1ModemCore.print(buf);
}

//Write socket character function.
void GSMSSLProvider::writeSocket(uint8_t c)
{
	theGSM3ShieldV1ModemCore.print(c);
}

//Write socket last chain main function.
void GSMSSLProvider::endWriteSocket()
{
	theGSM3ShieldV1ModemCore.openCommand(this,ENDWRITESOCKET);
	endWriteSocketContinue();
}

//Write socket last chain continue function.
void GSMSSLProvider::endWriteSocketContinue()
{
	bool resp;
	// 1: Write text (ctrl-Z)
	// 2: Wait for OK
	switch (theGSM3ShieldV1ModemCore.getCommandCounter()) {
	case 1:
		theGSM3ShieldV1ModemCore.write(26); // Ctrl-Z
		theGSM3ShieldV1ModemCore.setCommandCounter(2);
		break;
	case 2:
		if(theGSM3ShieldV1ModemCore.genericParse_rsp(resp))
		{
			// OK received
			if (resp){
				theGSM3ShieldV1ModemCore.closeCommand(1);
				//theGSM3ShieldV1ModemCore.openCommand(this,WAITURC);
			}
			else{
				//theGSM3ShieldV1ModemCore.closeCommand(3);
				theGSM3ShieldV1ModemCore.setCommandCounter(2);
			}
		}
		break;
	}
}

//Available socket main function.
int GSMSSLProvider::availableSocket(bool client1Server0, int id_socket)
{

	if(theGSM3ShieldV1ModemCore.getCommandError()==3){
		return -1;
	}
	if(theGSM3ShieldV1ModemCore.theBuffer().storedBytes()>0 && toRead>0)
	{
		return theGSM3ShieldV1ModemCore.theBuffer().storedBytes();
	}else if(toRead>0){
		toReadCnt++;
		delay(100);
		if(toReadCnt>254){  //tread timeout
			toRead = 0;
			toReadCnt = 0;
		}else{
			return 0;
		}
	}else if(theGSM3ShieldV1ModemCore.getOngoingCommand()==NONE){
		idSocket = id_socket;
		theGSM3ShieldV1ModemCore.openCommand(this,AVAILABLESOCKET);
		availableSocketContinue();
	}
	return 0;
}

//Available socket continue function.
void GSMSSLProvider::availableSocketContinue()
{
	bool resp;
	// 1: AT+QIRD
	// 2: Wait for OK and Next necessary AT+QIRD

	switch (theGSM3ShieldV1ModemCore.getCommandCounter()) {
	case 1:
		theGSM3ShieldV1ModemCore.genericCommand_rq(PSTR("AT+QSSLRECV=0,"),false);
//		theGSM3ShieldV1ModemCore.print('1');
//		theGSM3ShieldV1ModemCore.print(',');
		theGSM3ShieldV1ModemCore.print(idSocket);
		theGSM3ShieldV1ModemCore.print(",");
		theGSM3ShieldV1ModemCore.print(theGSM3ShieldV1ModemCore.theBuffer().availableBytes());
		theGSM3ShieldV1ModemCore.print('\r');
		theGSM3ShieldV1ModemCore.setCommandCounter(2);
		break;
	case 2:
		if(parseQIRD_head(resp))
		{
			if (!resp)
			{
				theGSM3ShieldV1ModemCore.closeCommand(4);
			}
			else
			{
				flagReadingSocket=1;
				theGSM3ShieldV1ModemCore.closeCommand(1);
			}
		}
		else
		{
			theGSM3ShieldV1ModemCore.closeCommand(3);
		}
		break;
	}
}
	
//Read Socket Parse head.
bool GSMSSLProvider::parseQIRD_head(bool& rsp)
{
	char _qird [7];
	prepareAuxLocate(PSTR(",TCP,"), _qird);
	//fullBufferSocket = (theGSM3ShieldV1ModemCore.theBuffer().availableBytes()<3);
	if(theGSM3ShieldV1ModemCore.theBuffer().chopUntil(_qird, true))
	{
		// Saving more memory, reuse _qird
		char num[5];
		for(int i=0;i<5;i++){
			num[i] = theGSM3ShieldV1ModemCore.theBuffer().read();
			if(num[i]=='\r'){ //skip enter
				continue;
			}
			if(num[i]=='\n'){
				num[i]='\0';
				toRead = atoi(num);
#if DEBUG
				Tr("tr:");
				Trln(toRead);
#endif

				//sscanf(myarray, "%d", &i);

				break;
			}
		}
		rsp = true;
		return true;
	}
	else if(theGSM3ShieldV1ModemCore.theBuffer().locate("OK"))
	{
		rsp = false;
		//theGSM3ShieldV1ModemCore.openCommand(this,WAITURC);
		return true;
	}
	else
	{
		rsp = false;
		return false;
	}
}

void GSMSSLProvider::isUrc(){
	switch (theGSM3ShieldV1ModemCore.getCommandCounter()) {
	case 1:
		char _qird [11];
		prepareAuxLocate(PSTR("+QSSLURC:"), _qird);
		if(theGSM3ShieldV1ModemCore.theBuffer().chopUntil(_qird, true))
		{
			theGSM3ShieldV1ModemCore.openCommand(this,AVAILABLESOCKET);
			availableSocketContinue();
		}
		break;
	}

}



int GSMSSLProvider::readSocket()
{
	
	if(toRead==0)
	{
#if DEBUG
		Trln("*");
#endif
		return -1;
	}else if(theGSM3ShieldV1ModemCore.theBuffer().storedBytes()>0){
		toRead--;
		return theGSM3ShieldV1ModemCore.theBuffer().read();
	}

	if(toRead>0){
		delay(200);
	}

	if(theGSM3ShieldV1ModemCore.theBuffer().storedBytes()==0)
	{
#if DEBUG
		Trln("*");
#endif
		return -1;
	}
	toRead--;
	return theGSM3ShieldV1ModemCore.theBuffer().read();

}

//Read socket main function.
int GSMSSLProvider::peekSocket()
{
	return theGSM3ShieldV1ModemCore.theBuffer().peek(0);
}


//Flush SMS main function.
void GSMSSLProvider::flushSocket()
{
	toRead=0;
	theGSM3ShieldV1ModemCore.theBuffer().flush();

//	theGSM3ShieldV1ModemCore.openCommand(this,FLUSHSOCKET);
//	flushSocketContinue();
}

//Send SMS continue function.
void GSMSSLProvider::flushSocketContinue()
{
	//bool resp;
	// 1: Deleting SMS
	// 2: wait for OK
	switch (theGSM3ShieldV1ModemCore.getCommandCounter()) {
    case 1:
		//DEBUG
#if DEBUG
		Trln("Flush.");
#endif
			theGSM3ShieldV1ModemCore.theBuffer().flush();
			theGSM3ShieldV1ModemCore.closeCommand(1);
//			if (fullBufferSocket)
//				{
//					Trln("Buffer flushed.");
//					theGSM3ShieldV1ModemCore.gss.spaceAvailable();
//				}
//			else
//				{
//					Trln("Socket flushed completely.");
//					theGSM3ShieldV1ModemCore.closeCommand(1);
//				}
		break;
	}
}

//URC recognize.
// Momentarily, we will not recognize "closes" in client mode
bool GSMSSLProvider::recognizeUnsolicitedEvent(byte oldTail)
{
	return false;
}

int GSMSSLProvider::getSocket(int socket)
{
	if(socket==-1)
	{
		int i;
		for(i=minSocket(); i<=maxSocket(); i++)
		{
			if (!(sockets&(0x0001<<i)))
			{
				sockets|=((0x0001)<<i);
				return i;
			}
		}
	}
	else
	{
		if (!(sockets&(0x0001<<socket)))
		{
			sockets|=((0x0001)<<socket);
			return socket;
		}
	}
	return -1;
}

void GSMSSLProvider::releaseSocket(int socket)
{
	if (sockets&((0x0001)<<socket))
		sockets^=((0x0001)<<socket);
}

bool GSMSSLProvider::getStatusSocketClient(uint8_t socket)
{
	if(socket>8)
		return 0;
	if(sockets&(0x0001<<socket))
		return 1;
	else
		return 0;
};



