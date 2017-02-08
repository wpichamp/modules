/*
 * message.h
 * A library passing messages using the easyRS485 protocol
 * Created By Devon Bray for CHAMP - champ.wpi.edu, 1 February 2017
*/

#ifndef Message_h
#define Message_h

#include "Arduino.h"

#define MESSAGESIZE 10

class Message
{
  public:
    
    Message();
    void readIn(byte message_bytes[]);
    byte* getMessageBytes();
    
    byte to_id;
    byte from_id;
    byte checksum;
    byte message_type;
    
    byte d0;
    byte d1;
    byte d2; 
    byte d3;
    byte d4;
    byte d5;
};

#endif
