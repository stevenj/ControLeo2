#ifndef __RELAYS__
#define __RELAYS__

// Due to the slow nature of AC PWM, we break the PWM stream into 4 equal parts and divide the PWM between them,
// such that at the end of the 4 phases, a full OWM of the required Duty has been completed.
// However this allows the PWM to be changed much faster and maintain the required Duty cycles.

#define AC_HZ            (50)             // 50 or 60 Hz Electricity
#define AC_HALF_WAVE_HZ  (AC_HZ * 2)      // Number of Half waves per second

#define AC_HZ_MIN_CYCLES (4)              // Minimum number of Half Wave cycles SSR Enabled for. (Also Multiple of enable)

#define PWM_MIN_FREQ_US  ((1000000 * AC_HZ_MIN_CYCLES) / AC_HALF_WAVE_HZ)
                                          // Microseconds per SSR Update for PWM

class ControLeo2_Relays {

  public:

    enum RELAY {
      RELAY_UNUSED,         // Virtual Relay - Unused
      RELAY_COOLING_FAN,    // Virtual Relay - Switches Cooling Fan On/Off
      RELAY_CONVECTION_FAN, // Virtual Relay - Switches Convection Fan On/Off
      RELAY_BOTTOM_ELEMENT, // Virtual Relay - Switches Bottom Heating Element On/Off
      RELAY_BOOST_ELEMENT,  // Virtual Relay - Switches Boost Heating Element On/Off
      RELAY_TOP_ELEMENT,    // Virtual Relay - Switches Top Heating Element On/Off
      
      RELAY_D4,             // Physical Relay - Relay Switch on IO D4
      RELAY_D5,             // Physical Relay - Relay Switch on IO D5
      RELAY_D6,             // Physical Relay - Relay Switch on IO D6
      RELAY_D7,             // Physical Relay - Relay Switch on IO D7
    };

    ControLeo2_Relays(void);
    
    void AssignRelay(RELAY Phys, RELAY Virt);
    void SetRelay(RELAY relay, uint8_t duty);

    void ProcessRelays(void);

  private:
    uint32_t lastRelayTime; // Relay Timer
    uint8_t  PWMCounter;    // PWM State Counter (0-99)

    RELAY   RelayAssignment[RELAY_TOP_ELEMENT]; // Virtual to Physical Mapping
    
    uint8_t RelayDuty[(RELAY_D7 - RELAY_D4) + 1];      
};



#endif

