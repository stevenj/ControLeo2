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

#ifndef CONTROLEO2_LCD_h
#define CONTROLEO2_LCD_h

class ControLeo2_LCD {
  public:
      ControLeo2_LCD(void);

      void clear(void);
      
      void PrintStr(uint8_t x, uint8_t y, const char* str);
      void PrintStr(uint8_t x, uint8_t y, const __FlashStringHelper* str);

      void PrintInt(uint8_t x, uint8_t y, uint8_t width, uint16_t value, char fill);
      void PrintInt(uint8_t x, uint8_t y, uint8_t width, uint16_t value);

      void ScrollLine(uint8_t y, uint8_t rpt, const __FlashStringHelper* str);

      void CursorOff(void);
      void CursorOn(uint8_t x, uint8_t y, bool blink, bool underline);
      void CursorOn(uint8_t x, uint8_t y, bool blink);
      void CursorOn(uint8_t x, uint8_t y);
      
      void ScreenOff(void);
      void ScreenOn(void);

      void setChar(uint8_t x, uint8_t y, uint8_t character);
  
      void defineCustomChars(const uint8_t *charmap);

      void refresh(void);
      
      void send(uint8_t, uint8_t);
      
  private:      
      void upload_charset(const uint8_t *charmap);
      void write4bits(uint8_t);
      
      uint8_t _displaycontrol;

      const uint8_t *_custom_chars; // Address of Standard mode Custom Characters in Flash.

      union
      {
        uint8_t _cursor_xy;
        struct {
          uint8_t _cursor_x:4;  // X Position of Cursor on a Line (0-15)
          uint8_t _cursor_y:1;  // Y Position of Cursor (Line) (0 or 1)
          bool _cursor_blink:1; // Blink character at Cursor, or not.
          bool _cursor_on:1;    // Is the Cursor Displayed or Not.
          bool _screen_on:1;    // Is the Screen Displayed or Not.
        };
      };

      uint8_t _frame_buffer[2][16];
      uint8_t *_scroll_msg;               // Scrolling message
      union
      {
        uint8_t _scroll_line_rpt;         // Bit 7 = Line, Bit 0-6 = Number of times to show it. (0 = Off, 127 = Forever)
        struct {
          uint8_t _scroll_rpt:7;          // Repeat Counter
          uint8_t _scroll_line:1;         // Scroll Line
        };
      };
      int8_t  _scroll_x;                // Position of Scroll. -16 -> -1 = Spaces, 0 -> _scroll_size = Characters from the buffer
                                        // Because its signed, maximum practical line size is 126 Characters.
};

#endif //CONTROLEO2_LCD_h
