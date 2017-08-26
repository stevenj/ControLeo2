// Timer 1 is used for 1 thing:
// 1. Control the servo used to open the oven door
//
// Servo timer interrupt operation
// ===============================
// See http://en.wikipedia.org/wiki/Servo_control for more information regarding servo operation.
// Note: Some (most) servos only rotate around 90°, not the 180° that is possible in theory
// The servo position information should be sent every 20ms, or 50 times per second.  To do this:
//   - Timer 1 is set to CTC mode
//   - Compare A is set to a value to force a timer interrupt every 20ms
// If servo movement is enabled (interrupt on Compare B, OCIE1B is set) then the servo pin
// is set high.  It must be lowered somewhere between 1ms and 2ms later, depending on the desired
// position.  To do this, the appropriate value is written to OCR1B.  Keep in mind that unlike
// Compare A, Compare B does not reset Timer 1's counter.  The steps look something like this:
//   a. Counter = 0: Write servo pin HIGH, and set Compare B to correct duration.
//   b. Counter = Compare B: Write servo pin low
//   c. Counter = Compare A: Counter is set back to 0 (go to a.)
// Once the servo has reached the desired position, the Compare B continues to generate a pulse the appropriate width
//
// With a 16MHz clock, the prescaler is set to 8.  This gives a timer speed of 16,000,000 / 8 = 2,000,000. This means
// the timer counts from 0 to 2,000,000 in one second.  We'd like the interrupt to fire 50 times per second so we set
// the compare register OCR1A to 2,000,000 / 50 = 40,000.
// Servo can move bettween 0 Degrees (1ms Pulse) and 180 Degrees (2ms Pulse)
// 1ms = 2,000,000/1,000 = 2,000
// 2ms = 2,000,000/500   = 4,000
// So we can control the servo in up to 2000 steps (0.09 Degrees) and no finer.

#include "ControLeo2.h"

#define SERVO_CLK (16000000)
#define SERVO_PS  (8)
#define SERVO_HZ  (50)
#define SERVO_PERIOD ((SERVO_CLK/SERVO_PS) / SERVO_HZ)

#define SERVO_MS_PER_S (1000)
#define SERVO_MIN_MS   (1)
#define SERVO_MAX_MS   (2)

#define SERVO_CLK_PER_MS ((SERVO_CLK/SERVO_PS) / SERVO_MS_PER_S)

#define MIN_PULSE_WIDTH   (SERVO_CLK_PER_MS * SERVO_MIN_MS)
#define MAX_PULSE_WIDTH   (SERVO_CLK_PER_MS * SERVO_MAX_MS)

// #define MIN_PULSE_WIDTH       544     // The shortest pulse sent to a servo (from Arduino's servo library)
// #define MAX_PULSE_WIDTH      2400     // The longest pulse sent to a servo (from Arduino's servo library)

// Variables used to control servo movement
static uint16_t servoEndValue;       // The desired pulse width
static int16_t  servoIncrement;      // The amount to increase/decrease the pulse every interrupt

// Initialize Timer 1
// This timer controls the servo
// It should fire 50 times every second (every 20ms)
void initializeServo(void) {
  Serial.println("Starting Servo");

  // Set the servo pin as output - Servo Disabled (Low Pulse)
  pinMode(SERVO_OUTPUT, OUTPUT);
  digitalWrite(SERVO_OUTPUT, LOW);

  // Put the Servo into the configured Retracted position.
  servoEndValue = degreesToTimerCounter(readGlobalSetting(SG_SERVO_RETRACT_DEG));
  // servoEndValue = (MIN_PULSE_WIDTH + MAX_PULSE_WIDTH) /2 ;
  servoIncrement = 0;

  cli();                               // Disable global interrupts
  TCCR1A =  0x00;                      // COM1x1/COM1x0 = 00 = Normal port operation, OCnA/OCnB/OCnC disconnected
                                       // WGM11/WGM10 = 00
  
  TCCR1B =  _BV(WGM12) + _BV(CS11);    // WGM3210 = 0100 = Clear Timer on Compare with OCR1A 
                                       // ICNC1 = 0 = Input Capture Noise Canceler DISABLED
                                       // ICES1 = 0 = Input Capture Falling Edge (UNUSED)
                                       // CS12/CS11/CS10 = 010 = PRESCALE CLK DIV 8
                                       
  TCNT1  =  0x0000;                    // Clear the timer count 
  
  OCR1A  =  SERVO_PERIOD;              // Set compare match so the interrupt occurs 50 times per second
  OCR1B  =  servoEndValue;             // Default Servo starting position. 
  
  TIMSK1 = _BV(OCIE1A) | _BV(OCIE1B);  // IRQ when Timer = OCR1A & OCR1B
  
  sei();                               // Enable global interrupts

  Serial.println("Servo Hardware Initialised"); 
  
}

