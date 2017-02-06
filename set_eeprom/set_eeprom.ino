#include <EEPROM.h>

int id = 1; // set id to the value you want to be stored in the eeprom

void setup()
{
  Serial.begin(115200);
  Serial.print("Writing To EEProm..");
  EEPROM.write(0, id);
  Serial.println("Complete");
}

void loop()
{
  Serial.print("Testing Read of EEProm: ");
  byte b =  EEPROM.read(0);
  Serial.println(b);
  delay(100);
}

