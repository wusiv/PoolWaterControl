/*
* Name:		PumpRX.ino
* Created:	24/07/2016 14:46
* Author:	Yusuf KALSEN
* URL: http://github.com/wusiv

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
#define DEBUG;

SoftwareSerial rf(9, 10); // TX -RX Port
RTC_DS1307 RTC;

uint8_t devRelayAlter = 8; //alternate relay port
uint8_t devRelay = 7;
uint8_t startHour = 23; // pump start hour 0-23
uint8_t startMin = 10; // pump start Minute 0-59
uint8_t workHour = 3; // Working Time (Hour)
uint8_t workMin = 30; // Working Time (Minute) (1-60)
unsigned long milStart; // start millis
unsigned long taskStopTime; // 
unsigned long taskDuration; //
uint8_t hourNow;   //data&time vars.
uint8_t minNow;
uint8_t dayNow;
uint8_t monthNow;
uint16_t yearNow;
uint8_t secNow;

uint8_t taskType = 0; // task type 11= Run now, 21 =RunNight
uint8_t pumpStatus = 0; // 111=Pump is Runing 211=Pump is runing for night

uint8_t value = 0;





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
	
	

	delay(1000);
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
	Serial.println(taskDuration); /// debug text
	Serial.print("* Current Millis: ");
	Serial.println(millis());
	Serial.println("******************************");


	*/
	

	pumpStatus = EEPROM.read(20);
	taskType = EEPROM.read(24);

	#ifdef DEBUG
		Serial.print("Pump Status : "); Serial.println(pumpStatus);
		Serial.print("Task Type : "); Serial.println(taskType);
	#endif // DEBUG


	value = CheckMessage();
	#ifdef DEBUG
		Serial.print("Value : "); Serial.println(value);
	#endif // DEBUG


	switch (value)
	{
		case 10: //STOP 
			#ifdef DEBUG
				Serial.println(" - Pump is OFF");
			#endif // DEBUG

			break;

		case 11:   // RUN NOW
			#ifdef DEBUG
				Serial.println(" - Pump is ON");
			#endif // DEBUG
			break;

		case 21:  // NIGHT RUN

			#ifdef DEBUG
				Serial.println(" - Pump is waiting for Night Run");
			#endif // DEBUG

			if ((((hourNow == 22) && (minNow >= 10)) || (hourNow == 0)) || ((hourNow >= 1) || ((hourNow <= 5) && (minNow <= 45)))) {
				NightRun();
			}
			break;


		default:
			break;
	}
	}

int CheckMessage() {
	
	if (rf.available()) {
		int value = rf.read();
		rf.flush();
		return value;
	}
	return 0;
}

void RunNow() {
	
	 


}


void NightRun() {



/*

	if ((hourNow == startHour)||(pumpStatus==221)) {
	milStart = (millis()); //First Run (millisecond)
	taskStopTime = milStart + taskDuration; // task End time calculation

	if ((minNow == startMin)||(pumpStatus==221) ) {
		for (;;) {
			delay(1000);
			Serial.println("========== D E B U G =================");
			Serial.print("= MilStart: ");
			Serial.println(milStart);
			Serial.print("= Current Millis: ");
			Serial.println(millis());
			Serial.print("= taskStopTime: ");
			Serial.println(taskStopTime);
			SendMessage(211);
			if ((millis()) < taskStopTime) {
				Serial.println("= Pump Running...");
				digitalWrite(devRelay, LOW);
				EEPROMWritelong(1, millis());
				EEPROMWritelong(5, taskStopTime);
				EEPROM.write(10, now.day());
				EEPROM.write(12, now.month());
				EEPROM.write(14, (now.year() - 2000));
				EEPROM.write(16, now.hour());
				EEPROM.write(18, now.minute());
				EEPROM.write(20, 211 );
				EEPROM.write(24,21);
			}
			else {
				break;
				digitalWrite(devRelay, HIGH);
			}
		}
	}
	else {
		digitalWrite(devRelay, HIGH);
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



void SendMessage(uint8_t msg) {

	switch (msg)
	{
		
	case 211:
		rf.write(211); // pump is now runing for night task
		//pumpStatus = 211;
		#ifdef DEBUG
			EEPROM.write(20, 211);
			EEPROM.write(24, 21);
		#endif // DEBUG

		break;
	case 111:
		rf.write(111);
		//pumpStatus = 111;
		#ifdef DEBUG
			EEPROM.write(22, 111);
			EEPROM.write(24, 11);
		#endif // DEBUG
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
