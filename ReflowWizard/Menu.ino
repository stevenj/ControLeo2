#include "ControLeo2.h"
#include "MD_Menu.h"
#include "Menu.h"

#define AUTO_START 0
#define MENU_TIMEOUT 10000

bool display(MD_Menu::userDisplayAction_t action, char *msg);
MD_Menu::userNavAction_t navigation(uint16_t &incDelta);

// Global menu data and definitions
uint8_t fruit = 2;
bool bValue = true;
int8_t  int8Value = 99;
int16_t int16Value = 999;
int32_t int32Value = 9999;
float floatValue = 999.99;

// Menu Headers --------
const PROGMEM MD_Menu::mnuHeader_t mnuHdr[] =
{
  { 10, "MD_Menu",      10, 14, 0 },
  { 11, "Input Data",   20, 27, 0 },
  { 12, "Serial Setup", 30, 33, 0 },
  { 13, "LED Menu",     40, 41, 0 },
  { 14, "FF Menu",     50, 51, 0 },
};

// Menu Items ----------
const PROGMEM MD_Menu::mnuItem_t mnuItm[] =
{
  // Starting (Root) menu
  { 10, "Input Test", MD_Menu::MNU_MENU, 11 },
  { 11, "Serial",     MD_Menu::MNU_MENU, 12 },
  { 12, "LED",        MD_Menu::MNU_MENU, 13 },
  { 13, "More Menu",  MD_Menu::MNU_MENU, 10 },
  { 14, "Flip-Flop",  MD_Menu::MNU_MENU, 14 },
  // Input Data submenu
  { 20, "Fruit List", MD_Menu::MNU_INPUT, 10 },
  { 21, "Boolean",    MD_Menu::MNU_INPUT, 11 },
  { 22, "Integer 8",  MD_Menu::MNU_INPUT, 12 },
  { 23, "Integer 16", MD_Menu::MNU_INPUT, 13 },
  { 24, "Integer 32", MD_Menu::MNU_INPUT, 14 },
  { 25, "Hex 16",     MD_Menu::MNU_INPUT, 15 },
  { 26, "Float",      MD_Menu::MNU_INPUT, 16 },
  { 27, "Reset Menu", MD_Menu::MNU_INPUT, 17 },
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
};

// Input Items ---------
const PROGMEM char listFruit[] = "Apple|Pear|Orange|Banana|Pineapple|Peach";
const PROGMEM char listCOM[] = "COM1|COM2|COM3|COM4";
const PROGMEM char listBaud[] = "9600|19200|57600|115200";
const PROGMEM char listParity[] = "O|E|N";
const PROGMEM char listStop[] = "0|1";

const PROGMEM MD_Menu::mnuInput_t mnuInp[] =
{
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

bool display(MD_Menu::userDisplayAction_t action, char *msg)
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
    lcd.PrintStr(6, 1, msg);
    break;
  }

  return(true);
}

MD_Menu::userNavAction_t navigation(uint16_t &incDelta __attribute__ ((unused)))
{
  uint8_t key = buttons.GetKeypress();

  switch (key) {
    case BUTTON_TOP_PRESS:        break;                    // Not Handled
    case BUTTON_TOP_RELEASE:      return(MD_Menu::NAV_INC); // Increment, if no Encoder
    case BUTTON_TOP_LONG_HOLD:    break;                    // Display HELP (To Implement)
    case BUTTON_TOP_LONG_RELEASE: break;                    // Not Handled

    case BUTTON_BOT_PRESS:        break;                    // Not Handled
    case BUTTON_BOT_RELEASE:      return(MD_Menu::NAV_SEL); // Select (Short Press)
    case BUTTON_BOT_LONG_HOLD:    return(MD_Menu::NAV_ESC); // Escape (Long Press)
    case BUTTON_BOT_LONG_RELEASE: break;                    // Not Handled

    case ENCODER_INC:             return(MD_Menu::NAV_INC); // Increment
    case ENCODER_DEC:             return(MD_Menu::NAV_DEC); // Decrement
  }

  return(MD_Menu::NAV_NULL);
}

