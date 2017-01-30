#define TX HIGH
#define RX LOW

#define MESSAGESIZE 10

#include <EEPROM.h>

// int my_id = EEPROM.read(0);
int my_id = 2;
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
  
  setMode(RX);
}

byte button_state;
byte pot_state;
byte s;

int new_message = 0;

byte message[MESSAGESIZE];


byte target_id;
byte checksum;
byte blank;
byte d0;
byte d1;
byte d2; 
byte d3;
byte d4;
byte d5;
byte d6;

void loop()
{
  setMode(RX);
  
  if (Serial.available() >= 10)
  {
    Serial.readBytes(message, MESSAGESIZE);

    target_id = message[0];
    checksum = message[1];
    blank = message[2];
    d0 = message[3];
    d1 = message[4];
    d2 = message[5];
    d3 = message[6];
    d4 = message[7];
    d5 = message[8];
    d6 = message[9];
    
    if (my_id == target_id)
    {
      analogWrite(led_PIN, d0);
      new_message = 1;
    }
    else
    {
      clearBuffer();
    }    
  }

  if (new_message == 1)
  {
    
    delay(45);
    
    setMode(TX); // enable tx mode on the MAX485
    
    button_state = digitalRead(button_PIN);   
    pot_state = map(analogRead(0), 0, 1023, 0, 255); 
    
    Serial.write(master_id);    // target_id
    Serial.write(0);            // checksum
    Serial.write(0);            // blank

    Serial.write(my_id);        // d0
    Serial.write(button_state); // d1
    Serial.write(pot_state);    // d2
    Serial.write(0);            // d3
    Serial.write(0);            // d4
    Serial.write(0);            // d5
    Serial.write(0);            // d6
    
    // block until the data has been all written out
    Serial.flush();
    
    new_message = 0; 
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

