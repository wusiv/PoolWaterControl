/*
* Name:		PumpRX.ino
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



#include <Wire.h>
#include <RTClib.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>
#define ENABLE_DEBUG
#define RUN_NOW		11
#define RUN_NIGHT	21
#define STOP	10
#define PUMP_RUN_NOW 111
#define PUMP_RUN_NIGHT 211

//SoftwareSerial rf(9, 10); // TX -RX Port
#ifdef ENABLE_DEBUG
SoftwareSerial rf(8, 9); // TX -RX Port
#else
SoftwareSerial rf(9, 10); // TX -RX Port
#endif // ENABLE_DEBUG


RTC_DS1307 RTC;

uint8_t devRelayAlter = 8; //alternate relay port
uint8_t devRelay = 7;


uint8_t startHour = 23; // pump start hour 0-23 (22:00-06:00 electric price per KW is  %50 off-price  for my country )
uint8_t startMin = 10; // pump start Minute 0-59 

/*Pool full time  */
uint8_t workHour = 3; // Working Time (Hour)
uint8_t workMin = 30; // Working Time (Minute) (1-60)

unsigned long milStart; // start millis
unsigned long taskStopTime; // 
unsigned long taskDuration; //

//data&time vars.

uint8_t hourNow;
uint8_t minNow;
uint8_t dayNow;
uint8_t monthNow;
uint16_t yearNow;
uint8_t secNow;


// date time vars end



uint8_t taskType = 0; // task type RUN_NOW= Run now, RUN_NIGHT =RunNight
uint8_t pumpStatus = 0; // 111=Pump is Runing PUMP_RUN_NIGHT=Pump is runing for night
uint8_t commandType = 0;
uint16_t value = 0;





void setup() {

	Serial.begin(9600);
	rf.begin(9600);

	/*

		Wire.begin();
		digitalWrite(devRelay, HIGH); //relay close (for No(normally open) connection )
		pinMode(devRelay, OUTPUT);
		RTC.begin();
		taskDuration = (calcSec(workHour, workMin) * 1000); // Working Time (Hour,Minute) to millis
		//RTC.adjust(DateTime(2016, 07, 16, 21, 17, 40));

	*/


}

void loop() {



	value = CheckMessage();
	pumpStatus = EEPROM.read(20);
	taskType = EEPROM.read(24);



	#ifdef ENABLE_DEBUG
		Serial.print("Pump Status : "); Serial.println(pumpStatus);
		Serial.print("Task Type : "); Serial.println(taskType);
	#endif // ENABLE_DEBUG



	#ifdef ENABLE_DEBUG
		Serial.print("Value : "); Serial.println(value);
	#endif // ENABLE_DEBUG



	/*




	   DateTime now = RTC.now();

	   hourNow = now.hour();
	   minNow = now.minute();
	   dayNow = now.day();
	   monthNow = now.month();
	   yearNow = now.year();
	   secNow = now.second();

	   Serial.println("#### I N F O #########################");
	   Serial.print("# lastMillis: ");
	   Serial.println(EEPROMReadlong(1));
	   Serial.print("# TaskStopTime: ");
	   Serial.println(EEPROMReadlong(5));
	   Serial.print("# endTime: ");
	   Serial.print(EEPROM.read(10)); Serial.print("-"); Serial.print(EEPROM.read(12)); Serial.print("-"); Serial.print(EEPROM.read(14));
	   Serial.print(" / "); Serial.print(EEPROM.read(16)); Serial.print(":"); Serial.println(EEPROM.read(18));
	   Serial.println("######################################");
	   Serial.println("******* D E B U G *************");
	   Serial.print("* Date: ");
	   Serial.print(dayNow, DEC);
	   Serial.print("."); Serial.print(monthNow, DEC);
	   Serial.print("."); Serial.print(yearNow, DEC);
	   Serial.print(" / ");
	   Serial.print(hourNow, DEC);
	   Serial.print(":"); Serial.print(minNow, DEC);
	   Serial.print(":"); Serial.println(secNow , DEC);
	   Serial.print("* taskDuration: ");
	   Serial.println(taskDuration); /// ENABLE_DEBUG text
	   Serial.print("* Current Millis: ");
	   Serial.println(millis());
	   Serial.println("******************************");


	   */

	
	if ((value == STOP) || (taskType == STOP)) {

		EEPROM.write(24, STOP);
		StopPump();
		#ifdef ENABLE_DEBUG
				Serial.println(" - Pump is OFF");
		#endif // ENABLE_DEBUG

	}


	if ((value == RUN_NOW) || (taskType == RUN_NOW)) {

		EEPROM.write(24, RUN_NOW);
		RunNow();
		#ifdef ENABLE_DEBUG
				Serial.println(" - Pump is ON");
		#endif // ENABLE_DEBUG
	}



if ((value == RUN_NIGHT) || (taskType == RUN_NIGHT)) {

		EEPROM.write(24, RUN_NIGHT);
		
		
		if ((((hourNow == 22) && (minNow >= 10)) || (hourNow == 0)) || (((hourNow >= 1) && (hourNow <= 5) && ((hourNow == 5) && (minNow <= 45))))) {
			NightRun();
		}


		delay(5000);

		hourNow = 22;
		minNow = 11;
		#ifdef ENABLE_DEBUG
				Serial.println(" - Pump is waiting for Night Run");
		#endif // ENABLE_DEBUG
	}


}

