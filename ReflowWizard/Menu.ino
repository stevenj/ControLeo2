#include "ControLeo2.h"
#include "MD_Menu.h"
#include "Menu.h"

#define AUTO_START (true)
#define MENU_TIMEOUT (0)

bool display(MD_Menu::userDisplayAction_t action, char *msg);
MD_Menu::userNavAction_t navigation(uint16_t &incDelta);

#if 0
// Global menu data and definitions
uint8_t fruit = 2;
bool bValue = true;
int8_t  int8Value = 99;
int16_t int16Value = 999;
int32_t int32Value = 9999;
float floatValue = 999.99;
#endif

// Menu Header
#define MENU_ID_START (10)
#define MENU(X)       ( MENU_ID_START + X)

// Main Menu Items (MM)
#define MM_ITEM_FIRST (10)
#define MM_ITEM(X)    (MM_ITEM_FIRST + X)

// Global Setting Items (GS)
#define GS_ITEM_FIRST (20)
#define GS_ITEM(X)    (GS_ITEM_FIRST + X)

// Menu Headers --------
const PROGMEM MD_Menu::mnuHeader_t mnuHdr[] =
{
  //    "1234567890123456"  
  { MENU(0), "Reflow Wizard V3", MM_ITEM(0), MM_ITEM(5), 0 }, 
  { MENU(1), "Configure Menu  ", GS_ITEM(0), GS_ITEM(10), 0 },
  { MENU(2), "Add/Edit Mode   ", 0, 1, 0 },
  { MENU(3), "Test HW Menu    ", 0, 1, 0 },
  { MENU(4), "Learn Menu      ", 0, 1, 0 },
  { MENU(5), "Reflow Menu     ", 0, 1, 0 },
  { MENU(6), "Bake Menu       ", 0, 1, 0 },
};

#define NO_HELP nullptr

FLASH_STRING(CONF_HELP) = "Configure Global Settings";
FLASH_STRING(EMOD_HELP) = "Add/Edit Reflow/Bake Modes";
FLASH_STRING(TEST_HELP) = "Test Hardware Interactively";
FLASH_STRING(LERN_HELP) = "Run a Mode to 'Learn' its baking parameters";
FLASH_STRING(RFLW_HELP) = "Run any Learnt Reflow Mode";
FLASH_STRING(BAKE_HELP) = "Run any Learnt Bake Mode";

FLASH_STRING(RELY_HELP) = "Select Hardware controlled by this Relay";
FLASH_STRING(COOL_HELP) = "Set Temperature Oven is considered 'COOL'";
FLASH_STRING(TMAX_HELP) = "Set Temperature Oven should not be set higher than";
FLASH_STRING(TOVH_HELP) = "Set Temperature Oven is OVERHEATED and will be shut down";
FLASH_STRING(DHOM_HELP) = "Set Retracted or Home Position of the Door Open Servo";
FLASH_STRING(DARM_HELP) = "Set Armed or Pre Opening Position of the Door Open Servo";
FLASH_STRING(DOPN_HELP) = "Set Fully Open Position of the Door Open Servo";
FLASH_STRING(DOPT_HELP) = "Set Time to Open Door, in 10ths of a second";


