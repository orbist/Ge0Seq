

void pinInit() {

  // Setup pins
  // Pins D 2-5 are outputs, 6,7 inputs, default state for outputs is low.
  // DDRD = ( 1 << ACCENT_DDR ) | ( 1 << GATE_DDR ) | ( 1 << CLOCK_DDR ) | ( 1 << TRIG_DDR );
  // led pin to output, leave low for now;
  DDRB |= ( 1 << LED_DDR );
  DDRB |= ( 1 << LED_BOOT_DDR );
  NOP;

  pinMode( GATE,    OUTPUT );
  pinMode( TRIG,    OUTPUT );
  pinMode( CLOCK,   OUTPUT );
  pinMode( ACCENT,  OUTPUT );

  pinMode( CLOCK_IN, INPUT );
  pinMode( RUN_IN,   INPUT );

  pinMode( REV_IN, INPUT );
  
  digitalWrite( GATE,   LOW );
  digitalWrite( TRIG,   LOW );
  digitalWrite( CLOCK,  LOW );
  digitalWrite( ACCENT, LOW );
  
}

void encoderInit( void ) {

  // Set encoder A and B as INPUT_PULLUP
  PORTC = ( 1 << ENC_A_DDR ) | ( 1 << ENC_B_DDR );
  NOP;

  //pinMode( ENC_A,   INPUT_PULLUP );
  //pinMode( ENC_B,   INPUT_PULLUP );
  encButton.attach( ENC_BUT, INPUT_PULLUP );
  encButton.interval(5);
  

}


// pin state transition helper - only react if state has changed from last known state
inline bool pinHasChanged( uint8_t pin ) {

  //bool current = (pin & bit( pin_bit )) == 0;
  bool current = digitalRead( pin );

  switch( pin ) {

    case RUN_IN:
     
      if( current != is_true( &bitmap, BIT_LAST_RUN_STATE ) ) {
        if( current == 1) {
          set_true( &bitmap, BIT_LAST_RUN_STATE );
        } else {
          set_false( &bitmap, BIT_LAST_RUN_STATE );
        }
        return 1;
      } else {
        return 0;
      }
      break;

    case CLOCK_IN:

      if( current != is_true( &bitmap, BIT_LAST_CLK_STATE ) ) {
        if( current == 1) {
          set_true( &bitmap, BIT_LAST_CLK_STATE );
        } else {
          set_false( &bitmap, BIT_LAST_CLK_STATE );
        }
        return 1;
      } else {
        return 0;
      }
      break;
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
    LED_PORT |= ( 1 << LED_BIT );
    //digitalWrite( LED, HIGH );
    delay( FAST_FLASH );
    LED_PORT &= ~( 1 << LED_BIT );
    //digitalWrite( LED, LOW );
    delay( FAST_FLASH );
  }
  delay( PAUSE_FLASH );
}

