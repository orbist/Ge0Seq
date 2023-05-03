
void displayInit() {

  char ver[10];

  display.begin( &Adafruit128x64, SCREEN_ADDRESS );
  display.setFont(Adafruit5x7);

  sprintf_P( ver, PSTR( "v%s.%s.%s.%s"), MAJOR_VERSION_STR, 
                                         MINOR_VERSION_STR,
                                         FIX_VERSION_STR,
                                         BUILD_VERSION_STR );

  Serial.print(F("Version : "));
  Serial.println(ver);
 
  display.clear();
  display.setCursor(0,0);
  display.set2X();
  display.println(F("  Ge0sync"));
  display.set1X();
  display.println();
  display.println(F(PRODUCT_NAME));
  display.println();
  display.print(F("       "));
  display.println(ver);
  delay(3000);
  
  display.clear();
  displayCleared = 1;
}


void displayHdr() {

  display.clear();
  display.setCursor(0,0);
  display.setInvertMode(1);
  display.println(F(PRODUCT_NAME));
  display.setInvertMode(0);
  display.println();

}

void displayHomeHdr() {

  display.clear();
  display.setCursor(0,0);
  display.setInvertMode(1);
  display.println(F(PRODUCT_NAME));
  display.print(F(" CH:"));
  display.print(midiChannel);
  display.print(F(" ModCC:"));
  display.print(midiCCnumber);
  display.print(F(" NP:"));
  display.print(priorityMenu);
  display.println(F(" "));
  display.setInvertMode(0);
  display.println();
    
}

void displayTestComms( bool match ) {
  displayHdr();
  display.println(F("TestCom SUCCESS!"));
  display.println();
  if( !match ) {
    display.println(F("WARNING!!!"));
    display.println(F("FW and app version"));
    display.println(F("mismatch!"));
  }
  delay(4000);
  display.clear();
  displayCleared = 1;
}


void displayHomePage() {

  // display home page 
  display.clear();
  displayHomeHdr();
  display.println(F(" D   1   2   3   4 "));

  // show which slots are used - seq_stored[] setup in EEPROMinit()
  for( uint8_t i = 0; i <= NUM_SEQ; i++ ) {
    if( i == 5 ) {
      //half way, start new line
      display.println();
      display.println(F("     5   6   7   8 "));
      display.print(F("    "));
    }
    if( seq_stored[i] == 1 ) {
      if( i == activeSlot ) {
        display.setInvertMode(1);
      } else {
        display.setInvertMode(0);
      }
      display.print(F(" * "));
      display.setInvertMode(0);
      display.print(F(" "));
    } else {
      display.print(F("    "));
    }
  }

  display.println();
  displaySettingsButton(); 

}

void displayMsgPlaying( bool cv_play ) {

    display.clear();
    display.setCursor(0,1);
    display.setInvertMode(1);
    display.println(F("  >>>  Playing  >>>   "));
    display.setInvertMode(0);
    display.println();
    display.print(F("Sequence  "));
    display.println( activeSlot );
    display.print(F("via "));
    if( cv_play ) {
      display.println(F("CV Clock..."));
    } else {
      display.println(F("MIDI Clock..."));
    }
}


void displaySettingsButton() {

  display.print(F("      "));
  display.setInvertMode(1);
  display.println(F(" Settings "));
  display.setInvertMode(0);

}


void displayDONE( bool home ) {

  display.println();
  display.println(F("DONE!"));
  delay(2000);
  
  if( home ) {
    display.clear();
    displayCleared = 1;
  }
}


void displayAbout() {
  char ver[7];
      
  sprintf_P( ver, PSTR( "v%s.%s.%s"), MAJOR_VERSION_STR, 
                                      MINOR_VERSION_STR,
                                      FIX_VERSION_STR );

  display.setCursor(0,0);
  display.setInvertMode(0);
  display.println(F("        ABOUT"));
  display.println(F(PRODUCT_NAME));
  display.println();
  display.print(F("Version: "));
  display.print(ver);
  display.print(F("."));
  display.println(F(BUILD_VERSION_STR));
  display.print(F("Build date: "));
  display.println(F(BUILD_DATE));
  display.print(F("EEPROM type:"));
  display.println(ver);
  display.println();
  display.println(F("(c)Ge0sync Synth 2023"));
  delay(10000);

}


