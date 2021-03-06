
/*
*
* Name:		PoolTX.ino
* Created:	24/07/2016 14:46
* Author:	Yusuf KALSEN
* URL: http://github.com/wusiv
*
* This project for my little Walnut orchard.
*
* Electric Price  ;
* NORMAL(Kw/$)		:0,1452 $
* EXPENSIVE(Kw/$)	:0,2179 $ (+ 50% than Normal)
* OFF-PRICE (Kw/$)	:0,0927 $ (- 36% than Normal )
*
* 06:00 - 16:59 - is NORMAL
* 17:00 - 21:59 - is EXPENSIVE
* 22:00 - 05:59 - is OFF-PRICE
*
*
*
*
* Arduino Pool Water Level is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published
* by the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Arduino Pool Water Level is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with Arduino Pool Water Level. If not, see <http://www.gnu.org/licenses/>.
*
*
*
*
*
*/

/* pull down resistor
*  Pull-Down Resistor.
*/

#include <EEPROM.h>
#include <SoftwareSerial.h>


//#define ENABLE_DEBUG
#define RUN_NOW		11
#define RUN_NIGHT	21
#define STOP		10
#define PUMP_RUN_NOW 111
#define PUMP_RUN_NIGHT 211
#define STOP_PUMP 100
#define PUMP_WAIT_NIGHT 210

uint8_t btnRun = 10;  //start stop buton pin
uint8_t btnNight = 2; // night start stop button pin
uint8_t wtrSen = 3;  // water sensor pin



bool buttonState = 0;
uint8_t push = 0;
uint8_t nightPush = 0;
bool nightButtonState = false;
bool isFull = false;
bool runFlag = 0;
bool nightFlag = 0;
uint16_t messageTmp = 0;
uint8_t LastMessage = 0;
uint8_t poolCount = 0;




SoftwareSerial rf(8, 9);



void setup()
{

	rf.begin(9600);
	Serial.begin(9600);

	pinMode(btnRun, INPUT);
	pinMode(btnNight, INPUT);

	pinMode(A0, OUTPUT);  // Water LED
	pinMode(A1, OUTPUT);  // Led night 
	pinMode(A2, OUTPUT);  // Led RUN
	pinMode(A3, OUTPUT);  // led Power
	pinMode(wtrSen, INPUT);
	digitalWrite(A3, HIGH);

	#ifdef ENABLE_DEBUG;
	Serial.println(EEPROM.read(9));
	Serial.println(isFull);
	Serial.println(EEPROM.read(11));
	#endif // !ENABLE_DEBUG;
	Serial.println(F("Starting...."));

	delay(2000);



}

void loop()
{
	//Serial.print("\nbuttonState1: "); Serial.println(buttonState);
	



	MessageTempOrganizer();

	LastMessageOrganizer();


	//poolCount = EEPROM.read(13);
	//Serial.print(F("- Pool Count: ")); Serial.println(poolCount);
	


	isFull = digitalRead(wtrSen);

	//Serial.print("isFull: "); Serial.println(isFull);
	if ((isFull)) {
		Serial.println(F("POOL_IS_FULL\n"));
		waterLevel();
	/*	#ifndef ENABLE_DEBUG
		EEPROM.write(13, (poolCount + 1));
		#endif // !ENABLE_DEBUG*/



	}
	else {
		digitalWrite(A0, LOW);
	}


	buttonState = digitalRead(btnRun);
	//Serial.print("buttonState 3: "); Serial.println(buttonState);
	if (buttonState) {
		//Serial.println("Run STAGE :1 ");

		RunNow();
	}

	nightButtonState = digitalRead(btnNight);
	if (nightButtonState) {
		RunNight();
	}



}


