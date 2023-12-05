/* ==========================================================================================
 * EEPROM FUNCTIONS
 * ========================================================================================== */

// Function   :   eepromInit( void )
// Purpose    :   check if the eeprom has been initialised :
//                  if not do it now and reboot
//                  if yes, read in the settings - check they are valid
//                  if not valid, call eppromClear and reboot to force format (recursive!)
//                NB: Can force format if EEPROM_INIT_TRUE changes, i.e. increment if eeprom
//                    format is not backwards compatible
void eepromInit() {
  if( EEPROM.read( EEPROM_INITED_OFFSET ) != EEPROM_INIT_TRUE ) {
    
    #ifdef DEBUG
      Serial.println(F("Virgin EEPROM, initialise now..."));
    #endif
    displayHdr();
    display.println(F("Initialise EEPROM..."));
    
    //first ever run, set EEPROM to formatted - flash LED 5 times
    do_some_flash(5);
    
    EEPROM.write( EEPROM_INITED_OFFSET, EEPROM_INIT_TRUE );
    for( uint8_t i = 1; i <= NUM_SEQ; i++ ) {
      EEPROM.write( i,0 );
    }  
    
    // bytes
    EEPROM.write( SEQ_ACTIVE_SLOT_OFFSET, 0 );
    EEPROM.write( EEPROM_NP_OFFSET, NP_LAST );
    EEPROM.write( EEPROM_CH_OFFSET, 0);
    EEPROM.write( EEPROM_PQ_OFFSET, 4);
    EEPROM.write( EEPROM_CO_OFFSET, 4);
    EEPROM.write( EEPROM_TD_OFFSET, 20 );
    EEPROM.write( EEPROM_CD_OFFSET, 20 );
    EEPROM.write( EEPROM_CC_OFFSET, 1);
    EEPROM.write( EEPROM_ST_OFFSET, 1);
    EEPROM.write( EEPROM_AC_OFFSET, 80);

    // longs and floats
    EEPROM.put( EEPROM_SF_OFFSET, 1.0f );
    EEPROM.put( EEPROM_GD_OFFSET, (unsigned long)500 );
    EEPROM.put( EEPROM_MD_OFFSET, (unsigned long)0 );
    

    displayDONE(0);
    display.println();
    display.println(F("Rebooting in 3s..."));
    delay( FACTORY_RESET_TIMEOUT );
    resetFunc();

  } else {
    // EEPROM is valid, so read all settings
    bool valid = 1;

    for( int i = 1; i <= NUM_SEQ; i++ ) {
      // setup seq_stored array in memory
      seq_stored[i] = EEPROM.read(i);
      if( seq_stored[i] > 1 ) { valid = 0; }
    }
    seq_stored[0] = 1; // always have the default seq

    activeSlot   = EEPROM.read( SEQ_ACTIVE_SLOT_OFFSET );
    if( activeSlot > 8 ) { valid = 0; }

    notePriority = EEPROM.read( EEPROM_NP_OFFSET );
    if( notePriority > 2 ) { valid = 0; }

    midiChannel  = EEPROM.read( EEPROM_CH_OFFSET );
    if( midiChannel > 15 ) { valid = 0; }

    cvClockPPQN  = EEPROM.read( EEPROM_PQ_OFFSET );
    if( cvClockPPQN > 24 ) { valid = 0; }
    tick_per_step = my_div( cvClockPPQN, 4 );

    cvClockOutPPQN = EEPROM.read( EEPROM_CO_OFFSET );
    if( cvClockOutPPQN > 24 ) { valid = 0; }

    trigDuration = EEPROM.read( EEPROM_TD_OFFSET );
    if( trigDuration < 10 || trigDuration > 100 ) { valid = 0; }

    clockDuration = EEPROM.read( EEPROM_CD_OFFSET );
    if( clockDuration < 10 || clockDuration > 100  ) { valid = 0; }

    midiCCnumber = EEPROM.read( EEPROM_CC_OFFSET );
    if( midiCCnumber < 1 || midiCCnumber > 127 ) { valid = 0; }

    // These two added in 1.5.3 release, but no need to blat EEPROM

    midiStartEnable = EEPROM.read( EEPROM_ST_OFFSET );
    if( midiStartEnable == EEPROM_BLANK_BYTES ) {
      // previously blank space, so write it
      EEPROM.write( EEPROM_ST_OFFSET, 1 );
    }
    if( midiStartEnable > 1 ) { valid = 0; }

    midiCCaccent = EEPROM.read( EEPROM_AC_OFFSET );
    if( midiCCaccent == EEPROM_BLANK_BYTES ) {
      EEPROM.write( EEPROM_AC_OFFSET, 80 );
    }
    if( midiCCaccent < 1 || midiCCaccent > 127 ) { valid = 0; }

    // floats and longs

    EEPROM.get( EEPROM_SF_OFFSET, sfAdj );
    if( ( sfAdj < 0.9f ) || ( sfAdj > 1.1f ) || isnan( sfAdj ) ) { sfAdj = 1.0f; valid = 0; }

    EEPROM.get( EEPROM_GD_OFFSET, gateDelay );
    if( gateDelay < 10 || gateDelay > 1000 || isnan( gateDelay) ) { valid = 0; }
    
    EEPROM.get( EEPROM_MD_OFFSET, modDuration );
    if( modDuration > 1000 || isnan( modDuration ) ) { valid = 0; }

    
    if( !valid ) {
      display.clear();
      display.setCursor(0,2);
      display.println(F("ERR: EEPROM invalid"));
      display.println();
      display.println(F("Resetting..."));
      delay( FACTORY_RESET_TIMEOUT );
      eepromClear();
      eepromInit(); //recursive!
    }
  
  }
}

