/*
 * NMEA Bearing Reader v2
 * by David McDonald 1203/2019
 * Someone wanted to be able to display HDT (true heading) / HDM (magnetic heading)
 * NMEA-0183 data to an LCD display so they could hook an Ardunio up to an NMEA-0183 device
 * and go from there.
 * 
 * This program & setup is designed to listen for HDT/HDM NMEA-0183 messages
 * on the hardware serial interface, and output them to the LCD display.
 * 
 * It'll also listen for a button press which will reset the remembered data.
 * Buttons are the ones on the Freetronics LCD Shield, or read from a single analog pin.
 */

#include <LiquidCrystal.h>
#include "NmeaParser.h"

// Freetronics button stuff
#define BUTTON_ADC_PIN            A0  // A0 is the button ADC input
#define RIGHT_10BIT_ADC           0  // right
#define UP_10BIT_ADC            145  // up
#define DOWN_10BIT_ADC          329  // down
#define LEFT_10BIT_ADC          505  // left
#define SELECT_10BIT_ADC        741  // right
#define BUTTONHYSTERESIS         10  // hysteresis for valid button sensing window
//return values for ReadButtons()
#define BUTTON_NONE               0  // 
#define BUTTON_RIGHT              1  // 
#define BUTTON_UP                 2  // 
#define BUTTON_DOWN               3  // 
#define BUTTON_LEFT               4  // 
#define BUTTON_SELECT             5  // 

byte buttonJustPressed  = false;         //this will be true after a ReadButtons() call if triggered
byte buttonJustReleased = false;         //this will be true after a ReadButtons() call if triggered
byte buttonWas          = BUTTON_NONE;   //used by ReadButtons() for detection of button events

// Constants
const char SPACE_CHAR = ' ';
const char COMMA_CHAR = ',';
const char QUOTE_CHAR = '"';
const char DIFF_POSITIVE_CHAR = 'W';
const char DIFF_NEGATIVE_CHAR = 'E';
const char BLANK_NUMBER_CHAR = '-';
const char COLON_CHAR = ':';
const char MAGNETIC_BEARING_NMEA_CHAR = 'M';
const char MAGNETIC_BEARING_DISPLAY_CHAR = 'C'; // C for Compass
const char TRUE_BEARING_NMEA_CHAR = 'T';
const char TRUE_BEARING_DISPLAY_CHAR = 'T';


// Working variables
float bearingM = -1;
float bearingT = -1;
bool resetButtonDown = false;

// Messages
const char STARTUP_MSG[] = "Starting...";
const char DIFF_PREFIX[] = "ERROR:";


// LCD stuff
LiquidCrystal lcd( 8, 9, 4, 5, 6, 7); // Init the LCD object, with the default PINs
const unsigned int LCD_WIDTH = 16;
const unsigned int LCD_DIFF_POS_OFFSET = 10;

// NMEA Parser
NmeaParser nmeaParser;

void setup() {
  //Serial
  Serial.begin(9600);
  Serial.flush();
  Serial.println(STARTUP_MSG);

  // LCD
  lcd.begin(LCD_WIDTH, 2); // Init, with correct dimensions (16 columns, 2 rows)
  lcd.clear();
  lcd.print(STARTUP_MSG);

  // Setup some button stuff
  pinMode( BUTTON_ADC_PIN, INPUT );         //ensure A0 is an input
  digitalWrite( BUTTON_ADC_PIN, LOW );      //ensure pullup is off on A0

  // Ready
  Serial.println("Ready!");
  UpdateLcd();
}

void loop() {
  // put your main code here, to run repeatedly:

  // Button stuff
  byte button = ReadButtons();
  if (button == BUTTON_SELECT && !resetButtonDown) {
    resetButtonDown = true;
    Reset();
  } else if (button != BUTTON_SELECT && resetButtonDown) {
    resetButtonDown = false;
  }
}

