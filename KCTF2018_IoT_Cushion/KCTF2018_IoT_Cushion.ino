/*
 Name:		KCTF2018_IoT_Cushion.ino
 Created:	2018-07-19 오전 4:44:43
 Author:	BetaMan
*/

#include <LiquidCrystal_I2C.h>
#include <DS1302.h>
#include "Strings.h"

// Rotary Encoder
#define DT 2
#define CLK 3
#define SW 4
// HardReset Pin
#define RST 5
// Buzzer
#define BZ 6
// RGB LED
#define R 8
#define G 9
#define B 10
// FSR
#define L_FSR 0
#define R_FSR 1

int8_t i = 0;
long j = 0;
uint8_t ScreenScroll = 0;
uint8_t ScreenCursor = 0;
uint8_t Screen = 0;
int8_t EncValue;
int oldA = HIGH;
int oldB = HIGH;
int8_t Hour = 0;
int8_t Min = 0;
int8_t Range_H = -1;
int8_t Range_M = -1;
int8_t Goal_H = -1;
int8_t Goal_M = -1;
uint8_t old_Min = 0;
uint8_t Study_H = 0;
uint8_t Study_M = 0;
uint8_t Count_StandUp = 0;

byte chairChar[8] = { B00000,B10000,B10000,B11111,B10001,B10001,B00000 };

LiquidCrystal_I2C LCD(0x27, 16, 2);
DS1302 RTC(11, 12, 13);

int getEncoderTurn(void)
{
	int result = 0;
	int newA = digitalRead(DT);
	int newB = digitalRead(CLK);
	if (newA != oldA || newB != oldB)
	{
		if (oldA == HIGH && newA == LOW)
		{
			result = (oldB * 2 - 1);
		}
	}
	oldA = newA;
	oldB = newB;
	return result;
}

void Screen_Reset() {ScreenScroll = 0; ScreenCursor = 0; LCD.clear();}

void(*resetFunc) (void) = 0;

uint8_t getTimeElement(uint8_t index) { return (uint8_t)StrToFloat(strSplit(RTC.getTimeStr(), ":", index)); }

void Start_Screen() {
	LCD.setCursor(0, 0); LCD.print("=Study  Cushion=");
	LCD.setCursor(0, 1); LCD.print("---2018  KCTF---");
	digitalWrite(G, HIGH); digitalWrite(B, HIGH);
	
	tone(BZ, 784, 125);	delay(125);
	tone(BZ, 1047, 125); delay(125);
	tone(BZ, 1175, 125); delay(125);
	tone(BZ, 880, 125); delay(125);
	tone(BZ, 1175, 125); delay(125);
	tone(BZ, 1319, 125); delay(125);
	tone(BZ, 1397, 1000); delay(1000);

	delay(1000);
	
	LCD.clear();
	digitalWrite(G, LOW); digitalWrite(B, LOW);
}

void startStudy_Screen() {
	LCD.setCursor(0, 0); LCD.print("Please sit down");
	LCD.setCursor(0, 1); LCD.print("the Cushion");
	while (Screen == 2) {
		if (analogRead(L_FSR) > 400 && analogRead(R_FSR) > 400) {
			tone(BZ, 784, 250); delay(250);
			tone(BZ, 988, 250); delay(250);
			tone(BZ, 1175, 250); delay(250);

			Screen = 3;
			Screen_Reset();
			studyTime_Measure();
		}
		if (digitalRead(SW) == 0) {
			tone(BZ, 784, 125); delay(125);
			Screen = 1;
			Screen_Reset();
			while (digitalRead(SW) == 0);
		}
	}
}

