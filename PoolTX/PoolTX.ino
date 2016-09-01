
/*
* Name:		PoolTX.ino
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
*/


/* pull down resistor
*  Pull-Down Resistor. 
*/

#include <SoftwareSerial.h>
//#define DEBUG;


uint8_t btnRun = 10;  //start stop buton
uint8_t btnNight = 2; // night start stop button
uint8_t wtrSen = 3;
bool buttonState = false;
uint8_t push = 0;
uint8_t nightPush = 0;
bool nightButtonState = false;
bool isFull = false;
bool runFlag = 0;
bool nightFlag = 0;

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
	#ifdef DEBUG
		Serial.println(nightPush);
		Serial.println(isFull);
		Serial.println(push);
	#endif // !DEBUG
	Serial.println(F("Starting...."));
}

void loop()
{

	checkMessage();
	isFull = digitalRead(wtrSen);
	if (isFull) {
		waterLevel();
	}
	else {
		digitalWrite(A0, LOW);
		runNow();
		runNight();
	}
}
void sendCommand(char* status) {
	rf.write(status);
	#ifdef DEBUG
		Serial.print(F("command : ")); Serial.println(status); //debug
		Serial.println("");
	#endif // DEBUG
	rf.flush();
	Serial.flush();
}

uint8_t checkMessage() {
	int msg = 0;
	if (rf.available()) {
		msg = rf.read();
		return msg;
	}
	else {
		return 0;
	}
	rf.flush();
}

void waterLevel() {
	delay(1000);
	#ifdef DEBUG
		Serial.println(F("=====Water level==========="));//Debug
	#endif // !DEBUG


	sendCommand("10");
	digitalWrite(A0, HIGH);
	#ifdef DEBUG
		Serial.print("= IN night: "); Serial.println(night);//Debug
		Serial.print("= IN RUN: "); Serial.println(runFlag);//Debug
	#endif // !DEBUG



	if (runFlag) {
		#ifdef DEBUG
				Serial.println("= push mod 2"); // Debug
		#endif // !DEBUG


		runFlag = 0;
		digitalWrite(A2, LOW); //run
	}
	if (nightFlag) {
		#ifdef DEBUG
				Serial.println("= night push mod 2"); // Debug
		#endif // !DEBUG

		nightFlag = 0;
		digitalWrite(A1, LOW); // night

	}
		#ifdef DEBUG

			Serial.print("= Exit RUN: "); Serial.println(runFlag); //Debug
			Serial.print("= Exit night: "); Serial.println(night); //Debug
			Serial.println("=============================\n"); //debug
		#endif // !DEBUG
}

void runNow() {
	buttonState = digitalRead(btnRun);
	if ((buttonState) && (!(nightFlag)) && (!(isFull))) {
		#ifdef DEBUG
				Serial.println("##### BUTTON RUN #####");//Debug
				Serial.print("\n# buttonState :"); Serial.println(buttonState);//Debug
		#endif // !DEBUG
		delay(300);
		if (push >= 1) {
			push = 0;
		}
		else {

			push++;
		}

		runFlag = (push % 2);
		if (runFlag == 0) {
			digitalWrite(A2, LOW);

			sendCommand("10");
			#ifdef DEBUG
						Serial.print("# Push : "); Serial.print(push); Serial.print(" MOD: "); Serial.println(runFlag);//Debug
			#endif // !DEBUG

		}

		else {
			digitalWrite(A2, HIGH);
			#ifdef DEBUG
						Serial.print("# Push : "); Serial.print(push); Serial.print(" MOD: "); Serial.println(runFlag);//Debug
			#endif // !DEBUG

			sendCommand("11");


		}
			#ifdef DEBUG


					Serial.println("#########################\n");//Debug
			#endif // !DEBUG


	}

}
void runNight() {


	nightButtonState = digitalRead(btnNight);
	if ((nightButtonState) && (!(runFlag)) && (!(isFull))) {
		#ifdef DEBUG
				Serial.println("******night BUTTON*******"); //Debug
				Serial.print("\n* nightButtonState :"); Serial.println(nightButtonState); //Debug
		#endif // !DEBUG

		delay(300);
		if (nightPush >= 1) {
			nightPush = 0;
		}
		else {

			nightPush++;
		}
		nightFlag = (nightPush % 2);
		if (nightFlag == 0) {
			digitalWrite(A1, LOW);

			sendCommand("10");
			
			#ifdef DEBUG
						Serial.println((nightPush % 2));
						Serial.print("nightPush : "); Serial.print(nightPush); Serial.print(" MOD : "); Serial.println(night); //Debug
			#endif // !DEBUG


		}
		else {
			digitalWrite(A1, HIGH);
			sendCommand("21");
			#ifdef DEBUG
						Serial.print("* nightPush : "); Serial.print(nightPush); Serial.print(" MOD : "); Serial.println(night); //Debug
			#endif // !DEBUG


		}
		#ifdef DEBUG
				Serial.println("*************************\n"); //Debug
		#endif // DEBUG



	}



}


			
