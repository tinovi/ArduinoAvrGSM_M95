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
#include "GSM3ShieldV1AccessProvider.h"
#include <Arduino/Arduino.h>
#include "GSM3IO.h"
#include <util/delay.h>
#include "debug.h"

//#define __TOUTSHUTDOWN__ 8000
#define __TOUTMODEMCONFIGURATION__ 5000//equivalent to 30000 because of time in interrupt routine.
#define __TOUTAT__ 5000

const char _command_AT[] PROGMEM = "AT";
const char _command_CGREG[] PROGMEM = "AT+CGREG?";


GSM3ShieldV1AccessProvider::GSM3ShieldV1AccessProvider(bool debug)
{
	theGSM3ShieldV1ModemCore.setDebug(debug);
}

void GSM3ShieldV1AccessProvider::manageResponse(byte from, byte to)
{
	switch(theGSM3ShieldV1ModemCore.getOngoingCommand())
	{
		case MODEMCONFIG:
			ModemConfigurationContinue();
			break;
		case ALIVETEST:
			isModemAliveContinue();
			break;
	}
}

int GSM3ShieldV1AccessProvider::getModemState(){
//#if DEMO
//	return digitalRead(__STATUSPIN__);
//#else
	if(digitalRead(__STATUSPIN__)){
		return false;
	}else{
		return true;
	}
//#endif
}

///////////////////////////////////////////////////////CONFIGURATION FUNCTIONS///////////////////////////////////////////////////////////////////

void GSM3ShieldV1AccessProvider::init(){
	pinMode(__POWERPIN__, OUTPUT);
	digitalWrite(__POWERPIN__, LOW);
	pinMode(__STATUSPIN__, INPUT_PULLUP);
}

// Begin
// Restart or start the modem
// May be synchronous
GSM3_NetworkStatus_t GSM3ShieldV1AccessProvider::begin(char* pin, bool restart, bool synchronous)
{	

	HWoff();
	HWstart();

	// Launch modem configuration commands
	ModemConfiguration(pin);
	//delay(1000);
	// If synchronous, wait till ModemConfiguration is over
	if(synchronous)
	{
		// if we shorten this delay, the command fails
		int a=20;
		//int ct=theGSM3ShieldV1ModemCore.getCommandCounter();
		while(ready('A')==0 && a>0){
			delay(2000);
			a--;
			if(!getModemState()){
				return getStatus();
			}
		}
	}
	return getStatus();
}


//Initial configuration main function.
int GSM3ShieldV1AccessProvider::ModemConfiguration(char* pin)
{
	theGSM3ShieldV1ModemCore.theBuffer().flush();
	theGSM3ShieldV1ModemCore.setPhoneNumber(pin);
	theGSM3ShieldV1ModemCore.openCommand(this,MODEMCONFIG);
	theGSM3ShieldV1ModemCore.setCommandCounter(1);
	theGSM3ShieldV1ModemCore.setStatus(CONNECTING);
	theGSM3ShieldV1ModemCore.genericCommand_rq(PSTR("ATE0"));
	//ModemConfigurationContinue();

	return theGSM3ShieldV1ModemCore.getCommandError();
}

