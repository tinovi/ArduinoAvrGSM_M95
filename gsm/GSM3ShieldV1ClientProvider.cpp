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
#include "GSM3ShieldV1ClientProvider.h"
#include "GSM3ShieldV1ModemCore.h"
#include "debug.h"

GSM3ShieldV1ClientProvider::GSM3ShieldV1ClientProvider()
{
	theGSM3MobileClientProvider=this;
};

//Response management.
void GSM3ShieldV1ClientProvider::manageResponse(byte from, byte to)
{
	switch(theGSM3ShieldV1ModemCore.getOngoingCommand())
	{
		case NONE:
			theGSM3ShieldV1ModemCore.gss.cb.deleteToTheEnd(from);
			break;
		case CONNECTTCPCLIENT:
			connectTCPClientContinue();
			break;
		case DISCONNECTTCP:
			disconnectTcpContinue();
			break;
		case FLUSHSOCKET:
			flushSocketContinue();
			break;	
	}
}



//Connect TCP main function.
int GSM3ShieldV1ClientProvider::connectTCPClient(const char* server, int port, int id_socket)
{
	if(theGSM3ShieldV1ModemCore.getOngoingCommand()==CONNECTTCPCLIENT &&  theGSM3ShieldV1ModemCore.getCommandCounter()>1){
		connectTCPClientContinue();
		return theGSM3ShieldV1ModemCore.getCommandError();
	}
	delay(300);
	theGSM3ShieldV1ModemCore.theBuffer().flush();
	theGSM3ShieldV1ModemCore.setPort(port);		
	idSocket = id_socket;
	
	theGSM3ShieldV1ModemCore.setPhoneNumber((char*)server);
	theGSM3ShieldV1ModemCore.openCommand(this,CONNECTTCPCLIENT);
	theGSM3ShieldV1ModemCore.registerUMProvider(this);
	if(theGSM3ShieldV1ModemCore.getStatus()==TRANSPARENT_INIT_DONE){
		theGSM3ShieldV1ModemCore.setCommandCounter(7);
	}
	connectTCPClientContinue();
	return theGSM3ShieldV1ModemCore.getCommandError();	
}


int GSM3ShieldV1ClientProvider::connectTCPClient(IPAddress add, int port, int id_socket)
{
	remoteIP=add;
	theGSM3ShieldV1ModemCore.setPhoneNumber(0);
	return connectTCPClient(0, port, id_socket);
}