void studyTime_Measure() {
	Study_H = 0;
	Study_M = 0;
	Count_StandUp = 0;

	LCD.setCursor(0, 0); LCD.print("Goal:");
	LCD.setCursor(6, 0);
	if (Goal_H < 10) LCD.print("0" + String(Goal_H));
	else LCD.print(String(Goal_H));
	LCD.setCursor(8, 0); LCD.print(":");
	LCD.setCursor(9, 0);
	if (Goal_M < 10) LCD.print("0" + String(Goal_M));
	else LCD.print(String(Goal_M));
	LCD.setCursor(0, 1); LCD.print("Time:");
	LCD.setCursor(6, 1);
	if (Goal_H < 10) LCD.print("0" + String(Study_H));
	else LCD.print(String(Study_H));
	LCD.setCursor(8, 1); LCD.print(":");
	LCD.setCursor(9, 1);
	if (Goal_M < 10) LCD.print("0" + String(Study_M));
	else LCD.print(String(Study_M));
	LCD.setCursor(13, 1); LCD.write(1);

	old_Min = getTimeElement(2); Serial.println(old_Min);

	while (Screen == 3) {
		if (analogRead(L_FSR) > 200 && analogRead(R_FSR) > 200) {
			digitalWrite(G, HIGH); digitalWrite(R, LOW);
			LCD.setCursor(6, 1);
			if (Goal_H < 10) LCD.print("0" + String(Study_H));
			else LCD.print(String(Study_H));
			LCD.setCursor(8, 1); LCD.print(":");
			LCD.setCursor(9, 1);
			if (Goal_M < 10) LCD.print("0" + String(Study_M));
			else LCD.print(String(Study_M));

			LCD.setCursor(15, 1); LCD.print("O");
			Serial.println(String(old_Min)+ " " + String(getTimeElement(2)));
			if (!(old_Min == getTimeElement(2))) {
				old_Min = getTimeElement(2); Serial.println(old_Min);
				if (Study_M == 59) {
					Study_H += 1;
					Study_M = 0;
				}
				else {
					Study_M += 1;
				}
			}

			if ((getTimeElement(1) == Range_H) && (getTimeElement(2) == Range_M)) {
				Screen = 4;
				Screen_Reset();
				result_Screen();
			}
		}
		else {
			Count_StandUp += 1;
			LCD.setCursor(15, 1); LCD.print("X");
			digitalWrite(R, HIGH); digitalWrite(G, LOW);
			
			j = millis();
			while (!(analogRead(L_FSR) > 200 && analogRead(R_FSR) > 200)) {
				if (millis() - j > 10000) {
					tone(BZ, 2000);
				}
				if ((getTimeElement(1) == Range_H) && (getTimeElement(2) == Range_M)) {
					Screen = 4;
					Screen_Reset();
					result_Screen();
				}
			}
			noTone(BZ);
		}
	}
}