// Function   :   eepromClear( void )
// Purpose    :   format EEPROM back to all bytes = 255
void eepromClear() {
  
  #ifdef DEBUG
    Serial.println(F("Erasing EEPROM completely..."));
  #endif

  displayHdr();
  display.println(F("Erasing EEPROM..."));
  
  //clear eeprom - flash LED 3 times
  do_some_flash(3);
  
  for (int i = 0; i < SEQ_END; i++) {
    EEPROM.write( i, EEPROM_BLANK_BYTES );
  }
  displayDONE(1);
}


/* ==========================================================================================
 * EEPROM parameter update functions
 * ========================================================================================== */

void saveByteSetting( uint8_t offset, uint8_t value, bool reboot ) {

  displayHdr();
  EEPROM.write( offset, value );

  switch( offset ) {

    case EEPROM_NP_OFFSET:
      display.println(F("Saved note priority:"));
      display.println( priorityMenu );
      break;

    case EEPROM_CH_OFFSET:
      display.print(F("Saved MIDI Ch: "));
      display.println( value );
      display.println();
      display.println(F("Rebooting in 5s to"));
      display.println(F("use new MIDI channel"));
      break;

    case EEPROM_PQ_OFFSET:
      display.println(F("Saved CV In PPQN: "));
      display.println( value );
      display.print(F("i.e. "));
      display.print( tick_per_step );
      display.println(F(" per step."));
      break;

    case EEPROM_CO_OFFSET:
      display.println(F("Saved CV Out PPQN:"));
      display.println( value );
      if( value == 4 ) { display.println(F("e.g. 1 tick per step")); }
      break;

    case EEPROM_TD_OFFSET:
      display.println(F("Saved CV trigger out"));
      display.print(F("duration (ms) : "));
      display.println( value );
      break;
    
    case EEPROM_CD_OFFSET:
      display.println(F("Saved CV clock out"));
      display.print(F("duration (ms) : "));
      display.println( value );
      break;

    case EEPROM_CC_OFFSET:
      display.println(F("Saved modulation src"));
      display.print(F("MIDI CC : "));
      display.println( value );
      break;

    case EEPROM_ST_OFFSET:
      display.println(F("Saved MIDI start"));
      display.print(F("Enabled : "));
      display.println( value );
      break;

    case EEPROM_AC_OFFSET:
      display.println(F("Saved Accent trig"));
      display.print(F("MIDI CC : "));
      display.println( value );
      break;

    default:
      display.println(F("ERROR BYTE NOT SAVE"));
      display.print( offset );
      display.print(F(" : "));
      display.print( value );
      delay(5000);

  }
  delay(2000);
  
  if( reboot ) {
    delay(3000);
    resetFunc();
  } else {
    display.clear();
    set_true( &bitmap, BIT_DISPLAY_CLEARED );
  }

}

