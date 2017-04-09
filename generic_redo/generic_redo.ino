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
int led_PIN = 3;

int debug_led_PIN = 13;

byte message_buff[MESSAGESIZE];

void setup() 
{
  Serial.begin(9600);
  debugPort.begin(9600);
  pinMode(re_PIN, OUTPUT);
  pinMode(de_PIN, OUTPUT);
  
  pinMode(button_PIN, INPUT);
  pinMode(led_PIN, OUTPUT);

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

void loop()
{
  
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
    
    debugPort.write(incomingChecksum);
    debugPort.write(calculatedChecksum);
    
    int goodChecksum = (calculatedChecksum == incomingChecksum);

    if (goodChecksum)
    {
      digitalWrite(debug_led_PIN, goodChecksum);
      analogWrite(led_PIN, payload);
      clearBuffer();
    } 
    else
    {
      digitalWrite(debug_led_PIN, LOW);
    }
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

