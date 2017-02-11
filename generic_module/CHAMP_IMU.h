/*
 * CHAMP_IMU.h
 * A wrapper for making interfacing with the LSM9DS1 IMU even easier than the provided library makes it
 * This is basically a library for a library for the LSM9DS1 originally written by Jim Lindblom, SparkFun Electronics - https://www.sparkfun.com/products/13284
 * Created By Devon Bray for CHAMP - champ.wpi.edu
 * Created: 9 February 2017
 * Last Updated: 10 February 2017
*/

#ifndef CHAMP_IMU_h
#define CHAMP_IMU_h

#include "Arduino.h"
#include "Wire.h"
#include "SPI.h"
#include "SparkFunLSM9DS1.h"


class CHAMP_IMU
{
  public:
    // members

    // methods
    CHAMP_IMU();                  // constructor
    boolean init();
    float getRoll();
    float getPitch();
    float getHeading();
    
  private:
    // members
    LSM9DS1 imu;
    float r;
    float p;
    float h;

    // methods
    void setReadings();
    void calculateAttitude();

};

#endif