// Timer 1 ISR
// This timer fires 50 times per second (every 20ms)
ISR(TIMER1_COMPA_vect)
{ 
    uint16_t ServoPos;

    //digitalWrite(4, HIGH);    // Test Output to see if ISR fires.
    
    // Write the servo signal pin high.  This is lowered when Compare B fires
#if (SERVO_OUTPUT == 3) 
    // Optimised set of the servo pin.
    PORTD |= 0x01;
#else
    digitalWrite(SERVO_OUTPUT, HIGH);
#endif            

    ServoPos = (uint16_t)OCR1B;
    if (servoEndValue != ServoPos) { // Then Movement is needed
        ServoPos += servoIncrement;
        
        if (((servoIncrement > 0) && (ServoPos > servoEndValue)) ||
            ((servoIncrement < 0) && (ServoPos < servoEndValue)) ||
            (servoIncrement == 0))
        {
            ServoPos = servoEndValue;
        }
        // Move the servo to the next position
        OCR1B = ServoPos;
    }
}


// Timer 1 Compare B interrrupt
// This interrupt fires once the desired pulse duration has been sent to the servo
#if (SERVO_OUTPUT == 3) // Optimised version - Two instructions (Set port bit low and return).
ISR(TIMER1_COMPB_vect, ISR_NAKED)
{
  PORTD &= 0xFE;
  reti();
}
#else
ISR(TIMER1_COMPB_vect)
{
  digitalWrite(SERVO_OUTPUT, LOW);
}
#endif

// Move the servo to servoDegrees, in timeToTake milliseconds (1/1000 second)
void setServoPosition(uint8_t servoDegrees, uint16_t timeToTake) {
    int16_t newEnd;
    int16_t currentEnd;
    int16_t newStep;
  
    // Make sure the degrees are 0 - 180
    if (servoDegrees > 180)
        servoDegrees = 180;
      
    // Figure out what the end timer value should be
    newEnd = degreesToTimerCounter(servoDegrees);
    
    // If the servo is already in this position, then don't do anything
    currentEnd = (int16_t)OCR1B;
    
    if (newEnd != currentEnd) {
        if (timeToTake != 0) {
            // Figure out the timer increment to achieve the end value
            newStep = ((newEnd - currentEnd) << 4) / (timeToTake / (SERVO_MS_PER_S/SERVO_HZ));
            // Round - Away from Zero.
            if (newEnd > currentEnd) {
              newStep = (newStep + 8) >> 4; 
            } else {
              newStep = (newStep - 8) >> 4; 
            }              
            
            if (newStep == 0) { // Underflow to correct
              if (newEnd > currentEnd) {
                newStep = 1;
              } else {
                newStep = -1;
              }              
            }
        } else {
            // Immediate Update
            newStep = 0;            
        }
        
        cli();                    // Prevent a Servo Pulse mid way through an update.
        servoEndValue = newEnd;   // The desired pulse width
        servoIncrement = newStep; // The amount to increase/decrease the pulse every interrupt
        sei();
    }
} 


// Convert degrees (0-180) to a timer counter value
int16_t degreesToTimerCounter(uint16_t servoDegrees) {
  // Get the pulse duration in microseconds (as a timer value);
  return (map(servoDegrees, 0, 180, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH));
}

// Check if the servo is moving, or stationary. 
// (Actually just checks if the pulse is changing, servo movement could be delayed from the pulse.)
bool ServoMoving(void) {
  return ((uint16_t)OCR1B != servoEndValue);
}

