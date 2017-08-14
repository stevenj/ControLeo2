// This library is derived from RocketScream.com's I2C MAX31855 library.
// Please support them from buying products from their web site.
// https://github.com/rocketscream/MAX31855/blob/master/MAX31855.cpp
//
// This is a library for the MAX31855 thermocouple IC used on ControLeo
// The pin connections are as follows:
// MISO - D8
// SCK  - D9
// CS   - D10
//
// Written by Peter Easton
// Released under WTFPL license
//
// Change History:
// 14 August 2014        Initial Version
// 01 August 2017        Steven Johnson - Heavily Modified
//                       Modified to maintain state and periodically refresh.
//                       Temp readings are averaged over the last 4 readings to improve
//                       read to read error, and also track temp direction.

#include "ControLeo2.h"

#define DRIFT_UP     (0x0A)
#define DRIFT_DOWN   (0x0B)
#define DRIFT_STABLE (0x0C)

#define THERMOCOUPLE_CONVERSION_RATE (250000) // 250 ms (4 per second. Max rate is 100ms/10 per second)


ControLeo2_MAX31855::ControLeo2_MAX31855(void)
{
  // MAX31855 data output pin
  pinMode(THERMOCOUPLE_MISO_PIN, INPUT);
  // MAX31855 chip select input pin
  pinMode(THERMOCOUPLE_CS_PIN, OUTPUT);
  // MAX31855 clock input pin
  pinMode(THERMOCOUPLE_CLK_PIN, OUTPUT);
 
  // Default output pins state
  digitalWrite(THERMOCOUPLE_CS_PIN, HIGH);
  digitalWrite(THERMOCOUPLE_CLK_PIN, LOW);

  memset(_RawTemp,0x00,sizeof(_RawTemp));
  memset(_RawJunctionTemp,0x00,sizeof(_RawJunctionTemp));
  _nexttemp = 0;
  _last_temp_time = 0;
  _Tdrift = DRIFT_STABLE;
  _Jdrift = DRIFT_STABLE;

}

/*******************************************************************************
* Name: readThermocouple
* Description: Return the Thermocouple Temperature
*
*
* Return      Description
* =========   ===========
* temperature To either Degrees C, 0.5 Degree or 0.25 Degree
*
*******************************************************************************/
int16_t  ControLeo2_MAX31855::readThermocouple(uint8_t places)
{
    return _readTemp(_RawTemp, 2-places);     
}

/*******************************************************************************
* Name: readThermocoupleDrift
* Description: Return the Temperature Drift.
* Plus or minus 0.25 of a degree is considered stable.
*
*
* Return      Description
* =========   ===========
* drift       Temp Drift (0x0A = UP, 0x0B = Down, 0x0C = Stable)
*
*******************************************************************************/
uint8_t ControLeo2_MAX31855::readThermocoupleDrift(void)
{
    return _Tdrift;
}

/*******************************************************************************
* Name: readJunction
* Description: Return the Junction Temperature
*
*
* Return      Description
* =========   ===========
* temperature To either Degrees C, 0.5 Degree or 0.25 Degree
*
*******************************************************************************/
int16_t  ControLeo2_MAX31855::readJunction(uint8_t places)
{
    return _readTemp(_RawJunctionTemp, 2-places);    
}

/*******************************************************************************
* Name: readJunctionDrift
* Description: Return the Junction Temperature Drift.
* Plus or minus 0.25 of a degree is considered stable.
*
*
* Return      Description
* =========   ===========
* drift       Temp Drift (0x0A = UP, 0x0B = Down, 0x0C = Stable)
*
*******************************************************************************/
uint8_t ControLeo2_MAX31855::readJunctionDrift(void)
{
    return _Jdrift;
}

/*******************************************************************************
* Name: getFault
* Description: get Temperature Reading Fault.
*
* Return      Description
* =========   ===========
* fault       0x00 = No Fault
*             0x41 = No Thermocouple
*             0x42 = Short to GND
*             0x43 = Short to VCC
*             Anything Else = Unknown Fault.
*
*******************************************************************************/
uint8_t ControLeo2_MAX31855::getFault(void)
{
    return _fault;
}

// Read and average a temperature Array.
int16_t  ControLeo2_MAX31855::_readTemp(int16_t Temps[],uint8_t shift)
{
    int16_t total = 0;
    total = Temps[0] + Temps[1] + Temps[2] + Temps[3];
    total = total >> (2+shift);        

    return total;
}

// Check Direction of Temperature movement. (+/- 0.5 Degree is Stable)
uint8_t ControLeo2_MAX31855::_calcDrift(int16_t Temps[])
{
    uint8_t newest = (_nexttemp - 1) & 0x3; // Newest current temp reading.

    if (Temps[newest] > (Temps[_nexttemp]+2)) {
      return DRIFT_UP;
    } else if (Temps[newest] < (Temps[_nexttemp]-2)) {
      return DRIFT_DOWN;
    }
    return DRIFT_STABLE;
}

/*******************************************************************************
* Name: RefreshTemps
* Description:  Shift in 32-bit of data from MAX31855 chip. Store Temps and 
*               Fault flags.  
*               Minimum clock width is 100 ns. No delay is required in this case.
*******************************************************************************/
void ControLeo2_MAX31855::RefreshTemps(void) {
    int     bitCount;
    int16_t temp;
    uint32_t current_time = micros();
  
    if ((current_time - _last_temp_time) >= THERMOCOUPLE_CONVERSION_RATE) {
        _last_temp_time = current_time;
        
        // Clear data 
        _fault = 0;        
        temp = 0;

        // Select the MAX31855 chip
        digitalWrite(THERMOCOUPLE_CS_PIN, LOW);
        
        // Shift in Data
        for (bitCount = 31; bitCount >= 0; bitCount--)
        {
            digitalWrite(THERMOCOUPLE_CLK_PIN, HIGH);
            
            // If data bit is high
            if (digitalRead(THERMOCOUPLE_MISO_PIN))
            {
                if (bitCount == 31) {
                    temp = 0xE000;                  // Sign extend
                } else if (bitCount >= 18) {
                    temp |= (1 << (bitCount-18));   // 14 Bit Signed number in a 16 bit qty.
                } else if (bitCount >= 16) {
                    _fault |= (1 << (bitCount-10)); // Reserved bit Plus Fault bit.
                } else if (bitCount == 15) {
                    temp = 0xFE00;                  // Sign extend
                } else if (bitCount >= 6) {
                    temp |= (1 << (bitCount-6));    // 10 Bit Signed number in a 16 bit qty.
                } else if (bitCount >= 4) {
                    // Ignore bits;                 // Ignore Precision less than 0.25 degrees in junction temp.
                } else {
                    _fault |= (1 << bitCount);      // Fault bits
                }
            }

            // Save Temps once fully assembled.
            if (bitCount == 16) {
                _RawTemp[_nexttemp] = temp;  
                temp = 0;
            } else if (bitCount == 0) {
                _RawJunctionTemp[_nexttemp] = temp;    
            }
            
            digitalWrite(THERMOCOUPLE_CLK_PIN, LOW);
        }
        
        _nexttemp = (_nexttemp+1) & 0x3; // Step through readings arrays.
        _Tdrift = _calcDrift(_RawTemp);
        _Jdrift = _calcDrift(_RawJunctionTemp);
        
        // Deselect MAX31855 chip
        digitalWrite(THERMOCOUPLE_CS_PIN, HIGH);
    }
}
