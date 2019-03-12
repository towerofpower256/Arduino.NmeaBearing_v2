/*
	NmeaParser - Library for parsing NMEA-0182 messages
	By David McDonald 12/03/2019
*/


#ifndef NmeaParser_h
#define NmeaParser_h

#include <Arduino.h>

#define NMEA_MSG_MAX_LENGTH 80
#define NMEA_MSG_MAX_TERMS 16
#define NMEA_TALKER_START_CHAR 1
#define NMEA_TALKER_MAX_LENGTH 2
#define NMEA_DESC_START_CHAR 3
#define NMEA_DESC_MAX_LENGTH 3
#define NMEA_READER_STATE_WAITING_START 0
#define NMEA_READER_STATE_READING_TERMS 1

class NmeaParser
{
  public:
    NmeaParser();
    bool doNext(char nextChar);
    bool validateChecksum();
    String getTerm(int targetTermIndex);
    String getSentence();
    String getTalkerID();
    String getMessageDescription();

  private:
    void processMessage();
    void reset();
    void addChar(char inChar);
    bool canAddChar();
    char _buffer[NMEA_MSG_MAX_LENGTH];
    int _readerState;
    int _charCount;
};

#endif