// Menu Items ----------
const PROGMEM MD_Menu::mnuItem_t mnuItm[] =
{
  // Starting (Root) menu 
                //678901234
  { MM_ITEM(0),  "Configure",    MD_Menu::MNU_MENU,  MENU(1), CONF_HELP },
  { MM_ITEM(1),  "Edit Mode",    MD_Menu::MNU_MENU,  MENU(2), EMOD_HELP },
  { MM_ITEM(2),  "Tst HWare",    MD_Menu::MNU_MENU,  MENU(3), TEST_HELP },
  { MM_ITEM(3),  "Learn    ",    MD_Menu::MNU_MENU,  MENU(4), LERN_HELP },
  { MM_ITEM(4),  "Reflow   ",    MD_Menu::MNU_MENU,  MENU(5), RFLW_HELP },
  { MM_ITEM(5),  "Bake     ",    MD_Menu::MNU_MENU,  MENU(6), BAKE_HELP },
  
  // Input Data submenu
  { GS_ITEM(0),  "Relay D4 ",    MD_Menu::MNU_INPUT, GS_ITEM(0),  RELY_HELP}, // D4 Relay Setting
  { GS_ITEM(1),  "Relay D5 ",    MD_Menu::MNU_INPUT, GS_ITEM(1),  RELY_HELP}, // D5 Relay Setting
  { GS_ITEM(2),  "Relay D6 ",    MD_Menu::MNU_INPUT, GS_ITEM(2),  RELY_HELP}, // D6 Relay Setting
  { GS_ITEM(3),  "Relay D7 ",    MD_Menu::MNU_INPUT, GS_ITEM(3),  RELY_HELP}, // D7 Relay Setting
  { GS_ITEM(4),  "T:Cool  \x01", MD_Menu::MNU_INPUT, GS_ITEM(4),  COOL_HELP}, // Temperature at which Oven considered COOL.
  { GS_ITEM(5),  "T:Max   \x01", MD_Menu::MNU_INPUT, GS_ITEM(5),  TMAX_HELP}, // Max Temperature setting.
  { GS_ITEM(6),  "T:OHeat \x01", MD_Menu::MNU_INPUT, GS_ITEM(6),  TOVH_HELP}, // Max Overheat Temperature setting.
  { GS_ITEM(7),  "D:Home  \x08", MD_Menu::MNU_INPUT, GS_ITEM(7),  DHOM_HELP}, // Door Servo Retract Position.
  { GS_ITEM(8),  "D:Armed \x08", MD_Menu::MNU_INPUT, GS_ITEM(8),  DARM_HELP}, // Door Armed (Closed, but ready to open)
  { GS_ITEM(9),  "D:Open  \x08", MD_Menu::MNU_INPUT, GS_ITEM(9),  DOPN_HELP}, // Door Open 100%
  { GS_ITEM(10), "D:OpnTime",    MD_Menu::MNU_INPUT, GS_ITEM(10), DOPT_HELP}, // Door Open 100%

#if 0  
  
  // Serial Setup
  { 30, "COM Port",  MD_Menu::MNU_INPUT, 30 },
  { 31, "Speed",     MD_Menu::MNU_INPUT, 31 },
  { 32, "Parity",    MD_Menu::MNU_INPUT, 32 },
  { 33, "Stop Bits", MD_Menu::MNU_INPUT, 33 },
  
  // LED  
  { 40, "Turn Off", MD_Menu::MNU_INPUT, 40 },
  { 41, "Turn On",  MD_Menu::MNU_INPUT, 41 },
  
  // Flip-flop - boolean controls variable edit
  { 50, "Flip", MD_Menu::MNU_INPUT, 50 },
  { 51, "Flop", MD_Menu::MNU_INPUT, 51 },
#endif  
};

#if 0
// Input Items ---------
const PROGMEM char listFruit[] = "Apple|Pear|Orange|Banana|Pineapple|Peach";
const PROGMEM char listCOM[] = "COM1|COM2|COM3|COM4";
const PROGMEM char listBaud[] = "9600|19200|57600|115200";
const PROGMEM char listParity[] = "O|E|N";
const PROGMEM char listStop[] = "0|1";
#endif

