#include <SFE_BMP180.h>
#include <Wire.h>
#include "CHAMP_BMP.h"

double starting_altitude = 147;

CHAMP_BMP bmp(starting_altitude);

void setup()
{
  Serial.begin(9600);
  Serial.println("REBOOT"); 
  
  bmp.init();
}

void loop()
{
  Serial.print("Temp: "); 
  double tmp = bmp.getTemp();
  Serial.print(tmp);

  Serial.print(" Pressure: "); 
  double pres = bmp.getPres();
  Serial.print(pres);

  Serial.print(" Altitude: "); 
  double alt = bmp.getAlti();
  Serial.print(alt);

  Serial.println("");
}