//Initial configuration continue function.
void GSM3ShieldV1AccessProvider::ModemConfigurationContinue()
{
	bool resp;
	//Trln("_c_");

	// 1: Send AT
	// 2: Wait AT OK and SetPin or CGREG
	// 3: Wait Pin OK and CGREG
	// 4: Wait CGREG and Flow SW control or CGREG
	// 5: Wait IFC OK and SMS Text Mode
	// 6: Wait SMS text Mode OK and Calling line identification
	// 7: Wait Calling Line Id OK and Echo off
	// 8: Wait for OK and COLP command for connecting line identification.
	// 9: Wait for OK.
	int ct=theGSM3ShieldV1ModemCore.getCommandCounter();
	if(ct==1)
	{
		// Launch AT	
		theGSM3ShieldV1ModemCore.setCommandCounter(2);
		theGSM3ShieldV1ModemCore.genericCommand_rq(PSTR("AT+QIURC=0"));
		theGSM3ShieldV1ModemCore.genericCommand_rq(_command_AT);
	}
	else if(ct==2)
	{
		// Wait for AT - OK.
	   if(theGSM3ShieldV1ModemCore.genericParse_rsp(resp))
	   {
			if(resp)
			{ 
				// OK received
				if(theGSM3ShieldV1ModemCore.getPhoneNumber() && (theGSM3ShieldV1ModemCore.getPhoneNumber()[0]!=0))
					{
						theGSM3ShieldV1ModemCore.genericCommand_rq(PSTR("AT+CPIN="), false);
						theGSM3ShieldV1ModemCore.setCommandCounter(3);
						theGSM3ShieldV1ModemCore.genericCommand_rqc(theGSM3ShieldV1ModemCore.getPhoneNumber());
					}
				else
					{
						//DEBUG	
						theGSM3ShieldV1ModemCore.setCommandCounter(4);
						theGSM3ShieldV1ModemCore.takeMilliseconds();
						theGSM3ShieldV1ModemCore.genericCommand_rq(_command_CGREG);
					}
			}
			else theGSM3ShieldV1ModemCore.closeCommand(3);
		}
	}
	else if(ct==3)
	{
		if(theGSM3ShieldV1ModemCore.genericParse_rsp(resp))
	    {
			if(resp)
			{
				theGSM3ShieldV1ModemCore.setCommandCounter(4);
				theGSM3ShieldV1ModemCore.takeMilliseconds();
				theGSM3ShieldV1ModemCore.delayInsideInterrupt(2000);
				theGSM3ShieldV1ModemCore.genericCommand_rq(_command_CGREG);
			}
			else theGSM3ShieldV1ModemCore.closeCommand(3);
	    }
	}
	else if(ct==4)
	{
		char auxLocate1 [12];
		char auxLocate2 [12];
		prepareAuxLocate(PSTR("+CGREG: 0,1"), auxLocate1);
		prepareAuxLocate(PSTR("+CGREG: 0,5"), auxLocate2);
		if(theGSM3ShieldV1ModemCore.genericParse_rsp(resp, auxLocate1, auxLocate2))
		{
			if(resp)
			{
				theGSM3ShieldV1ModemCore.setStatus(GSM_READY);
				theGSM3ShieldV1ModemCore.closeCommand(1);
//				theGSM3ShieldV1ModemCore.setCommandCounter(5);
//				theGSM3ShieldV1ModemCore.genericCommand_rq(PSTR("AT+IFC=0,0"));
			}
			else
			{
				// If not, launch command again
				if(theGSM3ShieldV1ModemCore.takeMilliseconds() > __TOUTMODEMCONFIGURATION__)
				{
					theGSM3ShieldV1ModemCore.closeCommand(3);
				}
				else 
				{
					theGSM3ShieldV1ModemCore.delayInsideInterrupt(2000);
					theGSM3ShieldV1ModemCore.genericCommand_rq(_command_CGREG);
				}
			}
		}	
	}
//	else if(ct==5)
//	{
//		// 5: Wait IFC OK
//		if(theGSM3ShieldV1ModemCore.genericParse_rsp(resp))
//		{
//			//Delay for SW flow control being active.
//			theGSM3ShieldV1ModemCore.delayInsideInterrupt(2000);
//			// 9: SMS Text Mode
//			theGSM3ShieldV1ModemCore.setCommandCounter(11);
//			theGSM3ShieldV1ModemCore.genericCommand_rq(PSTR("AT+CMGF=1"));
//		}
//	}
//	else if(ct==6)
//	{
//		// 6: Wait SMS text Mode OK
//		if(theGSM3ShieldV1ModemCore.genericParse_rsp(resp))
//		{
//			//Calling line identification
//			theGSM3ShieldV1ModemCore.setCommandCounter(7);
//			theGSM3ShieldV1ModemCore.genericCommand_rq(PSTR("AT+CLIP=1"));
//		}
//	}
//	else if(ct==7)
//	{
//		// 7: Wait Calling Line Id OK
//		if(theGSM3ShieldV1ModemCore.genericParse_rsp(resp))
//		{
//			// Echo off
//			theGSM3ShieldV1ModemCore.setCommandCounter(8);
//			theGSM3ShieldV1ModemCore.genericCommand_rq(PSTR("ATE0"));
//		}
//	}

//	else if(ct==11)
//	{
//		// 9: Wait ATCOLP OK
//		if(theGSM3ShieldV1ModemCore.genericParse_rsp(resp))
//		{
//			if (resp)
//				{
//					theGSM3ShieldV1ModemCore.setStatus(GSM_READY);
//					theGSM3ShieldV1ModemCore.closeCommand(1);
//				}
//			else {
//				theGSM3ShieldV1ModemCore.closeCommand(3);
//			}
//		}
// 	}
}

//Alive Test main function.
int GSM3ShieldV1AccessProvider::isAccessAlive()
{
	theGSM3ShieldV1ModemCore.setCommandError(0);
	theGSM3ShieldV1ModemCore.setCommandCounter(1);
	theGSM3ShieldV1ModemCore.openCommand(this,ALIVETEST);
	isModemAliveContinue();
	int a=8;
	while(ready('L')==0 && a>0){
		delay(500);
		a--;
	}
	theGSM3ShieldV1ModemCore.theBuffer().flush();
	return theGSM3ShieldV1ModemCore.getCommandError();
}

