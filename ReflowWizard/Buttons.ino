#if 0
/**** User Interface Inputs ****/
#define ENCODER_A              0      // PD2/INT2
#define ENCODER_B              1      // PD3/INT3
#define BUTTON_BOTTOM          2      // PD1/INT1
#define BUTTON_TOP            11      // PB7/PCINT7/OC1C/OC0A
#define ENCODER_C             12      // PD6/T1/OC4D
#endif

#include "ControLeo2.h"

const uint8_t encoder_states[8] PROGMEM = {
  // Specific to the Encoder on my oven, you will need to tweak to suit yours.
  // This Encoder ONLY has 6 states, so List those first, 
  // followed by the INVALID States
  // Bit 7 is set in an INVALID STATE
  
  B00001100, // 0x00
  B00000100, // 0x01
  B01000000, // 0x02
  B00001000, // 0x03
  B00000000, // 0x04
  B01000100, // 0x05
  B10001100, // 0x06
  B11001100, // 0x07
};

#define MAX_ENCODER (5)

int8_t  EncoderValue(uint8_t buttons) {
    // Get a Encoder Value for a Particular Input State
    uint8_t state;
    uint8_t val = 0;
    buttons &= B01001100; // Mask the Encoder Input Bits.

    do {
        state = pgm_read_byte_near(encoder_states + val);
        if (buttons == (state & 0x7F)) break;
        val++;
    } while ((state < 0x80) && (val < 8));

    if (state > 0x80) val = 0xFF; // Invalid State.

    return val;
}

ControLeo2_Buttons::ControLeo2_Buttons(void) {
    uint8_t buttons;

#define COMPATIBLE 0

#if COMPATIBLE
    pinMode(ENCODER_A,     INPUT_PULLUP);
    pinMode(ENCODER_B,     INPUT_PULLUP);
    pinMode(ENCODER_C,     INPUT_PULLUP);
    pinMode(BUTTON_BOTTOM, INPUT_PULLUP);
    pinMode(BUTTON_TOP,    INPUT_PULLUP);
#else
    // Actually best to initialise ALL IO ports once.
    DDRD  = DDRD  & B10110001; // Port D Inputs as Inputs
    DDRB  = DDRB  & B01111111; // Port B Input as Inputs
    PORTD = PORTD | B01001110; // Port D Inputs, pullups enabled
    PORTB = PORTB | B10000000; // Port B Input, pullup enabled
#endif    

    _queue_head = 0;
    memset(_button_queue, 0x00, sizeof(_button_queue));

    _top_press_start = 0;
    _bot_press_start = 0;

    buttons = PIND & B01001100;  // Read Encoder Input and mask everything else.
    _encoder_value = EncoderValue(buttons); // Starting Encoder Value

}

#define DEBOUNCE_TIME   (50000)  // 50ms Debounce Time
#define LONG_HOLD_TIME (500000) // 500ms Long Hold Time

int8_t ControLeo2_Buttons::GetAbsoluteEncoder(void) {
    return _encoder_value;  
}

uint8_t ControLeo2_Buttons::GetKeypress(void) {
    uint8_t keypress = NO_BUTTON_PRESSED;
    
    if (_queue_head != _queue_tail) {
        keypress = _button_queue[_queue_head];
        _queue_head = (_queue_head + 1) & (sizeof(_button_queue)-1);
    }
    
    return keypress;  
}

void ControLeo2_Buttons::push_keypress_on_queue(uint8_t keypress) {
    uint8_t next_tail = (_queue_tail+1) & (sizeof(_button_queue)-1);

    //Serial.print("Key Queued : ");
    //Serial.println(keypress);

    if (next_tail != _queue_head) { // Cant add to full queue
      _button_queue[_queue_tail] = keypress;
      _queue_tail = next_tail;
    }
}

uint32_t ControLeo2_Buttons::ProcessButton(uint8_t mask, uint8_t buttons, uint8_t BaseEvent, uint32_t current_time, uint32_t start_time) {
    // Process Button
    if ((_stable_buttons & mask) != (buttons & mask)) {
        if ((buttons & mask) == 0x00) {
            // KEYPRESS          
            push_keypress_on_queue(BaseEvent);
            start_time = current_time;
        } else {
            // RELEASE
            if (start_time == 0) {  // LONG HOLD RELEASE
                push_keypress_on_queue(BaseEvent+3);
            } else { // SHORT PRESS RELEASE
                push_keypress_on_queue(BaseEvent+1);              
                start_time = 0;                  
            }
        }
    }  

    return start_time;
}

uint32_t ControLeo2_Buttons::ProcessHold(uint8_t Event, uint32_t current_time, uint32_t start_time) {
    if (start_time != 0) {
        if ((current_time - start_time) > LONG_HOLD_TIME) {
            push_keypress_on_queue(Event);
            start_time = 0;
        }
    }    
    return start_time;
}

