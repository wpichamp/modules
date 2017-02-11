/*
 * CHAMP_IMU.h
 * A wrapper for making interfacing with the LSM9DS1 IMU even easier than the provided library makes it
 * This is basically a library for a library for the LSM9DS1 originally written by Jim Lindblom, SparkFun Electronics - https://www.sparkfun.com/products/13284
 * Created By Devon Bray for CHAMP - champ.wpi.edu
 * Created: 9 February 2017
 * Last Updated: 10 February 2017
*/

#define LSM9DS1_M 0x1E // Would be 0x1C if SDO_M is LOW
#define LSM9DS1_AG  0x6B // Would be 0x6A if SDO_AG is LOW

// Earth's magnetic field varies by location. Add or subtract 
// a declination to get a more accurate heading. Calculate 
// your's here:
// http://www.ngdc.noaa.gov/geomag-web/#declination
#define DECLINATION -8.58 // Declination (degrees) in Boulder, CO.

#include "Arduino.h"
#include "Wire.h"
#include "SPI.h"

#include "CHAMP_IMU.h"
#include "SparkFunLSM9DS1.h"

/* Constructor */ 
CHAMP_IMU::CHAMP_IMU()
{
  
}

// public method
boolean CHAMP_IMU::init()
{
  imu.settings.device.commInterface = IMU_MODE_I2C;
  imu.settings.device.mAddress = LSM9DS1_M;
  imu.settings.device.agAddress = LSM9DS1_AG;
  return imu.begin();
}

// public method
float CHAMP_IMU::getRoll()
{
  calculateAttitude();
  return r;
}

// public method
float CHAMP_IMU::getPitch()
{
  calculateAttitude();
  return p;
}

// public method
float CHAMP_IMU::getHeading()
{
  calculateAttitude();
  return h;
}

// private method
void CHAMP_IMU::setReadings()
{
  // Update the sensor values whenever new data is available
  if ( imu.gyroAvailable() )
  {
    imu.readGyro();
  }
  if ( imu.accelAvailable() )
  {
    imu.readAccel();
  }
  if ( imu.magAvailable() )
  {
    imu.readMag();
  }
}

// FROM THE SFE LIB
// Calculate pitch, roll, and heading.
// Pitch/roll calculations take from this app note:
// http://cache.freescale.com/files/sensors/doc/app_note/AN3461.pdf?fpsp=1
// Heading calculations taken from this app note:
// http://www51.honeywell.com/aero/common/documents/myaerospacecatalog-documents/Defense_Brochures-documents/Magnetic__Literature_Application_notes-documents/AN203_Compass_Heading_Using_Magnetometers.pdf

// private method
void CHAMP_IMU::calculateAttitude()
{
  setReadings();

  float ax = imu.ax;
  float ay = imu.ay;
  float az = imu.az;
  
  float mx = -imu.my; // why god why did sparkfun do this
  float my = -imu.mx; // why was this swapped
  float mz = imu.mz;
    
  float roll = atan2(ay, az);
  float pitch = atan2(-ax, sqrt(ay * ay + az * az)); 
  float heading;
  
  if (my == 0)
    heading = (mx < 0) ? PI : 0;
  else
    heading = atan2(mx, my);
    
  heading -= DECLINATION * PI / 180;
  
  if (heading > PI) heading -= (2 * PI);
  else if (heading < -PI) heading += (2 * PI);
  else if (heading < 0) heading += 2 * PI;
  
  // Convert everything from radians to degrees:
  heading *= 180.0 / PI;
  pitch *= 180.0 / PI;
  roll  *= 180.0 / PI;

  h = heading;
  r = roll; 
  p = pitch;
  
}




