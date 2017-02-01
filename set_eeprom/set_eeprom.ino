#include <EEPROM.h>

int id = 1;

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

