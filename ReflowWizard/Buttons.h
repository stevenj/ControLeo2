#ifndef __BUTTONS__
#define __BUTTONS__

#define NO_BUTTON_PRESSED       (0x00)

#define BUTTON_TOP_PRESS        (0x01)
#define BUTTON_TOP_RELEASE      (0x02)
#define BUTTON_TOP_LONG_HOLD    (0x03)
#define BUTTON_TOP_LONG_RELEASE (0x04)

#define BUTTON_BOT_PRESS        (0x05)
#define BUTTON_BOT_RELEASE      (0x06)
#define BUTTON_BOT_LONG_HOLD    (0x07)
#define BUTTON_BOT_LONG_RELEASE (0x08)

#define ENCODER_INC             (0x09)
#define ENCODER_DEC             (0x0A)

class ControLeo2_Buttons {
  public:
      ControLeo2_Buttons(void);
      
      void     ButtonProcessing(void);

      int8_t   GetAbsoluteEncoder(void);
      uint8_t  GetKeypress(void);

  private:
      void     push_keypress_on_queue(uint8_t keypress);
      uint32_t ProcessButton(uint8_t mask, uint8_t buttons, uint8_t BaseEvent, uint32_t current_time, uint32_t start_time);
      uint32_t ProcessHold(uint8_t Event, uint32_t current_time, uint32_t start_time);
      
      uint8_t  _button_queue[4];
      uint8_t  _queue_head;
      uint8_t  _queue_tail;

      uint8_t  _stable_buttons;
      int8_t   _encoder_value;

      uint32_t _top_press_start;
      uint32_t _bot_press_start;
      
};

#endif
