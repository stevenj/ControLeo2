// This library is derived from Adafruit's I2C LiquidCrystal library.
// Adafruit provide excellent products and support for makers and tinkerers.
// Please support them from buying products from their web site.
// https://github.com/adafruit/LiquidCrystal/blob/master/LiquidCrystal.cpp
//
// This library controls the LCD display, the LCD backlight and the buzzer.  These
// are connected to the microcontroller using a I2C GPIO IC (MCP23008).
//
// Written by Peter Easton
// Released under WTFPL license
//
// Change History:
// 14 August 2014        Initial Version
//  1 August 2017        Steven Johnson
//                       Rewritten to act like a refreshed frame buffer.
//                       Provide an ability to show scrolling long messages on one line at a time.
//                       Need to call "refresh" at an appropriate interval to allow the screen to redraw
//                       20 times a second is more than fast enough.
//            

#include "ControLeo2.h"

// Commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// Flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// Flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// Flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// Flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

#define LCD_COMMAND(x) send(x, LOW);
#define LCD_DATA(x)    send(x, HIGH);

// Pin Definitions for using the LCD (Constant and in Flash)
const uint8_t _lcd_pins[] PROGMEM = {LCD_D0, LCD_D1, LCD_D2, LCD_D3, LCD_RS, LCD_ENABLE};

// Helper Macro to read Flash Array
#define LCD_PIN(x) pgm_read_byte_near(_lcd_pins + x)
#define LCD_RS_PIN  (4)
#define LCD_ENABLE_PIN (5)

#define SCROLL_SPEED (4) // HZ (Maximum)


ControLeo2_LCD::ControLeo2_LCD(void) {
  
    // Set all the pins to be outputs
    for (uint8_t i=0; i < sizeof(_lcd_pins); i++) {
      pinMode(LCD_PIN(i), OUTPUT);     
    }      
  
    // Reset Frame Buffer
    clear();
    _scroll_line_rpt  = 0; // 0 = Off, 127 = Continuous, 1-126 = Show this many times.
    
    _cursor_xy = 0x00;     // Home Cursor position by default, cursor not displayed, screen is displayed;
    _screen_on = true;

    // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
    // According to datasheet, we need at least 40ms after power rises above 2.7V
    // before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
    delayMicroseconds(50000);
    
    // Now we pull both RS and R/W low to begin commands
    digitalWrite(LCD_PIN(LCD_RS_PIN), LOW);
    digitalWrite(LCD_PIN(LCD_ENABLE_PIN), LOW);
    
    //Put the LCD into 4 bit mode
    // This is according to the hitachi HD44780 datasheet figure 24, pg 46
    // We start in 8bit mode, try to set 4 bit mode
    write4bits(0x03);
    delayMicroseconds(4500); // wait min 4.1ms
    
    // Second try
    write4bits(0x03);
    delayMicroseconds(4500); // wait min 4.1ms
    
    // Third go!
    write4bits(0x03);
    delayMicroseconds(150);
    
    // Finally, set to 8-bit interface
    write4bits(0x02);
    
    // Finally, set # lines, font size, etc. - Never changes, only set once.
    LCD_COMMAND(LCD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS);

    // Initialize to default text direction (for romance languages) - Never Changes
    LCD_COMMAND(LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);
  
    // Clear the display
    LCD_COMMAND(LCD_CLEARDISPLAY);    // Clear display, set cursor position to zero
    delayMicroseconds(2000);          // This command takes a long time!    
    LCD_COMMAND(LCD_RETURNHOME);      // set cursor position to zero and remove any shifts, etc.
    delayMicroseconds(2000);          // This command takes a long time!    

}

// Allows us to fill the first 8 CGRAM locations
// with custom characters - Note, the character map is expected to be constant and in Flash (PROGMEM)
void ControLeo2_LCD::defineCustomChars(const uint8_t *charmap) {
    _custom_chars = charmap;
    upload_charset(_custom_chars);
}

void ControLeo2_LCD::upload_charset(const uint8_t *charmap) {
    if (charmap != NULL) {
        LCD_COMMAND(LCD_SETCGRAMADDR); // Start of LCD Character Ram
        
        for (uint8_t i=0; i < 8*8; i++) { // Have 8 Soft Characters (8 Bytes Wide) we can set.
            LCD_DATA(pgm_read_byte_near(charmap + i));
        }
    }
}

void ControLeo2_LCD::clear(void) {
    memset(_frame_buffer, ' ', sizeof(_frame_buffer));
}

void ControLeo2_LCD::PrintStr(uint8_t x, uint8_t y, const char* str) {
    uint8_t character = *str++;
    
    while ((x < 16) && (y < 2) && (character != 0x00)) {
        setChar(x,y,character);
        x++;
        character = *str++;
    }
}

void ControLeo2_LCD::PrintStr(uint8_t x, uint8_t y, const __FlashStringHelper* str) {
    uint8_t *strp = (uint8_t*)(str);
    uint8_t character = pgm_read_byte_near(strp);
    strp++;
    
    while ((x < 16) && (y < 2) && (character != 0x00)) {
        setChar(x,y,character);
        x++;
        character = pgm_read_byte_near(strp);
        strp++;
    }
}

extern char *ltostr(char *buf, uint8_t bufLen, int32_t v, uint8_t base, bool leadZero = false);