/*
LastMessageOrganizer()
Organize LED States 

*/
void LastMessageOrganizer() {

	LastMessage = EEPROM.read(3);
	Serial.print(millis()); Serial.print(F(" - LastMessage: ")); Serial.println(LastMessage);
	if (LastMessage == PUMP_RUN_NOW) {
		#ifdef ENABLE_DEBUG
		Serial.print(F("LastMessage: ")); Serial.println(LastMessage);
		Serial.println(F("RED LED HIGH"));
		#endif // ENABLE_DEBUG
		digitalWrite(A2, HIGH);
		delay(250);
		digitalWrite(A2, LOW);
		//LastMessage = 0;
		delay(250);
		Serial.println(F("PUMP_RUNING_NOW\n"));

	}
	else {
		#ifdef ENABLE_DEBUG
		Serial.print(F("LastMessage: ")); Serial.println(LastMessage);
		Serial.println(F("RED LED LOW"));
		#endif // ENABLE_DEBUG


		digitalWrite(A2, LOW);

	}

	if (LastMessage == PUMP_RUN_NIGHT) {
		#ifdef ENABLE_DEBUG
		Serial.print(F("LastMessage: ")); Serial.println(LastMessage);
		Serial.println(F("GREEN LED HIGH"));
		#endif // ENABLE_DEBUG
		digitalWrite(A1, HIGH);
		delay(250);
		digitalWrite(A1, LOW);
		//LastMessage = 0;
		delay(250);
		Serial.println(F("PUMP_IS_RUN_FOR_NIGHT\n"));
	}
	else {
		#ifdef ENABLE_DEBUG
		Serial.print(F("LastMessage: ")); Serial.println(LastMessage);
		Serial.println(F(" GREEN LED LOW"));
		#endif // ENABLE_DEBUG

		digitalWrite(A1, LOW);
	}


	if (LastMessage == STOP_PUMP) {
		#ifdef ENABLE_DEBUG
		Serial.print(F("LastMessage: ")); Serial.println(LastMessage);
		Serial.println(F("Run and Night LEDs is LOW"));
		#endif // ENABLE_DEBUG
		digitalWrite(A2, LOW);
		digitalWrite(A1, LOW);

	}

	if (LastMessage == PUMP_WAIT_NIGHT) {
		#ifdef ENABLE_DEBUG
		Serial.print(F("LastMessage: ")); Serial.println(LastMessage);
		Serial.println(F("GREEN LED HIGH"));
		#endif // ENABLE_DEBUG
		delay(250);
		digitalWrite(A1, HIGH);
		delay(250);
		Serial.println(F("PUMP_IS_WAIT_NIGHT\n"));
	}
	else {
		#ifdef ENABLE_DEBUG
		Serial.print(F("LastMessage: ")); Serial.println(LastMessage);
		Serial.println(F(" GREEN LED LOW"));
		#endif // ENABLE_DEBUG

		digitalWrite(A1, LOW);
	}

}


/*
MessageTempOrganizer()
Receive message from pump Station 
add to EEPROM,

*/
void MessageTempOrganizer() {

	messageTmp = CheckMessage();


	switch (messageTmp) {
	case PUMP_RUN_NIGHT:
		EEPROM.write(3, PUMP_RUN_NIGHT);
		#ifndef ENABLE_DEBUG
		Serial.print(F("- MessageTemp:PUMP_RUN_NIGHT: ")); Serial.println(EEPROM.read(3));
		#endif // !ENABLE_DEBUG
		break;
	case STOP_PUMP:
		EEPROM.write(3, STOP_PUMP);
		#ifndef ENABLE_DEBUG
		Serial.print(F("- MessageTemp:STOP_PUMP: ")); Serial.println(EEPROM.read(3));
		#endif // !ENABLE_DEBUG
		break;
	case PUMP_RUN_NOW:
		EEPROM.write(3, PUMP_RUN_NOW);
		#ifndef ENABLE_DEBUG
		Serial.print(F("- MessageTemp:PUMP_RUN_NOW ")); Serial.println(EEPROM.read(3));
		#endif // !ENABLE_DEBUG
		break;
	case PUMP_WAIT_NIGHT:
		EEPROM.write(3, PUMP_WAIT_NIGHT);
		#ifndef ENABLE_DEBUG
		Serial.print(F("- MessageTemp:PUMP_WAIT_NIGHT ")); Serial.println(EEPROM.read(3));
		#endif // !ENABLE_DEBUG
		break;
	default:
		break;
	}

}



