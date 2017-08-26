#ifndef __CONFIG_H_
#define __CONFIG_H_

// Note temperatures are *2, so 0 = 0C, 1 = 2C, 100 = 200C, 255 = 510C (Maximum possible temp, allows a wide temp range, stored in a byte)

#define TEMPERATURE_OFFSET                    150  // To allow temperature to be saved in 8-bits (0-255)
#define BAKE_TEMPERATURE_STEP                 5    // Allows the storing of the temperature range in one byte
#define BAKE_MAX_DURATION                     176  // 176 = 18 hours (see getBakeSeconds)
#define BAKE_MIN_TEMPERATURE                  40   // Minimum temperature for baking
#define BAKE_MAX_TEMPERATURE                  200  // Maximum temperature for baking

#if 0
enum Element_Type_t
{
  // Output type - NON Heating Relay Outputs first
  RELAY_UNUSED,                                   // Unused Relay Output
  RELAY_CONVECTION_FAN,                           // Relay Output for Convection Fan
  RELAY_COOLING_FAN,                              // Relay Output for Cooling Fan
  // Heating Elements Last
  RELAY_BOTTOM_ELEMENT,                           // Relay Output for Bottom Element
  RELAY_BOOST_ELEMENT,                            // Relay Output for Boost Element
  RELAY_TOP_ELEMENT                               // Relay Output for Top Element
};
#endif

#define isHeatingElementX(x) (x >= RELAY_BOTTOM_ELEMENT)

extern const PROGMEM char listRelayType[];


// Global Configuration
enum SG_Entries_t
{
  SG_D4_TYPE,                           // Relay D4 Setting (Heating Element, Fan Type or unused)
  SG_D5_TYPE,                           // Relay D5 Setting (Heating Element, Fan Type or unused)
  SG_D6_TYPE,                           // Relay D6 Setting (Heating Element, Fan Type or unused)
  SG_D7_TYPE,                           // Relay D7 Setting (Heating Element, Fan Type or unused)

  SG_D4_MAXPWR,                         // Maximum Power allowed on Relay D4, target Power is scaled to maximum power. (Percentage 0-100)
  SG_D5_MAXPWR,                         // Maximum Power allowed on Relay D5, target Power is scaled to maximum power. (Percentage 0-100)
  SG_D6_MAXPWR,                         // Maximum Power allowed on Relay D6, target Power is scaled to maximum power. (Percentage 0-100)
  SG_D7_MAXPWR,                         // Maximum Power allowed on Relay D7, target Power is scaled to maximum power. (Percentage 0-100)

  SG_COOL_TEMPERATURE,                  // Temperature at which Oven considered COOL.
  SG_MAX_TEMPERATURE,                   // Maximum oven temperature.  No Operating Temperature will be able to be set above this.
  SG_OVER_TEMPERATURE,                  // Oven Over temperature.  Causes all Relays to be immediately disabled and any heating mode aborted.

  SG_SERVO_RETRACT_DEG,                 // Minimum Servo Value (Degrees, 0-180) (Retracted position)
  SG_SERVO_ARMED_DEG,                   // Servo point just prior to opening door. (Degrees, 0-180) (Armed position)
  SG_SERVO_OPEN_DEG,                    // Servo point, door open Fully.           (Degrees, 0-180) (MAX Open position)
  SG_SERVO_OPEN_TIME,                   // Time to open door                       (10ths of a second)
  
  SG_CHECK_VALUE,                       // Check if Global Settings are correct - Always Last Element
};

typedef struct Global_Settings_t {
  uint8_t& operator[](int i) { return byte[i]; }
  uint8_t  byte[SG_CHECK_VALUE+1];
} Global_Settings_t;

enum Mode_Type_t
{
  // Modes, Only 2 Modes, with 2 States, Reflow/Bake, Learn needed or not.
  UNUSED,                                         // Unused Mode
  REFLOW_LEARN,                                   // Reflow, and Learning Needed
  REFLOW,                                         // Reflow, Ready to USE.
  BAKE_LEARN,                                     // Bake, and Learning Needed
  BAKE,                                           // Bake, Ready to USE.
};

// Reflow Configuration
enum SR_Entries_t {
  SR_TYPE,                              // BAKE or REFLOW setting. (Settings are overlaid in EEPROM, gives best flexibility and EEPROM reuse)
  
  SR_NAME0,                             // First Byte of Name of Entry
  SR_NAME1,                             // Next  Byte of Name of Entry
  SR_NAME2,                             // Next  Byte of Name of Entry
  SR_NAME3,                             // Next  Byte of Name of Entry
  SR_NAME4,                             // Next  Byte of Name of Entry
  SR_NAME_END,                          // Last  Byte of Name of Entry

