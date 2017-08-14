// Create the degree symbol for the LCD - you can display this with lcd.print("\1") or lcd.write(1)
const uint8_t custom_chars[64] PROGMEM  = 
    {
      // 0x00/0x08 - Stand Alone Degree Symbol
      B01100,
      B10010,
      B10010,
      B01100,
      B00000,
      B00000,
      B00000,
      B00000,

      // 0x01/0x09 - Degree Symbol AND Celsius Abreviation
      B01000,
      B10100,
      B01000,
      B00110,
      B01001,
      B01000,
      B01001,
      B00110,

      // 0x02/0x0A - UP Arrow
      B00100,
      B01110,
      B11111,
      B00100,
      B00100,
      B00100,
      B00100,
      B00000,
      
      // 0x03/0x0B - Down Arrow
      B00000,
      B00100,
      B00100,
      B00100,
      B00100,
      B11111,
      B01110,
      B00100,
      
      // 0x04/0x0C - Stable
      B01000,
      B11101,
      B10111,
      B00010,
      B01000,
      B11101,
      B10111,
      B00010,
      
      // 0x05/0x0D - Unused
      B00000,
      B00000,
      B00000,
      B00000,
      B00000,
      B00000,
      B00000,
      B00000,
      
      // 0x06/0x0E - Unused
      B00000,
      B00000,
      B00000,
      B00000,
      B00000,
      B00000,
      B00000,
      B00000,
      
      // 0x07/0x0F - Unused
      B00000,
      B00000,
      B00000,
      B00000,
      B00000,
      B00000,
      B00000,
      B00000,
    };