/*
SendCommand();
Sending Commands to Pump
*/

void SendCommand(uint16_t status) {
	rf.write(status);
	/*	switch (status)
	{
	case RUN_NOW:
	//	EEPROM.write(2, RUN_NOW);
	break;
	case RUN_NIGHT:
	//EEPROM.write(2, RUN_NIGHT);
	break;
	case STOP:
	//EEPROM.write(2, STOP);
	break;
	default:
	break;
	}*/

	#ifdef ENABLE_DEBUG;
	Serial.print(F("command : ")); Serial.println(status); //ENABLE_DEBUG;
	Serial.println(F(""));
	#endif // ENABLE_DEBUG;
	rf.flush();
	Serial.flush();
}


/*
CheckMessage()
receiving  Messages  from pump

*/
uint16_t CheckMessage() {
	int msg = 0;
	if (rf.available() >= 1) {
		msg = rf.read();
		return msg;
	}
	else {
		return 0;
	}
	rf.flush();
}


/*
waterLevel()
pool water Control

*/

void waterLevel() {

	delay(1000);

	#ifdef ENABLE_DEBUG;
	Serial.println(F("=====Water level==========="));//ENABLE_DEBUG;
	#endif // !ENABLE_DEBUG;


	digitalWrite(A0, HIGH);
	if (LastMessage != STOP_PUMP) {
		for (uint8_t i = 0; i < 2; i++) {
			SendCommand(STOP);
			delay(250);
		}
	}


	#ifdef ENABLE_DEBUG;
	Serial.print(F("= IN night: ")); Serial.println(nightFlag);//ENABLE_DEBUG;
	Serial.print(F("= IN RUN: ")); Serial.println(runFlag);//ENABLE_DEBUG;
	#endif // !ENABLE_DEBUG;


	uint8_t tmpRun = EEPROM.read(11);
	if (tmpRun) {
		#ifdef ENABLE_DEBUG;
		Serial.println(F("= push mod 2")); // ENABLE_DEBUG;
		#endif // !ENABLE_DEBUG;


		EEPROM.write(11, 0);
		digitalWrite(A2, LOW); //run
	}

	uint8_t tmpNight = EEPROM.read(9);
	if (tmpNight) {
		#ifdef ENABLE_DEBUG;
		Serial.println(F("= night push mod 2")); // ENABLE_DEBUG;
		#endif // !ENABLE_DEBUG;

		EEPROM.write(9, 0);
		digitalWrite(A2, LOW); // night

	}
	#ifdef ENABLE_DEBUG;

	Serial.print(F("= Exit RUN: ")); Serial.println(runFlag); //ENABLE_DEBUG;
	Serial.print(F("= Exit night: ")); Serial.println(nightFlag); //ENABLE_DEBUG;
	Serial.println(F("=============================\n")); //ENABLE_DEBUG;
	#endif // !ENABLE_DEBUG;


}
/*

RunNow()
Run pump

*/
void RunNow() {

	nightPush = EEPROM.read(9);
	nightFlag = (nightPush % 2);

	delay(300);
	digitalWrite(A2, HIGH);

	//Serial.println("RUN STAGE: 2 ");
	if ((!(nightFlag)) && (!(isFull))) {
		//Serial.println("RUN STAGE: 3 ");
		#ifdef ENABLE_DEBUG;
		Serial.println(F("##### BUTTON RUN #####"));//ENABLE_DEBUG;
		Serial.print(F("\n# buttonState :")); Serial.println(buttonState);//ENABLE_DEBUG;
		#endif // !ENABLE_DEBUG;
																		  //delay(300);
		push = EEPROM.read(11);
		//Serial.print(F("RUN()-push 1 is : ")); Serial.println(push);
		if (push >= 1) {
			//Serial.print(F("RUN()-push is >1 "));
			push = 0;
			EEPROM.write(11, push);
		}
		else {
			//Serial.print(F("RUN()-push is =0 "));
			push++;
			EEPROM.write(11, push);
		}

		runFlag = (push % 2);

		if (runFlag == 0) { //Close Pump
			Serial.println(F("CLOSE_PUMP"));
			digitalWrite(A2, LOW);

			for (uint8_t i = 0; i < 2; i++) {
				SendCommand(STOP);
				delay(250);
			}

			#ifdef ENABLE_DEBUG;
			Serial.print(F("# Push : ")); Serial.print(push); Serial.print(F(" MOD: ")); Serial.println(runFlag);//ENABLE_DEBUG;
			#endif // !ENABLE_DEBUG;

		}

		else { // Run Pump
			Serial.println(F("RUN_NOW"));
			for (uint8_t i = 0; i < 2; i++) {

				SendCommand(RUN_NOW);
				delay(250);
			}


			if (LastMessage == PUMP_RUN_NOW) {
				#ifdef ENABLE_DEBUG;
				Serial.print(F("MessageType ")); Serial.print(messageTmp);
				#endif // !ENABLE_DEBUG;
				digitalWrite(A2, HIGH);
			}

			#ifdef ENABLE_DEBUG;
			Serial.print(F("# Push : ")); Serial.print(push); Serial.print(F(" MOD: ")); Serial.println(runFlag);//ENABLE_DEBUG;
			#endif // !ENABLE_DEBUG;

		}
		#ifdef ENABLE_DEBUG;
		Serial.println(F("#########################\n"));//ENABLE_DEBUG;
		#endif // !ENABLE_DEBUG;

	}


}