  SR_PRESOAK_MAXTEMP,                   // Maximum Temparature of Presoak  (J-STD-20 = 150C) (ControlLeo used MaxTemp * 3/5)
  SR_PRESOAK_MINTIME,                   // Minimum Time of Cycle (seconds) (J-STD-20 = 60)
  SR_PRESOAK_MAXTIME,                   // Maximum Time of Cycle (seconds) (J-STD-20 = 110)
  SR_PRESOAK_CONVFAN_DUTY_CYCLE,        // Duty cycle (0-100) that Convection Fan     must be used during presoak
  SR_PRESOAK_BOTTOM_DUTY_CYCLE,         // Duty cycle (0-100) that Bottom     Element must be used during presoak
  SR_PRESOAK_BOOST_DUTY_CYCLE,          // Duty cycle (0-100) that Boost      Element must be used during presoak
  SR_PRESOAK_TOP_DUTY_CYCLE,            // Duty cycle (0-100) that Top        Element must be used during presoak

  SR_SOAK_MAXTEMP,                      // Maximum Temparature of Presoak  (J-STD-20 = 200C) (ControlLeo used MaxTemp * 4/5)
  SR_SOAK_MINTIME,                      // Minimum Time of Cycle (seconds) (J-STD-20 = 80)
  SR_SOAK_MAXTIME,                      // Maximum Time of Cycle (seconds) (J-STD-20 = 140)
  SR_SOAK_CONVFAN_DUTY_CYCLE,           // Duty cycle (0-100) that Convection Fan     must be used during soak
  SR_SOAK_BOTTOM_DUTY_CYCLE,            // Duty cycle (0-100) that Bottom     Element must be used during soak
  SR_SOAK_BOOST_DUTY_CYCLE,             // Duty cycle (0-100) that Boost      Element must be used during soak
  SR_SOAK_TOP_DUTY_CYCLE,               // Duty cycle (0-100) that Top        Element must be used during soak

  SR_REFLOW_MAXTEMP,                    // Maximum Temparature of Presoak  (J-STD-20 = 240C) (ControlLeo default is 240C)
  SR_REFLOW_MINTIME,                    // Minimum Time of Cycle (seconds) (J-STD-20 = 60)
  SR_REFLOW_MAXTIME,                    // Maximum Time of Cycle (seconds) (J-STD-20 = 100)
  SR_REFLOW_CONVFAN_DUTY_CYCLE,         // Duty cycle (0-100) that Convection Fan     must be used during reflow
  SR_REFLOW_BOTTOM_DUTY_CYCLE,          // Duty cycle (0-100) that Bottom     Element must be used during reflow
  SR_REFLOW_BOOST_DUTY_CYCLE,           // Duty cycle (0-100) that Boost      Element must be used during reflow
  SR_REFLOW_TOP_DUTY_CYCLE,             // Duty cycle (0-100) that Top        Element must be used during reflow

  SR_WAIT_MINTIME,                      // Time to wait in Liquidous state

  SR_COOL_FANSPEED,                     // Cooling Fan Speed (0 = Off, 100 = Fastest)
  SR_COOL_DOOROPEN,                     // Door Open Distance (0 = Closed, 100 = Maximum Open)
                                        // COOL temperature is defined globally.

  SR_CHECK_VALUE,                       // Check if Reflow Settings are correct - Always Last Element
};
            
// Bake Configuration
enum SB_Entries_t {
  SB_TYPE,                              // BAKE or REFLOW setting. (Settings are overlaid in EEPROM, gives best flexibility and EEPROM reuse)
  
  SB_NAME0,                             // First Byte of Name of Entry
  SB_NAME1,                             // Next  Byte of Name of Entry
  SB_NAME2,                             // Next  Byte of Name of Entry
  SB_NAME3,                             // Next  Byte of Name of Entry
  SB_NAME4,                             // Next  Byte of Name of Entry
  SB_NAME_END,                          // Last  Byte of Name of Entry

  SB_BAKE_TEMPERATURE_HI,               // High Byte of The baking temperature
  SB_BAKE_TEMPERATURE_LO,               // High Byte of The baking temperature
  SB_BAKE_DURATION_HI,                  // The baking duration Hi Byte (Seconds)
  SB_BAKE_DURATION_LO,                  // The baking duration Lo Byte (Seconds)

  SB_COOL_FANSPEED,                     // Cooling Fan Speed (0 = Off, 100 = Fastest)
  SB_COOL_DOOROPEN,                     // Door Open Distance (0 = Closed, 100 = Maximum Open)
                                        // COOL temperature is defined globally.
  SB_COOL_TEMP,                         // Temperature considered COOL.  COOL = the Min of This and the Global Value.

  SB_CHECK_VALUE,                       // Check if Bake Settings are correct - Always Last Element.  
  
};

// Mode (Reflow or Bake) Reflow is the biggest set of settings, so size accordingly.
typedef struct Mode_Settings_t {
  char& operator[](int i) { return byte[i]; }
  char byte[SR_CHECK_VALUE+1];
} Mode_Settings_t;

// Total Modes is (1024 Bytes - sizeof(Global Settings)) / sizeof(Mode Settings)
// Global Settings ~= 14 Bytes
// Mode Settings ~= 32 Bytes
// Therefore Total Modes ~= (1024 - 14) / 32 ~= 31 Maximum Reflow/Baking Modes.  Which is A LOT.

#endif