void saveUlongSetting( uint8_t offset, unsigned long value, bool reboot ) {

  displayHdr();
  EEPROM.put( offset, value );

  switch( offset ) {

    case EEPROM_GD_OFFSET:
      display.println(F("Saved gate off delay"));
      display.print(F("as : "));
      display.print( value );
      display.println(F(" (us)"));
      display.println();
      display.println(F("Only used if clk in"));
      display.println(F("PPQN=4 (TPS=1)"));
      delay(2000);
      break;

    case EEPROM_MD_OFFSET:
      display.println(F("Saved CV mod out"));
      display.print(F("duration (ms) : "));
      display.println( value );
      display.println(F("(Default 0 = stay on)"));
      delay(1000);
      break;
    
    default:
      display.println(F("ERROR ULONG NOT SAVE"));
      display.print( offset );
      display.print(F(" : "));
      display.print( value );
  }
  delay(2000);
  
  if( reboot ) {
    delay(3000);
    resetFunc();
  } else {
    display.clear();
    set_true( &bitmap, BIT_DISPLAY_CLEARED );
  }
}


void saveFloatSetting( uint8_t offset, float value, bool reboot ) {

  displayHdr();
  EEPROM.put( offset, value );

  switch( offset ) {

    case EEPROM_SF_OFFSET:
      display.println(F("Set CV V/Oct scale"));
      display.println(F("factor adjustment"));
      display.print(F("to : "));
      display.print(value,3);
      display.println(F("%"));
      break;

    default:
      display.println(F("ERROR FLOAT NOTSAVE"));
      display.print( offset );
      display.print(F(" : "));
      display.print( value );
  }
  delay(2000);
  
  if( reboot ) {
    delay(3000);
    resetFunc();
  } else {
    display.clear();
    set_true( &bitmap, BIT_DISPLAY_CLEARED );
  }
}



/* ==========================================================================================
 * EEPROM SEQEUENCE SLOT update functions
 * ========================================================================================== */

// Function   :   clearSequence( slot, hdr_bool )
// Purpose    :   clear the sequence in slot, update seq_stored array in memory
//                revert activeSeq to (D)efault if this was activeSeq
//                if no seq in that slot, return error
//                hdr_bool defines if we have a blank canvas, and need to show header
//                location used to calcuate the eeprom offset where this sequence is stored

void clearSequence( uint8_t slot, bool hdr ) {
  int location = slot - 1;
  int i = SEQ_ADDR + ( location * SEQ_LENGTH );
  int last = i + SEQ_LENGTH;
  
  if( hdr ) {
    display.clear();
    displayHdr();
  }

  if( seq_stored[slot] ) {

    #ifdef DEBUG
      Serial.print(F("Clearing seq in: ")); Serial.println( slot );
    #endif
    
    display.print(F("Clearing seq in : "));
    display.println( slot );
    delay(2000);

    EEPROM.write( slot, 0 ); // sequence empty
    seq_stored[slot] = 0;

    if( activeSlot == slot ) { 
      activeSlot = 0;
      EEPROM.write( SEQ_ACTIVE_SLOT_OFFSET, 0 );
    }
    while( i < last ) {
      EEPROM.write( i, 255);
      i++;
    }

    if( menu == SETTINGS ) {
      displayDONE(1); 
    } else {
      displayDONE(0);
    }
  } else {
    display.println(F("ERR: No sequence"));
    display.print(F("in slot : "));
    display.println( slot );
    delay(2000);
    if( menu == SETTINGS ) {
      display.clear();
      set_true( &bitmap, BIT_DISPLAY_CLEARED );
    }
  }
}

