/*
 * CHAMP_Message.cpp
 * A library passing messages using the easyRS485 protocol
 * Created By Devon Bray for CHAMP - champ.wpi.edu, 1 February 2017
*/

#include "Arduino.h"
#include "CHAMP_Message.h"

/* Constructor */ 
CHAMP_Message::CHAMP_Message()
{
  /* init all the values in the message to 0 */
  
  to_id = 0;
  from_id = 0;
  checksum = 0;
  message_type = 0;
  
  d0 = 0;
  d1 = 0;
  d2 = 0;
  d3 = 0;
  d4 = 0;
  d5 = 0;
  
}

void CHAMP_Message::readIn(byte message_bytes[])
{
    to_id = message_bytes[0];
    from_id = message_bytes[1];
    checksum = message_bytes[2];
    message_type = message_bytes[3];
    
    d0 = message_bytes[4];
    d1 = message_bytes[5];
    d2 = message_bytes[6];
    d3 = message_bytes[7];
    d4 = message_bytes[8];
    d5 = message_bytes[9];
}

byte* CHAMP_Message::getMessageBytes()
{ 
  static byte m[10];

  m[0] = to_id;
  m[1] = from_id;
  m[2] = checksum;
  m[3] = message_type;
  
  m[4] = d0;
  m[5] = d1;
  m[6] = d2;
  m[7] = d3;
  m[8] = d4;
  m[9] = d5;
  
  return m;
}