void ControLeo2_Buttons::ButtonProcessing(void) {
    // This function has specific knowledge of the IO Ports used and doesnt use Arduino IO.
    uint8_t buttons; 
    static uint8_t last_buttons = 0xff;
    
    unsigned long current_time  = micros();
    static unsigned long previous_time = 0;
  
    buttons = PIND & B01001110;  // Read Button Inputs and mask everything else.
    buttons = buttons | (PINB & B10000000); // Read the one button on Port B, merge with the others.

    //Serial.println(buttons);
  
    // Buttons now is the current state of all user inputs.
    // Bit7 = Top Button
    // Bit6 = Encoder C
    // Bit3 = Encoder B
    // Bit2 = Encoder A
    // Bit1 = Bottom Button
  
    if (last_buttons != buttons) {
      // Then unstable, so record new button state and time
      previous_time = current_time;
    } else {
      // Stable, so check if stable for long enough
      if ((current_time - previous_time) > DEBOUNCE_TIME) {
        if (_stable_buttons != buttons) {
          int8_t encoded = EncoderValue(buttons);

          if (encoded != _encoder_value) {
              // First process the encoder if its state changed.
#if 1              
              if ((encoded == _encoder_value + 1) ||
                  ((encoded == 0) && (_encoder_value == MAX_ENCODER))) {
                  // Incremented
                  push_keypress_on_queue(ENCODER_INC);
              } else if ((encoded == _encoder_value - 1) ||
                  ((encoded == MAX_ENCODER) && (_encoder_value == 0))) {
                  // Decremented
                  push_keypress_on_queue(ENCODER_DEC);
#else
              if (encoded == (_encoder_value + 1) % (MAX_ENCODER+1)) {
                  // Incremented
                  push_keypress_on_queue(ENCODER_INC);
              } else if (encoded == (_encoder_value-1) % (MAX_ENCODER+1)) {
                  // Decremented
                  push_keypress_on_queue(ENCODER_DEC);

#endif                  
              }  else {
                  // Neither Incremented or Decremented - so preserve state.
                  encoded = _encoder_value;
              }
              _encoder_value = encoded; // Save current stable encoder value
          }

#if 0
          // Process Top Button
          if ((_stable_buttons & 0x80) != (buttons & 0x80)) {
              if ((buttons & 0x80) == 0x00) {
                  push_keypress_on_queue(BUTTON_TOP_PRESS);
                  _top_press_start = current_time;
              } else {
                  push_keypress_on_queue(BUTTON_TOP_RELEASE);
                  _top_press_start = 0;                  
              }
          } else if (_top_press_start != 0) {
              if ((current_time - _top_press_start) > LONG_HOLD_TIME) {
                  push_keypress_on_queue(BUTTON_TOP_LONG_HOLD);
                  _top_press_start = 0;
              }
          }

          // Process Bottom Button
          if ((_stable_buttons & 0x02) != (buttons & 0x02)) {
              if ((buttons & 0x02) == 0x00) {
                  push_keypress_on_queue(BUTTON_BOT_PRESS);
                  _bot_press_start = current_time;
              } else {
                  push_keypress_on_queue(BUTTON_BOT_RELEASE);
                  _bot_press_start = 0;                  
              }
          } else if (_bot_press_start != 0) {
              if ((current_time - _bot_press_start) > LONG_HOLD_TIME) {
                  push_keypress_on_queue(BUTTON_BOT_LONG_HOLD);
                  _bot_press_start = 0;
              }
          }
#else
          // Process Top Button
          _top_press_start = ProcessButton(0x80, buttons, BUTTON_TOP_PRESS, current_time, _top_press_start);
          // Process Bottom Button
          _bot_press_start = ProcessButton(0x02, buttons, BUTTON_BOT_PRESS, current_time, _bot_press_start);
#endif

          _stable_buttons = buttons;
        } else {        
          // Process Long Hold
#if 1          
          if (_top_press_start != 0) {
              if ((current_time - _top_press_start) > LONG_HOLD_TIME) {
                  push_keypress_on_queue(BUTTON_TOP_LONG_HOLD);
                  _top_press_start = 0;
              }
          }
          if (_bot_press_start != 0) {
              if ((current_time - _bot_press_start) > LONG_HOLD_TIME) {
                  push_keypress_on_queue(BUTTON_BOT_LONG_HOLD);
                  _bot_press_start = 0;
              }
          }
#else
          _top_press_start = ProcessHold(BUTTON_TOP_LONG_HOLD, current_time, _top_press_start);
          _bot_press_start = ProcessHold(BUTTON_BOT_LONG_HOLD, current_time, _bot_press_start);
          
#endif          
        }
      }  
    }
    last_buttons = buttons;
}

