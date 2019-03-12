/*
	NmeaParser - Library for parsing NMEA-0182 messages
	By David McDonald 12/03/2019
*/

#include "NmeaParser.h"
#include <Arduino.h>

const char NMEA_START_CHAR = '$';
const char NMEA_DELIM_CHAR = ',';
const char NMEA_PARITY_CHAR = '*';
const char NMEA_END_CHAR = '\r';
const char NMEA_NULL_CHAR = '\0';

NmeaParser::NmeaParser() {
  reset();
}

// Process a character
bool NmeaParser::doNext(char nextChar) {

  if (_readerState == NMEA_READER_STATE_WAITING_START) {
    // The reader is waiting for the start character
    if (nextChar == NMEA_START_CHAR) { 
      reset();
      addChar(nextChar);
      _readerState = NMEA_READER_STATE_READING_TERMS; // We are now reading the terms / the data
      return false;
    }
  }
  
  if (_readerState == NMEA_READER_STATE_READING_TERMS) {
    
    
    // Is this the end message char?
    if (nextChar == NMEA_END_CHAR) {
      // This is the end char, message reading complete
      _readerState = NMEA_READER_STATE_WAITING_START;
      return true;
    }

    // Keep adding chars
    // TODO: Error check for max message length 
    addChar(nextChar);

    return false;
  }

  // Just in case
  return false;
  
}

// Just return what's in the buffer
String NmeaParser::getSentence() {
  return String(_buffer);
}

// Add a character to the buffer
void NmeaParser::addChar(char inChar) {
  //Serial.println(inChar);
  
  _buffer[_charCount] = inChar;
  _charCount++;
}

// Can the reader fit another char
bool NmeaParser::canAddChar() {
  return _charCount < NMEA_MSG_MAX_LENGTH;
}

// Reset the reader to get ready to read another message
void NmeaParser::reset() {
  // Clear the buffer
  //memset(_buffer, NMEA_NULL_CHAR, NMEA_MSG_MAX_LENGTH); // Not sure if we need to clear the char array
  
  _charCount = 0;
}

// Get a specific term or segment of the message
String NmeaParser::getTerm(int targetTermIndex) {
  int termIndex = 0;
  int termBufferIndex = 0;
  char termBuffer[NMEA_MSG_MAX_LENGTH];
  memset(termBuffer, NMEA_NULL_CHAR, sizeof(termBuffer));

  // Iterate through the characters in the message buffer
  // Copy everything from the n'th delimeter, up to the next delimeter
  // E.g. term 2 in "$ASDFA,,34,89*58" would be "34"
  for (int i=1; i < _charCount; i++) {
    char ic = _buffer[i];

    // Recognise the end, or after we've passed the term that we care about
    if (ic == NMEA_PARITY_CHAR || ic == NMEA_END_CHAR || termIndex > targetTermIndex) {
      break;
    }

    // If the next character is the delimiter character, ++ the term counter
    if (ic == NMEA_DELIM_CHAR) {
      termIndex++;
    }
    
    // if we're inside the desired term, add the next char to the buffer
    else if (targetTermIndex == termIndex) {
      termBuffer[termBufferIndex] = ic;
      termBufferIndex++;
    }
  }

  // return the buffer
  if (termBufferIndex == 0) {
    return String();
  } else {
    //char r[termBufferIndex];
    //memcpy(r, termBuffer, termBufferIndex);
    //return String(r);
    return String(termBuffer);
  }
}

// Get the talker ID in the currently stored message
// Should be the first 2 characters of the first term
String NmeaParser::getTalkerID() {
  char talkerBuffer[NMEA_TALKER_MAX_LENGTH+1];
  int talkerBufferIndex = 0;
  memset(talkerBuffer, NMEA_NULL_CHAR, sizeof(talkerBuffer));
  for (int i = NMEA_TALKER_START_CHAR; i < _charCount; i++) {
    // Detect maximum talkerID length
    if (talkerBufferIndex >= NMEA_TALKER_MAX_LENGTH) break;
    
    talkerBuffer[talkerBufferIndex] = _buffer[i];
    talkerBufferIndex++;
  }

  if (talkerBufferIndex == 0) {
    return String();
  } else {
    return String(talkerBuffer);
  }
}

// Get the message description in the currently stored message
// Should be the last 3 characters of the first term
String NmeaParser::getMessageDescription() {
  char descBuffer[NMEA_DESC_MAX_LENGTH+1];
  int descBufferIndex = 0;
  memset(descBuffer, NMEA_NULL_CHAR, sizeof(descBuffer));
  for (int i = NMEA_DESC_START_CHAR; i < _charCount; i++) {
    // Detect maximum message description length
    if (descBufferIndex >= NMEA_DESC_MAX_LENGTH) break;
    
    descBuffer[descBufferIndex] = _buffer[i];
    descBufferIndex++;
  }

  if (descBufferIndex == 0) {
    return String();
  } else {
    return String(descBuffer);
  }
}
