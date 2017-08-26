// Relay Management and Control

#include "Relays.h"
      
ControLeo2_Relays::ControLeo2_Relays(void) {
  for (uint8_t i = RELAY_COOLING_FAN; i <= RELAY_D7; i++) {
    // Array only has Physical entries, so adjust the index accordingly.
    if (i <= RELAY_TOP_ELEMENT) {
      RelayAssignment[i-1] = RELAY_UNUSED;
    } else {
      // Initialise Relay Driver Outputs
      pinMode(4 + i - RELAY_D4, OUTPUT);
      digitalWrite(4 + i - RELAY_D4, LOW);   
      RelayDuty[i - RELAY_D4] = 0; // Default to 0% Duty Cycle (Relay off)
    }
  }
  
  PWMCounter = 0;
}

// Assign a physical relay to a virtual Relay
void ControLeo2_Relays::AssignRelay(ControLeo2_Relays::RELAY Phys, ControLeo2_Relays::RELAY Virt) {
  if ((Phys >= RELAY_D4) && (Virt < RELAY_D4)) {
    RelayAssignment[Virt-1] = Phys; // Virt is -1 because there is no "virtual" UNUSED Relay.
  }
}

void ControLeo2_Relays::SetRelay(ControLeo2_Relays::RELAY relay, uint8_t duty) {
  uint8_t max_pwr;
  
  duty = min(duty,100);
  
  if ((relay > RELAY_UNUSED) && (relay <= RELAY_TOP_ELEMENT)) {
    relay = RelayAssignment[relay-1]; // Convert Virtual Relay to Physical Relay
  }

  if (relay >= RELAY_D4) { // Here we should only have a Physical Relay, or an Unused Relay
    // Do nothing unless its a physical relay, ignore Unused ones.
          
    // Convert relay to be from 0-4.
    relay = (ControLeo2_Relays::RELAY)(relay - RELAY_D4);

    Serial.print("Relay ");
    Serial.print(relay);

    max_pwr = readGlobalSetting((SG_Entries_t)(SG_D4_MAXPWR + relay));

    Serial.print(" max_pwr = ");
    Serial.print(max_pwr);

    Serial.print(" duty = ");
    Serial.print(duty);

    if (max_pwr == 0) { // Max Power is Zero, so never turn on the relay.
      duty = 0; 
    } else if (max_pwr < 100) { // Max Power less than 100, so scale power down to that range.
      // Rescale the Relays Power to match the MAXIMUM Power allowed on the Relay.
      // So for example, if the Boost Element is set at 60%, then setting
      // its Duty cycle to 100% will result in a Duty Cycle of 60% = 60% of Power.
      // Duty=50%, Power=60%, Resultant Duty = 30% (50% of Maximum Power), and so on.
      
      duty = map(duty, 0, 100, 0, max_pwr);
    } // Otherwise max_pwr = 100, so do not alter power settings.

    Serial.print(" power = ");
    Serial.println(duty);
    
    RelayDuty[relay] = duty; // Set New Duty Cycle.
  }
  
}

void ControLeo2_Relays::ProcessRelays(void) {
  uint8_t local_PWMCounter;
  uint8_t dutyAdjust;
  uint8_t phaseDuty;

  const uint8_t dutySpread[4] = {0,2,1,3};
  
  auto incPWM = [&]( uint8_t PWM, uint8_t inc ) -> uint8_t
  {
    PWM += inc;
    if (PWM > 99) PWM -= 100; // Wrap PWM at end of range (0-99)
    return PWM;
  };
  
  // Handle the Relay Slow PWM.
  if (lastRelayTime + PWM_MIN_FREQ_US <= micros()) {
    lastRelayTime = micros();
    
    PWMCounter = incPWM(PWMCounter,1); // Next PWM State
    local_PWMCounter = PWMCounter;

    for (uint8_t i = 0; i < sizeof(RelayDuty); i++) {
      // Which phase in the 100 state PWM are we in, 0-3.
      dutyAdjust = local_PWMCounter / 25;    

      // Create a Current Duty relative to the current phase.
      // Spreads the expected Duty evenly over 4 sub phases, rather than
      // clustering the duty into the bottom of the entire PWM state.
      phaseDuty = (dutyAdjust * 25) + ((RelayDuty[i] + dutySpread[dutyAdjust]) >> 2); // Div 4
     
      if (phaseDuty > local_PWMCounter) {
        Serial.print((i*20)+10); 
        Serial.print("\t");
        
        // Assert Relay
        digitalWrite(4 + i, HIGH);   
      } else {
        Serial.print((i*20)); 
        Serial.print("\t");
        
        // Negate Relay
        digitalWrite(4 + i, LOW);   
      }

      // Each Relay is offset by 6 PWM Steps, to spread the turnon/off over the one second sub interval.
      // We do this to help reduce power surges from turning on all heating elements simultaneously.
      local_PWMCounter = incPWM(local_PWMCounter,6);       
    }
    Serial.println();
  }
}

