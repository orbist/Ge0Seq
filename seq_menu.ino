

void updateEncoderPos() {
    static int encoderA, encoderB, encoderA_prev;

    // Debounce the encoder rotation by 10ms to de-glitch
    if( encoderTimer - millis() > 10 ) {
      encoderTimer = millis();

      encoderA = ( ENC_A_PIN & bit(ENC_A_BIT) ) == 0;
      encoderB = ( ENC_B_PIN & bit(ENC_B_BIT) ) == 0;
      //encoderA = digitalRead(ENC_A); 
      //encoderB = digitalRead(ENC_B);
      
      if( ( !encoderA ) && ( encoderA_prev ) ){ // A has gone from high to low 
        if( is_true( &bitmap2, BIT2_HIGHLIGHT_ENABLED ) ) { // Update encoder position
          encoderPosPrev = encoderPos;
          encoderB ? encoderPos++ : encoderPos--;  
        }
        else { 
          set_true( &bitmap2, BIT2_HIGHLIGHT_ENABLED );
          encoderPos = 0;  // Reset encoder position if highlight timed out
          encoderPosPrev = 0;
        }
        highlightTimer = millis(); 
        updateSelection();
      }
      encoderA_prev = encoderA; 
    }    
}

uint8_t tmp;

void updateMenu() {  // Called whenever button is pushed

  if( is_true( &bitmap2, BIT2_HIGHLIGHT_ENABLED ) ) { // Highlight is active, choose selection
    switch( menu ) {

      case SETTINGS:
        switch( mod( encoderPos, 6 ) ) {
          case 0: 
            menu = CV_SETTINGS;
            break;
          case 1: 
            menu = MIDI_SETTINGS;
            break;
          case 2: 
            menu = SEQ_SETTINGS;
            break;
          case 3: 
            menu = FACTORY_RESET;
            break;
          case 4:
            menu = ABOUT;
            break;
          case 5:
            menu = EXIT;
            break;
        }
        break;

      case CV_SETTINGS:
        tmp = mod( encoderPos, 6 );
        switch( tmp ) {
          case 0:
            menu = CV_CLOCK_PPQN_SET;
            break;
          case 1:
            menu = CV_OUT_PPQN_SET;
            break;
          case 2:
            menu = CV_GATE_OFF_DELAY_SET;
            break;
          case 3:
            menu = SCALE_FACTOR_SET;
            break;
          case 4:
            menu = CV_SETTINGS2;
            break;
          case 5:
            menu = SETTINGS;
            break;
        }
        break;

      case CV_SETTINGS2:
        tmp = mod( encoderPos, 4 );
        switch( tmp ) {
          case 0:
            menu = CV_TRIG_DURATION_SET;
            break;
          case 1:
            menu = CV_CLOCK_DURATION_SET;
            break;
          case 2:
            menu = CV_MOD_DURATION_SET;
            break;
          case 3:
            menu = SETTINGS;
            break;
        }
        break;

      case MIDI_SETTINGS:
        tmp = mod( encoderPos, 6 );
        switch( tmp ) {
          case 0:
            menu = NOTE_PRIORITY_SET;
            break;
          case 1:
            menu = MIDI_CHANNEL_SET;
            break;
          case 2:
            menu = MIDI_CC_MOD_SET;
            break;
          case 3:
            menu = MIDI_CC_ACCENT_SET;
            break;
          case 4:
            menu = MIDI_START_ENABLE_SET;
            break;
          case 5:
            menu = SETTINGS;
            break;
        }
        break;

      case SEQ_SETTINGS:
        tmp = mod( encoderPos, 6 );
        switch( tmp ) {
          case 0:
            menu = ACTIVATE_SEQ_SET;
            break;
          case 1:
            menu = CLEAR_SEQ_SET;
            break;
          case 2:
            menu = SAVE_ACTIVE_SEQ_SET;
            break;
          case 3:
            menu = MODIFY_SEQ;
            break;
          case 4:
            menu = SEQ_SETTINGS2;
            break;
          case 5:
            menu = SETTINGS;
            break;
        }
        break;

      case SEQ_SETTINGS2:
        tmp = mod( encoderPos, 5 );
        switch( tmp ) {
          case 0:
            menu = SHIFT_SEQ_NOTE_LEFT;
            break;
          case 1:
            menu = SHIFT_SEQ_NOTE_RIGHT;
            break;
          case 2:
            menu = SHIFT_SEQ_ALL_LEFT;
            break;
          case 3:
            menu = SHIFT_SEQ_ALL_RIGHT;
            break;
          case 4:
            menu = SETTINGS;
            break;
        }
        break;
        
      case FACTORY_RESET:
        tmp = mod( encoderPos, 2 );
        switch( tmp ) {
          case 0: 
            menu = FACTORY_RESET_CONFIRM;
            break;
          case 1: 
            menu = SETTINGS;
            break;
        }
        break;

      // Execute the tasks

      // CV SETTINGS

      case CV_CLOCK_PPQN_SET:
        menu = CV_SETTINGS;
        tick_per_step = my_div( cvClockPPQN, 4 );
        cv_step_length = 0;
        saveByteSetting( EEPROM_PQ_OFFSET, cvClockPPQN, 0 );
        break;

      case CV_OUT_PPQN_SET:
        menu = CV_SETTINGS;
        saveByteSetting( EEPROM_CO_OFFSET, cvClockOutPPQN, 0 );
        break;

      case CV_GATE_OFF_DELAY_SET:
        menu = CV_SETTINGS;
        saveUlongSetting( EEPROM_GD_OFFSET, gateDelay, 0 );
        break;

      case SCALE_FACTOR_SET: 
        menu = CV_SETTINGS;
        saveFloatSetting( EEPROM_SF_OFFSET, sfAdj, 0 );
        break;

      // CV2 SETTINGS

      case CV_TRIG_DURATION_SET:
        menu = CV_SETTINGS2;
        saveByteSetting( EEPROM_TD_OFFSET, trigDuration, 0 );
        break;
      
      case CV_CLOCK_DURATION_SET:
        menu = CV_SETTINGS2;
        saveByteSetting( EEPROM_CD_OFFSET, clockDuration, 0 );
        break;

      case CV_MOD_DURATION_SET:
        menu = CV_SETTINGS2;
        saveUlongSetting( EEPROM_MD_OFFSET, modDuration, 0 );
        break;

      // MIDI SETTINGS

      case MIDI_CHANNEL_SET:  // Save MIDI channel setting to EEPROM
        menu = MIDI_SETTINGS;
        saveByteSetting( EEPROM_CH_OFFSET, midiChannel, 1 );
        break;
  
      case NOTE_PRIORITY_SET:  // Save note priority setting to EEPROM
        menu = MIDI_SETTINGS;
        if( !strcmp_P( priorityMenu, PSTR( "Last" ) ) ) { notePriority = NP_LAST; }
        if( !strcmp_P( priorityMenu, PSTR( "Top"  ) ) ) { notePriority = NP_HIGH; }
        if( !strcmp_P( priorityMenu, PSTR( "Botm" ) ) ) { notePriority = NP_LOW; }
        saveByteSetting( EEPROM_NP_OFFSET, notePriority, 0 );
        break;

      case MIDI_CC_MOD_SET: 
        menu = MIDI_SETTINGS;
        saveByteSetting( EEPROM_CC_OFFSET, midiCCnumber, 0 );
        break;

      case MIDI_CC_ACCENT_SET: 
        menu = MIDI_SETTINGS;
        saveByteSetting( EEPROM_AC_OFFSET, midiCCaccent, 0 );
        break;
      
      case MIDI_START_ENABLE_SET: 
        menu = MIDI_SETTINGS;
        saveByteSetting( EEPROM_ST_OFFSET, midiStartEnable, 0 );
        break;

      // SEQ SETTINGS
          
      case ACTIVATE_SEQ_SET:
        menu = SEQ_SETTINGS;
        readSequence( activeSlotMenu + 1, 1 );
        break;
      
      case CLEAR_SEQ_SET:
        menu = SEQ_SETTINGS;
        clearSequence( clearSlotMenu + 1, 1 );
        break;

      case SAVE_ACTIVE_SEQ_SET:
        menu = SEQ_SETTINGS;
        writeSequence( saveSlotMenu + 1, 1 );
        break;

      // SEQ_SETTINGS2

      case SHIFT_SEQ_NOTE_LEFT:
        menu = SEQ_SETTINGS2;
        shiftSequence( shiftNumber, 1, 0 );
        break;

      case SHIFT_SEQ_NOTE_RIGHT:
        menu = SEQ_SETTINGS2;
        shiftSequence( shiftNumber, 0, 0 );
        break;

      case SHIFT_SEQ_ALL_LEFT:
        menu = SEQ_SETTINGS2;
        shiftSequence( shiftNumber, 1, 1 );
        break;

      case SHIFT_SEQ_ALL_RIGHT:
        menu = SEQ_SETTINGS2;
        shiftSequence( shiftNumber, 0, 1 );
        break;

      // FACTORY RESET     
      
      case FACTORY_RESET_CONFIRM:
        menu = SETTINGS;
        if( !strcmp_P( fresetMenu, PSTR( "Yes" ) ) ) { 
          eepromClear();
          eepromInit();
        }
        break;
    }
  }
  else { // Highlight wasn't visible, reinitialize highlight timer
    highlightTimer = millis();
    set_true( &bitmap2, BIT2_HIGHLIGHT_ENABLED );
  }
  encoderPos = 0;  // Reset encoder position
  encoderPosPrev = 0;

  if( menu == EXIT ) {
    menu = SETTINGS;    // Return to main screen and hide menu
    set_false( &bitmap2, BIT2_HIGHLIGHT_ENABLED );
    display.clear();
    set_true( &bitmap, BIT_DISPLAY_CLEARED );
  } else {
    updateSelection(); // Refresh menu screen
  }
}


