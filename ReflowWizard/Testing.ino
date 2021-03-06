// Testing menu
// Called from the main loop
// Allows the user to test the outputs
// Buttons: The bottom button moves to the next output
//          The top button toggles the output on and off

// Called when in testing mode
// Return false to exit this mode
boolean Testing() {
  static boolean firstRun = true;
  static boolean channelIsOn = true;
  static int channel = 4; 
  
  // Is this the first time "Testing" has been run?
  if (firstRun) {
    firstRun = false;
    lcdPrintLine_P(0, PSTR("Test Outputs"));
    lcdPrintLine_P(1, PSTR("Output 4"));
    displayOnState(channelIsOn);
  }
  
  // Turn the currently selected channel on, and the others off
  for (int i=4; i<8; i++) {
    if (i == channel && channelIsOn)
      digitalWrite(i, HIGH);
    else
      digitalWrite(i, LOW);
  }
  
  // Was a button pressed?
  switch (buttons.GetKeypress()) {
    case BUTTON_TOP_RELEASE:
      // Toggle the output on and off
      channelIsOn = !channelIsOn;
      displayOnState(channelIsOn);
      break;
    case BUTTON_BOT_RELEASE:
      // Move to the next output
      channel = channel + 1;
      if (channel == 8) {
        // Turn all the outputs off
        for (int i=4; i<8; i++)
          digitalWrite(i, LOW);
        // Initialize variables for the next time through
        firstRun = true;
        channel = 4;
        channelIsOn = true;
        // Return to the main menu
        return false;
      }
      lcd.PrintInt(7,1,1,channel);
      displayOnState(channelIsOn);
      break;
  }
  
  // Stay in this mode;
  return true;
}


void displayOnState(boolean isOn) {
  lcd.PrintStr(9,1, isOn? "is on ": "is off");
}