const PROGMEM MD_Menu::mnuInput_t mnuInp[] =
{//               0123456789012345 (label can be input - 8 chars wide)
 // id,           label:[78901234],       action,             value callback, width, {parameters}
  { GS_ITEM(0),  "",      MD_Menu::INP_LIST,  mnuGSValueRqst,  8,     {.pList = listRelayType }}, // D4 Relay Setting
  { GS_ITEM(1),  "",      MD_Menu::INP_LIST,  mnuGSValueRqst,  8,     {.pList = listRelayType }}, // D5 Relay Setting
  { GS_ITEM(2),  "",      MD_Menu::INP_LIST,  mnuGSValueRqst,  8,     {.pList = listRelayType }}, // D6 Relay Setting
  { GS_ITEM(3),  "",      MD_Menu::INP_LIST,  mnuGSValueRqst,  8,     {.pList = listRelayType }}, // D7 Relay Setting
  { GS_ITEM(4),  "     ", MD_Menu::INP_INT16, mnuGSValueRqst,  3,     {.range = { .min= 0, .max=  75, .base=10 } }}, // Temperature at which Oven considered COOL.
  { GS_ITEM(5),  "     ", MD_Menu::INP_INT16, mnuGSValueRqst,  3,     {.range = { .min= 0, .max= 290, .base=10 } }}, // Max Temperature setting.
  { GS_ITEM(6),  "     ", MD_Menu::INP_INT16, mnuGSValueRqst,  3,     {.range = { .min= 0, .max= 300, .base=10 } }}, // Max Overheat Temperature setting.
  { GS_ITEM(7),  "     ", MD_Menu::INP_INT16, mnuGSValueRqst,  3,     {.range = { .min= 0, .max= 180, .base=10 } }}, // Door Servo Retract Position.
  { GS_ITEM(8),  "     ", MD_Menu::INP_INT16, mnuGSValueRqst,  3,     {.range = { .min= 0, .max= 180, .base=10 } }}, // Door Armed (Closed, but ready to open)
  { GS_ITEM(9),  "     ", MD_Menu::INP_INT16, mnuGSValueRqst,  3,     {.range = { .min= 0, .max= 180, .base=10 } }}, // Door Open 100%
  { GS_ITEM(10), "     ", MD_Menu::INP_FLOAT, mnuGSValueRqst,  4,     {.range = { .min= 0, .max= 255, .base=1  } }}, // 0-25.5 Seconds.
  
#if 0  
  
  { 10, "List",    MD_Menu::INP_LIST,  mnuLValueRqst,      6,      {.pList = listFruit }}, // shorter and longer list labels
  { 11, "Bool",    MD_Menu::INP_BOOL,  mnuBValueRqst,      1,      {}},
  { 12, "Int8",    MD_Menu::INP_INT8,  mnuIValueRqst,      4,      {.range = { .min=   -128, .max=    127, .base=10 } }},
  { 13, "Int16",   MD_Menu::INP_INT16, mnuIValueRqst,      4,      {.range = { .min= -32768, .max=  32767, .base=10 } }}, // -32768,  32767, 10, nullptr },  // test field too small
  { 14, "Int32",   MD_Menu::INP_INT32, mnuIValueRqst,      6,      {.range = { .min= -66636, .max=  65535, .base=10 } }}, // -66636,  65535, 10, nullptr },
  { 15, "Hex16",   MD_Menu::INP_INT16, mnuIValueRqst,      4,      {.range = { .min= 0x0000, .max= 0xFFFF, .base=16 } }}, //0x0000, 0xffff, 16, nullptr },  // test hex display
  { 16, "Confirm", MD_Menu::INP_RUN,   myCode,             0,      {}}, // 0, 0, 10, nullptr },

  { 30, "Port",     MD_Menu::INP_LIST, mnuSerialValueRqst, 4, {.pList = listCOM }}, //0, 0, 0, listCOM },
  { 31, "Bits/s",   MD_Menu::INP_LIST, mnuSerialValueRqst, 6, {.pList = listBaud }}, //0, 0, 0, listBaud },
  { 32, "Parity",   MD_Menu::INP_LIST, mnuSerialValueRqst, 1, {.pList = listParity }}, //0, 0, 0, listParity },
  { 33, "No. Bits", MD_Menu::INP_LIST, mnuSerialValueRqst, 1, {.pList = listStop }}, //0, 0, 0, listStop },

  { 40, "Confirm",  MD_Menu::INP_RUN,  myLEDCode,          0,  {}}, //0, 0, 0, nullptr },  // test using index in run code
  { 41, "Confirm",  MD_Menu::INP_RUN,  myLEDCode,          0,  {}}, //0, 0, 0, nullptr },

  { 50, "Flip",     MD_Menu::INP_INT8, mnuFFValueRqst,     4, {.range = { .min= -128, .max= 127, .base=10 } }}, //-128, 127, 10, nullptr },
  { 51, "Flop",     MD_Menu::INP_INT8, mnuFFValueRqst,     4, {.range = { .min= -128, .max= 127, .base=16 } }}, //-128, 127, 16, nullptr },
#endif  
};

