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

/*******************************************************************************
* Name: getFaultStr
* Description: get Temperature Reading Fault as a short string (5 Characters).
*
* Return      Description
* =========   ===========
* fault       None, Empty String
*             No Thermocouple = "MISNG"
*             Short to GND    = "SHT-G"
*             Short to VCC    = "SHT-V"
*             OverTemp        = "OVERT"
*             Unknown         = "?ERR?"
*
*******************************************************************************/
const __FlashStringHelper* ControLeo2_MAX31855::getFaultStr(void)
{
    if      (_fault == FAULT_NONE)      return FM("");
    else if (_fault == FAULT_OPEN)      return FM("MISNG");
    else if (_fault == FAULT_SHORT_GND) return FM("SHT-G");
    else if (_fault == FAULT_OVERTEMP)  return FM("OVERT");
    return FM("?ERR?");
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
    int            bitCount;
    int16_t        temp;
    uint32_t       current_time = micros();
    static uint8_t fault_count = 0;
  
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
                if (_fault == 0x00) { // Dont store temps when a fault occurs
                  if (temp <= (MAX_TEMPERATURE*4)) {
                      _RawTemp[_nexttemp] = temp;  
                  } else {
                    _fault = FAULT_OVERTEMP;
                  }
                }                  
                temp = 0;
            } else if (bitCount == 0) {
                if (_fault == 0x00) { // Dont store temps when a fault occurs
                  _RawJunctionTemp[_nexttemp] = temp;    
                }
            }
            
            digitalWrite(THERMOCOUPLE_CLK_PIN, LOW);
        }

        if (_fault != 0x00) {
            if (fault_count < 4) {
              _fault = 0x00; // Only record the fault
              fault_count++;
            } else if (fault_count == 5) {
              _RawTemp[_nexttemp] = (MAX_TEMPERATURE*4) + 1;
              _nexttemp = (_nexttemp+1) & 0x3; // Step through readings arrays.
              fault_count++;
            }
        } else {
          fault_count = 0;

          // Linearise the temperature recorded.
          // _RawTemp[_nexttemp] = LinearizeThermocouple(_RawTemp[_nexttemp], _RawJunctionTemp[_nexttemp]); 
          
          _nexttemp = (_nexttemp+1) & 0x3; // Step through readings arrays.
        }
        
        _Tdrift = _calcDrift(_RawTemp);
        _Jdrift = _calcDrift(_RawJunctionTemp);
        
        // Deselect MAX31855 chip
        digitalWrite(THERMOCOUPLE_CS_PIN, HIGH);
    }
}

int16_t LinearizeThermocouple(int16_t temp, int16_t juncTemp) {
  double internalTemp = juncTemp / 4.0;
  double rawTemp = temp / 4.0;
  double thermocoupleVoltage= 0;
  double internalVoltage = 0;
  double correctedTemp = 0;
  double totalVoltage = 0;
  uint8_t i;

  // Steps 1 & 2. Subtract cold junction temperature from the raw thermocouple temperature.
  thermocoupleVoltage = (rawTemp - internalTemp) * 0.041276;  // C * mv/C = mV

  // Step 3. Calculate the cold junction equivalent thermocouple voltage.
  // Coefficients and equations available from http://srdata.nist.gov/its90/download/type_k.tab
  
  static double c[] = {-0.176004136860E-01,  0.389212049750E-01,  0.185587700320E-04, 
                       -0.994575928740E-07,  0.318409457190E-09, -0.560728448890E-12,  
                        0.560750590590E-15, -0.320207200030E-18,  0.971511471520E-22, 
                       -0.121047212750E-25};

  // Count the the number of coefficients. There are 10 coefficients for positive temperatures (plus three exponential coefficients),
  // but there are 11 coefficients for negative temperatures.
  const int cLength = sizeof(c) / sizeof(c[0]);

  // Exponential coefficients. Only used for positive temperatures.
  const double a0 =  0.118597600000E+00;
  const double a1 = -0.118343200000E-03;
  const double a2 =  0.126968600000E+03;

  // From NIST: E = sum(i=0 to n) c_i t^i + a0 exp(a1 (t - a2)^2), where E is the thermocouple voltage in mV and t is the temperature in degrees C.
  // In this case, E is the cold junction equivalent thermocouple voltage.
  // Alternative form: C0 + C1*internalTemp + C2*internalTemp^2 + C3*internalTemp^3 + ... + C10*internaltemp^10 + A0*e^(A1*(internalTemp - A2)^2)
  // This loop sums up the c_i t^i components.
  for (i = 0; i < cLength; i++) {
    internalVoltage += c[i] * pow(internalTemp, i);
  }
  // This section adds the a0 exp(a1 (t - a2)^2) components.
  internalVoltage += a0 * exp(a1 * pow((internalTemp - a2), 2));

  // Step 4. Add the cold junction equivalent thermocouple voltage calculated in step 3 to the thermocouple voltage calculated in step 2.
  totalVoltage = thermocoupleVoltage + internalVoltage;

  if ((totalVoltage > 0) && (totalVoltage < 20.644)) { // Temperature is between 0C and 500C.
    static double d[] = { 0.000000E+00,  2.508355E+01,  7.860106E-02, 
                         -2.503131E-01,  8.315270E-02, -1.228034E-02, 
                          9.804036E-04, -4.413030E-05,  1.057734E-06, 
                         -1.052755E-08};                  
    const int dLength = sizeof(d) / sizeof(d[0]);
    
    for (i = 0; i < dLength; i++) {
      correctedTemp += d[i] * pow(totalVoltage, i);
    }
    correctedTemp *= 4.0; // integer temps have 2 binary digits places.
  } else {
    correctedTemp = temp; // Couldn't convert, so just use original temp.
  }

  return (int16_t)correctedTemp;

}
 

