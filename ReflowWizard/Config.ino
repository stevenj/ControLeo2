// Configuration Data Handler
// We have a memory configuration and a EEProm one.
// The Memory one is active, the EEProm one is read at start, and saved when changed (committed).
#include "Config.h"
#include <avr/eeprom.h>
#include "Relays.h"

#define EEPROM_START (0)
#define GLOBAL_CONFIG_START (EEPROM_START)
#define GLOBAL_CONFIG_SIZE  (sizeof(Global_Settings_t))
#define MODE_CONFIG_START   (32)
#define MODE_CONFIG_SIZE    (sizeof(Mode_Settings_t))
#define MAX_MODES           (16)

Global_Settings_t GlobalSettings;
uint8_t           CurrentMode;

// Menu List to match Relay Setting Enum.  
const PROGMEM char listRelayType[] = "Unused|Fan:Conv|Fan:Cool|E:Bottom|E:Boost|E:Top";

const PROGMEM Global_Settings_t DefaultGlobalSettings = {
  ControLeo2_Relays::RELAY_UNUSED, // SG_D4_TYPE - Default to Unused because there are no safe assumptions about what they could be connected to.
  ControLeo2_Relays::RELAY_UNUSED, // SG_D5_TYPE
  ControLeo2_Relays::RELAY_UNUSED, // SG_D6_TYPE
  ControLeo2_Relays::RELAY_UNUSED, // SG_D7_TYPE

  100, // 0,                               // SG_D4_MAXPWR (0%)
  100,  // 0,                               // SG_D5_MAXPWR (0%)
  100,  // 0,                               // SG_D6_MAXPWR (0%)
  100,  // 0,                               // SG_D7_MAXPWR (0%)

  (50/2),       // SG_COOL_TEMPERATURE (50 degrees C)
  (250/2),      // SG_MAX_TEMPERATURE  (250 degrees C)
  (280/2),      // SG_OVER_TEMPERATURE (280 degrees C)

  45,           // SG_SERVO_RETRACT_DEG (45 Degree)
  90,           // SG_SERVO_ARMED_DEG   (90 Degree)
  135,          // SG_SERVO_OPEN_DEG    (135 Degree)
  15,           // SG_SERVO_OPEN_TIME   (1.5 Seconds)
  
  0xFF,         // Check if Global Settings are correct - Always Last Element  
};



bool CheckConfig(uint16_t start, uint8_t size, uint8_t &check_value) {
  // Calculate and verify a check byte in a range of eeprom.
  // start = eeprom start address
  // size  = size of eeprom data (Can only be 2-255 bytes big)  (last byte = check digit)
  // check_value is returned with the calculated check value (regardless of if the check is correct or not)
  // return true if the buffer checks OK, false otherwise.

  check_value = 0xA5; // Starting Value
  size = size - 1;    // Remove check digit from data
  while (size > 0) {
    check_value ^= eeprom_read_byte((const uint8_t *)start);
    start++;
    size--;
  }
  // Safety check, blank state is 0xFF, to ensure that a 0xFF result is made into something not 0xFF.
  if (check_value == 0xFF) check_value = 0x5A; 
  
  return (check_value == eeprom_read_byte((const uint8_t *)start));
}

void ReadGlobalConfig(void) {
  uint8_t check_value;

  // If Global Config reloaded, always reset current Mode to first Mode
  CurrentMode = 0;
  
  if (CheckConfig(GLOBAL_CONFIG_START, GLOBAL_CONFIG_SIZE, check_value)) {
    // Read config from flash
    eeprom_read_block (&GlobalSettings,
                       (void *)GLOBAL_CONFIG_START,
                       GLOBAL_CONFIG_SIZE);
  } else {
    // Default Config
    memcpy_P( &GlobalSettings,
              &DefaultGlobalSettings,
              GLOBAL_CONFIG_SIZE);
  }
  
}

uint16_t readGlobalSetting(SG_Entries_t entry) {
  uint16_t value = GlobalSettings[entry];
  
  // Check temp entries, and return actual value.
  if ((entry == SG_COOL_TEMPERATURE) ||
      (entry == SG_MAX_TEMPERATURE)  ||
      (entry == SG_OVER_TEMPERATURE)) {
    value *= 2;
  } 
  
  return value;
}