//Connect TCP continue function.
void GSM3ShieldV1ClientProvider::connectTCPClientContinue()
{
	bool resp;
	// 1: AT+QIMUX=0 for single socket
	// 2: AT+QIMODE=1 transparent mode
	// 3: AT+QIDNSIP= DNS lookup or IP
	// 4: Start-up TCP connection "AT+QIOPEN"
	// 5: Wait for connection OK
	// 6: Wait for CONNECT
	char _ok [4];
	prepareAuxLocate(PSTR("OK\r"), _ok);

	switch (theGSM3ShieldV1ModemCore.getCommandCounter()) {
	case 1:// AT+QIMUX=0 for single socket
		theGSM3ShieldV1ModemCore.genericCommand_rq(PSTR("AT+QIMUX=0"));
		theGSM3ShieldV1ModemCore.setCommandCounter(2);
		break;
	case 2://AT+QIMODE=1 transparent
		if(theGSM3ShieldV1ModemCore.genericParse_rsp(resp))
	    {
			if(resp)
			{
				theGSM3ShieldV1ModemCore.genericCommand_rq(PSTR("AT+QIMODE=1"));
				theGSM3ShieldV1ModemCore.setCommandCounter(4);
			}
			else theGSM3ShieldV1ModemCore.closeCommand(3);
		}
		break;
	case 3://AT+QITCFG=3,1,512,1 transparent config
		if(theGSM3ShieldV1ModemCore.genericParse_rsp(resp))
	    {
			if(resp)
			{
				theGSM3ShieldV1ModemCore.genericCommand_rq(PSTR("AT+QITCFG=3,1,512,1"));
				theGSM3ShieldV1ModemCore.setCommandCounter(4);
			}
			else theGSM3ShieldV1ModemCore.closeCommand(3);
		}
		break;
	case 4: //DNS lookup or IP
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
		theGSM3ShieldV1ModemCore.setCommandCounter(5);
		break;
	case 5: // Register the TCP/IP stack.
		if(theGSM3ShieldV1ModemCore.genericParse_rsp(resp))
		{
			if(resp)
			{
				// AT+QIREGAPP
				theGSM3ShieldV1ModemCore.genericCommand_rq(PSTR("AT+QIREGAPP"));
				theGSM3ShieldV1ModemCore.setCommandCounter(6);
			}
			else theGSM3ShieldV1ModemCore.closeCommand(3);
		}
		break;
	case 6: //Activate FGCNT
		if(theGSM3ShieldV1ModemCore.genericParse_rsp(resp))
		{
			if(resp)
			{
				// AT+QIACT
				theGSM3ShieldV1ModemCore.genericCommand_rq(PSTR("AT+QIACT"));
				theGSM3ShieldV1ModemCore.setCommandCounter(7);
			}
			else theGSM3ShieldV1ModemCore.closeCommand(3);
		}
		break;
	case 7://open connection
		resp = true;
		if(theGSM3ShieldV1ModemCore.getStatus()!=TRANSPARENT_INIT_DONE){
			if(!theGSM3ShieldV1ModemCore.genericParse_rsp(resp)){
				return;
			}
		}
		if(resp)
		{
			// AT+QIOPEN  connection
			theGSM3ShieldV1ModemCore.setStatus(TRANSPARENT_INIT_DONE);
			theGSM3ShieldV1ModemCore.genericCommand_rq(PSTR("AT+QIOPEN="),false);
			theGSM3ShieldV1ModemCore.print("\"TCP\",\"");
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
			theGSM3ShieldV1ModemCore.print('\r');
			theGSM3ShieldV1ModemCore.setCommandCounter(8);
		}
		else theGSM3ShieldV1ModemCore.closeCommand(3);
		break;
	
	case 8:
		if(theGSM3ShieldV1ModemCore.genericParse_rsp(resp))
	    {
			// Response received
			if(resp)
			{
				// OK Received
				// Great. Go for the next step
				theGSM3ShieldV1ModemCore.setCommandCounter(9);
			}
			else{
				theGSM3ShieldV1ModemCore.closeCommand(3);
				disconnectTCP(0,0);
			}
		}	
		break;
	case 9:


		char auxLocate [12];
		prepareAuxLocate(PSTR("FAIL"), auxLocate);
		if(theGSM3ShieldV1ModemCore.genericParse_rsp(resp,auxLocate))
	    {
			// Response received
			if(resp)
			{
				theGSM3ShieldV1ModemCore.closeCommand(3);
				return;
			}
		}

		prepareAuxLocate(PSTR("CONN"), auxLocate);
		if(theGSM3ShieldV1ModemCore.genericParse_rsp(resp,auxLocate))
	    {
			// Response received
			if(resp)
			{
				// Received CONNECT OK
				// Great. We're done
				theGSM3ShieldV1ModemCore.setStatus(TRANSPARENT_CONNECTED);
				//theGSM3ShieldV1ModemCore.theBuffer().chopUntil(auxLocate, true);
				theGSM3ShieldV1ModemCore.gss.setTransparent(true);
				//theGSM3ShieldV1ModemCore.theBuffer().flush();
				theGSM3ShieldV1ModemCore.closeCommand(1);
			}
			else 
				theGSM3ShieldV1ModemCore.closeCommand(3);
		}		
		break;
		
	}
}



bool GSM3ShieldV1ClientProvider::isConn()
{
	String number(NULL);
	// AT command for obtain IMEI
	String modemResponse = modemAccess.writeModemCommand("AT+QISTAT", 500);
	// Parse and check response
	char res_to_compare[modemResponse.length()];
	modemResponse.toCharArray(res_to_compare, modemResponse.length());
	if(strstr(res_to_compare,"INITIAL") != NULL)
		return false;
	return true;
}


int GSM3ShieldV1ClientProvider::disconnectTCP(bool client1Server0, int id_socket)
{		
	if((theGSM3ShieldV1ModemCore.getStatus()==TRANSPARENT_INIT_DONE
			|| (theGSM3ShieldV1ModemCore.getStatus()==GPRS_READY)
			&& theGSM3ShieldV1ModemCore.getCommandCounter()==1)){
		return 0;
	}
#if DEBUG
	Trln("!!CLOSE!!!");
	Trln();
#endif
	theGSM3ShieldV1ModemCore.openCommand(this,DISCONNECTTCP);
	theGSM3ShieldV1ModemCore.print("+++");
	delay(100);
	theGSM3ShieldV1ModemCore.theBuffer().flush();
	theGSM3ShieldV1ModemCore.genericCommand_rq(PSTR("AT+QICLOSE"));
	int cnt = 100; //10sec
	do
	{
		delay(100);
		cnt--;
	}while(theGSM3ShieldV1ModemCore.theBuffer().storedBytes()==0 && cnt>0);

	do
	{
		theGSM3ShieldV1ModemCore.theBuffer().flush();
		delay(100);
	}while(theGSM3ShieldV1ModemCore.theBuffer().storedBytes()>0);
	theGSM3ShieldV1ModemCore.gss.setTransparent(false);
	theGSM3ShieldV1ModemCore.closeCommand(1);
	if(theGSM3ShieldV1ModemCore.getStatus()==TRANSPARENT_CONNECTED){
		theGSM3ShieldV1ModemCore.setStatus(TRANSPARENT_INIT_DONE);
	}else{
		theGSM3ShieldV1ModemCore.setStatus(GPRS_READY);
	}
	theGSM3ShieldV1ModemCore.unRegisterUMProvider(this);

	//disconnectTcpContinue();

	return theGSM3ShieldV1ModemCore.getCommandError();
}


