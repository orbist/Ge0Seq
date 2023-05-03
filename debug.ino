#ifdef DEBUG

void debugStart() { 
  
  Serial.println();
  Serial.println(F("***********BOOT************"));
  Serial.flush();

  // Useful for testing
  
  if( CLEAR_EEPROM )   { eepromClear(); }
  if( WRITE_SEQUENCE ) { writeSequence( WRITE_SEQUENCE, 1 ); }
  if( CLEAR_SEQUENCE ) { clearSequence( CLEAR_SEQUENCE, 1 ); }
  
}

void debugHome() {
  bool found = 0;

  #ifdef DEBUG_FULL
    debugSettings();
  #endif
  #ifdef DEBUG_SEQ_SLOTS
    debugShowAllSeq();
  #endif

  // display home page 
  Serial.println(F(" D   1   2   3   4 "));

  // show which slots are used - seq_stored[] setup in EEPROMinit()
  for( uint8_t i = 0; i <= NUM_SEQ; i++ ) {
    if( i == 5 ) {
      //half way, start new line
      Serial.println();
      Serial.println(F("     5   6   7   8 "));
      Serial.print(F("    "));
    }
    if( seq_stored[i] == 1 ) {
      if( i == activeSlot ) {
        Serial.print(F(" #  "));
      } else {
        Serial.print(F(" *  "));
      }
    } else {
      Serial.print(F("    "));
    }
  }
  Serial.println();
}

#endif

#ifdef DEBUG_FULL

void debugSettings() {

  Serial.println(F("EEPROM Pages"));
  for( int i = 0; i < EEPROM_NEXT_OFFSET + 1; i++) {
    Serial.print(i);
    Serial.print(F(" : "));
    if( i == EEPROM_SF_OFFSET ) {
      float adj;
      EEPROM.get( i, adj );
      Serial.println( adj );
      i = i + 3;
    } else if( i == EEPROM_MD_OFFSET || 
               i == EEPROM_GD_OFFSET ) {
      unsigned long tmp;
      EEPROM.get( i, tmp );
      Serial.println( tmp );
      i = i + 3;
    } else {
      Serial.println( EEPROM.read(i) );
    }
  }
  Serial.flush();
  
}

#endif


#ifdef DEBUG_SEQ_SLOTS

void debugShowAllSeq() {
	
  Serial.println();
  for(uint8_t i = 1; i <= NUM_SEQ; i++ ) {
    debugShowSeq(i);
  }
  debugShowSeq(99);
  Serial.print(F("Active slot is : "));
  Serial.println(activeSlot);
  Serial.flush();
  
}


/* ==========================================================================================
 * DEBUG CONSOLE OUTPUT FUNCTIONS - can comment out to save tmpSeq memory
 * ========================================================================================== */


void debugShowSeq(int slot) {

    Serial.println();
    Serial.print(F("Sequence Location: "));

    if( slot != 99) {
      Serial.println(slot);
      readTmpSequence(slot-1);
    } else {
      tmpSeq = activeSeq;
      Serial.println(F("activeSeq"));
    }

    Serial.print(F("   Slide Type : "));
    Serial.println( tmpSeq.slide_type );
    
    Serial.print(F("Note is:\t\t"));
    for (int i = 0; i < STEP_LEN; i++) {
      Serial.print( tmpSeq.note[i] );
      Serial.print(F(" "));
    }
    Serial.println();

    Serial.print(F("Leng is:\t\t"));
    for (int i = 0; i < STEP_LEN; i++) {
      Serial.print( tmpSeq.length[i] );
      Serial.print(F(" "));
    }
    Serial.println();

    Serial.print(F("Acct is:\t\t"));
    for (int i = 0; i < STEP_LEN; i++) {
      Serial.print( tmpSeq.accent[i] );
      Serial.print(F(" "));
    }
    Serial.println();

    Serial.print(F("Rest is:\t\t"));
    for (int i = 0; i < STEP_LEN; i++) {
      Serial.print( tmpSeq.rest[i] );
      Serial.print(F(" "));
    }
    Serial.println();

    Serial.print(F("Slid is:\t\t"));
    for (int i = 0; i < STEP_LEN; i++) {
      Serial.print( tmpSeq.slide[i] );
      Serial.print(F(" "));
    }
    Serial.println();
  
} 

// can comment out to save tmpSeq memory

void readTmpSequence(int location) {
  int index = SEQ_ADDR + ( location * SEQ_LENGTH );
  int last = index + STEP_LEN;
  int i = 0;

  while (index < last) {
    int next = EEPROM.read( index );
    tmpSeq.note[i] = next;
    
    next = EEPROM.read( STEP_LEN + index );
    tmpSeq.length[i] = next;
    
    next = EEPROM.read( ( 2 * STEP_LEN ) + index );
    tmpSeq.accent[i] = next;

    next = EEPROM.read( ( 3 * STEP_LEN ) + index );
    tmpSeq.rest[i] = next;
    
    next = EEPROM.read( ( 4 * STEP_LEN ) + index );
    tmpSeq.slide[i] = next;
    
    index++;
    i++;
  }
  tmpSeq.slide_type = EEPROM.read( SEQ_ADDR + ( location * SEQ_LENGTH ) + SEQ_SLIDE_T_OFFSET );
} 

#endif