void writeGlobalSetting(SG_Entries_t entry, uint16_t value) {
  // Check temp entries, and store corrected temp value (div 2).
  if ((entry == SG_COOL_TEMPERATURE) ||
      (entry == SG_MAX_TEMPERATURE)  ||
      (entry == SG_OVER_TEMPERATURE)) {
    value /= 2;
  }

  if (value != GlobalSettings[entry]) {
    GlobalSettings[entry] = (uint8_t)value;
    GlobalSettings[SG_CHECK_VALUE] = 0xFF; // Mark Global Settings as DIRTY.
  }
}


#if 0
// Setup menu
// Called from the main loop
// Allows the user to configure the outputs and maximum temperature

// Called when in setup mode
// Return false to exit this mode
boolean Config() {
  static int setupPhase = 0;
  static int output = 4;    // Start with output D4
  static int type = TYPE_TOP_ELEMENT;
  static boolean drawMenu = true;
  static int maxTemperature;
  static int servoDegrees;
  static int servoDegreesIncrement = 5;
  static int selectedServo = SETTING_SERVO_OPEN_DEGREES;
  static int bakeTemperature;
  static int bakeDuration;
  int oldSetupPhase = setupPhase;
  
  switch (setupPhase) {
    case 0:  // Set up the output types
      if (drawMenu) {
        drawMenu = false;
        lcd.PrintStr(0,0, FM("Dx is"));
        lcd.PrintInt(1,0,1,output);
        type = getSetting(SETTING_D4_TYPE - 4 + output);
        lcdPrintLine(1, outputDescription[type]);
      }
  
      // Was a button pressed?
      switch (buttons.GetKeypress()) {
        case BUTTON_TOP_RELEASE:
          // Move to the next type
          type = (type+1) % NO_OF_TYPES;
          lcdPrintLine(1, outputDescription[type]);
          break;
        case BUTTON_BOT_RELEASE:
          // Save the type for this output
          setSetting(SETTING_D4_TYPE - 4 + output, type);
          // Go to the next output
          output++;
          if (output != 8) {
            lcd.PrintInt(1,0,1,output);
            type = getSetting(SETTING_D4_TYPE - 4 + output);
            lcd.PrintStr(2, 0, outputDescription[type]);
            break;
          }
          
          // Go to the next phase.  Reset variables used in this phase
          setupPhase++;
          output = 4;
          break;
      }
      break;
      
    case 1:  // Get the maximum temperature
      if (drawMenu) {
        drawMenu = false;
        lcdPrintLine_P(0, PSTR("Max temperature"));
        lcdPrintLine_P(1, PSTR("xxx\1C"));
        maxTemperature = getSetting(SETTING_MAX_TEMPERATURE);
        displayMaxTemperature(maxTemperature);
      }
      
      // Was a button pressed?
      switch (buttons.GetKeypress()) {
        case BUTTON_TOP_RELEASE:
          // Increase the temperature
          maxTemperature++;
          if (maxTemperature > 280)
            maxTemperature = 175;
          displayMaxTemperature(maxTemperature);
          break;
        case BUTTON_BOT_RELEASE:
          // Save the temperature
          setSetting(SETTING_MAX_TEMPERATURE, maxTemperature);
          // Go to the next phase
          setupPhase++;
      }
      break;
    
    case 2:  // Get the servo open and closed settings
      if (drawMenu) {
        drawMenu = false;
        lcdPrintLine_P(0, PSTR("Door servo"));
        lcdPrintLine_P(1, selectedServo == SETTING_SERVO_OPEN_DEGREES? PSTR("open:") : PSTR("closed:"));
        servoDegrees = getSetting(selectedServo);
        displayServoDegrees(servoDegrees);
        // Move the servo to the saved position
        setServoPosition(servoDegrees, 1000);
      }
      
      // Was a button pressed?
      switch (buttons.GetKeypress()) {
        case BUTTON_TOP_RELEASE:
          // Should the servo increment change direction?
          if (servoDegrees >= 180)
            servoDegreesIncrement = -5;
          if (servoDegrees == 0)
            servoDegreesIncrement = 5;
          // Change the servo angle
          servoDegrees += servoDegreesIncrement;
          // Move the servo to this new position
          setServoPosition(servoDegrees, 200);
          displayServoDegrees(servoDegrees);
          break;
        case BUTTON_BOT_RELEASE:
          // Save the servo position
          setSetting(selectedServo, servoDegrees);
          // Go to the next phase.  Reset variables used in this phase
          if (selectedServo == SETTING_SERVO_OPEN_DEGREES) {
            selectedServo = SETTING_SERVO_CLOSED_DEGREES;
            drawMenu = true;
          }
          else {
            selectedServo = SETTING_SERVO_OPEN_DEGREES;
            // Go to the next phase
            setupPhase++;
          }
      }
      break;

    case 3:  // Get bake temperature
      if (drawMenu) {
        drawMenu = false;
        lcdPrintLine_P(0, PSTR("Bake temperature"));
        lcdPrintLine_P(1, PSTR(""));
        bakeTemperature = getSetting(SETTING_BAKE_TEMPERATURE);
        lcd.PrintInt(0,1,3,bakeTemperature);
        lcd.PrintStr(3,1,"\1C ");
      }

      // Was a button pressed?
      switch (buttons.GetKeypress()) {
        case BUTTON_TOP_RELEASE:
          // Increase the temperature
          bakeTemperature += BAKE_TEMPERATURE_STEP;
          if (bakeTemperature > BAKE_MAX_TEMPERATURE)
            bakeTemperature = BAKE_MIN_TEMPERATURE;
          lcd.PrintInt(0,1,3,bakeTemperature);
          lcd.PrintStr(3,1,"\1C ");
          break;
        case BUTTON_BOT_RELEASE:
          // Save the temperature
          setSetting(SETTING_BAKE_TEMPERATURE, bakeTemperature);
          // Go to the next phase
          setupPhase++;
      }
      break;

    case 4:  // Get bake duration
      if (drawMenu) {
        drawMenu = false;
        lcdPrintLine_P(0, PSTR("Bake duration"));
        lcdPrintLine_P(1, PSTR(""));
        bakeDuration = getSetting(SETTING_BAKE_DURATION);
        displayDuration(0, getBakeSeconds(bakeDuration));
      }

      // Was a button pressed?
      switch (buttons.GetKeypress()) {
        case BUTTON_TOP_RELEASE:
          // Increase the duration
          bakeDuration++;
          bakeDuration %= BAKE_MAX_DURATION;
          displayDuration(0, getBakeSeconds(bakeDuration));
          break;
        case BUTTON_BOT_RELEASE:
          // Save the temperature
          setSetting(SETTING_BAKE_DURATION, bakeDuration);
          // Go to the next phase
          setupPhase++;
      }
      break;      

    case 5: // Restart learning mode
      if (drawMenu) {
        drawMenu = false;
        if (getSetting(SETTING_LEARNING_MODE) == false) {
          lcdPrintLine_P(0, PSTR("Restart learning"));
          lcdPrintLine_P(1, PSTR("mode?      No ->"));
        }
        else
        {
          lcdPrintLine_P(0, PSTR("Oven is in"));
          lcdPrintLine_P(1, PSTR("learning mode"));
        }
      }
      
      // Was a button pressed?
      switch (buttons.GetKeypress()) {
        case BUTTON_TOP_RELEASE:
          // Turn learning mode on
          setSetting(SETTING_LEARNING_MODE, true);
          drawMenu = true;
          break;
        case BUTTON_BOT_RELEASE:
            // Go to the next phase
            setupPhase++;
       }
      break;

     case 6: // Restore to factory settings
      if (drawMenu) {
        drawMenu = false;
        lcdPrintLine_P(0, PSTR("Restore factory"));
        lcdPrintLine_P(1, PSTR("settings?  No ->"));
      }
      
      // Was a button pressed?
      switch (buttons.GetKeypress()) {
        case BUTTON_TOP_RELEASE:
          // Reset EEPROM to factory
          lcdPrintLine_P(0, PSTR("Please wait ..."));
          lcdPrintLine_P(1, PSTR(""));
          setSetting(SETTING_EEPROM_NEEDS_INIT, true);
          InitializeSettingsIfNeccessary();

          // Intentional fall-through
        case BUTTON_BOT_RELEASE:
            // Go to the next phase
            setupPhase++;
       }
      break;

   
  }
  
  // Does the menu option need to be redrawn?
  if (oldSetupPhase != setupPhase)
    drawMenu = true;
  if (setupPhase > 6) {
    setupPhase = 0;
    return false;
  }
  return true;
}
#endif

void displayMaxTemperature(int maxTemperature) {
  lcd.PrintInt(0,1,3,maxTemperature);
}


void displayServoDegrees(int degrees) {
  lcd.PrintInt(8,1,3,degrees);
  lcd.PrintStr(11,1,"\1 ");
}


void displayDuration(int offset, uint16_t duration) {
  lcd.PrintInt(offset,1,2,duration/3600);
  lcd.PrintStr(offset+2,1,":");
  lcd.PrintInt(offset+3,1,2,(duration % 3600) / 60);
  lcd.PrintStr(offset+5,0,":");
  lcd.PrintInt(offset+6,1,2,(duration % 60));
}