uint16_t CheckMessage() {

	if (rf.available()) {
		uint16_t value = rf.read();
		rf.flush();
		return value;
	}
	return 0;
}

void RunNow() {

	if ((pumpStatus != PUMP_RUN_NIGHT)) {

		digitalWrite(devRelay, LOW);
		for (uint8_t i = 0; i < 4; i++) {
			SendMessage(RUN_NOW);
			delay(500);
		}


#ifdef ENABLE_DEBUG
		Serial.println("RunNow()-if()");
		Serial.print("Rec: "); Serial.println(value);
		Serial.print("PUMP_RUN_NOW"); Serial.println(PUMP_RUN_NOW);
#endif // ENABLE_DEBUG

	}
	else {
#ifdef ENABLE_DEBUG
		Serial.print("Rec: "); Serial.println(value);
		Serial.println("RunNow()-Else{} ");
#endif // ENABLE_DEBUG

	}



}


void NightRun() {

#ifdef ENABLE_DEBUG

	for (uint8_t i = 0; i < 2; i++) {
		SendMessage(RUN_NIGHT);
		delay(500);
	}

	for (;;) {

		//delay(1000);
		SendMessage(PUMP_RUN_NIGHT);
		delay(1000);
		
		Serial.println("PUMP WORK FOR NIGHT....");
		value = CheckMessage();
		if (value == 10) {
			StopPump();
			break;
		}

	}
	Serial.print("Rec: "); Serial.println(value);
	Serial.print("RUN_NIGHT: "); Serial.println(RUN_NIGHT);
#endif // ENABLE_DEBUG;


	/*

		if ((hourNow == startHour)||(pumpStatus==PUMP_RUN_NIGHT)) {
		milStart = (millis()); //First Run (millisecond)
		taskStopTime = milStart + taskDuration; // task End time calculation

		if ((minNow == startMin)||(pumpStatus==PUMP_RUN_NIGHT) ) {
			for (;;) {
			//	delay(1000);
				Serial.println("========== D E B U G =================");
				Serial.print("= MilStart: ");
				Serial.println(milStart);
				Serial.print("= Current Millis: ");
				Serial.println(millis());
				Serial.print("= taskStopTime: ");
				Serial.println(taskStopTime);

				if ((millis()) < taskStopTime) {
					Serial.println("= Pump is Running...");
					digitalWrite(devRelay, LOW);
					EEPROMWritelong(1, millis());
					EEPROMWritelong(5, taskStopTime);
					EEPROM.write(10, now.day());
					EEPROM.write(12, now.month());
					EEPROM.write(14, (now.year() - 2000));
					EEPROM.write(16, now.hour());
					EEPROM.write(18, now.minute());
					SendMessage(PUMP_RUN_NIGHT);
					}

				else {
					break;
					digitalWrite(devRelay, HIGH);

					for (uint8_t i=0;i<5;i++) {

					SendMessage(100);
					delay(500);
					}
				}
			}
		}
		else {
			digitalWrite(devRelay, HIGH);

			for (uint8_t i=0;i<5;i++) {
				SendMessage(100);
				delay(500);
			}
			Serial.println("* Pump does NOT Work...");
		}

	*/
}


