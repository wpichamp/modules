#include <EEPROM.h>
#include <SoftwareSerial.h>

SoftwareSerial debugPort(8, 9); // RX, TX

#define MESSAGESIZE 8

/* For setting the mode of the MAX485 */
#define TX HIGH
#define RX LOW

byte my_id;
int master_id = 0;

int re_PIN = 10;
int de_PIN = 11;

int button_PIN = 4;

int led0_PIN = 3;
int led1_PIN = 5;
int led2_PIN = 6;

int debug_led_PIN = 13;

byte message_buff[MESSAGESIZE];

void setup() 
{
  Serial.begin(9600);
  debugPort.begin(9600);
  
  pinMode(re_PIN, OUTPUT);
  pinMode(de_PIN, OUTPUT);
  
  pinMode(button_PIN, INPUT);
  
  pinMode(led0_PIN, OUTPUT);
  pinMode(led1_PIN, OUTPUT);
  pinMode(led2_PIN, OUTPUT);

  pinMode(debug_led_PIN, OUTPUT); 

  my_id = EEPROM.read(0); // read the ID stored in eeprom
    
  setMode(RX);
}

byte prefex0;
byte prefex1;
byte prefex2;
byte prefex3;
byte prefex4;
byte payload;
byte depth;
byte incomingChecksum;

bool newMessage = false;

void loop()
{
  if (newMessage)
  {
    
    debugPort.write(prefex0);
    debugPort.write(prefex1);
    debugPort.write(prefex2);
    debugPort.write(prefex3);
    debugPort.write(prefex4);
    debugPort.write(payload);
    debugPort.write(depth);
    debugPort.write(incomingChecksum);
    
    
    switch(prefex0)
    {
      case 0:
        switch(prefex1)
        {
          case 0:
            analogWrite(led0_PIN, payload);
            break;
          case 1:
            analogWrite(led1_PIN, payload);
            break;
          case 2:
            analogWrite(led2_PIN, payload);
            break;
        }
        break;
    }
    newMessage = false;
  
  }
  
}

void serialEvent() {
  
  if (Serial.available() >= MESSAGESIZE)
  {
    digitalWrite(debug_led_PIN, HIGH);
    
    Serial.readBytes(message_buff, MESSAGESIZE);
    
    prefex0 = message_buff[0];
    prefex1 = message_buff[1];
    prefex2 = message_buff[2];
    prefex3 = message_buff[3];
    prefex4 = message_buff[4];
    payload = message_buff[5];
    depth = message_buff[6];
    incomingChecksum = message_buff[7];
    
    byte calculatedChecksum = sum(message_buff, 7);
    
    
    int goodChecksum = (calculatedChecksum == incomingChecksum);

    if (goodChecksum)
    {
      digitalWrite(debug_led_PIN, goodChecksum);
      newMessage = true;
    } 
    else
    {
      digitalWrite(debug_led_PIN, LOW);
      clearBuffer(); // give up on the current transmission
      newMessage = false;
    }
  }
  else
  {
    newMessage = false;
  }
}

byte sum(byte incomingArray[], int arraySize)
{
  byte sum = 0;
  for (int index = 0; index < arraySize; index++)
  {
    sum += incomingArray[index];
  }
  return sum;
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