void result_Screen() {
	if ((Goal_H > Study_H) || ((Goal_H == Study_H) && (Goal_M > Study_M))) {
		LCD.setCursor(0, 0); LCD.print("Fail to achieve");
		digitalWrite(R, HIGH); digitalWrite(G, LOW);
		tone(BZ, 988, 125); delay(125);
		tone(BZ, 880, 125); delay(125);
		tone(BZ, 831, 125); delay(125);
		tone(BZ, 880, 500); delay(500);
		tone(BZ, 659, 250); delay(250);
		tone(BZ, 698, 500); delay(500);
		tone(BZ, 880, 250); delay(250);
		tone(BZ, 659, 500); delay(500);
		tone(BZ, 523, 500); delay(500);
		tone(BZ, 494, 500); delay(500);
	}
	else {
		LCD.setCursor(0, 0); LCD.print("Congratulations!");
		digitalWrite(G, HIGH); digitalWrite(R, LOW);
		tone(BZ, 587, 125); delay(125);
		tone(BZ, 659, 125); delay(125);
		tone(BZ, 740, 125); delay(125);
		tone(BZ, 784, 250); delay(250);
		tone(BZ, 587, 250); delay(250);
		tone(BZ, 784, 125); delay(125);
		tone(BZ, 740, 125); delay(125);
		tone(BZ, 784, 125); delay(125);
		tone(BZ, 880, 250); delay(250);
		tone(BZ, 659, 250); delay(250);
	}
	while (Screen == 4) {
		EncValue = getEncoderTurn(); Serial.println(String(ScreenScroll) + " " + String(ScreenCursor));
		if (EncValue == 1) {
			if (ScreenScroll == 0) ScreenScroll = 1;
			else if (ScreenScroll == 1) ScreenScroll = 2;
			LCD.clear();
		}
		else if (EncValue == -1) {
			if (ScreenScroll == 2) ScreenScroll = 1;
			else if (ScreenScroll == 1) ScreenScroll = 0;
			LCD.clear();
		}

		if (ScreenScroll == 0) {
			if ((Goal_H > Study_H) || ((Goal_H == Study_H) && (Goal_M > Study_M))) {
				LCD.setCursor(0, 0); LCD.print("Fail to achieve");
			}
			else {
				LCD.setCursor(0, 0); LCD.print("Congratulations!");
			}

			LCD.setCursor(0, 1); LCD.print("Score:");
			if ((Goal_H > Study_H) || ((Goal_H == Study_H) && (Goal_M > Study_M))) {
				LCD.setCursor(7, 1); LCD.print("0");
			}
			else {
				LCD.setCursor(7, 1); LCD.print(String((100 - Count_StandUp * 2)));
			}
		}
		else if (ScreenScroll == 1) {
			LCD.setCursor(0, 0); LCD.print("Score:");
			if ((Goal_H > Study_H) || ((Goal_H == Study_H) && (Goal_M > Study_M))) {
				LCD.setCursor(7, 0); LCD.print("0");
			}
			else {
				LCD.setCursor(7, 0); LCD.print(String((100 - Count_StandUp * 2)));
			}

			LCD.setCursor(0, 1); LCD.print("Time:");
			LCD.setCursor(6, 1);
			if (Study_H < 10) LCD.print("0" + String(Study_H));
			else LCD.print(String(Study_H));
			LCD.setCursor(8, 1); LCD.print(":");
			LCD.setCursor(9, 1);
			if (Study_H < 10) LCD.print("0" + String(Study_M));
			else LCD.print(String(Study_M));
		}
		else {
			LCD.setCursor(0, 0); LCD.print("Time:");
			LCD.setCursor(6, 0);
			if (Study_H < 10) LCD.print("0" + String(Study_H));
			else LCD.print(String(Study_H));
			LCD.setCursor(8, 0); LCD.print(":");
			LCD.setCursor(9, 0);
			if (Study_H < 10) LCD.print("0" + String(Study_M));
			else LCD.print(String(Study_M));

			LCD.setCursor(0, 1); LCD.print("StandUp:");
			LCD.setCursor(9, 1); LCD.print(String(Count_StandUp));
		}
		
		if (digitalRead(SW) == 0) {
			tone(BZ, 784, 125); delay(125);
			resetFunc();
		}
	}
}

