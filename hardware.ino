

void pinInit() {

  // Setup pins
  pinMode( LED_PIN, OUTPUT );
  pinMode( GATE,    OUTPUT );
  pinMode( TRIG,    OUTPUT );
  pinMode( CLOCK,   OUTPUT );
  pinMode( ACCENT,  OUTPUT );

  pinMode( CLOCK_IN, INPUT );
  pinMode( RUN_IN,   INPUT );

  digitalWrite( GATE,   LOW );
  digitalWrite( TRIG,   LOW );
  digitalWrite( CLOCK,  LOW );
  digitalWrite( ACCENT, LOW );
  
}

void encoderInit( void ) {

  pinMode( ENC_A,   INPUT_PULLUP );
  pinMode( ENC_B,   INPUT_PULLUP );
  encButton.interval(25);
  encButton.attach( ENC_BUT, INPUT_PULLUP );

}


// pin state transition helper - only react if state has changed from last known state
inline bool pinHasChanged( uint8_t pin ) {

  bool current = digitalRead( pin );
  bool last;

  if( pin == RUN_IN ) {
    last = last_run_state;
  } else if( pin == CLOCK_IN ) {
    last = last_clock_state;
  }

  if( current != last ) {
    if( current == 1) {
      if( pin == RUN_IN ) {
        last_run_state = 1;
      } else if( pin == CLOCK_IN ) {
        last_clock_state = 1;
      }
    } else {
      if( pin == RUN_IN ) {
        last_run_state = 0;
      } else if( pin == CLOCK_IN ) {
        last_clock_state = 0;
      }
    }
    return 1;
  } else {
    return 0;
  }
}

// initialise activeSeq on boot up
void activeSeqInit() {

  // read the activeSlot from EEPROM, default init sets to 0 hardcoded sequence
  activeSlot = EEPROM.read( SEQ_ACTIVE_SLOT_OFFSET ); 

  // see if we have stored a sequence in that slot, if so load it to activeSeq
  if( activeSlot != 0 ) {
    display.clear();
    display.setInvertMode(1);
    display.println(F(BOOT_BANNER));
    display.setInvertMode(0);
    display.println();
    display.println(F("Loading..."));
    display.println();
    delay(500);
    readSequence( activeSlot, 0 );
  }
  
}

void midiInit() {

  // Callback fn for sysEx
  MIDI.setHandleSystemExclusive( sysExInterpreter );

  if( midiChannel == 0 ) {
    MIDI.begin( MIDI_CHANNEL_OMNI );
  } else {
    MIDI.begin( midiChannel );
  }
  MIDI.turnThruOff();

  
}

void dacInit() {

  dac.init();
  dac.turnOnChannelA();
  dac.turnOnChannelB();
  dac.setGainA( MCP4822::High );
  dac.setGainB( MCP4822::High );
  dac.setVoltageA( 0 ); // Set initial V/Oct voltage to 0v
  dac.setVoltageB( 0 ); // Set initial modulation voltage to 0v
  dac.updateDAC();

}
// led flash helper
void do_some_flash( uint8_t nof_flash ) {

  for( uint8_t i = 0; i < nof_flash; i++ ) {
    digitalWrite( LED_PIN, HIGH );
    delay( FAST_FLASH );
    digitalWrite( LED_PIN, LOW );
    delay( FAST_FLASH );
  }
  delay( PAUSE_FLASH );
}