/*
RunNight()
Run Pump for night

*/
void RunNight() {
	push = EEPROM.read(11);
	runFlag = (push % 2);
	delay(300);
	digitalWrite(A1, HIGH);
	//Serial.print("NightRun STAGE :1 runFlag: "); Serial.println(runFlag);
	if ((!(runFlag)) && (!(isFull))) {
		//Serial.println("NightRun STAGE :2- RunFlag = 0, isFull=1 ");
		#ifdef ENABLE_DEBUG;
		Serial.println(F("\n******night BUTTON*******")); //ENABLE_DEBUG;
		Serial.print(F("\n* nightButtonState :")); Serial.println(nightButtonState); //ENABLE_DEBUG;
		#endif // !ENABLE_DEBUG; 

		delay(300);
		nightPush = EEPROM.read(9);

		if (nightPush >= 1) {
			nightPush = 0;
			EEPROM.write(9, nightPush);
		}
		else {

			nightPush++;
			EEPROM.write(9, nightPush);
		}


		nightFlag = (nightPush % 2);

		if (nightFlag == 0) {
			digitalWrite(A1, LOW);
			//Serial.print("NightRun STAGE :3 CLOSE-nightFlag: "); Serial.println(nightFlag);
			Serial.println(F("CLOSE_PUMP"));
			for (uint8_t i = 0; i < 2; i++) {
				SendCommand(STOP);
				delay(250);
			}





			#ifdef ENABLE_DEBUG;
			//Serial.println((nightPush % 2));
			Serial.print(F("* nightPush : ")); Serial.print(nightPush); Serial.print(F(" MOD : ")); Serial.println(nightFlag); //ENABLE_DEBUG;
			#endif // !ENABLE_DEBUG;


		}
		else {

			for (uint8_t i = 0; i < 2; i++) {

				SendCommand(RUN_NIGHT);
				delay(250);
			}
			Serial.println(F("RUN_PUMP_FOR_NIGHT"));
			//Serial.print("NightRun STAGE :3 OPEN-nightFlag: "); Serial.println(nightFlag);
			//messageTmp = CheckMessage();


			if ((messageTmp == RUN_NIGHT)) {
				digitalWrite(A1, HIGH);

			}

			#ifdef ENABLE_DEBUG;
			Serial.print(F("* nightPush : ")); Serial.print(nightPush); Serial.print(F(" MOD : ")); Serial.println(nightFlag); //ENABLE_DEBUG;
			#endif // !ENABLE_DEBUG;


		}
		#ifdef ENABLE_DEBUG;
		Serial.println(F("*************************\n")); //ENABLE_DEBUG;
		#endif // ENABLE_DEBUG;



	}




}


