/*
 * CHAMP_BMP.cpp
 * A wrapper for making interfacing with the BMP180 Barometric Pressure Sensor even easier than the provided library makes it
 * This is basically a library for a library for the BMP180 originally written by  Mike Grusin, SparkFun Electronics - https://www.sparkfun.com/products/11824
 * Created By Devon Bray for CHAMP - champ.wpi.edu, 7 February 2017
*/

#include "Arduino.h"
#include "CHAMP_BMP.h"
#include "SFE_BMP180.h"

#define ALTITUDE 144 // Altitude the CHAMP Lab in Meters

/* Constructor */ 
CHAMP_BMP::CHAMP_BMP(double sa)
{
  starting_altitude = sa;
}

/* Public Methods */

// public method
// init method, to be called before the sensor is to be used
boolean CHAMP_BMP::init()
{
  boolean beginStatus = pressure.begin();
  setInitialAltitude();
  return beginStatus;
}

// public method
void CHAMP_BMP::setInitialAltitude()
{
  last_altitude = getAlti();
}

// public method
double CHAMP_BMP::getAltitudeDelta()
{
  double current_altitude = getAlti();
  double delta = current_altitude - last_altitude;

  last_altitude = current_altitude;
  
  return delta;
}

// public method
double CHAMP_BMP::getTemp()
{
  setReadings();
  return T;
}

// public method
double CHAMP_BMP::getPres()
{
  setReadings();
  return P;
}

// public method
double CHAMP_BMP::getAlti()
{
  setReadings();
  return a;
}

/* Private Methods */

// private method
double CHAMP_BMP::getSeaL()
{
  setReadings();
  return p0;
}

// private method
void CHAMP_BMP::setReadings()
{

  // The data read in for this sensor must be done sequentially, as many of the calcualtions are based on one another. 
  
  char status;
  
  status = pressure.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:
    delay(status);

    // Retrieve the completed temperature measurement:
    // Note that the measurement is stored in the variable T.
    // Function returns 1 if successful, 0 if failure.

    status = pressure.getTemperature(T);
    if (status != 0)
    {
      
      status = pressure.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status);
        
        status = pressure.getPressure(P,T);
        if (status != 0)
        {
          p0 = pressure.sealevel(P,ALTITUDE); // we're at 1655 meters (Boulder, CO)
          a = pressure.altitude(P,p0);
        }
      }
    }
  }
}