void GSM3ShieldV1ClientProvider::disconnectTcpContinue()
{
	bool resp;
	char auxLocate [12];
	prepareAuxLocate(PSTR("CLOSE"), auxLocate);
	// If we have incomed data
	if(theGSM3ShieldV1ModemCore.genericParse_rsp(resp,auxLocate))
    {
		// Response received
		if(resp)
		{
			if(theGSM3ShieldV1ModemCore.getStatus()==TRANSPARENT_CONNECTED){
				theGSM3ShieldV1ModemCore.setStatus(TRANSPARENT_INIT_DONE);
			}else{
				theGSM3ShieldV1ModemCore.setStatus(GPRS_READY);
			}
			theGSM3ShieldV1ModemCore.theBuffer().flush();
			theGSM3ShieldV1ModemCore.closeCommand(1);
			theGSM3ShieldV1ModemCore.unRegisterUMProvider(this);
		}
		else{
			//theGSM3ShieldV1ModemCore.genericCommand_rq(PSTR("AT+QICLOSE"));
			//theGSM3ShieldV1ModemCore.closeCommand(3);

		}
	}
}


//Write socket first chain main function.
void GSM3ShieldV1ClientProvider::beginWriteSocket(bool client1Server0, int id_socket)
{
}


//Write socket next chain function.
void GSM3ShieldV1ClientProvider::writeSocket(const char* buf)
{
	if(theGSM3ShieldV1ModemCore.getStatus()==TRANSPARENT_CONNECTED)
		theGSM3ShieldV1ModemCore.print(buf);
}

//Write socket character function.
void GSM3ShieldV1ClientProvider::writeSocket(uint8_t c)
{
	if(theGSM3ShieldV1ModemCore.getStatus()==TRANSPARENT_CONNECTED)
		theGSM3ShieldV1ModemCore.print((char)c);
}

//Write socket last chain main function.
void GSM3ShieldV1ClientProvider::endWriteSocket()
{		
}


//Available socket main function.
int GSM3ShieldV1ClientProvider::availableSocket(bool client1Server0, int id_socket)
{
	return theGSM3ShieldV1ModemCore.theBuffer().storedBytes();
}

int GSM3ShieldV1ClientProvider::readSocket()
{
		
	if(theGSM3ShieldV1ModemCore.theBuffer().storedBytes()==0)
	{
		return -1;
	}
	
	return theGSM3ShieldV1ModemCore.theBuffer().read();

}

//Read socket main function.
int GSM3ShieldV1ClientProvider::peekSocket()
{
	return theGSM3ShieldV1ModemCore.theBuffer().peek(0); 
}


//Flush SMS main function.
void GSM3ShieldV1ClientProvider::flushSocket()
{
	theGSM3ShieldV1ModemCore.theBuffer().debugBuffer();
	theGSM3ShieldV1ModemCore.theBuffer().flush();

//	theGSM3ShieldV1ModemCore.openCommand(this,FLUSHSOCKET);
//
//	flushSocketContinue();
}

//Send SMS continue function.
void GSM3ShieldV1ClientProvider::flushSocketContinue()
{
	// If we have incomed data
	if(theGSM3ShieldV1ModemCore.theBuffer().storedBytes()>0)
	{
		// Empty the local buffer, and tell the modem to XON
		// If meanwhile we receive a DISCONNECT we should detect it as URC.
		theGSM3ShieldV1ModemCore.theBuffer().flush();
//		theGSM3ShieldV1ModemCore.gss.spaceAvailable();
	}
	else 
	{
		//We're done		
		theGSM3ShieldV1ModemCore.closeCommand(1);
	}
}

// URC recognize.
// Yes, we recognize "closes" in client mode
bool GSM3ShieldV1ClientProvider::recognizeUnsolicitedEvent(byte oldTail)
{
	char auxLocate [12];
	prepareAuxLocate(PSTR("CLOSED"), auxLocate);

	if((theGSM3ShieldV1ModemCore.getStatus()==TRANSPARENT_CONNECTED) & theGSM3ShieldV1ModemCore.theBuffer().chopUntil(auxLocate, false, false))
	{
		theGSM3ShieldV1ModemCore.setStatus(GPRS_READY);
		theGSM3ShieldV1ModemCore.unRegisterUMProvider(this);
		return true;
	}
		
	return false;
}

int GSM3ShieldV1ClientProvider::getSocket(int socket)
{
	return 0;
}

void GSM3ShieldV1ClientProvider::releaseSocket(int socket)
{

}

bool GSM3ShieldV1ClientProvider::getStatusSocketClient(uint8_t socket)
{
	return (theGSM3ShieldV1ModemCore.getStatus()==TRANSPARENT_CONNECTED);

};