void setTime_Screen() {
	Serial.println("setTime");
	while (Screen == 5) {
		// setTime Menu
		if (ScreenScroll == 0) {
			LCD.setCursor(0, 0); LCD.print("> Set Range");
			LCD.setCursor(0, 1); LCD.print("> Set Goal");
		}
		else {
			LCD.setCursor(0, 0); LCD.print("> Set Goal");
			LCD.setCursor(0, 1); LCD.print("> Back");
		}
		if (ScreenCursor == 0) {
			LCD.setCursor(15, 0); LCD.print("<");
		}
		else {
			LCD.setCursor(15, 1); LCD.print("<");
		}

		EncValue = getEncoderTurn(); Serial.println(String(ScreenScroll) + " " + String(ScreenCursor));
		if (EncValue == 1) {
			if (ScreenScroll == 0) {
				if (ScreenCursor == 0) {
					ScreenCursor = 1;
				}
				else {
					ScreenScroll = 1;
				}
			}
			else {
				if (ScreenCursor == 0) {
					ScreenCursor = 1;
				}
			}
			LCD.clear();
		}
		else if (EncValue == -1) {
			if (ScreenScroll == 0) {
				if (ScreenCursor == 1) {
					ScreenCursor = 0;
				}
			}
			else {
				if (ScreenCursor == 0) {
					ScreenScroll = 0;
				}
				else {
					ScreenCursor = 0;
				}
			}
			LCD.clear();
		}

		if (digitalRead(SW) == 0) {
			tone(BZ, 784, 125); delay(125);
			if (ScreenScroll == 0 && ScreenCursor == 0) {
				Screen = 6;
				Screen_Reset();
				while (digitalRead(SW) == 0);
				setRange_Screen();
			}
			else if ((ScreenScroll == 0 && ScreenCursor == 1) || (ScreenScroll == 1 && ScreenCursor == 0)) {
				Screen = 7;
				Screen_Reset();
				while (digitalRead(SW) == 0);
				setGoal_Screen();
			}
			else {
				Screen = 1;
				Screen_Reset();
				while (digitalRead(SW) == 0);
			}
		}
	}
}

void timeSettingTool() {
	Serial.println("timeSettingTool");
	i = 1;
	while (!(i == 0)) {
		EncValue = getEncoderTurn(); Serial.println(String(ScreenScroll) + " " + String(ScreenCursor));
		if (EncValue == 1) {
			if (i == 1) {
				if (!(Hour == 23)) {
					Hour += 1;
				}
			}
			else if(i == 2){
				if (!(Min == 59)) {
					Min += 1;
				}
			}
		}
		else if (EncValue == -1) {
			if (i == 1) {
				if (!(Hour == 0)) {
					Hour -= 1;
				}
			}
			else if(i == 2) {
				if (!(Min == 0)) {
					Min -= 1;
				}
			}
		}

		LCD.setCursor(2, 0);
		if (Hour < 10) LCD.print("0" + String(Hour));
		else LCD.print(String(Hour));
		LCD.setCursor(5, 0);
		if (Min < 10) LCD.print("0" + String(Min));
		else LCD.print(String(Min));

		if (digitalRead(SW) == 0) {
			tone(BZ, 784, 125); delay(125);
			if (i == 1) i += 1;
			else if(i == 2) i = 0;
			while (digitalRead(SW) == 0);
		}
	}
}

void setRange_Screen() {
	if (!(Range_H == -1 && Range_M == -1)) {
		Hour = Range_H;
		Min = Range_M;
	}
	else {
		Hour = getTimeElement(1);
		Min = getTimeElement(2);
	}
	Serial.println("setRange");
	while (Screen == 6) {
		// setRange Menu
		if (ScreenScroll == 0) {
			LCD.setCursor(0, 0); LCD.print(">");
			LCD.setCursor(2, 0); 
			if (Hour < 10) LCD.print("0"+String(Hour));
			else LCD.print(String(Hour));
			LCD.setCursor(4, 0); LCD.print(":");
			LCD.setCursor(5, 0);
			if (Min < 10) LCD.print("0" + String(Min));
			else LCD.print(String(Min));
			LCD.setCursor(0, 1); LCD.print("> OK");
		}
		else {
			LCD.setCursor(0, 0); LCD.print("> OK");
			LCD.setCursor(0, 1); LCD.print("> Back");
		}
		if (ScreenCursor == 0) {
			LCD.setCursor(15, 0); LCD.print("<");
		}
		else {
			LCD.setCursor(15, 1); LCD.print("<");
		}

		EncValue = getEncoderTurn(); Serial.println(String(ScreenScroll) + " " + String(ScreenCursor));
		if (EncValue == 1) {
			if (ScreenScroll == 0) {
				if (ScreenCursor == 0) {
					ScreenCursor = 1;
				}
				else {
					ScreenScroll = 1;
				}
			}
			else {
				if (ScreenCursor == 0) {
					ScreenCursor = 1;
				}
			}
			LCD.clear();
		}
		else if (EncValue == -1) {
			if (ScreenScroll == 0) {
				if (ScreenCursor == 1) {
					ScreenCursor = 0;
				}
			}
			else {
				if (ScreenCursor == 0) {
					ScreenScroll = 0;
				}
				else {
					ScreenCursor = 0;
				}
			}
			LCD.clear();
		}

		if (digitalRead(SW) == 0) {
			tone(BZ, 784, 125); delay(125);
			if (ScreenScroll == 0 && ScreenCursor == 0) {
				while (digitalRead(SW) == 0);
				timeSettingTool();
			}
			else if ((ScreenScroll == 0 && ScreenCursor == 1) || (ScreenScroll == 1 && ScreenCursor == 0)) {
				Screen = 5;
				Range_H = Hour;
				Range_M = Min;
				Screen_Reset();
				while (digitalRead(SW) == 0);
			}
			else {
				Screen = 5;
				Screen_Reset();
				while (digitalRead(SW) == 0);
			}
		}
	}
}

