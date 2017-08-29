// Customised Version of MD_Menu for the ReflowWizard Project
//
// Implementation file for MD_Menu library
//
// See the main header file MD_Menu.h for more information

#include "MD_Menu.h"
#include "MD_Menu_lib.h"

// Temporary Buffers used to build strings.
static char temp_buf1[MAX_TEMPSTR_SIZE];
static char temp_buf2[MAX_TEMPSTR_SIZE];

/**
 * \file
 * \brief Main code file for MD_Menu library
 */
MD_Menu::MD_Menu(cbUserNav cbNav, cbUserDisplay cbDisp,
                const mnuHeader_t *mnuHdr, uint8_t mnuHdrCount,
                const mnuItem_t *mnuItm, uint8_t mnuItmCount,
                const mnuInput_t *mnuInp, uint8_t mnuInpCount) :
                _mnuHdr(mnuHdr), _mnuHdrCount(mnuHdrCount),
                _mnuItm(mnuItm), _mnuItmCount(mnuItmCount),
                _mnuInp(mnuInp), _mnuInpCount(mnuInpCount),
                _options(0), _timeout(0)

{
  setUserNavCallback(cbNav);
  setUserDisplayCallback(cbDisp);
}

void MD_Menu::reset(void)
{ 
  CLEAR_FLAG(F_INMENU); 
  CLEAR_FLAG(F_INEDIT); 
  _currMenu = 0; 
};

void MD_Menu::setUserNavCallback(cbUserNav cbNav) 
{ 
  if (cbNav != nullptr) 
    _cbNav = cbNav; 
};

void MD_Menu::setUserDisplayCallback(cbUserDisplay cbDisp) 
{ 
  if (cbDisp != nullptr) 
    _cbDisp = cbDisp; 
};

// Status and options
bool MD_Menu::isInMenu(void) { return(TEST_FLAG(F_INMENU)); };
bool MD_Menu::isInEdit(void) { return(TEST_FLAG(F_INEDIT)); };
void MD_Menu::setMenuWrap(bool bSet)  { if (bSet) { SET_FLAG(F_MENUWRAP); } else { CLEAR_FLAG(F_MENUWRAP); } };
void MD_Menu::setAutoStart(bool bSet) { if (bSet) { SET_FLAG(F_AUTOSTART); } else { CLEAR_FLAG(F_AUTOSTART); } };
void MD_Menu::setTimeout(uint32_t t) { _timeout = t; };

void MD_Menu::timerStart(void)
{
  _timeLastKey = millis();
}

void MD_Menu::timerCheck(void)
{
  if (_timeout == 0) return;    // not set

  if (millis() - _timeLastKey >= _timeout)
  {
    MD_PRINTS("\ntimerCheck: Menu timeout");
    reset();
  }
}

void MD_Menu::loadMenu(mnuId_t id)
// Load a menu header definition to the current stack position
{
  mnuId_t idx = 0;
  mnuHeader_t mh;

  if (id != -1)   // look for a menu with that id and load it up
  {
    for (uint8_t i = 0; i < _mnuHdrCount; i++)
    {
      memcpy_P(&mh, &_mnuHdr[i], sizeof(mnuHeader_t));
      if (mh.id == id)
      {
        idx = i;  // found it!
        break;
      }
    }
  }

  // we either found the item or we will load the first one by default
  memcpy_P(&_mnuStack[_currMenu], &_mnuHdr[idx], sizeof(mnuHeader_t));
}

MD_Menu::mnuItem_t* MD_Menu::loadItem(mnuId_t id)
// Find a copy the input item to the class private buffer
{
  for (uint8_t i = 0; i < _mnuItmCount; i++)
  {
    memcpy_P(&_mnuBufItem, &_mnuItm[i], sizeof(mnuItem_t));
    if (_mnuBufItem.id == id)
      return(&_mnuBufItem);
  }

  return(nullptr);
}

MD_Menu::mnuInput_t* MD_Menu::loadInput(mnuId_t id)
// Find a copy the input item to the class private buffer
{
  for (uint8_t i = 0; i < _mnuItmCount; i++)
  {
    memcpy_P(&_mnuBufInput, &_mnuInp[i], sizeof(mnuInput_t));
    if (_mnuBufInput.id == id)
      return(&_mnuBufInput);
  }

  return(nullptr);
}

