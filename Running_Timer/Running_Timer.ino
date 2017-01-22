/*
 Name:		Running_Timer.ino
 Created:	1/21/2017 8:31:54 PM
 Author:	M. Virdone
*/

// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"

#define INTERRUPT_PIN 2

// Create the PPS variable which is going to be set within the ISR on the square wave output from teh DS3231
volatile byte PPS = 0;

// Create the RTC object using the included libraries
RTC_DS3231 rtc;

// the setup function runs once when you press reset or power the board
void setup() {

    // Troubleshooting serial terminal
    Serial.begin(9600);

    delay(3000); // wait for console opening

    
    // RTC setup is from the example code
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        while (1);
    }
           
    if (rtc.lostPower()) {
        Serial.println("RTC lost power, lets set the time!");
        // following line sets the RTC to the date & time this sketch was compiled
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        // This line sets the RTC with an explicit date & time, for example to set
        // January 21, 2014 at 3am you would call:
        // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    }

    // END RTC Setup Example code

    // Setup the square wave for a 1 PPs
    rtc.writeSqwPinMode(DS3231_SquareWave1Hz);

    // Setup the interrupt to receive the square wave input and trigger on the rising edge
    pinMode(INTERRUPT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), myISR, RISING);

}

// the loop function runs over and over again until power down or reset
void loop() {

    int secondCounter = 0; 

    while (1)
    {
        // PPS should only be set to 1 inside the ISR
        if (PPS == 1)
        {
            // Clear the volatile flag
            PPS = 0;
            secondCounter++;

            Serial.print(secondCounter, DEC);
            
            Serial.print('|');

            DateTime now = rtc.now();

            Serial.print(now.year(), DEC);
            Serial.print('/');
            Serial.print(now.month(), DEC);
            Serial.print('/');
            Serial.print(now.day(), DEC);
            Serial.print(" ");            
            Serial.print(now.hour(), DEC);
            Serial.print(':');
            Serial.print(now.minute(), DEC);
            Serial.print(':');
            Serial.print(now.second(), DEC);
            Serial.println();

        }
    }



    delay(3000);
  
}

void myISR()
{
    PPS = 1;
}