void setGoal_Screen() {
	if (!(Goal_H == -1 && Goal_M == -1)) {
		Hour = Goal_H;
		Min = Goal_M;
	}
	else {
		Hour = 0;
		Min = 0;
	}
	Serial.println("setGoal");
	while (Screen == 7) {
		// setGoal Menu
		if (ScreenScroll == 0) {
			LCD.setCursor(0, 0); LCD.print(">");
			LCD.setCursor(2, 0);
			if (Hour < 10) LCD.print("0" + String(Hour));
			else LCD.print(String(Hour));
			LCD.setCursor(4, 0); LCD.print(":");
			LCD.setCursor(5, 0);
			if (Min < 10) LCD.print("0" + String(Min));
			else LCD.print(String(Min));
			LCD.setCursor(0, 1); LCD.print("> OK");
		}
		else {
			LCD.setCursor(0, 0); LCD.print("> OK");
			LCD.setCursor(0, 1); LCD.print("> Back");
		}
		if (ScreenCursor == 0) {
			LCD.setCursor(15, 0); LCD.print("<");
		}
		else {
			LCD.setCursor(15, 1); LCD.print("<");
		}

		EncValue = getEncoderTurn(); Serial.println(String(ScreenScroll) + " " + String(ScreenCursor));
		if (EncValue == 1) {
			if (ScreenScroll == 0) {
				if (ScreenCursor == 0) {
					ScreenCursor = 1;
				}
				else {
					ScreenScroll = 1;
				}
			}
			else {
				if (ScreenCursor == 0) {
					ScreenCursor = 1;
				}
			}
			LCD.clear();
		}
		else if (EncValue == -1) {
			if (ScreenScroll == 0) {
				if (ScreenCursor == 1) {
					ScreenCursor = 0;
				}
			}
			else {
				if (ScreenCursor == 0) {
					ScreenScroll = 0;
				}
				else {
					ScreenCursor = 0;
				}
			}
			LCD.clear();
		}

		if (digitalRead(SW) == 0) {
			tone(BZ, 784, 125); delay(125);
			if (ScreenScroll == 0 && ScreenCursor == 0) {
				while (digitalRead(SW) == 0);
				timeSettingTool();
			}
			else if ((ScreenScroll == 0 && ScreenCursor == 1) || (ScreenScroll == 1 && ScreenCursor == 0)) {
				Screen = 5;
				Goal_H = Hour;
				Goal_M = Min;
				Screen_Reset();
				while (digitalRead(SW) == 0);
			}
			else {
				Screen = 5;
				Screen_Reset();
				while (digitalRead(SW) == 0);
			}
		}
	}
}