// Function to run whenever there's a serial event on the hardware serial RX
// Remember: Multiple bytes of data may be available
void serialEvent() {
  // While 
  while (Serial.available()) { // While there is data in the serial buffer
    char inChar = (char)Serial.read();
    if (nmeaParser.doNext(inChar)) {
      bool shouldUpdateLcd = false;
      
      Serial.println("Read msg");
      Serial.println(nmeaParser.getSentence());
      Serial.print("Desc: "); Serial.print(QUOTE_CHAR); Serial.print(nmeaParser.getMessageDescription()); Serial.println(QUOTE_CHAR);

      String newValue = nmeaParser.getTerm(1);
      Serial.print(F("New value: ")); Serial.println(newValue);
      
      if (nmeaParser.getMessageDescription() == "HDT") {
        Serial.println(F("Is true heading update"));
        
        if (newValue != "") {
          bearingT = newValue.toFloat();
          shouldUpdateLcd = true;
        } 
      }
      else if (nmeaParser.getMessageDescription() == "HDM") {
        Serial.println(F("Is magnetic heading update"));
        
        if (newValue != "") {
          bearingM = newValue.toFloat();
          shouldUpdateLcd = true;
        } 
      }
      /*
      else if (nmeaParser.getMessageDescription() == "HDG") {
        Serial.println(F("Is a heading update"));

        String hdgType = nmeaParser.getTerm(2);
        
        if (newValue != "") {
          if (hdgType == String(MAGNETIC_BEARING_NMEA_CHAR)) {
            Serial.println(F("Is valid magnetic bearing"));
            bearingM = newValue.toFloat();
            shouldUpdateLcd = true;
          }
          else if (hdgType == String(TRUE_BEARING_NMEA_CHAR)) {
            Serial.println(F("Is valid true bearing"));
            bearingT = newValue.toFloat();
            shouldUpdateLcd = true;
          }
        }
      }
      */

      if (shouldUpdateLcd) UpdateLcd();
    }
  }
}

// Reset the display & remembered data
void Reset() {
  Serial.println(F("Resetting"));
  bearingM = -1;
  bearingT = -1;
  UpdateLcd();
}


void UpdateLcd() {
  
  lcd.clear();
  
  // Print magnetic bearing
  lcd.setCursor(0,1);
  lcd.print(MAGNETIC_BEARING_DISPLAY_CHAR);
  lcd.print(COLON_CHAR);
  if (bearingM < 0) lcd.print(BLANK_NUMBER_CHAR);
  else lcd.print(bearingM);

  // Print true bearing
  lcd.setCursor(0,0);
  lcd.print(TRUE_BEARING_DISPLAY_CHAR);
  lcd.print(COLON_CHAR);
  if (bearingT < 0) lcd.print(BLANK_NUMBER_CHAR);
  else lcd.print(bearingT);

  // Update and print difference / compass error
  // Magnetic compass error = true bearing - magnetic bearing
  lcd.setCursor(LCD_DIFF_POS_OFFSET, 0);
  lcd.print(DIFF_PREFIX);

  lcd.setCursor(LCD_DIFF_POS_OFFSET, 1);
  if (bearingT < 0 || bearingM < 0) {
    lcd.print(BLANK_NUMBER_CHAR);
  } else {
    float diff = ConstrainAngle(bearingM - bearingT);
    if (diff > 0) lcd.print(DIFF_POSITIVE_CHAR);
    else if (diff < 0) lcd.print(DIFF_NEGATIVE_CHAR);
    else lcd.print(SPACE_CHAR);
    lcd.print(abs(diff));
  }
}

// Constrain / normalize an angle value to accomodate for angle-wrap
// 0.8 - 353.60 should equal -7.2, not -352.8
// 45 - 315 should equal -90, not -270
// Credit: https://stackoverflow.com/questions/11498169/dealing-with-angle-wrap-in-c-code
float ConstrainAngle(float x) {
  x = fmod(x + 180, 360);
  if (x < 0)
    x += 360;
  
  x = x - 180;

  return x;
}

// Read the buttons
byte ReadButtons()
{
   unsigned int buttonVoltage;
   byte button = BUTTON_NONE;   // return no button pressed if the below checks don't write to btn
   
   //read the button ADC pin voltage
   buttonVoltage = analogRead( BUTTON_ADC_PIN );
   //sense if the voltage falls within valid voltage windows
   if( buttonVoltage < ( RIGHT_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_RIGHT;
   }
   else if(   buttonVoltage >= ( UP_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( UP_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_UP;
   }
   else if(   buttonVoltage >= ( DOWN_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( DOWN_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_DOWN;
   }
   else if(   buttonVoltage >= ( LEFT_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( LEFT_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_LEFT;
   }
   else if(   buttonVoltage >= ( SELECT_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( SELECT_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_SELECT;
   }
   //handle button flags for just pressed and just released events
   if( ( buttonWas == BUTTON_NONE ) && ( button != BUTTON_NONE ) )
   {
      //the button was just pressed, set buttonJustPressed, this can optionally be used to trigger a once-off action for a button press event
      //it's the duty of the receiver to clear these flags if it wants to detect a new button change event
      buttonJustPressed  = true;
      buttonJustReleased = false;
   }
   if( ( buttonWas != BUTTON_NONE ) && ( button == BUTTON_NONE ) )
   {
      buttonJustPressed  = false;
      buttonJustReleased = true;
   }
   
   //save the latest button value, for change event detection next time round
   buttonWas = button;
   
   return( button );
}
