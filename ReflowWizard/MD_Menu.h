#pragma once
/**
\mainpage MD_Menu Library
Yet Another Menu Manager for Small Displays
------------------------------

This is a menu management library created as a front end to set 
parameters in embedded hardware control applications, laeving the 
back end under application control. It is suitable for text based 
displays (eg, LCD modules) and with 1 or 2 lines available for display.

The library allows user code to define
- Static menu definitions to minimised RAM footprint. 
- Callbacks for navigation and display control
- Menu inactivity timeout
- Auto start on key press or manual start by user code
- Input methods available for
  + Boolean (Y/N) values
  + Pick List selection
  + 8/16/32 bit signed integers
  + Decimal floating point representation

Menu managers in embedded systems are generally not the main function 
of the embedded application software, so this library minimises the 
use of RAM and has a small memory footprint overall, leaving more space 
for what really matters.

- \subpage pageMenu
- \subpage pageRevisionHistory
- \subpage pageCopyright

Using the Library
-----------------
The MD_Menu library allows definition and navigation of a menu system by moving 
between the menu nodes. At a leaf node, MD_Menu can either manage editing values 
or call user code.

Menu structures are defined in PROGMEM memory. They are linked into tree structures 
relationships using the IDs of other menu nodes nodes or leaf nodes. Leaf nodes 
are always associated with a user variable.

Extensive use is made of callbacks to user code to manage user input, display
output and getting/setting values for variables.

## User Input for Menu Navigation

Menu navigation is carried out under the control of user code invoked as
a callback routine. The code must comply with the *cbUserNav* function 
prototype. This callback routine implementation is dependent on the type 
of input hardware, but the return codes for the required actions must 
be one of the standardised *userNavAction_t* enumerated type.

Navigation is carried out with 4 keys:
- **INCREMENT** (NAV_INC). Move down a menu, move to the next value 
in a pick list or increment a numeric value being edited.
- **DECREMENT** (NAV_DEC). Move up a menu, move to the previous value 
in a pick list or decrement a numeric value being edited.
- **SELECT** (NAV_SEL). Select the current menu or pick list item, or 
confirm an edited numeric value.
- **ESCAPE** (NAV_ESC). Escape the current menu (back up one level) or
cancel changes to an edited value.

A variety of input hardware setups are demonstrated in the Test 
example code provided.

## Menu Display

Menu display is enabled by user code as a callback routine from the 
library. The callback must comply with the *cbUserDisplay* function 
prototype.

The callback is provided with a request of type *userDisplayAction_t* and 
a message to display.

Display hardware must be able to display one or two lines for the menu 
display. All menu screens are structured with the first line as title 
and the second as the current menu selection or currently edited value, 
as appropriate. If the display can only support one line, the first line 
is discarded and only the second line displayed.

A variety of display hardware setups are demonstrated in the Test 
example code provided.

## Memory Footprint

The limited amount of RAM available in micro controllers is a challenge for
menu systems, as they often contain large amounts of 'static' data as text 
labels and other status information.

The MD_Menu library uses statically allocated data located in PROGMEM for 
the menu system and only copies the current menu record into RAM. All user 
values reside in user code and are not duplicated in the library.

## Menu Management

![Data Structure Map] (Data_Structures.jpg "Data Structure Map")

As shown in the figure above, the library uses three types of objects, each
identified with a unique id within the object type. A menu header (of 
type *mnuHeader_t*) defines a menu. The header contains a label for the title and
the range of menu items (of type *mnuItem_t*) that should be displayed for 
the menu. Menu item ids between the start and end id locations include 
all the ids in locations in between, and should be in number sequence.

A menu item may lead to another menu (if it is a node in the menu tree) or an
input item (of type *mnuInput_t*) if it is a leaf of the menu system. The depth
of the menu tree is restricted by the defined MENU_STACK_SIZE constant. When
this limit is exceeded, the library will just ignore requests that cause
additinal menu depth but continues to run.

Menu input items define the type of value that is to be edited by the user and
parameters associated with managing the input for that value. Before the value
is edited a callback following the *cbValueRequest* prototype is called to 'get'
the pointer to the variable with the current value. The input item id is provided 
to identify which value is being requested. A copy of the user variable is used 
for editing and a second *cbValueRequest* (conceptually a 'set') is invoked 
after the value is updated, enabling the user code to take action on the change. 
If the variable edit is cancelled, the second *cbValueRequest* 'set' call does 
not occur.

Variables may be of the following types:
- **Pick List** specifies a PROGMEM character string with list items separated
by the '|' character (defined as INPUT_SEPARATOR), for example "Apple|Orange|Pear".
The list is specified as the pList parameter and the get/set value callback expects
a pointer to a uint8_t value that is the index of the current selection (zero based).
- **Boolean** for Input of boolean (Y/N) values. As the user makes changes, the
value changes between displays of 'Y' and 'N' (defined as INP_BOOL_T and INP_BOOL_F).
The get/set callback expects a pointer to bool type.
- **Integer** values can be specified as 8, 16 or 32 bits in size, with the get/set
callback expecting pointers to int8_t, int16_t and int32_t (note all signed values).
The input specification also allows a lower and upper bound to be set, as well as the
number's base (2 through 16) to be specified. Numeric values that overflow the
specified field with are prefixed by the '#' character (defined as INP_NUMERIC_OVERFLOW)
to indicate that this has occurred.
- **Floating point** where the library uses a 32 bit long integer and assumes 
the last 2 digits (defined by FLOAT_DECIMALS) to be the fraction after the decimal 
point (character defined as DECIMAL_POINT). Specification allows lower and upper bound 
to be set. The base specification field is used to represent the minimum increment or 
decrement of the fractional component of value input (ie, with 2 decimals, 1 is .01, 5 
is .05, 50 is 0.50, etc).
- **Run Code** specifies input fields that are designed to execute a user function
when selected. As there is no value to 'get' the get/set callback is only called when the
input is confirmed. User code can then be executed as part of the 'set' invocation.


\page pageCopyright Copyright
Copyright
---------
Copyright (C) 2017 Marco Colli. All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

\page pageRevisionHistory Revision History
Revision History
----------------
Jun 2017 version 1.2.0
- Returning nullptr from value request callback now stops value being edited.

Jun 2017 version 1.1.0
- Removed index field from menu item definition. Not useful in practice.
- Added real number (float) input

Jun 2017 version 1.0.1
- Added setAutoStart() method and code
- Internal flags now a bit field
- Added setTimeout() method and code

May 2017 version 1.0.0
- First implementation
*/
#include <Arduino.h>

