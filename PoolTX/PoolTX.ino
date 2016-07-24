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
*
*
*
*
*/




#include <RCSwitch.h>
/* pull down resistor
*  Pull-Down Resistor. 
*/
int push = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button
int pump = 0;
int button = 10;
int ledGreen = 13;
RCSwitch sw = RCSwitch();

void setup()
{
	Serial.begin(115200);
	pinMode(ledGreen, OUTPUT);
	pinMode(button, INPUT);
	sw.enableTransmit(2);
}

void loop()
{

	buttonState = digitalRead(button);
	Serial.println(F("*******D E B U G*******"));
	if (buttonState != lastButtonState) {  

		if (buttonState == HIGH) {
			delay(500);// buttondan dolayý ark olusumunu engellemek icin bekleme

			if (push >= 1) {
				push = 0;
			}
			else {
				push++;
			}
		}

	}

	if ((push % 2) == 0) {  // Pump close (0 mod 2=0)
		digitalWrite(ledGreen, LOW);
		Serial.println(F("* Pump Close"));
		pump = 0;
	}
	else {

		digitalWrite(ledGreen, HIGH);
		Serial.println(F("* Pump Open"));
		pump = 1;
	}
	Serial.println(F("*********************\n"));
	sw.send(pump, 24);
}