uint8_t MD_Menu::listCount(const /*PROGMEM*/ char *p)
// Return a count of the items in the list
{
  uint8_t count = 0;
  char c;

  if (p != nullptr)
  {
    if (pgm_read_byte(p) != '\0')   // not empty list
    {
      do
      {
        c = pgm_read_byte(p++);
        if (c == LIST_SEPARATOR) count++;
      } while (c != '\0');

      // if the list is not empty, then the last element is 
      // terminated by '\0' and we have not counted it, so 
      // add it now
      count++;
    }
  }

  return(count);
}

char *MD_Menu::listItem(const /*PROGMEM*/ char *p, uint8_t idx, char *buf, uint8_t bufLen)
// Find the idx'th item in the list and return in fixed width, padded
// with trailing spaces. 
{
  // fill the buffer with '\0' so we know that string will
  // always be terminted within this buffer
  memset(buf, '\0', bufLen);

  if (p != nullptr)
  {
    char *psz;
    char c;

    // skip items before the one we want
    while (idx > 0)
    {
      do
        c = pgm_read_byte(p++);
      while (c != '\0' && c != LIST_SEPARATOR);
      idx--;
    }

    // copy the next item over
    psz = buf;
    for (uint8_t i = 0; i < bufLen - 1; psz++, i++)
    {
      *psz = pgm_read_byte(p++);
      if (*psz == LIST_SEPARATOR) *psz = '\0';
      if (*psz == '\0') break;
    }

    // Pad out any short string with trailing spaces
    // The trailing buffer is already filled with '\0'
    while ((uint8_t)strlen(buf) < bufLen - 1)
      *psz++ = ' ';
  }

  return(buf);
}

void MD_Menu::strPreamble(char *psz, uint8_t psz_size, mnuInput_t *mInp)
// Create the start to a variable CB_DISP
{
  strlcpy(psz, mInp->label, psz_size);
  strlcat_P(psz, FLD_PROMPT, psz_size);
  strlcat_P(psz, FLD_DELIM_L, psz_size);
}

void MD_Menu::strPostamble(char *psz, uint8_t psz_size, mnuInput_t *mInp __attribute__((__unused__)))
// Attach the tail of the variable CB_DISP
{
  strlcat_P(psz, FLD_DELIM_R, psz_size);
}

bool MD_Menu::processList(userNavAction_t nav, mnuInput_t *mInp)
// Processing for List based input
// Return true when the edit cycle is completed
{
  bool endFlag = false;
  bool update = false;

  switch (nav)
  {
  case NAV_NULL:    // this is to initialise the CB_DISP
  {
    uint8_t size = listCount(mInp->pList);

    if (size == 0)
    {
      MD_PRINTS("\nEmpty list selection!");
      endFlag = true;
    }
    else
    {
      _pValue = mInp->cbVR(mInp->id, VAL_OP_GET);

      if (_pValue == nullptr)
      {
        MD_PRINTS("\nList cbVR(GET) == NULL!");
        endFlag = true;
      }
      else
      {
        _iValue = *((uint8_t*)_pValue);
        if (_iValue > size - 1)   // index set incorrectly
          _iValue = 0;
        update = true;
      }
    }
  }
  break;

  case NAV_DEC:
    {
      uint8_t listSize = listCount(mInp->pList);
      if (_iValue > 0)
      {
        _iValue--;
        update = true;
      }
      else if (_iValue == 0 && TEST_FLAG(F_MENUWRAP))
      {
        _iValue = listSize - 1;
        update = true;
      }
    }
    break;

  case NAV_INC:
    {
      uint8_t listSize = listCount(mInp->pList);

      if (_iValue < listSize - 1)
      {
        _iValue++;
        update = true;
      }
      else if (_iValue == listSize - 1 && TEST_FLAG(F_MENUWRAP))
      {
        _iValue = 0;
        update = true;
      }
    }
    break;

  case NAV_SEL:
    *((uint8_t*)_pValue) = _iValue;
    mInp->cbVR(mInp->id, VAL_OP_SET);
    endFlag = true;
    break;

  case NAV_ESC:
  case NAV_HELP:
    break;
  }

  if (update)
  {
    //char szItem[mInp->fieldWidth + 1];
    //char sz[INP_PRE_SIZE(mInp) + sizeof(szItem) + INP_POST_SIZE(mInp) + 1];

    strPreamble(temp_buf1, sizeof(temp_buf1), mInp);
    strcat(temp_buf1, listItem(mInp->pList, _iValue, temp_buf2, mInp->fieldWidth + 1));
    strPostamble(temp_buf1, sizeof(temp_buf1), mInp);

    _cbDisp(DISP_L1, temp_buf1);
  }

  return(endFlag);
}