/**
 * \file
 * \brief Main header file for class definition of the MD_Menu library
 */

// Label size definitions
#define MAX_TEMPSTR_SIZE (16+1)           ///< Maximum String Size for Temporary Buffers

const uint8_t HEADER_LABEL_SIZE = 16;   ///< Displayed length of a menu header label
const uint8_t ITEM_LABEL_SIZE = 9;     ///< Displayed length of a menu item label
const uint8_t INPUT_LABEL_SIZE = 9;    ///< Displayed length of an input item label

// Miscellaneous defines
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))  ///< Generic macro for obtaining number of elements of an array
const uint8_t MNU_STACK_SIZE = 4;       ///< Maximum menu 'depth'. Starting (root) menu occupies first level.

/**
 * Core object for the MD_Menu library
 */
class MD_Menu
{
public:
  //--------------------------------------------------------------
  /** \name Enumerated values and Typedefs.
  * @{
  */
  /** 
  * Common Action Id type
  * 
  * Record id numbers link the different parts of the menu together.
  * typedef this to make it easier to change to a different type in
  * future if required. Note that id -1 is used to indicate error or no
  * id, so value must be signed.
  */
  typedef int8_t mnuId_t;

  /**
  * Return values for the user input handler
  *
  * The menu navigation keys are implemented by user code that must
  * return one of these definesd types when it is invoked. The menu
  * navigation in the libray and data input is fully controlled by
  * this returned value.
  */
  enum userNavAction_t
  {
    NAV_NULL,  ///< There was no current selection to process
    NAV_INC,   ///< INCREMENT. Move to the next menu item or increment a value.
    NAV_DEC,   ///< DECREMENT. Move to the previous menu item or decrement a value.
    NAV_SEL,   ///< SELECT the current menu item or confirm a new value.
    NAV_ESC,   ///< ESCAPE from current menu or abandon editing a value (remains unchanged).
    NAV_HELP,  ///< HELP - Display a Help String (TODO)
  };

