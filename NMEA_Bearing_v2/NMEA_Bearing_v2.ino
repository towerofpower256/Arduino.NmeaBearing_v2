#include <LiquidCrystal.h>
#include "NmeaParser.h"

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

  // Ready
  Serial.println("Ready!");
  UpdateLcd();
}

void loop() {
  // put your main code here, to run repeatedly:

  
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


void UpdateLcd() {
  Serial.println(F("Updating LCD"));
  
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

  // Print diff
  //int diffPadding = LCD_WIDTH - sizeof(DIFF_PREFIX);
  lcd.setCursor(LCD_DIFF_POS_OFFSET, 0);
  lcd.print(DIFF_PREFIX);

  lcd.setCursor(LCD_DIFF_POS_OFFSET, 1);
  if (bearingT < 0 || bearingM < 0) {
    lcd.print(BLANK_NUMBER_CHAR);
  } else {
    float diff = bearingM - bearingT;
    if (diff > 0) lcd.print(DIFF_POSITIVE_CHAR);
    else if (diff < 0) lcd.print(DIFF_NEGATIVE_CHAR);
    else lcd.print(SPACE_CHAR);
    lcd.print(abs(diff));
  }
  
}
