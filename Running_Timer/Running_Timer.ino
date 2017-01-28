/*
 Name:		Running_Timer.ino
 Created:	1/21/2017 8:31:54 PM
 Author:	M. Virdone
*/

// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"

#define INTERRUPT_PIN 2

#define LED_PIN_A 6

#define HOUR_PIN 19
#define TEN_MIN_PIN 18
#define MIN_PIN 17
#define TEN_SEC_PIN 16
#define SEC_PIN 15

// Create the PPS variable which is going to be set within the ISR on the square wave output from teh DS3231
volatile byte PPS = 0;

// Create the RTC object using the included libraries
RTC_DS3231 rtc;

// Define 7 segment display (see Running_Timer_Design.xlsx), bit ordering is hfedcba and these are represented in decimal bytes
const byte sevenSegment[] = {63, 6, 91, 79, 102, 109, 125, 7, 127, 111};

// the setup function runs once when you press reset or power the board
void setup() {

    // Hardware setup
    
    // Setup the interrupt to receive the square wave input and trigger on the rising edge
    pinMode(INTERRUPT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), myISR, RISING);

    // Setup the A-G 7 segment pins. Lazy code on my part to assume the pins are all sequential
    for (int i = 0; i < 7; i++)
    {
        // Setup the 7 pins starting at LED_PIN_A as outputs
        pinMode(i + LED_PIN_A, OUTPUT);
    }

    // Setup the individual PNP pins for the digit enables
    pinMode(HOUR_PIN, OUTPUT);
    pinMode(TEN_MIN_PIN, OUTPUT);
    pinMode(MIN_PIN, OUTPUT);
    pinMode(TEN_SEC_PIN, OUTPUT);
    pinMode(SEC_PIN, OUTPUT);

    // All LED On for startup BIST
    digitalWrite(HOUR_PIN, LOW);
    digitalWrite(TEN_MIN_PIN, LOW);
    digitalWrite(MIN_PIN, LOW);
    digitalWrite(TEN_SEC_PIN, LOW);
    digitalWrite(SEC_PIN, LOW);

    for (int i = 0; i < 7; i++)
    {
        // Setup the 7 pins starting at LED_PIN_A as outputs
        digitalWrite(i + LED_PIN_A, HIGH);
    }

    // Wait 3 seconds
    delay(3000);

    // All LED On for startup BIST
    digitalWrite(HOUR_PIN, HIGH);
    digitalWrite(TEN_MIN_PIN, HIGH);
    digitalWrite(MIN_PIN, HIGH);
    digitalWrite(TEN_SEC_PIN, HIGH);
    digitalWrite(SEC_PIN, HIGH);

    for (int i = 0; i < 7; i++)
    {
        // Setup the 7 pins starting at LED_PIN_A as outputs
        digitalWrite(i + LED_PIN_A, LOW);
    }

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



}

// the loop function runs over and over again until power down or reset
void loop() {

    int secondCounter = -60;


    while (1)
    {
        

        // PPS should only be set to 1 inside the ISR
        if (PPS == 1)
        {
            
            // Clear the volatile flag
            PPS = 0;

            // Increment the second counter
            secondCounter++;


            /*

            if (secondCounter < 0)
            {
                Serial.print("-");
            }
            Serial.print(hourDigit, DEC);
            Serial.print(':');
            Serial.print(tenMinuteDigit, DEC);
            Serial.print(minuteDigit, DEC);
            Serial.print(':');
            Serial.print(tenSecondDigit, DEC);
            Serial.print(secondDigit, DEC);

            
            Serial.print('|');

            */

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

        updateTimeDisplay(secondCounter);
        
    }



    delay(3000);
  
}

void updateTimeDisplay(int secondCounter)
{
    int absSecondCounter = 0;

    byte secondDigit = 0;
    byte tenSecondDigit = 0;
    byte minuteDigit = 0;
    byte tenMinuteDigit = 0;
    byte hourDigit = 0;

    // Use the absolute value of the seconds for determining the digits
    // This is required to provide negaitve seconds
    // Why do you need negative seconds? So that you can have a countdown before the race starts!
    absSecondCounter = abs(secondCounter);



    // Take the secondCounter and split it into its associated digits
    hourDigit = absSecondCounter / 3600;
    tenMinuteDigit = (absSecondCounter % 3600) / 600;
    minuteDigit = (absSecondCounter % 600) / 60;
    tenSecondDigit = (absSecondCounter % 60) / 10;
    secondDigit = absSecondCounter % 10;

    // the ULQ2003A has a 1 uS maximum Thl and Tlh (time from high-to-low and low-to-high)
    // I will need to account for this if it is a problem.

    // Seconds
    digitalWrite(HOUR_PIN, HIGH);
    digitalWrite(SEC_PIN, LOW);    
    digitWrite(secondDigit);
    delay(1);

    // Tens of seconds
    digitalWrite(SEC_PIN, HIGH);
    digitalWrite(TEN_SEC_PIN, LOW);
    digitWrite(tenSecondDigit);
    delay(1);

    // Minutes    
    digitalWrite(TEN_SEC_PIN, HIGH);
    digitalWrite(MIN_PIN, LOW);
    digitWrite(minuteDigit);
    delay(1);

    // Tens of minutes
    digitalWrite(MIN_PIN, HIGH);
    digitalWrite(TEN_MIN_PIN, LOW);
    digitWrite(tenMinuteDigit);
    delay(1);

    // Hours
    digitalWrite(TEN_MIN_PIN, HIGH);
    digitalWrite(HOUR_PIN, LOW);
    digitWrite(hourDigit);
    delay(1);

    // Turn off, just in case we're servicing an interrupt and it takes a while
    digitalWrite(HOUR_PIN, HIGH);

}

void digitWrite(byte digit)
{
    // Turn the digit into the 7 segments which need to be illuminated
    byte currentSegments = sevenSegment[digit];

    // Shift through all of the individual LEDs to turn them on/off
    for (int i = 0; i < 7; i++)
    {
        // Bit shift the segment right, bitwise AND and see if it is "on" or "off" and set the LED output as needed
        if (((currentSegments >> i) & 1) > 0)
        {
            digitalWrite(LED_PIN_A + i, HIGH);
        }
        else // it was off
        {
            digitalWrite(LED_PIN_A + i, LOW);
        }
    }
}

// This ISR is setup to trigger on the rising edge (of the 1 PPS square wave)
void myISR()
{
    // Upon rising edge, set PPS to 1, this will be cleared in the main loop
    PPS = 1;
}