// Function   :   writeSequence( slot, hdr_bool )
// Purpose    :   write the current activeSeq data in slot, update seq_stored array in memory
//                activeSeq becomes this slot automatically
//                hdr_bool defines if we have a blank canvas, and need to show header
//                location used to calcuate the eeprom offset where this sequence is stored

void writeSequence( uint8_t slot, bool hdr ) {
  int location = slot - 1;
  int index = SEQ_ADDR + ( location * SEQ_LENGTH);
  int last = index + STEP_LEN;
  int i = 0;

  activeSlot = slot;

  #ifdef DEBUG
    Serial.print(F("Writing seq in : ")); Serial.println( slot );
    Serial.print(F("ActiveSlot now : ")); Serial.println( activeSlot );
  #endif


  if( hdr ) {
    display.clear();
    displayHdr();
  }

  display.println(F("Writing & activating"));
  display.print(F("sequence in slot : "));
  display.println( activeSlot );
  delay(2000);

  EEPROM.write( activeSlot, 1 );  // sequence stored
  EEPROM.write( SEQ_ACTIVE_SLOT_OFFSET, activeSlot ); // last active slot
  seq_stored[activeSlot] = 1; // update memory
  
  while (index < last) {
    EEPROM.write( index, activeSeq.note[i] );
    EEPROM.write( STEP_LEN + index, activeSeq.length[i] );
    EEPROM.write( ( 2 * STEP_LEN ) + index, activeSeq.accent[i] );
    EEPROM.write( ( 3 * STEP_LEN ) + index, activeSeq.rest[i] );
    EEPROM.write( ( 4 * STEP_LEN ) + index, activeSeq.slide[i] );
    index++;
    i++;
  }
  EEPROM.write( SEQ_ADDR + ( location * SEQ_LENGTH ) + SEQ_SLIDE_T_OFFSET, activeSeq.slide_type );

  if( menu == SETTINGS ) {
    displayDONE(1); 
  } else {
    displayDONE(0);
  }
}

// Function   :   readSequence( slot, hdr_bool )
// Purpose    :   read the sequence from slot into activeSeq, update seq_stored array in memory
//                if no seq in that slot, return error
//                hdr_bool defines if we have a blank canvas, and need to show header
//                location used to calcuate the eeprom offset where this sequence is stored

void readSequence( uint8_t slot, bool hdr ) {
  int location = slot - 1;
  int index = SEQ_ADDR + ( location * SEQ_LENGTH );
  int last = index + STEP_LEN;
  int i = 0;

  if( hdr ) {
    display.clear();
    displayHdr();
  } 

  if( seq_stored[slot] ) {

    activeSlot = slot;

    #ifdef DEBUG
      Serial.print(F("Reading seq from : ")); Serial.println( slot );
      Serial.print(F("ActiveSlot now : ")); Serial.println( activeSlot ); 
    #endif
  
    display.println(F("Reading & activating"));
    display.print(F("sequence in slot : "));
    display.println( activeSlot );
    delay(2000);

    EEPROM.write( SEQ_ACTIVE_SLOT_OFFSET, activeSlot ); // last active slot

    while (index < last) {
      int next = EEPROM.read(index);
      activeSeq.note[i] = next;

      next = EEPROM.read( STEP_LEN + index );
      activeSeq.length[i] = next;
    
      next = EEPROM.read( ( 2 * STEP_LEN ) + index );
      activeSeq.accent[i] = next;
    
      next = EEPROM.read( ( 3 * STEP_LEN ) + index );
      activeSeq.rest[i] = next;
    
      next = EEPROM.read( ( 4 * STEP_LEN ) + index );
      activeSeq.slide[i] = next;
    
      index++;
      i++;
    }
    activeSeq.slide_type = EEPROM.read( SEQ_ADDR + ( location * SEQ_LENGTH ) + SEQ_SLIDE_T_OFFSET );

    if( menu == SETTINGS ) {
      displayDONE(1); 
    } else {
      displayDONE(0);
    }

  } else {
    // no sequence in that slot, error
    display.println(F("ERR: No sequence"));
    display.print(F("in slot : "));
    display.println( slot );
    delay(2000);
    if( menu == SETTINGS ) {
      display.clear();
      set_true( &bitmap, BIT_DISPLAY_CLEARED );
    }
  }
}