// bring it all together in the global menu object
MD_Menu M(navigation, display,        // user navigation and display
          mnuHdr, ARRAY_SIZE(mnuHdr), // menu header data
          mnuItm, ARRAY_SIZE(mnuItm), // menu item data
          mnuInp, ARRAY_SIZE(mnuInp));// menu input data


void InitMenu(void) {
    M.begin();
    M.setMenuWrap(true);
    M.setAutoStart(AUTO_START);
    M.setTimeout(MENU_TIMEOUT);
}

static uint32_t tempConf32;
    
// Callback code for menu set/get input values
void *mnuGSValueRqst(MD_Menu::mnuId_t id, MD_Menu::cdValueOp_t op)
// Value request callback for Global Settings variables
{
  SG_Entries_t entry = (SG_Entries_t)(id - GS_ITEM_FIRST);
  
  switch(op) {
    case MD_Menu::VAL_OP_GET:
      tempConf32 = readGlobalSetting(entry);
      return &tempConf32;
    break;
    
    case MD_Menu::VAL_OP_SET:
      writeGlobalSetting(entry, tempConf32);
    break;

    case MD_Menu::VAL_OP_TRY:  
      // For the servos, as they are set, move the real servo, so it can be set up properly.
      if ((entry >= SG_SERVO_RETRACT_DEG) && (entry <= SG_SERVO_OPEN_DEG)) {
        setServoPosition(tempConf32, 0);
      }
    break;  
  }
  return(nullptr);
}

#if 0
// Callback code for menu set/get input values
void *mnuLValueRqst(MD_Menu::mnuId_t id, bool bGet)
// Value request callback for list selection variable
{
  if (id == 10)
  {
    if (bGet)
      return((void *)&fruit);
    else
    {
      Serial.print(F("\nList index changed to "));
      Serial.print(fruit);
    }
  }

  return(nullptr);
}

void *mnuBValueRqst(MD_Menu::mnuId_t id, bool bGet)
// Value request callback for boolean variable
{
  if (id == 11)
  {
    if (bGet)
      return((void *)&bValue);
    else
    {
      Serial.print(F("\nBoolean changed to "));
      Serial.print(bValue);
    }
  }
  
  return(nullptr);
}

void *mnuIValueRqst(MD_Menu::mnuId_t id, bool bGet)
// Value request callback for integers variables
{
  switch (id)
  {
  case 12:
    if (bGet)
      return((void *)&int8Value);
    else
    {
      Serial.print(F("\nInt8 value changed to "));
      Serial.print(int8Value);
    }
    break;

  case 13:
  case 15:
    if (bGet)
      return((void *)&int16Value);
    else
    {
      Serial.print(F("\nInt16 value changed to "));
      Serial.print(int16Value);
    }
    break;

  case 14:
    if (bGet)
      return((void *)&int32Value);
    else
    {
      Serial.print(F("\nInt32 value changed to "));
      Serial.print(int32Value);
    }
    break;
  }

  return(nullptr);
}

void *mnuSerialValueRqst(MD_Menu::mnuId_t id, bool bGet)
// Value request callback for Serial parameters
{
  static uint8_t port = 0, speed = 0, parity = 0, stop = 0;

  switch (id)
  {
  case 30:
    if (bGet)
      return((void *)&port);
    else
    {
      Serial.print(F("\nPort index="));
      Serial.print(port);
    }
    break;

  case 31:
    if (bGet)
      return((void *)&speed);
    else
    {
      Serial.print(F("\nSpeed index="));
      Serial.print(speed);
    }
    break;

  case 32:
    if (bGet)
      return((void *)&parity);
    else
    {
      Serial.print(F("\nParity index="));
      Serial.print(parity);
    }
    break;

  case 33:
    if (bGet)
      return((void *)&stop);
    else
    {
      Serial.print(F("\nStop index="));
      Serial.print(stop);
    }
    break;
  }
  return(nullptr);
}

