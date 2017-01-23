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

// Define 7 segment display (see Running_Timer_Design.xlsx), bit ordering is hfedcba and these are represented in decimal bytes
const byte sevenSegment[] = {63, 6, 91, 79, 102, 109, 125, 7, 127, 111};

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

    byte secondDigit = 0;
    byte tenSecondDigit = 0;
    byte minuteDigit = 0;
    byte tenMinuteDigit = 0;
    byte hourDigit = 0;

    while (1)
    {
        

        // PPS should only be set to 1 inside the ISR
        if (PPS == 1)
        {
            secondCounter++;

            // Clear the volatile flag
            PPS = 0;

            // Note the elaborate conditional statement below to manage each digit of time independently
            // This probably could have been done with a modulo as well, but this seemed easy enogh
            // On each PPS, increment the seconds counter, and update on the 9s and 59s as needed

            // Find the 9s or increment
            if (secondCounter == 9)
            {
                // Reset the second counter
                secondCounter = 0;

                // Find the 59s or increment 
                if (tenSecondCounter == 5)
                {
                    tenSecondCounter = 0;

                    // Find the 9m 59s or increment
                    if (minuteCounter == 9)
                    {
                        minuteCounter = 0;
                        
                        // Find the 59m 59s or increment
                        if (tenMinuteCounter == 5)
                        {
                            tenMinuteCounter = 0;
                            hourCounter++;

                        }
                        else // Not 59 minute, increment the 10 minutes
                        {
                            tenMinuteCounter++;
                        }
                    }
                    else // not 9m increment the minute
                    {
                        minuteCounter++;
                    }
                }
                else // not a 59s, increment 10 seconds 
                {
                    
                    tenSecondCounter++;
                }
            }
            else // Not a 9s, increment seconds
            {                
                secondCounter++;
            }
            



            Serial.print(hourCounter, DEC);
            Serial.print(':');
            Serial.print(tenMinuteCounter, DEC);
            Serial.print(minuteCounter, DEC);
            Serial.print(':');
            Serial.print(tenSecondCounter, DEC);
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

// This ISR is setup to trigger on the rising edge (of the 1 PPS square wave)
void myISR()
{
    // Upon rising edge, set PPS to 1, this will be cleared in the main loop
    PPS = 1;
}