  /**
  * User input function prototype
  *
  * The user input function must handle the physical user interface
  * (eg, switches, rotary encoder) and return one of the userNavAction_t
  * enumerated types to trigger the next menu action.
  * The user funtion can also specify the incremental change quantity that 
  * to be applied for INC and DEC actions when editing a numeric variable
  * (default is 1) by changing the incDelta variable.
  */
  typedef userNavAction_t(*cbUserNav)(uint16_t &incDelta);

  /**
  * Request values for user display handler
  *
  * The display handler will receive requests that tell it what
  * needs to be done. The display must implement appropriate actions
  * on the display device to carry out the request.
  */
  enum userDisplayAction_t
  {
    DISP_CLEAR, ///< Clear the display. Message parameter is not defined
    DISP_L0,    ///< Display the data provided in line 0 (first line). For single line displays, this should be ignored. 
    DISP_L1,    ///< Display the data provided in line 1 (second line). This must always be implemented. 
    DISP_HELP,  ///< Display a help message (however you like). Text Data is always in Flash.
  };

  /**
  * User input function prototype
  *
  * The user input function must handle the physical user interface
  * (eg, switches, rotary encoder) and return one of the userNavAction_t
  * enumerated types to trigger the next menu action.
  */
  typedef bool(*cbUserDisplay)(userDisplayAction_t action, const char *msg);

  /**
  * Menu input type enumerated type specification.
  *
  * Used to define the the type input action a for variable menu
  * item so that it can be appropriately processed by the library.
  */
  enum inputAction_t
  {
    INP_LIST,   ///< The item is for selection from a defined list of values
    INP_BOOL,   ///< The item is for input of a boolean variable (Y/N)
    INP_INT8,   ///< The item is for input of an 8 bit unsigned integer
    INP_INT16,  ///< The item is for input of an 16 bit unsigned integer
    INP_INT32,  ///< The item is for input of an 32 bit unsigned integer
    INP_FLOAT,  ///< The item is for input of a real number representation with 2 decimal digits 
    INP_RUN,    ///< The item will run a user function
  };

  enum cdValueOp_t 
  {
    VAL_OP_GET,   // Get the value
    VAL_OP_SET,   // Set the value
    VAL_OP_TRY,   // Try the current value (ie, if a hardware value, feedback its result. eg, a servo will move to match the setting.)
  };
  /**
  * Data input/output function prototype
  *
  * This user function must handle the get/set of the input value
  * currently being handled by the menu.
  * When bGet is true, the function must return the pointer to the
  * data identified by the ID. Return nullptr to stop the menu from
  * editing the value.
  * 
  * When specified for a Menu, it can control: 
  * VAL_OP_GET : the Loading of items ready for the submenu
  * VAL_OP_SET : the Saving of items in the submenu
  * VAL_OP_TRY : Not Used for a submenu
  * 
  */
  typedef void*(*cbValueRequest)(mnuId_t id, cdValueOp_t op);

  /**
  * Input field defintion
  *
  * Defines the data value for input from the user. Each definition
  * contains enough data for the data collection to be managed by the
  * library.
  */
  struct mnuInput_t
  {
    mnuId_t        id;                          ///< Identifier for this item
    char           label[INPUT_LABEL_SIZE + 1]; ///< Label for this menu item
    inputAction_t  action;                      ///< Type of action required for this value
    cbValueRequest cbVR;                        ///< Callback function to get/set the value
    uint8_t        fieldWidth;                  ///< Width of the displayed field between delimiters
    // A union to save Flash Space
    union {
      struct {
        int32_t min;  ///< min/max values an integer Only for INY_INT* & INP_FLOAT
        int32_t max;  ///< min/max values an integer Only for INY_INT* & INP_FLOAT
        uint8_t base; ///< number base for display (2 through 16), for a float, number of decimals to show.
      } range;
      
      const char *pList;   ///< pointer to list string - Only for INP_LIST
    };
  };

  /**
  * Menu input type enumerated type specification.
  *
  * Used to define the the type input action a for variable menu
  * item so that it can be appropriately processed by the library.
  */
  enum mnuAction_t
  {
    MNU_MENU,   ///< The item is for selection of a new menu
    MNU_INPUT,  ///< The item is for input of a value
  };