bool MD_Menu::processBool(userNavAction_t nav, mnuInput_t *mInp)
// Processing for Boolean (true/false) value input
// Return true when the edit cycle is completed
{
  bool endFlag = false;
  bool update = false;

  switch (nav)
  {
  case NAV_NULL:    // this is to initialise the CB_DISP
    {
      _pValue = mInp->cbVR(mInp->id, VAL_OP_GET);

      if (_pValue == nullptr)
      {
        MD_PRINTS("\nBool cbVR(GET) == NULL!");
        endFlag = true;
      }
      else
      {
        _bValue = *((bool *)_pValue);
        update = true;
      }
    }
    break;

  case NAV_INC:
  case NAV_DEC:
    _bValue = !_bValue;
    update = true;
    break;

  case NAV_SEL:
    *((bool *)_pValue) = _bValue;
    mInp->cbVR(mInp->id, VAL_OP_SET);
    endFlag = true;
    break;

  case NAV_ESC:
  case NAV_HELP:
    break;
  }

  if (update)
  {
    // char sz[INP_PRE_SIZE(mInp) + strlen(INP_BOOL_T) + INP_POST_SIZE(mInp) + 1];

    strPreamble(temp_buf1, sizeof(temp_buf1), mInp);
    strlcat_P(temp_buf1, _bValue ? INP_BOOL_T : INP_BOOL_F, sizeof(temp_buf1));
    strPostamble(temp_buf1, sizeof(temp_buf1), mInp);

    _cbDisp(DISP_L1, temp_buf1);
  }

  return(endFlag);
}

char *ltostr(char *buf, uint8_t bufLen, int32_t v, uint8_t base, bool leadZero = false)
// Convert a long to a string right justified with leading spaces
// in the base specified (up to 16).
{
  char *ptr = buf + bufLen - 1; // the last element of the buffer
  bool sign = (v < 0);
  uint32_t t = 0, res = 0;
  uint32_t value = (sign ? -v : v);

  if (buf == nullptr) return(nullptr);

  *ptr = '\0'; // terminate the string as we will be moving backwards

  // now successively deal with the remainder digit 
  // until we have value == 0
  do
  {
    t = value / base;
    res = value - (base * t);
    if (res < 10)
      *--ptr = '0' + res;
    else if ((res >= 10) && (res < 16))
      *--ptr = 'A' - 10 + res;
    value = t;
  } while (value != 0 && ptr != buf);

  if (ptr != buf)      // if there is still space
  {
    if (sign) *--ptr = '-';  // put in the sign ...
    while (ptr != buf)       // ... and pad with leading character
      *--ptr = (leadZero ? '0' : ' ');
  }
  else if (value != 0 || sign) // insufficient space - show this
      *ptr = INP_NUMERIC_OFLOW;

  return(buf);
}