//Alive Test continue function.
void GSM3ShieldV1AccessProvider::isModemAliveContinue()
{
bool rsp;
switch (theGSM3ShieldV1ModemCore.getCommandCounter()) {
    case 1:
		theGSM3ShieldV1ModemCore.genericCommand_rq(_command_AT);
		theGSM3ShieldV1ModemCore.setCommandCounter(2);
      break;
	case 2:
		if(theGSM3ShieldV1ModemCore.genericParse_rsp(rsp))
		{
			if (rsp) theGSM3ShieldV1ModemCore.closeCommand(1);
			else theGSM3ShieldV1ModemCore.closeCommand(3);
		}
      break;
	}
}

//HWrestart.
int GSM3ShieldV1AccessProvider::HWstart()
{
#if DEBUG
	Trln("on??");
#endif
	while(!getModemState()){ // not running
#if DEBUG
		Trln("HWon..");
#endif
		delay(700);
		theGSM3ShieldV1ModemCore.setStatus(IDLE);
		theGSM3ShieldV1ModemCore.gss.begin(9600);
		pinMode(__POWERPIN__, OUTPUT);
		digitalWrite(__POWERPIN__, HIGH);
		//delay(2000);
		int i=0;
		while(!getModemState() && i<80){
			i++;
			delay(200);
		}
#if DEBUG
		Tr("i:");
		Trln(i);
#endif
		digitalWrite(__POWERPIN__, LOW);
		if (i==0){
			continue;
		}
		theGSM3ShieldV1ModemCore.genericCommand_rq(_command_AT);
		delay(4000);
		theGSM3ShieldV1ModemCore.theBuffer().flush();
//		if(!isAccessAlive()){
//			continue;
//		}
		return 1;
	}
	if(theGSM3ShieldV1ModemCore.getStatus()==OFF){
		theGSM3ShieldV1ModemCore.gss.begin(9600);
	}
	//theGSM3ShieldV1ModemCore.setStatus(IDLE);
	theGSM3ShieldV1ModemCore.theBuffer().flush();
	return 0; //configandwait(pin);
}

//HWrestart.
int GSM3ShieldV1AccessProvider::HWoff()
{
#if DEBUG
    Trln("off??");
#endif
	while(getModemState()){ // running

#if DEBUG
		  Trln("HWoff...");
#endif
	  pinMode(__POWERPIN__, OUTPUT);
	  digitalWrite(__POWERPIN__, HIGH);
	  delay(1600);
	  digitalWrite(__POWERPIN__, LOW);
	  for(int i=0;i<80 && getModemState();i++){
	  	delay(200);
	  }
	  theGSM3ShieldV1ModemCore.setStatus(OFF);
	  theGSM3ShieldV1ModemCore.gss.close();
	  return 1;
	}
	theGSM3ShieldV1ModemCore.setStatus(OFF);
	theGSM3ShieldV1ModemCore.gss.close();
	return 0; //configandwait(pin);
}


bool GSM3ShieldV1AccessProvider::shutdown()
{
#if DEBUG
	Trln("shutdown...");
#endif
	bool resp;
	char auxLocate [18];

	theGSM3ShieldV1ModemCore.theBuffer().flush();
	// It makes no sense to have an asynchronous shutdown
	pinMode(__POWERPIN__, OUTPUT);
	digitalWrite(__POWERPIN__, HIGH);
	delay(900);
	digitalWrite(__POWERPIN__, LOW);
	theGSM3ShieldV1ModemCore.setStatus(IDLE);

	prepareAuxLocate(PSTR("POWER DOWN"), auxLocate);
	for(int i=0;i<80;i++)
	{
		delay(100);
		if(theGSM3ShieldV1ModemCore.genericParse_rsp(resp, auxLocate)){
			theGSM3ShieldV1ModemCore.gss.close();
			return resp;
		}
	}
	return false;
}

//Secure shutdown.
bool GSM3ShieldV1AccessProvider::secureShutdown()
{
	// It makes no sense to have an asynchronous shutdown
	pinMode(__POWERPIN__, OUTPUT);
	digitalWrite(__POWERPIN__, HIGH);
	delay(1500);
	digitalWrite(__POWERPIN__, LOW);
	theGSM3ShieldV1ModemCore.setStatus(OFF);
	theGSM3ShieldV1ModemCore.gss.close();

	return true;
}