void *mnuFFValueRqst(MD_Menu::mnuId_t id, bool bGet)
// Value edit allowed request depends on another value
{
  static bool gateKeeper = false;

  switch (id)
  {
  case 50:
    if (bGet)
    {
      if (gateKeeper)
      {
        Serial.print(F("\nFlipFlop value blocked"));
        return(nullptr);
      }
      else
        return((void *)&int8Value);
    }
    else
    {
      Serial.print(F("\nFlipFlop value changed to "));
      Serial.print(int8Value);
      gateKeeper = !gateKeeper;
    }
    break;

  case 51:
    if (bGet)
    {
      if (!gateKeeper)    // reverse the logic of above
      {
        Serial.print(F("\nFlipFlop value blocked"));
        return(nullptr);
      }
      else
        return((void *)&int8Value);
    }
    else
    {
      Serial.print(F("\nFlipFlop value changed to "));
      Serial.print(int8Value);
      gateKeeper = !gateKeeper;
    }
    break;
  }
  
  return(nullptr);
}

void *myCode(MD_Menu::mnuId_t id, bool bGet __attribute__ ((unused)))
// Value request callback for run code input
{
  Serial.print(F("\nmyCode called id="));
  Serial.print(id);
  Serial.print(F(" to "));
  Serial.print(bGet ? F("GET") : F("SET - reset menu"));

  if (!bGet) M.reset();

  return(nullptr);
}

void *myLEDCode(MD_Menu::mnuId_t id, bool bGet __attribute__ ((unused)))
// Value request callback for run code input
// Only use the index here
{
  Serial.print(F("\nSwitchig LED "));
  Serial.print(id == 40 ? F("off") : F("on"));
  // digitalWrite(LED_PIN, id == 40 ? LOW : HIGH);
  return(nullptr);
}
#endif

bool display(MD_Menu::userDisplayAction_t action, const char *msg)
{
  switch (action)
  {
  case MD_Menu::DISP_CLEAR:
    lcd.clear();
    break;

  case MD_Menu::DISP_L0:
    lcd.PrintStr(0,0, msg);
    break;

  case MD_Menu::DISP_L1:
    lcd.PrintStr(5, 1, msg);
    break;

  case MD_Menu::DISP_HELP:
    lcd.ScrollLine(0,1,(const __FlashStringHelper*)msg);
  }

  return(true);
}


MD_Menu::userNavAction_t navigation(uint16_t &incDelta __attribute__ ((unused)))
{
  uint8_t key = buttons.GetKeypress();
  MD_Menu::userNavAction_t action = MD_Menu::NAV_NULL;

  switch (key) {
    case BUTTON_TOP_PRESS:        break;                             // Not Handled
    case BUTTON_TOP_RELEASE:      action = MD_Menu::NAV_INC;  break; // Increment, if no Encoder
    case BUTTON_TOP_LONG_HOLD:    action = MD_Menu::NAV_HELP; break; // Display HELP
    case BUTTON_TOP_LONG_RELEASE: break;                             // Not Handled

    case BUTTON_BOT_PRESS:        break;                             // Not Handled
    case BUTTON_BOT_RELEASE:      action = MD_Menu::NAV_SEL; break;  // Select (Short Press)
    case BUTTON_BOT_LONG_HOLD:    action = MD_Menu::NAV_ESC; break;  // Escape (Long Press)
    case BUTTON_BOT_LONG_RELEASE: break;                             // Not Handled

    case ENCODER_INC:             action = MD_Menu::NAV_INC; break;  // Increment
    case ENCODER_DEC:             action = MD_Menu::NAV_DEC; break;  // Decrement
  }

  return(action);
}