  /**
  * Menu item definition
  *
  * Defines the item data for the menu. The items are conceptually 'child'
  * records of a menu header item, where a continuous range form part
  * of the menu.
  */
  struct mnuItem_t
  {
    mnuId_t     id;                         ///< Identifier for this item
    char        label[ITEM_LABEL_SIZE + 1]; ///< Label for this menu item
    mnuAction_t action;                     ///< Selecting this item does this action
    mnuId_t     actionId;                   ///< Next menu or input field Id
    const char* HelpMsg;                    ///< Optional Help Message
  };

  /**
  * Menu header definition
  *
  * Defines the header data for a menu. The items are defined separately.
  * This data structure encodes the contiguous range of menu item numbers 
  * that form part of the menu.
  */
  struct mnuHeader_t
  {
    mnuId_t id;          ///< Identifier for this item
    char    label[HEADER_LABEL_SIZE + 1]; ///< Label for this menu item
    mnuId_t idItmStart;  ///< Start item number for menu
    mnuId_t idItmEnd;    ///< End item number for the menu
    mnuId_t idItmCurr;   ///< Current item being processed
    cbValueRequest cbMVR; ///< Callback function to get/set the values of the submenu
  };

  /** @} */
  //--------------------------------------------------------------
  /** \name Class constructor and destructor.
  * @{
  */
  /**
   * Class Constructor.
   *
   * Instantiate a new instance of the class. The parameters passed define the
   * data structures defining the menu items and function callbacks required for
   * the library to interact with user code.
   *
   * \param cbNav		navigation user callback function
   * \param cbDisp  display user callback function
   * \param mnuHdr  address of the menu headers data table
   * \param mnuHdrCount number of elements in the header table
   * \param mnuItm  address of the menu items data table
   * \param mnuItmCount number of elements in the item table
   * \param mnuInp  address of the input definitions data table
   * \param mnuInpCount number of elements in the input definitions table
   */
  MD_Menu(cbUserNav          cbNav,  cbUserDisplay cbDisp,
          const mnuHeader_t *mnuHdr, uint8_t mnuHdrCount,
          const mnuItem_t *mnuItm, uint8_t mnuItmCount,
          const mnuInput_t *mnuInp, uint8_t mnuInpCount);

  /**
   * Class Destructor.
   *
   * Released allocated memory and does the necessary to clean up once the queue is
   * no longer required.
   */
  ~MD_Menu(void) {};

  /** @} */
  //--------------------------------------------------------------
  /** \name Methods for core object control.
  * @{
  */
  /**
  * Initialize the object.
  *
  * Initialise the object data. This needs to be called during setup() to initialise new
  * data for the class that cannot be done during the object creation.
  */
  void begin(void) {};

  /**
   * Run the menu.
   *
   * This should be called each time through the loop() function.
   * The optional parameter should be set to true when the menu display needs
   * to start (or restart) and false (or omitted) for normal running. This 
   * allows the user code to trigger the menu starting unless the setAutoStart()
   * option is set to start the menu automatically.
   * When running, the menu code coordinates user callbacks to obtain input and
   * display the menu, as needed.
   *
   * \param bStart Set to true is the menu needs to be started; defaults to false if not specified
   * \return true if the menu is still running, false otherwise
   */
  bool runMenu(bool bStart = false);

  /**
  * Check if library is running a menu.
  *
  * Returns boolean with the running status.
  *
  * \return true if running menu, false otherwise
  */
  bool isInMenu(void);

  /**
  * Check if library is editing a field.
  *
  * Returns boolean with the edit status.
  *
  * \return true if editing field, false otherwise
  */
  bool isInEdit(void);

  /** @} */
  //--------------------------------------------------------------
  /** \name Support methods.
  * @{
  */
  /**
  * Reset the menu.
  *
  * Change the current menu state to be not running and reset all menu
  * conditions to start state.
  */
  void reset(void);

  /**
  * Set the menu wrap option.
  *
  * Set the menu wrap option on or off. When set on, reaching the end 
  * of the menu will wrap around to the start of the menu. Simialrly, 
  * reaching the end will restart from the beginning.
  * Default is set to no wrap.
  *
  * \param bSet true to set the option, false to un-set the option (default)
  */
  void setMenuWrap(bool bSet);

