#include <EEPROM.h>
#include "Message.h"

/* For setting the mode of the MAX485 */
#define TX HIGH
#define RX LOW

/* Message Type Mappings */
#define STATUS 0

#define MESSAGESIZE 10

byte my_id;
int master_id = 0;

int re_PIN = 10;
int de_PIN = 11;

int button_PIN = 4;
int led_PIN = 3;

int debug_led_PIN = 13;


Message rx_message = Message();
Message tx_message = Message();

void setup() 
{
  Serial.begin(115200);

  pinMode(re_PIN, OUTPUT);
  pinMode(de_PIN, OUTPUT);
  
  pinMode(button_PIN, INPUT);
  pinMode(led_PIN, OUTPUT);

  pinMode(debug_led_PIN, OUTPUT); 

  my_id = EEPROM.read(0); // read the ID stored in eeprom

  /* to_id and from_id should not change in outgoing messages */
  tx_message.to_id = master_id;
  tx_message.from_id = my_id;
    
  setMode(RX);
}

byte button_state;
byte pot_state;
byte s;

byte message_buff[MESSAGESIZE];

void loop()
{
  button_state = digitalRead(button_PIN);   
  pot_state = map(analogRead(0), 0, 1023, 0, 255); 
}

void serialEvent() {
  
  if (Serial.available() >= 10)
  {
    digitalWrite(debug_led_PIN, HIGH);
    
    Serial.readBytes(message_buff, MESSAGESIZE);

    rx_message.readIn(message_buff);
  
    if (rx_message.to_id == my_id)
    {
      switch(rx_message.message_type)
      {
        case(STATUS):
          
          delay(30); // delay to allow for the master to enter recieve mode
        
          setMode(TX); // enable tx mode on the MAX485
          
          /* insert the relevant data into the message object */
          tx_message.message_type = STATUS;
          tx_message.d0 = button_state;
          tx_message.d1 = pot_state;

          /* write the message back to the master */ 
          writeBytes(tx_message.getMessageBytes(), 10);
          
          setMode(RX);
      
          digitalWrite(debug_led_PIN, LOW);
          break;
        
        default:
          
          break;
      }
    }
    else
    {
      // since the ID didn't match, the rest of the buffer doesn't matter
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

void writeBytes(byte tx_bytes[], int num_bytes)
{
  for (int index = 0; index < num_bytes; index++)
  {
    Serial.write(tx_bytes[index]);
  }
  Serial.flush();
}