bool MD_Menu::processInt(userNavAction_t nav, mnuInput_t *mInp, uint16_t incDelta)
// Processing for Integer (all sizes) value input
// Return true when the edit cycle is completed
{
  auto writeBack_iValue = [&]( int32_t iValue ) -> void
  {
    switch (mInp->action)
    {
    case INP_INT8:  *((int8_t*)_pValue)  = iValue; break;
    case INP_INT16: *((int16_t*)_pValue) = iValue; break;
    case INP_INT32: // int32 and float write the same way 
    case INP_FLOAT: *((int32_t*)_pValue) = iValue; break;
    default: break;
    }
  };

  auto read_iValue = [&]( ) -> int32_t
  {
    switch (mInp->action)
    {
    case INP_INT8:  return *((int8_t*)_pValue);  break;
    case INP_INT16: return *((int16_t*)_pValue); break;
    case INP_INT32: // int32 and float read the same way
    case INP_FLOAT: return *((int32_t*)_pValue); break;
    default: return 0; break;
    }
  };

  bool endFlag = false;
  bool update = false;

  switch (nav)
  {
  case NAV_NULL:    // this is to initialise the CB_DISP
    {
      _pValue = mInp->cbVR(mInp->id, VAL_OP_GET);

      if (_pValue == nullptr)
      {
        MD_PRINTS("\nInt cbVR(GET) == NULL!");
        endFlag = true;
      }
      else
      {
        _iValue = read_iValue();
        update = true;
      }
    }
    break;

  case NAV_INC:
    if (_iValue + incDelta < mInp->range.max)
      _iValue += incDelta;
    else
      _iValue = mInp->range.max;
    update = true;
    break;

  case NAV_DEC:
    if (_iValue - incDelta > mInp->range.min)
      _iValue -= incDelta;
    else
      _iValue = mInp->range.min;
    update = true;
    break;

  case NAV_SEL:
    writeBack_iValue( _iValue );
    mInp->cbVR(mInp->id, VAL_OP_SET);
    endFlag = true;
    break;

  case NAV_ESC:
  case NAV_HELP:
    break;
  }

  if (update)
  {
    int32_t _iBackup = read_iValue();
    
    if (mInp->action == INP_FLOAT) {
      uint16_t divisor = 1;
      //uint8_t dp;
      //char sz[INP_PRE_SIZE(mInp) + mInp->fieldWidth + INP_POST_SIZE(mInp) + 1];
  
      for (uint8_t i = 0; i < mInp->range.base; i++)
        divisor *= 10;
  
      strPreamble(temp_buf1, sizeof(temp_buf1), mInp);
      ltostr(temp_buf1 + strlen(temp_buf1), mInp->fieldWidth - (mInp->range.base + 1) + 1, _iValue / divisor, 10);
      strlcat_P(temp_buf1, DECIMAL_POINT, sizeof(temp_buf1));
      //dp = strlen(temp_buf1);
      //temp_buf1[strlen(temp_buf1) + 1] = '\0';
      //temp_buf1[strlen(temp_buf1)] = DECIMAL_POINT;
      ltostr(temp_buf1 + strlen(temp_buf1), (mInp->range.base + 1), abs(_iValue % divisor), 10, true);
  
      strPostamble(temp_buf1, sizeof(temp_buf1), mInp);
    } else { // Display Integer
      //char sz[INP_PRE_SIZE(mInp) + mInp->fieldWidth + INP_POST_SIZE(mInp) + 1]; 
      strPreamble(temp_buf1, sizeof(temp_buf1), mInp);
      ltostr(temp_buf1 + strlen(temp_buf1), mInp->fieldWidth + 1, _iValue, mInp->range.base);
      strPostamble(temp_buf1, sizeof(temp_buf1), mInp);
    }    
  
    _cbDisp(DISP_L1, temp_buf1);

    // Try the value if its a hardware setting.
    writeBack_iValue( _iValue );      // Temporarily set the new value
    mInp->cbVR(mInp->id, VAL_OP_TRY); // Try It
    writeBack_iValue( _iBackup );     // Set back to original value
    
  }

  return(endFlag);
}

#if 0
bool MD_Menu::processFloat(userNavAction_t nav, mnuInput_t *mInp, uint16_t incDelta)
// Processing for Floating number representation value input
// The number is actually a uint32, where the last FLOAT_DECIMALS digits are taken
// to be fractional part of the floating numer. For all purposes, this number is a long
// integer except when displayed. The base field is used as the increment for the decimal
// part in single fractional units of the decimal part.
// Return true when the edit cycle is completed
{
  bool endFlag = false;
  bool update = false;

  switch (nav)
  {
  case NAV_NULL:    // this is to initialise the CB_DISP
  {
    _pValue = mInp->cbVR(mInp->id, VAL_OP_GET);

    if (_pValue == nullptr)
    {
      MD_PRINTS("\nFloat cbVR(GET) == NULL!");
      endFlag = true;
    }
    else
    {
      _iValue = *((int32_t*)_pValue);
      update = true;
    }
  }
  break;

  case NAV_INC:
    if (_iValue + incDelta < mInp->range.max)
      _iValue += incDelta;
    else
      _iValue = mInp->range.max;
    update = true;
    break;

  case NAV_DEC:
    if (_iValue - (incDelta * mInp->range.base) > mInp->range.min)
      _iValue -= (incDelta * mInp->range.base);
    else
      _iValue = mInp->range.min;
    update = true;
    break;

  case NAV_SEL:
    *((int32_t*)_pValue) = _iValue;
    mInp->cbVR(mInp->id, VAL_OP_SET);
    endFlag = true;
    break;

  case NAV_ESC:
    break;
  }

  if (update)
  {
    uint16_t divisor = 1;
    char sz[INP_PRE_SIZE(mInp) + mInp->fieldWidth + INP_POST_SIZE(mInp) + 1];

    for (uint8_t i = 0; i < FLOAT_DECIMALS; i++)
      divisor *= 10;

    strPreamble(sz, sizeof(sz), mInp);
    ltostr(sz + strlen(sz), mInp->fieldWidth - (FLOAT_DECIMALS + 1) + 1, _iValue / divisor, 10);
    sz[strlen(sz) + 1] = '\0';
    sz[strlen(sz)] = DECIMAL_POINT;
    ltostr(sz + strlen(sz), (FLOAT_DECIMALS + 1), abs(_iValue % divisor), 10, true);

    strPostamble(sz, sizeof(sz), mInp);

    _cbDisp(DISP_L1, sz);
  }

  return(endFlag);
}
#endif

