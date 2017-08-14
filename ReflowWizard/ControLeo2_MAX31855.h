// Written by Peter Easton
// Released under WTFPL license
//
// Change History:
// 14 August 2014        Initial Version
//
// 1 August 2017         Steven Johnson - heavily modified
// Modified to run periodically and average samples
// Modified to only use integer values

#ifndef CONTROLEO2_MAX31855_H
#define CONTROLEO2_MAX31855_H

#include "Arduino.h"

#define FAULT_NONE      0x00
#define FAULT_OPEN      0x41
#define FAULT_SHORT_GND 0x42
#define FAULT_SHORT_VCC 0x44
#define FAULT_UNKNOWN   0x4F

class ControLeo2_MAX31855
{
public:
    ControLeo2_MAX31855(void);

    void RefreshTemps(void);
  
    int16_t  readThermocouple(uint8_t places);
    uint8_t  readThermocoupleDrift(void);
    
    int16_t  readJunction(uint8_t places);
    uint8_t  readJunctionDrift(void);

    uint8_t  getFault(void);
    
private:
    int16_t _RawTemp[4];
    int16_t _RawJunctionTemp[4];
    uint8_t _nexttemp;
    uint8_t _fault;
    uint8_t _Tdrift;
    uint8_t _Jdrift;

    uint32_t _last_temp_time;
    
    unsigned long readData();

    int16_t  _readTemp(int16_t Temps[], uint8_t places);
    
    uint8_t  _calcDrift(int16_t Temps[]);
};
#endif  // CONTROLEO2_MAX31855_H
