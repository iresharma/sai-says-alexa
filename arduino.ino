#include <SPI.h>
#include <DMD.h>
#include <TimerOne.h>
#include "SystemFont5x7.h"
#include "Arial_black_16.h"
#include <Wire.h>
// #include <RTClib.h>

#define DISPLAYS_ACROSS 1
#define DISPLAYS_DOWN 1
DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);

// RTC_DS3231 rtc;
char* existing = "";

const int MAX_LENGTH = 1000;  // Define maximum length for your input
char input[MAX_LENGTH];


void ScanDMD()
{ 
  dmd.scanDisplayBySPI();
}


void setup(void)
{
  // rtc.begin();
  Timer1.initialize( 5000 );
  Timer1.attachInterrupt( ScanDMD );

  dmd.clearScreen( true );  
  dmd.selectFont(Arial_Black_16);

  Serial.begin(9600); // Match baud rate with ESP32
  displayText("Loading...");
}


void displayText(char *text) {
    // Check if the text contains a line break marker '|'
    char *lineBreak = strchr(text, '|');
    
    if (lineBreak != NULL) {
        // We have a two-line message
        // Temporarily replace the '|' with null terminator to split the string
        *lineBreak = '\0';
        
        // Display first line
        dmd.clearScreen(true);
        dmd.selectFont(System5x7);
        for (byte x=0;x<DISPLAYS_ACROSS;x++) {
          for (byte y=0;y<DISPLAYS_DOWN;y++) {
            dmd.drawString(  2+(32*x),  1+(16*y), text, 5, GRAPHICS_NORMAL );

            char *secondLine = lineBreak + 1;
            dmd.drawString(0, 16, secondLine, strlen(secondLine), GRAPHICS_NORMAL);
            dmd.drawString(  2+(32*x),  9+(16*y), secondLine, 5, GRAPHICS_NORMAL );
          }
        }
        
        // Display for a few seconds
        delay(5000);
        
        // Restore the '|' character
        *lineBreak = '|';
    } else {
        // Regular scrolling text for single line messages
        dmd.drawMarquee(text, strlen(text), (32*DISPLAYS_ACROSS)-1, 0);
        long start = millis();
        long timer = start;
        boolean ret = false;
        int count = 0;
        while(count < 5) {
            if ((timer+30) < millis()) {
                ret = dmd.stepMarquee(-1, 0);
                if(ret == true) {
                    count++;
                    dmd.drawMarquee(text, strlen(text), (32*DISPLAYS_ACROSS)-1, 0);
                }
                timer = millis();
            }
        }
    }
}

// void ShowClockNumbers() {
//     DateTime now = rtc.now();

//     unsigned int uiTime = now.hour() * 100 + now.minute();
    
//     byte bColonOn = (now.second() % 2 == 0);

//     dmd.clearScreen(true);
//     dmd.drawChar(  1,  3,'0'+((uiTime%10000)/1000), GRAPHICS_NORMAL );
//     dmd.drawChar(  8,  3, '0'+((uiTime%1000) /100),  GRAPHICS_NORMAL );
//     dmd.drawChar( 17,  3, '0'+((uiTime%100)  /10),   GRAPHICS_NORMAL );
//     dmd.drawChar( 25,  3, '0'+ (uiTime%10),          GRAPHICS_NORMAL );
//     if( bColonOn )
//         dmd.drawChar( 15,  3, ':', GRAPHICS_OR     );
//     else
//         dmd.drawChar( 15,  3, ':', GRAPHICS_NOR    );
    
//     delay(1000);
//     Serial.println('0'+((uiTime%10000)/1000) );
//     Serial.println('0'+((uiTime%1000) /100) );
//     Serial.println('0'+((uiTime%100)  /10));
//     Serial.println('0'+ (uiTime%10) );
// }

/*--------------------------------------------------------------------------------------
  loop
  Arduino architecture main loop
--------------------------------------------------------------------------------------*/


void loop(void)
{

  Serial.println("Working");
  if (Serial.available() > 0) {
    // Clear the input buffer before reading new data
    memset(input, 0, MAX_LENGTH);
    
    // Read characters into the buffer and add null terminator
    int i = 0;
    while (Serial.available() > 0 && i < MAX_LENGTH - 1) {
      char c = Serial.read();
      if (c == '\n') break;
      input[i++] = c;
    }
    input[i] = '\0';  // Add null terminator to make it a proper C string
    
    Serial.print("Received from ESP32: ");
    Serial.println(input);
  }
  
  displayText(input);
  existing = input;
  dmd.clearScreen( true );
   
}