void updateSelection() { // Called whenever encoder is turned

  display.clear();
  switch( menu ) {

    // MAIN MENU

    case SETTINGS:
      display.setCursor(0,0); 
      display.setInvertMode(0);
      display.println(F("       SETTINGS"));
      display.println();

      if( menu == SETTINGS ) { 
        setHighlight(0,6);
      } else { 
        display.setInvertMode(0); 
      }
      display.println(F(" CV Settings         "));

      if( menu == SETTINGS ) {
        setHighlight(1,6);
      } else {
        display.setInvertMode(0);
      }
      display.println(F(" MIDI Settings       "));

      if( menu == SETTINGS ) {
        setHighlight(2,6);
      } else {
        display.setInvertMode(0);
      }
      display.println(F(" Sequence Settings   "));

      if( menu == SETTINGS ) {
        setHighlight(3,6);
      } else {
        display.setInvertMode(0);
      }
      display.println(F(" Factory Reset       "));

      if( menu == SETTINGS ) {
        setHighlight(4,6);
      } else {
        display.setInvertMode(0);
      }
      display.println(F(" About               "));

      if (menu == SETTINGS ) { 
        setHighlight(5,6);
      } else {
        display.setInvertMode(0);
      } 
      display.println(F(" Return              "));       
      
      // Finished SETTINGS menu
      
      break;

    // CV_SETTINGS
    // First define the options for each selection
    // Then display the menu with appropriate highlghting / selection enabled

    case CV_CLOCK_PPQN_SET:
      if( menu == CV_CLOCK_PPQN_SET ) {
        switch (mod(encoderPos, 6)) {
          case 0:
            cvClockPPQN = 4;
            break;
          case 1:
            cvClockPPQN = 8;
            break;
          case 2:
            cvClockPPQN = 12;
            break;
          case 3:
            cvClockPPQN = 16;
            break;
          case 4:
            cvClockPPQN = 20;
            break;
          case 5:
            cvClockPPQN = 24;
            break;
        }
      }
      // No break  

    case CV_OUT_PPQN_SET:
      if( menu == CV_OUT_PPQN_SET ) {
        switch( mod( encoderPos, 5 ) ) {
          case 0:
            cvClockOutPPQN = 1;
            break;
          case 1:
            cvClockOutPPQN = 4;
            break;
          case 2:
            cvClockOutPPQN = 8;
            break;
          case 3:
            cvClockOutPPQN = 12;
            break;
          case 4:
            cvClockOutPPQN = 24;
            break;
        }
      }
      // No break

    case CV_GATE_OFF_DELAY_SET:
      if( menu == CV_GATE_OFF_DELAY_SET ) {
        if( ( encoderPos > encoderPosPrev ) && ( gateDelay < 1000 ) ) {
          gateDelay += 1;
        } else if( ( encoderPos < encoderPosPrev ) && ( gateDelay > 250 ) ) {
          gateDelay -= 1;
        }
      }
      // No break

    case SCALE_FACTOR_SET:
      if( menu == SCALE_FACTOR_SET ) {
        if( ( encoderPos > encoderPosPrev ) && ( sfAdj < 1.1 ) ) {
          sfAdj += 0.001f;
        } else if( ( encoderPos < encoderPosPrev ) && ( sfAdj > 0.9 ) ) {
          sfAdj -= 0.001f;
        }
      }
      // No break

    case CV_SETTINGS:

      display.setCursor(0,0); 
      display.setInvertMode(0);
      display.println(F("   CV SETTINGS"));
      display.println();
      
      if( menu == CV_SETTINGS ) {
        setHighlight(0,6);
      } else {
        display.setInvertMode(0);
      }                 
      display.print(F("CV Clk In PPQN  "));
      if( menu == CV_CLOCK_PPQN_SET ) { display.setInvertMode(1); }
      display.print( cvClockPPQN );
      if( cvClockPPQN < 10 ) {
        display.println(F("   "));
      } else {
        display.println(F("  "));
      }

      if( menu == CV_SETTINGS ) {
        setHighlight(1,6);
      } else {
        display.setInvertMode(0);
      }                 
      display.print(F("CV Clk Out PPQN "));
      if( menu == CV_OUT_PPQN_SET ) { display.setInvertMode(1); }
      display.print( cvClockOutPPQN );
      if( cvClockOutPPQN < 10 ) {
        display.println(F("   "));
      } else {
        display.println(F("  "));
      }

      if( menu == CV_SETTINGS ) {
        setHighlight(2,6);
      } else {
        display.setInvertMode(0);
      }                 
      display.print(F("CV Gate Off Dly "));
      if( menu == CV_GATE_OFF_DELAY_SET ) { display.setInvertMode(1); }
      display.print( gateDelay );
      if( gateDelay < 10 ) {
        display.println(F("   "));
      } else {
        display.println(F("  "));
      }
      
      if( menu == CV_SETTINGS ) {
        setHighlight(3,6);
      } else {
        display.setInvertMode(0);
      }
      display.print(F("CV adjust mV    "));
      if( menu == SCALE_FACTOR_SET ) { display.setInvertMode(1); }
      display.println(sfAdj,3);


      if( menu == CV_SETTINGS ) {
        setHighlight(4,6);
      } else {
        display.setInvertMode(0);
      }                 
      display.println(F("More CV Settings "));


      if (menu == CV_SETTINGS ) { 
        setHighlight(5,6);
      } else {
        display.setInvertMode(0);
      } 
      display.println(F("Return           "));       
      break;
    

    // CV_SETTINGS2 Menu

    case CV_TRIG_DURATION_SET:
      if( menu == CV_TRIG_DURATION_SET ) {
        if( ( encoderPos > encoderPosPrev ) && ( trigDuration < PULSE_DURATION_MAX ) ) {
          trigDuration += 1;
        } else if( ( encoderPos < encoderPosPrev ) && ( trigDuration > PULSE_DURATION_MIN ) ) {
          trigDuration -= 1;
        }
      }
      // No break

    case CV_CLOCK_DURATION_SET:
      if( menu == CV_CLOCK_DURATION_SET ) {
        if( ( encoderPos > encoderPosPrev ) && ( clockDuration < PULSE_DURATION_MAX ) ) {
          clockDuration += 1;
        } else if( ( encoderPos < encoderPosPrev ) && ( clockDuration > PULSE_DURATION_MIN ) ) {
          clockDuration -= 1;
        }
      }
      // No break

    case CV_MOD_DURATION_SET:
      if( menu == CV_MOD_DURATION_SET ) {
        if( ( encoderPos > encoderPosPrev ) && ( modDuration < MOD_DURATION_MAX ) ) {
          modDuration += 1;
        } else if( ( encoderPos < encoderPosPrev ) && ( modDuration > MOD_DURATION_MIN ) ) {
          modDuration -= 1;
        }
      }
      // No break

    case CV_SETTINGS2:

      display.setCursor(0,0); 
      display.setInvertMode(0);
      display.println(F("  CV SETTINGS Pg2"));
      display.println();

      if( menu == CV_SETTINGS2 ) {
        setHighlight(0,4);
      } else {
        display.setInvertMode(0);
      }                 
      display.print(F("CV Trig Duration "));
      if( menu == CV_TRIG_DURATION_SET ) { display.setInvertMode(1); }
      display.print( trigDuration );
      if( gateDelay < 100 ) {
        display.println(F(" "));
      } else {
        display.println(F(""));
      }

      if( menu == CV_SETTINGS2 ) {
        setHighlight(1,4);
      } else {
        display.setInvertMode(0);
      }                 
      display.print(F("CV Clk Duration  "));
      if( menu == CV_CLOCK_DURATION_SET ) { display.setInvertMode(1); }
      display.print( clockDuration );
      if( gateDelay < 100 ) {
        display.println(F(" "));
      } else {
        display.println(F(""));
      }

      if( menu == CV_SETTINGS2 ) {
        setHighlight(2,4);
      } else {
        display.setInvertMode(0);
      }                 
      display.print(F("CV Mod Duration  "));
      if( menu == CV_MOD_DURATION_SET ) { display.setInvertMode(1); }
      display.print( modDuration );
      if( gateDelay < 100 ) {
        display.println(F(" "));
      } else {
        display.println(F(""));
      }
      
      if (menu == CV_SETTINGS2 ) { 
        setHighlight(3,4);
      } else {
        display.setInvertMode(0);
      } 
      display.println(F("Return          "));     

      break;


    // MIDI_SETTINGS Menu

    case NOTE_PRIORITY_SET:
      if( menu == NOTE_PRIORITY_SET ) {
        switch (mod(encoderPos, 3)) {
          case 0:
            strcpy_P( priorityMenu, PSTR( "Last" ) );  // Last note
            break;
          case 1:
            strcpy_P( priorityMenu, PSTR( "Top" ) );  // Top note
            break;
          case 2: 
            strcpy_P( priorityMenu, PSTR( "Botm" ) );  // Bottom note
            break;
        }
      } 
      // No break

    case MIDI_CHANNEL_SET:
      if( menu == MIDI_CHANNEL_SET ) { midiChannel = mod(encoderPos, 16); }
      // No break    

    case MIDI_CC_MOD_SET:
      if( menu == MIDI_CC_MOD_SET ) { midiCCnumber = mod( encoderPos, 128); }
      // no break

    case MIDI_CC_ACCENT_SET:
      if( menu == MIDI_CC_ACCENT_SET ) { midiCCaccent = mod( encoderPos, 128); }
      //no break
    
    case MIDI_START_ENABLE_SET:
      if( menu == MIDI_START_ENABLE_SET ) { midiStartEnable = mod( encoderPos, 2); }
      //no break

    case MIDI_SETTINGS:

      display.setCursor(0,0); 
      display.setInvertMode(0);
      display.println(F("   MIDI SETTINGS"));
      display.println();

      if( menu == MIDI_SETTINGS ) {
        setHighlight(0,6);
      } else {
        display.setInvertMode(0);
      }
      display.print(F("Note Priority   "));
      if( menu == NOTE_PRIORITY_SET ) { display.setInvertMode(1); }
      display.println( priorityMenu );

      // For some reason, encoder values of 10 in settings below 
      // add an extra NewLine to priorityMenu string?!
      // So force the next line location here
      display.setCursor(0,3);
      
      // MIDI Channel Menu line

      if( menu == MIDI_SETTINGS ) {
        setHighlight(1,6);
      } else {
        display.setInvertMode(0);
      }
      display.print(F("MIDI Channel    "));
      if( menu == MIDI_CHANNEL_SET ) { display.setInvertMode(1); }
      display.print( midiChannel );
      if( midiChannel < 10 ) {
        display.println(F("   "));
      } else {
        display.println(F("  "));
      }

      if( menu == MIDI_SETTINGS ) {
        setHighlight(2,6);
      } else {
        display.setInvertMode(0);
      }
      display.print(F("MIDI CC Mod CV  "));
      if( menu == MIDI_CC_MOD_SET ) { display.setInvertMode(1); }
      display.print( midiCCnumber );
      if( midiCCnumber < 10 ) {
        display.println(F("   "));
      } else if( midiCCnumber < 100 ) {
        display.println(F("  "));
      } else {
        display.println(F(" "));
      }

      if( menu == MIDI_SETTINGS ) {
        setHighlight(3,6);
      } else {
        display.setInvertMode(0);
      }
      display.print(F("MIDI CC Accent  "));
      if( menu == MIDI_CC_ACCENT_SET ) { display.setInvertMode(1); }
      display.print( midiCCaccent );
      if( midiCCaccent < 10 ) {
        display.println(F("   "));
      } else if( midiCCaccent < 100 ) {
        display.println(F("  "));
      } else {
        display.println(F(" "));
      }

      if( menu == MIDI_SETTINGS ) {
        setHighlight(4,6);
      } else {
        display.setInvertMode(0);
      }
      display.print(F("MIDI Strt Enble "));
      if( menu == MIDI_START_ENABLE_SET ) { display.setInvertMode(1); }
      display.print( midiStartEnable );
      display.println(F("   "));

      if (menu == MIDI_SETTINGS ) { 
        setHighlight(5,6);
      } else {
        display.setInvertMode(0);
      } 
      display.println(F("Return          "));
          
      break;

    // SEQ_SETTINGS Menu

    case ACTIVATE_SEQ_SET:
      if( menu == ACTIVATE_SEQ_SET ) {
        activeSlotMenu = mod( encoderPos, 8 );  // 0-3
      }
      // No break 

    case CLEAR_SEQ_SET:
      if( menu == CLEAR_SEQ_SET ) {
        clearSlotMenu = mod( encoderPos, 8 );  // 0-3
      }
      // No break 
    
    case SAVE_ACTIVE_SEQ_SET:
      if( menu == SAVE_ACTIVE_SEQ_SET ) {
        saveSlotMenu = mod( encoderPos, 8 );  // 0-3
      }
      // No break 

    case MODIFY_SEQ:
      // TODO
      // For now, clear the screen after display a suitable msg
      if( menu == MODIFY_SEQ ) {
        display.clear();
        display.setCursor( 0, 2 );
        display.println(F(" Modifying on the"));
        display.println(F(" device is not yet"));
        display.println(F(" supported!"));
        delay(3000);
        menu = SEQ_SETTINGS;
      }
      // No break if not MODIFY_SEQ 

    case SEQ_SETTINGS:

      display.setCursor(0,0); 
      display.setInvertMode(0);
      display.println(F("    SEQ SETTINGS"));
      display.println();


      if (menu == SEQ_SETTINGS) {
        setHighlight(0,6);
      } else {
        display.setInvertMode(0);
      }
      display.print(F("Activate Seq    "));
      if( menu == ACTIVATE_SEQ_SET ) { display.setInvertMode(1); }
      display.print( activeSlotMenu+1 );
      display.println(F("  "));
      


      if (menu == SEQ_SETTINGS) {
        setHighlight(1,6);
      } else {
        display.setInvertMode(0);
      }
      display.print(F("Clear Seq       "));
      if( menu == CLEAR_SEQ_SET ) { display.setInvertMode(1); }
      display.print( clearSlotMenu+1 );
      display.println(F("  "));



      if (menu == SEQ_SETTINGS) {
        setHighlight(2,6);
      } else {
        display.setInvertMode(0);
      }
      display.print(F("Save Active Seq "));
      if( menu == SAVE_ACTIVE_SEQ_SET ) { display.setInvertMode(1); }
      display.print( saveSlotMenu+1 );
      display.println(F("  "));



      if (menu == SEQ_SETTINGS) {
        setHighlight(3,6);
      } else {
        display.setInvertMode(0);
      }
      display.println(F("Edit Active Seq "));
      

      if( menu == SEQ_SETTINGS ) {
        setHighlight(4,6);
      } else {
        display.setInvertMode(0);
      }                 
      display.println(F("More Seq Settings"));


      if (menu == SEQ_SETTINGS ) { 
        setHighlight(5,6);
      } else {
        display.setInvertMode(0);
      } 
      display.println(F("Return          "));    

      break;

    // SEQ_SETTINGS2

    case SHIFT_SEQ_NOTE_LEFT:
    case SHIFT_SEQ_NOTE_RIGHT:
    case SHIFT_SEQ_ALL_LEFT:
    case SHIFT_SEQ_ALL_RIGHT:
      if( menu == SHIFT_SEQ_NOTE_LEFT  ||
          menu == SHIFT_SEQ_NOTE_RIGHT ||
          menu == SHIFT_SEQ_ALL_LEFT   ||
          menu == SHIFT_SEQ_ALL_RIGHT ) {
        shiftNumber = mod( encoderPos, 16 );  // 0-15
      }
      // No break
    
    case SEQ_SETTINGS2:

      display.setCursor(0,0); 
      display.setInvertMode(0);
      display.println(F("  SEQ SETTINGS pg2"));
      display.println();


      if (menu == SEQ_SETTINGS2) {
        setHighlight(0,5);
      } else {
        display.setInvertMode(0);
      }
      display.print(F("Shift Note Left "));
      if( menu == SHIFT_SEQ_NOTE_LEFT ) { 
        display.setInvertMode(1);
        display.print( shiftNumber );
      } else {
        display.print( 0 );
      }
      display.println(F("  "));


      if (menu == SEQ_SETTINGS2) {
        setHighlight(1,5);
      } else {
        display.setInvertMode(0);
      }
      display.print(F("Shift Note Right"));
      if( menu == SHIFT_SEQ_NOTE_RIGHT ) { 
        display.setInvertMode(1);
        display.print( shiftNumber );
      } else {
        display.print( 0 );
      }
      display.println(F("  "));


      if (menu == SEQ_SETTINGS2) {
        setHighlight(2,5);
      } else {
        display.setInvertMode(0);
      }
      
      display.print(F("Shift ALL Left  "));
      if( menu == SHIFT_SEQ_ALL_LEFT ) { 
        display.setInvertMode(1);
        display.print( shiftNumber );
      } else {
        display.print( 0 );
      }
      display.println(F("  "));


      if (menu == SEQ_SETTINGS2) {
        setHighlight(3,5);
      } else {
        display.setInvertMode(0);
      }
      display.print(F("Shift ALL Right "));
      if( menu == SHIFT_SEQ_ALL_RIGHT ) { 
        display.setInvertMode(1);
        display.print( shiftNumber );
      } else {
        display.print( 0 );
      }
      display.println(F("  "));


      if (menu == SEQ_SETTINGS2 ) { 
        setHighlight(4,5);
      } else {
        display.setInvertMode(0);
      } 
      display.println(F("Return          "));     

      break;
      

    // ABOUT info

    case ABOUT:
      displayAbout();
      break;

    // FACTORY_RESET Menu

    case FACTORY_RESET_CONFIRM:
      switch (mod(encoderPos, 2)) {
        case 0:
          strcpy_P( fresetMenu, PSTR( "No" ) );  // Top note
          break;
        case 1:
          strcpy_P( fresetMenu, PSTR( "Yes" ) );  // Bottom note
          break;
      }
      // No break
    
    case FACTORY_RESET:
    
      display.setCursor(0,0); 
      display.setInvertMode(0);
      display.println(F("   FACTORY RESET!!!"));
      display.println();
      display.println(F("All settings & seqs"));
      display.println(F("will be erased!"));
      display.println();
      
      if( menu == FACTORY_RESET ) {
        setHighlight(0,2);
      }
      display.print(F("Are you sure?   "));
      if( menu == FACTORY_RESET_CONFIRM ) { display.setInvertMode(1); }
      display.print(F(" "));
      display.print( fresetMenu );
      display.println(F(" "));
      
      if (menu == FACTORY_RESET ) { 
        setHighlight(1,2);
      } else {
        display.setInvertMode(0);
      }
      display.println(F("Return          "));       

      break;       
  } 
}

// ------ PRIVATE ------ //

void setHighlight(int menuItem, int numMenuItems) {
  if( ( mod( encoderPos, numMenuItems ) == menuItem ) &&
      ( is_true( &bitmap2, BIT2_HIGHLIGHT_ENABLED ) ) ) {
    display.setInvertMode(1);
  }
  else {
    display.setInvertMode(0);
  }
}