bool MD_Menu::processRun(userNavAction_t nav, mnuInput_t *mInp)
// Processing for Run user code input field.
// When the field is selected, run the user variable code. For all other
// input do nothing. Return true when the element has run user code.
{
  if (nav == NAV_NULL)    // initialise the CB_DISP
  {
    // char sz[INP_PRE_SIZE(mInp) + INP_POST_SIZE(mInp) + 1];

    strlcpy_P(temp_buf1, FLD_DELIM_L, sizeof(temp_buf1));
    strlcat(temp_buf1, mInp->label, sizeof(temp_buf1));
    strlcat_P(temp_buf1, FLD_DELIM_R, sizeof(temp_buf1));

    _cbDisp(DISP_L1, temp_buf1);
  }
  else if (nav == NAV_SEL)
  {
    mInp->cbVR(mInp->id, VAL_OP_SET);
    return(true);
  }

  return(false);
}

void MD_Menu::handleInput(bool bNew)
{ 
  bool ended = false;
  uint16_t incDelta = 1;
  mnuItem_t *mi;
  mnuInput_t *me;

  auto runAction = [&]( userNavAction_t nav ) -> void
  {
    switch (me->action)
    {
    case INP_LIST: ended = processList(nav, me); break;
    case INP_BOOL: ended = processBool(nav, me); break;
    case INP_INT8:
    case INP_INT16:
    case INP_INT32: // ended = processInt(nav, me, incDelta); break;
    case INP_FLOAT: ended = processInt(nav, me, incDelta); break; // ended = processFloat(NAV_NULL, me, incDelta); break;
    case INP_RUN: ended = processRun(nav, me); break;
    }
  };

  if (bNew)
  {
    _cbDisp(DISP_CLEAR, nullptr);
    mi = loadItem(_mnuStack[_currMenu].idItmCurr);
    _cbDisp(DISP_L0, mi->label);
    me = loadInput(mi->actionId);
    if ((me == nullptr) || (me->cbVR == nullptr))
      ended = true;
    else
    {
      SET_FLAG(F_INEDIT);
      timerStart();
#if 1
      runAction(NAV_NULL);
#else
      switch (me->action)
      {
      case INP_LIST: ended = processList(NAV_NULL, me); break;
      case INP_BOOL: ended = processBool(NAV_NULL, me); break;
      case INP_INT8:
      case INP_INT16:
      case INP_INT32: // ended = processInt(NAV_NULL, me, incDelta); break;
      case INP_FLOAT: ended = processInt(NAV_NULL, me, incDelta); break; // ended = processFloat(NAV_NULL, me, incDelta); break;
      case INP_RUN: ended = processRun(NAV_NULL, me); break;
      }
#endif      
    }
  }
  else
  {
    userNavAction_t nav = _cbNav(incDelta);

    if (nav == NAV_ESC)
      ended = true;
    else if (nav != NAV_NULL)
    {
      timerStart();
      mi = loadItem(_mnuStack[_currMenu].idItmCurr);
      me = loadInput(mi->actionId);

#if 1
      runAction(nav);
#else      
      switch (me->action)
      {
      case INP_LIST: ended = processList(nav, me); break;
      case INP_BOOL: ended = processBool(nav, me); break;
      case INP_INT8:
      case INP_INT16:
      case INP_INT32: // ended = processInt(nav, me, incDelta); break;
      case INP_FLOAT: ended = processInt(nav, me, incDelta); break; // ended = processFloat(nav, me, incDelta); break;
      case INP_RUN: ended = processRun(nav, me); break;
      }
#endif      
    }
  }

  if (ended)
  {
    CLEAR_FLAG(F_INEDIT);
    handleMenu(true);
  }
}

