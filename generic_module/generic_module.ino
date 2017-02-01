#define TX HIGH
#define RX LOW

/* Message Type Mappings */
#define STATUS 0

#define MESSAGESIZE 10

#include <EEPROM.h>

byte my_id;
int master_id = 0;

int re_PIN = 10;
int de_PIN = 11;

int button_PIN = 4;
int led_PIN = 3;

int debug_led_PIN = 13;

void setup() 
{
  Serial.begin(115200);

  pinMode(re_PIN, OUTPUT);
  pinMode(de_PIN, OUTPUT);
  
  pinMode(button_PIN, INPUT);
  pinMode(led_PIN, OUTPUT);

  pinMode(debug_led_PIN, OUTPUT); 

  my_id = EEPROM.read(0); // read the ID stored in eeprom
    
  setMode(RX);
}

byte button_state;
byte pot_state;
byte s;

byte message[MESSAGESIZE];

/* message fields */
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
/* end of message fields */

void loop()
{
  setMode(RX);
  
  if (Serial.available() >= 10)
  {
    digitalWrite(debug_led_PIN, HIGH);
    Serial.readBytes(message, MESSAGESIZE);
  
    to_id = message[0];
    from_id = message[1];
    checksum = message[2];
    message_type = message[3];
    
    d0 = message[4];
    d1 = message[5];
    d2 = message[6];
    d3 = message[7];
    d4 = message[8];
    d5 = message[9];

    if (to_id == my_id)
    {
      switch(message_type)
      {
        case(STATUS):
          
          delay(45); // delay to allow for the master to enter recieve mode
        
          setMode(TX); // enable tx mode on the MAX485
          
          button_state = digitalRead(button_PIN);   
          pot_state = map(analogRead(0), 0, 1023, 0, 255); 
          
          Serial.write(master_id);    // to_id
          Serial.write(my_id);        // from_id
          Serial.write(0);            // checksum
          Serial.write(STATUS);       // message_type
          
          Serial.write(button_state); // d0
          Serial.write(pot_state);    // d1
          Serial.write(0);            // d2
          Serial.write(0);            // d3
          Serial.write(0);            // d4
          Serial.write(0);            // d5
          
          // block until the data has been all written out
          Serial.flush();
          digitalWrite(debug_led_PIN, LOW);
          break;
        default:
          setMode(RX);
          break;
      }
    }
    else
    {
      clearBuffer();
    }    
  }
}

void setMode(boolean state)
{
  digitalWrite(re_PIN, state);
  digitalWrite(de_PIN, state);
}

void clearBuffer()
{
  while (Serial.available() > 0)
  {
    Serial.read();
  }
}

