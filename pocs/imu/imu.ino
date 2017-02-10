#include "CHAMP_IMU.h"

CHAMP_IMU imu;

void setup() 
{
  Serial.begin(115200);
  imu.init();
}

void loop()
{
    float roll = imu.getRoll();
    float pitch = imu.getPitch();
    float heading = imu.getHeading();

    Serial.print("Roll: ");
    Serial.print(roll);
    Serial.print(" Pitch: ");
    Serial.print(pitch);
    Serial.print(" Heading: ");
    Serial.print(heading);

    Serial.println();
    
}