/*
This function will write a 4 byte (32bit) long to the eeprom at
the specified address to address + 3.
http://playground.arduino.cc/Code/EEPROMReadWriteLong
*/
void EEPROMWritelong(int address, long value)
{
	//Decomposition from a long to 4 bytes by using bitshift.
	//One = Most significant -> Four = Least significant byte
	byte four = (value & 0xFF);
	byte three = ((value >> 8) & 0xFF);
	byte two = ((value >> 16) & 0xFF);
	byte one = ((value >> 24) & 0xFF);
	//Write the 4 bytes into the eeprom memory.
	EEPROM.write(address, four);
	EEPROM.write(address + 1, three);
	EEPROM.write(address + 2, two);
	EEPROM.write(address + 3, one);
}
long EEPROMReadlong(long address)
{
	//Read the 4 bytes from the eeprom memory.
	long four = EEPROM.read(address);
	long three = EEPROM.read(address + 1);
	long two = EEPROM.read(address + 2);
	long one = EEPROM.read(address + 3);
	//Return the recomposed long by using bitshift.
	return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}


/*
 SendMessage()

*/
void SendMessage(uint8_t msg) {

	switch (msg)
	{
	case RUN_NOW:
		rf.write(RUN_NOW);
		//pumpStatus = PUMP_RUN_NOW;
#ifdef ENABLE_DEBUG
		EEPROM.write(20, PUMP_RUN_NOW);
		EEPROM.write(24, RUN_NOW);
#endif // ENABLE_DEBUG

		rf.flush();
		break;
	case RUN_NIGHT:

		rf.write(RUN_NIGHT);
#ifdef ENABLE_DEBUG

		EEPROM.write(24, RUN_NIGHT); //task type
#endif // ENABLE_DEBUG
		rf.flush();
		break;
	case PUMP_RUN_NIGHT:
		rf.write(PUMP_RUN_NIGHT); // pump is now runing for night task
		//pumpStatus = PUMP_RUN_NIGHT;
#ifdef ENABLE_DEBUG
		EEPROM.write(20, PUMP_RUN_NIGHT);// pump status
		EEPROM.write(24, RUN_NIGHT); //task type
#endif // ENABLE_DEBUG

		rf.flush();
		break;
	case PUMP_RUN_NOW:
		rf.write(PUMP_RUN_NOW);
		//pumpStatus = PUMP_RUN_NOW;
#ifdef ENABLE_DEBUG
		EEPROM.write(20, PUMP_RUN_NOW);
		EEPROM.write(24, RUN_NOW);
#endif // ENABLE_DEBUG

		rf.flush();
		break;
	case 100:
		rf.write(100);
#ifdef ENABLE_DEBUG
		EEPROM.write(20, 100);
		EEPROM.write(24, 10);
#endif // ENABLE_DEBUG
		break;

	case STOP:
		rf.write(10);
#ifdef ENABLE_DEBUG
		EEPROM.write(20, 100);
		EEPROM.write(24, 10);
#endif // ENABLE_DEBUG
		break;
	default:
		break;
	}



}

/**
Calc second
@INPUT: Hour,Minute (integer)
@OUT: second
*/
long calcSec(uint8_t hh, uint8_t mm) {
	int m;
	int h;
	if ((hh > 0) && (hh <= 24)) {
		h = hh * 3600;
	}
	else {
		h = 0;
	}
	if ((mm >= 0) && (mm <= 59)) {
		m = mm * 60;
	}
	else {
		m = 0;
	}

	h = hh * 3600;
	m = mm * 60;
	return (h + m);
}

void StopPump() {

	digitalWrite(devRelay, HIGH);

	for (uint8_t i = 0; i < 5; i++) {
		SendMessage(100);
		delay(500);
	}

	Serial.println("* Pump does NOT Work...");

}