void About_Screen() {
	Serial.println("About");
	while (Screen == 8) {
		// About Menu
		if (ScreenScroll == 0) {
			LCD.setCursor(0, 0); LCD.print("Ver. 1.0.0");
			LCD.setCursor(0, 1); LCD.print("CCL-BY-NC-SA");
		}
		else {
			LCD.setCursor(0, 0); LCD.print("CCL-BY-NC-SA");
			LCD.setCursor(0, 1); LCD.print("Spark Makers");
		}

		EncValue = getEncoderTurn(); Serial.println(String(ScreenScroll) + " " + String(ScreenCursor));
		if (EncValue == 1) {
			if (ScreenScroll == 0) {
				ScreenScroll = 1;
			}
			LCD.clear();
		}
		else if (EncValue == -1) {
			if (ScreenScroll == 1) {
				ScreenScroll = 0;
			}
			LCD.clear();
		}
		
		if (digitalRead(SW) == 0) {
			tone(BZ, 784, 125); delay(125);
			Screen = 1;
			Screen_Reset();
			while (digitalRead(SW) == 0);
		}
	}
}

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(9600);

	LCD.begin();
	LCD.backlight();
	LCD.createChar(1, chairChar);

	RTC.halt(false);
	RTC.writeProtect(false);

	// 변수 초기화 시작
	// 변수 초기화 끝

	pinMode(SW, INPUT);
	pinMode(RST, OUTPUT); digitalWrite(RST, HIGH);
	pinMode(BZ, OUTPUT); digitalWrite(BZ, LOW);
	pinMode(R, OUTPUT); digitalWrite(R, LOW);
	pinMode(G, OUTPUT); digitalWrite(G, LOW);
	pinMode(B, OUTPUT); digitalWrite(B, LOW);

	Start_Screen();
}

// the loop function runs over and over again until power down or reset
void loop() {	
	digitalWrite(BZ, LOW);
	// main Menu
	if (ScreenScroll == 0) {
		LCD.setCursor(0, 0); LCD.print("> Start Study");
		LCD.setCursor(0, 1); LCD.print("> Set Time");
	}
	else {
		LCD.setCursor(0, 0); LCD.print("> Set Time");
		LCD.setCursor(0, 1); LCD.print("> About");
	}
	if (ScreenCursor == 0) {
		LCD.setCursor(15, 0); LCD.print("<");
	}
	else {
		LCD.setCursor(15, 1); LCD.print("<");
	}
	
	EncValue = getEncoderTurn(); Serial.println(String(ScreenScroll) + " " + String(ScreenCursor));
	if (EncValue == 1) {
		if (ScreenScroll == 0) {
			if (ScreenCursor == 0) {
				ScreenCursor = 1;
			}
			else {
				ScreenScroll = 1;
			}
		}
		else {
			if (ScreenCursor == 0) {
				ScreenCursor = 1;
			}
		}
		LCD.clear();
	}
	else if (EncValue == -1) {
		if (ScreenScroll == 0) {
			if (ScreenCursor == 1) {
				ScreenCursor = 0;
			}
		}
		else {
			if (ScreenCursor == 0) {
				ScreenScroll = 0;
			}
			else {
				ScreenCursor = 0;
			}
		}
		LCD.clear();
	}

	if (digitalRead(SW) == 0) {
		tone(BZ, 784, 125); delay(125);
		if (ScreenScroll == 0 && ScreenCursor == 0) {
			Screen = 2;
			Screen_Reset();
			while (digitalRead(SW) == 0);
			startStudy_Screen();
		}
		else if ((ScreenScroll == 0 && ScreenCursor == 1) || (ScreenScroll == 1 && ScreenCursor == 0)) {
			Screen = 5;
			Screen_Reset();
			while (digitalRead(SW) == 0);
			setTime_Screen();
		}
		else {
			Screen = 8;
			Screen_Reset();
			while (digitalRead(SW) == 0);
			About_Screen();
		}
	}
}
