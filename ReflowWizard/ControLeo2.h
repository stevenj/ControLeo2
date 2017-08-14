// This library controls the functions of ControLeo
//  - LCD display and backlight
//  - Buzzer
//  - Thermocouple

// Written by Peter Easton
// Released under WTFPL license
//
// Change History:
// 14 August 2014        Initial Version

#ifndef CONTROLEO2_H
#define CONTROLEO2_H

/**** 2 Line x 16 Character HD44780U LCD ****/
#define LCD_RS                A0      // PF7
#define LCD_ENABLE            A1      // PF6
#define LCD_D0                A2      // PF5
#define LCD_D1                A3      // PF4
#define LCD_D2                A4      // PF1
#define LCD_D3                A5      // PF0

/**** User Interface Inputs ****/
#define ENCODER_A              0      // PD2/INT2
#define ENCODER_B              1      // PD3/INT3
#define BUTTON_BOTTOM          2      // PD1/INT1

/**** SERVO Output ****/
#define SERVO_OUTPUT           3      // PD0/INT0/OC0B

/**** Relay/Led IO ****/
#define RELAY_OUTPUT_1         4      // PD4
#define RELAY_OUTPUT_2         5      // PC6
#define RELAY_OUTPUT_3         6      // PD7
#define RELAY_OUTPUT_4         7      // PE6

/**** ThermoCouple IO ****/
#define THERMOCOUPLE_MISO_PIN  8      // PB4
#define THERMOCOUPLE_CS_PIN    9      // PB5
#define THERMOCOUPLE_CLK_PIN  10      // PB6

/**** User Interface Inputs ****/
#define BUTTON_TOP            11      // PB7/PCINT7/OC1C/OC0A
#define ENCODER_C             12      // PD6/T1/OC4D

/**** Piezo Buzzer Output ****/
#define BUZZER_OUTPUT         13      // PC7/OC4A

#include "ControLeo2_LCD.h"
#include "ControLeo2_MAX31855.h"
#include "Buttons.h"

#if 0
// Defines for the 2 buttons
#define CONTROLEO_BUTTON_TOP_PIN     11  // Top button is on D11
#define CONTROLEO_BUTTON_BOTTOM_PIN  2   // Bottom button is on D2
#define CONTROLEO_BUTTON_NONE        0
#define CONTROLEO_BUTTON_TOP         1  // S1
#define CONTROLEO_BUTTON_BOTTOM      2  // S2
#endif

// The Buzzer is on D13
#define CONTROLEO_BUZZER_PIN         13



#endif // CONTROLEO2_H