void ControLeo2_LCD::PrintInt(uint8_t x, uint8_t y, uint8_t width, uint16_t value, char fill) {
    
#if 1  
    //  Reuse ltostr from MD_Menu, rather than our own function, saves ROM space.
    if (x + width >= 16) {
      width = 16 - x;
    }
    ltostr((char*)&_frame_buffer[y][x], width+1, value, 10, (fill == '0'));
#else  
    char    character;
    uint8_t digit = width;
    while (digit > 0) {
        if (value == 0) {
            if (digit == width) {
                character = '0';
            } else {
                character = fill;
            }
        } else {
            character = 0x30 + (value % 10);
            value = value / 10;
        }
        
        digit--;
        setChar(x+digit, y, character);
    }
#endif    
}

void ControLeo2_LCD::PrintInt(uint8_t x, uint8_t y, uint8_t width, uint16_t value) {
  PrintInt(x,y,width,value,' ');
}

void ControLeo2_LCD::setChar(uint8_t x, uint8_t y, uint8_t character) {
    if ((x < 16) && (y < 2)) {
        _frame_buffer[y][x] = character;
    }
}

void ControLeo2_LCD::ScrollLine(uint8_t y, uint8_t rpt, const __FlashStringHelper* str) {
    _scroll_msg = (uint8_t*)(str);

    _scroll_rpt = rpt;
    _scroll_line = y;
    _scroll_x = -16;  // Start with Spaces.
}

bool ControLeo2_LCD::LineScrolling(void) {
  return (_scroll_rpt != 0);
}

void ControLeo2_LCD::CursorOn(uint8_t x, uint8_t y, bool blink, bool underline) {
    _cursor_on    = underline;
    _cursor_blink = blink;
    _cursor_y     = y & 0x1;  // 0-1 maximum range
    _cursor_x     = x & 0xF;  // 0-15 maximum range
}

void ControLeo2_LCD::CursorOn(uint8_t x, uint8_t y, bool blink = false) {
  CursorOn(x,y,blink,true);
}

void ControLeo2_LCD::CursorOn(uint8_t x, uint8_t y) {
    CursorOn(x,y,false,true);
}

void ControLeo2_LCD::CursorOff(void) {
    _cursor_on    = false;
    _cursor_blink = false;
}

void ControLeo2_LCD::ScreenOff(void) {
    _screen_on    = false;
}

void ControLeo2_LCD::ScreenOn(void) {
    _screen_on    = true;
}

void ControLeo2_LCD::refresh(void) {
    // Call this function periodically to update the LCD, otherwise nothing will be written.
    // Recomend calling approximately 20 times per second.  
    // Which should be more than fast enough for a two line LCD.
    uint8_t disp_cntl = LCD_DISPLAYCONTROL;
    uint8_t y = 0;
    uint8_t rpt;

    unsigned long current_time  = micros();
    static unsigned long previous_time = 0;
    
    if (_screen_on) {
        disp_cntl |= LCD_DISPLAYON;     
    }
   
    // If the screen is to be turned off, do it before updating, otherwise just hide cursor.
    LCD_COMMAND(disp_cntl)

    do {
        // Move Update Address to Required Line
        LCD_COMMAND(LCD_SETDDRAMADDR | (y * 0x40));

        rpt = 0;
        if (_scroll_line == y) rpt = _scroll_rpt;

        if (rpt > 0) {
            uint8_t char_x;
            int8_t  this_x = _scroll_x;
            bool    end_line = false;
            uint8_t x = 0;

            do {
                // Position Less than Zero is space filled.
                if (this_x < 0) {
                  char_x = ' ';
                } else {
                  char_x = pgm_read_byte_near(_scroll_msg + this_x);
                  if (char_x == 0x00) {
                    char_x = ' ';
                    this_x = -16;
                    if (x == 0) end_line = true;
                  }
                }

                this_x++;

                LCD_DATA(char_x);
                x++;
            } while (x < 16);

            if ((current_time - previous_time) >= (1000000 / SCROLL_SPEED)) {
              if (end_line) {
                _scroll_x = -16;
                if (_scroll_rpt < 127) _scroll_rpt--;
              } else {
                _scroll_x++;
              }           

              previous_time = current_time;
            }
        } else {
            for (uint8_t x = 0; x < 16; x++) {
                LCD_DATA(_frame_buffer[y][x]);
            }
        }
        y++;      
    } while (y < 2);      

    // If the screen is to be turned off, do it before updating.
    if (_screen_on) {
        if (_cursor_on) {
          disp_cntl |= LCD_CURSORON;
        }
        
        if (_cursor_blink) {
          disp_cntl |= LCD_BLINKON;
        }
        
        // Move cursor to the required location
        LCD_COMMAND(LCD_SETDDRAMADDR | ((0x40*_cursor_y) + (_cursor_x)) );
        
        LCD_COMMAND(disp_cntl)
    }
}

void ControLeo2_LCD::send(uint8_t value, uint8_t mode) {
    digitalWrite(LCD_PIN(LCD_RS_PIN), mode);
    
    write4bits(value>>4);
    write4bits(value);
}

void ControLeo2_LCD::write4bits(uint8_t value) {
    for (uint8_t i = 0; i < 4; i++)
        digitalWrite(LCD_PIN(i), (value >> i) & 0x01);
    
    // Pulse enable
    digitalWrite(LCD_PIN(LCD_ENABLE_PIN), LOW);
    delayMicroseconds(1);
    digitalWrite(LCD_PIN(LCD_ENABLE_PIN), HIGH);
    delayMicroseconds(1);    // enable pulse must be >450ns
    digitalWrite(LCD_PIN(LCD_ENABLE_PIN), LOW);
    delayMicroseconds(100);   // commands need > 37us to settle
}