// Function   :   shiftSequence( delta, direction, all )
// Purpose    :   shift the active sequence by "delta" positions 
//                if delta is more than the seq_length, return error
//                dir - 1 = left, 0 = right
//                all - 1 = all settings, 0 = notes only

void shiftSequence( uint8_t delta, bool dir, bool all ) {

  uint8_t i = 0;
  uint8_t count = STEP_LEN;
  uint8_t note = 36, length = 6, accent = 0, rest = 0, slide = 0;
  uint8_t v = activeSeq.note[i];

  displayHdr();

  if( delta == 0 ) {
    display.println(F("ERR: Cannot shift 0"));
    delay(2000);
    display.clear();
    set_true( &bitmap, BIT_DISPLAY_CLEARED );
    return;
  }

  while( v != LAST_STEP_NOTE ) {
    i++;
    if( i == STEP_END ) { 
      v = LAST_STEP_NOTE; 
    } else { 
      v = activeSeq.note[i]; 
    }
  }
  if( i != STEP_END ) { count = i; }

  if( delta > count-1 ) {
    display.println(F("ERR: Shift > seq_len"));
    delay(2000);
    display.clear();
    set_true( &bitmap, BIT_DISPLAY_CLEARED );
  } else {

    display.print(F("Active steps: "));
    display.println(count);
    display.print(F("Shifting "));
    if( all ) {
      display.println(F("ALL by "));
    } else {
      display.println(F("Notes by"));
    }
    display.print(delta);
    display.println(F(" steps to the"));
    if( dir ) {
      display.println(F("LEFT..."));
    } else {
      display.println(F("RIGHT..."));
    }

    delay(3000);

   
    // We are shifting the pattern, and end point is count-1
    while( delta ) {
      if( dir ) {
        // Shifting left, save first step
        note = activeSeq.note[0];
        if( all ) {
          length = activeSeq.length[0];
          accent = activeSeq.accent[0];
          rest   = activeSeq.rest[0];
          slide  = activeSeq.slide[0];
        }
        for( i = 0; i < count-1; i++ ) {
          activeSeq.note[i] = activeSeq.note[i+1];
          if( all ) {
            activeSeq.length[i] = activeSeq.length[i+1];
            activeSeq.accent[i] = activeSeq.accent[i+1];
            activeSeq.rest[i]   = activeSeq.rest[i+1];
            activeSeq.slide[i]  = activeSeq.slide[i+1];
          }
        }
        activeSeq.note[count-1]   = note;
        if( all ) {
          activeSeq.length[count-1] = length;
          activeSeq.accent[count-1] = accent;
          activeSeq.rest[count-1]   = rest;
          activeSeq.slide[count-1]  = slide;
        }
      } else {
        int j;
        note = activeSeq.note[count-1];
        if( all ) {
          length = activeSeq.length[count-1];
          accent = activeSeq.accent[count-1];
          rest   = activeSeq.rest[count-1];
          slide  = activeSeq.slide[count-1];
        }
        for( j = count - 1; j > 0; j-- ) {
          activeSeq.note[j] = activeSeq.note[j-1];
          if( all ) {
            activeSeq.length[j] = activeSeq.length[j-1];
            activeSeq.accent[j] = activeSeq.accent[j-1];
            activeSeq.rest[j]   = activeSeq.rest[j-1];
            activeSeq.slide[j]  = activeSeq.slide[j-1];
          }
        }
        activeSeq.note[0]   = note;
        if( all ) {
          activeSeq.length[0] = length;
          activeSeq.accent[0] = accent;
          activeSeq.rest[0]   = rest;
          activeSeq.slide[0]  = slide;
        }
      }
      delta--;
    } // while
  } // count non-zero
}