  /**
  * Set the menu auto start option.
  *
  * Set the menu to start automatically in response to the SEL navigation selection.
  * When set on, pressing SEL when the menu is not running will start the menu display.
  * If the option is not set, the starting trigger needs to be monitored by the user 
  * code and the menu started by caling runMenu().
  * Default is not to auto start.
  *
  * \param bSet true to set the option, false to un-set the option (default)
  */
  void setAutoStart(bool bSet);

  /**
  * Set the menu inactivity timeout.
  *
  * Set the menu inactivity timeout to the specified value im milliseconds.
  * The menu will automatically reset is there is no key pressed in the specified
  * time. It is up to the user code to detect the menu is no longer running and 
  * transition to normal mode. A value of 0 disables the timeout (default).
  *
  * \param t the timeout time in milliseconds, 0 to disable (default)
  */
  void setTimeout(uint32_t t);
  
  /**
  * Set the user navigation callback function.
  *
  * Replace the current callback function with the new function.
  *
  * \param cbNav the callback function pointer.
  */
  void setUserNavCallback(cbUserNav cbNav);

  /**
  * Set the user display callback function.
  *
  * Replace the current callback function with the new function.
  *
  * \param cbDisp the callback function pointer.
  */
  void setUserDisplayCallback(cbUserDisplay cbDisp);

  /** @} */
private:
  // initialisation parameters and data tables
  cbUserNav _cbNav;       ///< User navigation function
  cbUserDisplay _cbDisp;  ///< User display function

  const mnuHeader_t *_mnuHdr;   ///< Menu header table
  uint8_t _mnuHdrCount;   ///< Number of items in the header table
  const mnuItem_t *_mnuItm;     ///< Menu item table
  uint8_t _mnuItmCount;   ///< Number of items in the item table
  const mnuInput_t *_mnuInp;    ///< Input item table
  uint8_t _mnuInpCount;   ///< Number of items in the input table

  // Status values and global flags
  uint8_t _options;       ///< bit field for options and flags

  // Timeout related
  uint32_t _timeLastKey;  ///< Time a menu key was last pressed
  uint32_t _timeout;      ///< Menu inactivity timeout in milliseconds

  // Input editing buffers and tracking
  void    *_pValue;  ///< Pointer to the user value being edited
  bool    _bValue;   ///< Copy of boolean value being edited
  int32_t _iValue;   ///< Copy of the integer/list index value being edited

  // static buffers for find functions, keep accessible copies of data in PROGMEM
  uint8_t     _currMenu;                ///< Index of current menu displayed in the stack
  mnuHeader_t _mnuStack[MNU_STACK_SIZE];///< Stacked trail of menus being executed
  mnuInput_t  _mnuBufInput;             ///< menu input buffer for load function
  mnuItem_t   _mnuBufItem;              ///< menu item buffer for load function

  // Private functions
  void       loadMenu(mnuId_t id = -1);   ///< find the menu header with the specified ID
  mnuItem_t  *loadItem(mnuId_t id);       ///< find the menu item with the specified ID
  mnuInput_t *loadInput(mnuId_t id);      ///< find the input item with the specified ID
  uint8_t    listCount(const /*PROGMEM*/ char *p);  ///< count the items in a list selection string 
  char       *listItem(const /*PROGMEM*/ char *p, uint8_t idx, char *buf, uint8_t bufLen);  ///< extract the idx'th item from the list selection string
  void       strPreamble(char *psz, uint8_t psz_size, mnuInput_t *mInp);  ///< format a preamble to the a variable display
  void       strPostamble(char *psz, uint8_t psz_size, mnuInput_t *mInp); ///< attach a postamble to a variable display
  
  void timerStart(void);    ///< Start (reset) the timout timer
  void timerCheck(void);    ///< Check if timout has expired and reset menu if it has

  void handleMenu(bool bNew = false);  ///< handling display menu seitems and navigation
  void handleInput(bool bNew = false); ///< handling user input to edit values

  // Process the different types of input requests
  // All return true when edit changes are finished (SELECT or ESCAPE).
  bool processList(userNavAction_t nav, mnuInput_t *mInp);
  bool processBool(userNavAction_t nav, mnuInput_t *mInp);
  bool processInt(userNavAction_t nav, mnuInput_t *mInp, uint16_t incDelta);
  bool processFloat(userNavAction_t nav, mnuInput_t *mInp, uint16_t incDelta);
  bool processRun(userNavAction_t nav, mnuInput_t *mInp);
};