void MD_Menu::handleMenu(bool bNew)
{
  bool update = false;
  userNavAction_t nav = NAV_NULL;

  if (bNew)
  {
    // Enter Menu, so run Enter Menu Callback
    if (_mnuStack[_currMenu].cbMVR != nullptr) {
      _mnuStack[_currMenu].cbMVR(_mnuStack[_currMenu].id, VAL_OP_GET);
    }
        
    _cbDisp(DISP_CLEAR, nullptr);
    _cbDisp(DISP_L0, _mnuStack[_currMenu].label);
    if (_mnuStack[_currMenu].idItmCurr == 0)
      _mnuStack[_currMenu].idItmCurr = _mnuStack[_currMenu].idItmStart;
    SET_FLAG(F_INMENU);
    timerStart();
    update = true;
  }
  else
  {
    uint16_t incDelta = 1;
    nav = _cbNav(incDelta);

    if (nav != NAV_NULL) timerStart();

    switch (nav)
    {
    case NAV_DEC:
      if (_mnuStack[_currMenu].idItmCurr > _mnuStack[_currMenu].idItmStart)
      {
        _mnuStack[_currMenu].idItmCurr--;
        update = true;
      }
      else if (TEST_FLAG(F_MENUWRAP))
      {
        _mnuStack[_currMenu].idItmCurr = _mnuStack[_currMenu].idItmEnd;
        update = true;
      }
      break;

    case NAV_INC:
      if (_mnuStack[_currMenu].idItmCurr < _mnuStack[_currMenu].idItmEnd)
      {
        _mnuStack[_currMenu].idItmCurr++;
        update = true;
      }
      else if (TEST_FLAG(F_MENUWRAP))
      {
        _mnuStack[_currMenu].idItmCurr = _mnuStack[_currMenu].idItmStart;
        update = true;
      }
      break;

    case NAV_SEL:
      {
        mnuItem_t *mi = loadItem(_mnuStack[_currMenu].idItmCurr);

        switch (mi->action)
        {
        case MNU_MENU:
          if (_currMenu < MNU_STACK_SIZE - 1)
          {
            _currMenu++;
            loadMenu(mi->actionId);
            handleMenu(true);
          }
          break;

        case MNU_INPUT:
          if (loadInput(mi->actionId) != nullptr) {
            handleInput(true);
          } else {
            MD_PRINTS("\nInput definition not found");
          }
          break;
        }
      }
      break;

    case NAV_ESC:
      if (_currMenu == 0)
      {
        CLEAR_FLAG(F_INMENU);
      }
      else
      {
        if (_mnuStack[_currMenu].cbMVR != nullptr) {
          _mnuStack[_currMenu].cbMVR(_mnuStack[_currMenu].id, VAL_OP_SET);
        }
        
        _currMenu--;
        handleMenu(true);  // just one level of recursion;
      }
      break;

    case NAV_HELP:
      update = true;
      break;

    case NAV_NULL:
      break;
    }
  }

  if (update) // update L1 on the CB_DISP
  {
    mnuItem_t *mi = loadItem(_mnuStack[_currMenu].idItmCurr);

    if (nav == NAV_HELP) {
      _cbDisp(DISP_HELP, mi->HelpMsg);
    }

    if (mi != nullptr)
    {
      char sz[MAX_TEMPSTR_SIZE + 1]; // temporary string

      strlcpy_P(sz, MNU_DELIM_L, MAX_TEMPSTR_SIZE + 1);
      strlcat(sz, mi->label, MAX_TEMPSTR_SIZE + 1);
      strlcat_P(sz, MNU_DELIM_R, MAX_TEMPSTR_SIZE + 1);

      _cbDisp(DISP_L1, sz);
    }
  }
}

bool MD_Menu::runMenu(bool bStart)
{
  // check if we need to process anything
  if (!TEST_FLAG(F_INMENU) && !bStart)
  {
    // uint16_t dummy;

    bStart = (TEST_FLAG(F_AUTOSTART) /* && _cbNav(dummy) == NAV_SEL */ ); // We always AUTORUN, dont need a keypress to do it.
    if (bStart) { MD_PRINTS("\nrunMenu: Auto Start detected"); }
    if (!bStart) return(false);   // nothing to do
  }

  if (bStart)   // start the menu
  {
    MD_PRINTS("\nrunMenu: Starting menu");
    _currMenu = 0;
    loadMenu();
    handleMenu(true);
  }
  else    // keep running current menu
  {
    if (TEST_FLAG(F_INEDIT))
      handleInput();
    else
      handleMenu();

    timerCheck();  // check for timeout before we end here

    if (!TEST_FLAG(F_INMENU))
    {
      _cbDisp(DISP_CLEAR, nullptr);
      MD_PRINTS("\nrunMenu: Ending Menu");
    }
  }

  return(TEST_FLAG(F_INMENU));
}
