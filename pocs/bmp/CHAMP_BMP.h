/*
 * CHAMP_BMP.h
 * A wrapper for making interfacing with the BMP180 even easier than the provided library makes it
 * This is basically a library for a library for the BMP180 originally written by  Mike Grusin, SparkFun Electronics - https://www.sparkfun.com/products/11824
 * Created By Devon Bray for CHAMP - champ.wpi.edu, 7 February 2017
*/

#ifndef CHAMP_BMP_h
#define CHAMP_BMP_h

#include "Arduino.h"
#include "SFE_BMP180.h"

class CHAMP_BMP
{
  public:
    
    CHAMP_BMP(double sa);     // constructor

    void init();

    double starting_altitude;

    double getTemp();       // get temperature in celcius
    double getPres();       // get pressure in millibar (mb) aka hectopascals (hPa).
    double getAlti();       // get altitude in meters

    void setInitialAltitude();
    double getAltitudeDelta();

  private:
    
    SFE_BMP180 pressure; 
    
    double T;
    double P;
    double p0;
    double a;

    double last_altitude;

    double getSeaL();
    void setReadings();
};

#endif
