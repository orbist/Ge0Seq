/* 
 *    Ge0synchronous Sequencer - Ge0seq
 *
 *    Basic MIDI to CV conversion, with various note priority.
 *    303 Style Sequencer running on MIDI start and clock.
 *    Program via associated Ge0sync Sequence Editor App.
 *    Menu system for NV settings.
 *
 *    Runs on Atmega328PU CPU - development via Arduino UNO
 *
 *    Available as stand alone PCB (used in Ge0sync desktops)
 *    Available as Eurorack module. 
 *
 *    Copyright (c) Barry Whyte 2023 - Ge0synchronous Synths
 *   
 *    Credit to Larry McGovern - the basic MIDI 2 CV handling is
 *    based on his project : https://github.com/elkayem/midi2cv
 *    and the Menu system from his USB MIDI 2 CV project. As these
 *    are distributed under GNU ...
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License <http://www.gnu.org/licenses/> for more details. 
 */

// Debug mode - requires access to serial console - see deeper debug modes in debug.h
//#define DEBUG 1               // uncomment for basic debug

#include <stdint.h>
#include <MIDI.h>
#include <SPI.h>
#include <MCP48xx.h>
#include <EEPROM.h>
#include <Bounce2.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"

#include "definitions.h"
#include "variables.h"
#include "debug.h"
#include "hardware.h"
#include "display.h"
#include "seq_eeprom.h"
#include "seq_menu.h"


/* TODO
 * 1. update gui for new settings
 * 2. check how sysex sends a large number...
 * 3. add mod_duration and sfAdj to gui??? ^^^
 */


/* ==========================================================================================
 * SETUP
 * ========================================================================================== */

void setup() {

  Serial.begin( 31250 );

  #ifdef DEBUG
   debugStart();
  #endif

  pinInit();                      // PIN modes and setup
  encoderInit();                  // Configure the Encoder
  displayInit();                  // Initialise display and show splash
  eepromInit();                   // Check EEPROM is inited & setup defaults if its valid
  midiInit();                     // Midi configuration
  dacInit();                      // DAC configuration
  activeSeqInit();                // Read in last activeSeq if defined
  setNotePriorityText();          // ensure menu has the right text decode
  digitalWrite( LED_PIN, HIGH );  // Setup done, ready to go, turn the RDY LED on

  #ifdef DEBUG
    debugHome();                  // Debug dump of the sequencer slots & EEPROM settings
  #endif

}


/* ==========================================================================================
 * MAIN LOOP
 * ========================================================================================== */


void loop() {

  uint8_t noteMsg;
  static unsigned long clock_timer = 0, clock_timeout = 0, mod_timer = 0, display_timer = 0;
  static unsigned int clock_count = 0, tick_count = 0, cv_clock_count = 0;

  static bool cv_playing = 0;                  // sequence controlled by CV clock not MIDI
  static bool playing = 0;                     // sequence is running
  static bool starting = 0;                    // start has been recieved about to be playing

  // make sure we are showing something
  // either the main summary screen, or the 'playing screen'
  // display_timer user to clear ERR msgs after 3s

  if( displayCleared || display_timer > 0 ) {
    if( playing || cv_playing ) {
      if( ( millis() - display_timer )  > 3000 ) {
        displayMsgPlaying( cv_playing );
        display_timer = 0;
        displayCleared = 0;
      }
    } else if( displayCleared ) {
      displayHomePage();
      displayCleared = 0;
    }
  }

  // Set trigger low after 20 msec  
  if( ( trigTimer > 0 ) && ( millis() - trigTimer > trigDuration ) ) {
    digitalWrite( TRIG, LOW );  
    trigTimer = 0;
  }

   // Set clock pulse low after 20 msec
  if( ( clock_timer > 0 ) && ( millis() - clock_timer > clockDuration ) ) {
    digitalWrite( CLOCK, LOW ); 
    clock_timer = 0;
  }

  // Set mod low after XXX ms
  // if value = 0 then leave mod ouput mV alone
  if( ( mod_timer > 0 ) && ( millis() - mod_timer > modDuration ) ) {
    dac.setVoltageB( 0 ); 
    dac.updateDAC();
    mod_timer = 0;
  }

  // Main encoder/menu loop, menu shows after first enc button press, updated on each subsequent
  updateEncoderPos();  
  encButton.update();
  if( encButton.fell() ) { updateMenu(); } // 

  // Check if menu highlighting timer expired, and remove highlighting if so
  if( highlightEnabled && ( ( millis() - highlightTimer ) > HIGHLIGHT_TIMEOUT ) ) {  
    highlightEnabled = false;
    menu = SETTINGS;    // Return to main screen and hide menu
    display.clear();
    displayCleared = 1;
  }

  // Check if we have CV RUN trigger
  if( pinHasChanged( RUN_IN ) ) {
    // pin state has changed
    if( last_run_state ) {
      // pin has gone HIGH
      if( cv_playing ) {
        // were playing so stop
        cv_playing = 0;
        cv_clock_count = 0;
        dac.setVoltageA( 0 );
        dac.updateDAC();
        digitalWrite( GATE, LOW );

        displayCleared = 1;
        display.clear();

      } else {
        // start playing on next CV clock tick
        if( !playing ) {
          // check MIDI isn't already playing
          step_i = STEP_START;
          cv_playing = 1;
          starting = 1;
          cv_clock_count = 0;
          tick_count = 0;
          step_length = 0;
          tick_per_step = my_div( cvClockPPQN, 4 );
          if( tick_per_step == 1 ) { activeSeq.slide_type = FULL; } // make sure we sort of slide
          slide_div = isSlidePossible();
          displayMsgPlaying( cv_playing );
        } else if( display_timer == 0 ) {
          // got CV run but MIDI playing show error for a bit
          display.println();
          display.println(F("ERR: Already playing!"));
          display_timer = millis();
        }
      }
    } 
  }

  // Main CV Clock loop
  if( pinHasChanged( CLOCK_IN ) ) {
    // we have a state change, if gone HIGH then its a new tick
    if( last_clock_state ) {
      // new tick
      if( starting && cv_playing ) {
        // first tick after run, so play
        starting = 0;
        
        playNextNote();
      } else if( cv_playing ) {
        // inc/dec counters for this tick
        cv_clock_count++;   // count the ticks up to PPQN
        tick_count++;       // count the ticks per step
        cv_step_length--;   // countdown the ticks for this note's total length (maybe multiple steps)

        if( tick_count == tick_per_step ) { 
          // tick count has reached the end of a step (1/16th note)
          tick_count = 0; // reset

          if( cv_step_length == 0 ) {
            //we have reached the end of this note
            //turn off accent as we are on to a new step, will get turned on if needed by playNextNote
            digitalWrite( ACCENT, LOW );
            // because we don't have interim ticks with 4 PPQN, turn off gate
            // need to delay after this or it turns on again immediately in playNextNote
            // can modify gateDelay via menu - approx 500 us seems about right
            if( tick_per_step == 1 && !sliding ) { 
              digitalWrite( GATE, LOW ); 
              delayMicroseconds( gateDelay ); 
            }
            playNextNote();  // increments step_i at end of function
          } else {
            // we haven't reach the end of this note
            // because step_length is more than one step
            // keep things moving on however
            step_i++; 
          }
        }
        
        // Configurable pulses per quarter node - 4,8,12,16,20,24 (divide by 4 for pulse per step)
        if( cv_clock_count == cvClockPPQN ) {
          cv_clock_count = 0;

        // turn off gate one tick before next step if we aren't sliding and have interim ticks
        } else if( !sliding && cv_step_length == 1 && tick_per_step != 1 ) {
          digitalWrite( GATE, LOW );

        // if we are sliding, check if we are into the slide ticks 
        // (based on slide_type) and inc/dec voltage to slide up/down
        } else if( sliding && slideVoltStart != 0 && ( slide_div >= cv_step_length ) ) {
          unsigned int mV = slideVoltStart; //safety
          uint8_t deltaFactor = 0;

          if( slide_div < 6 ) {
            deltaFactor = slide_div - cv_step_length;
          } else {
            deltaFactor = cv_step_length;
          }

          if( deltaUp ) {
            mV = ( unsigned int )( ( float )( slideVoltStart + ( deltaFactor * delta ) ) );
          } else {
            mV = ( unsigned int )( ( float )( slideVoltStart - ( deltaFactor * delta ) ) );
          }

          dac.setVoltageA( mV );
          dac.updateDAC();
          
        }
      } // else not cv_playing, ignore
    } // else pin has gone low, ignore
  }

  // Main MIDI loop - only respond to MIDI when not already playing via CV RUN/CLOCK IN

  if( MIDI.read() && !cv_playing ) {
    
    byte type = MIDI.getType();
    switch( type ) {

      case midi::NoteOn:
      case midi::NoteOff:

        noteMsg = MIDI.getData1() ;  // C0 = 12, Top Note = 108, C8
        if( noteMsg < 0 ) { noteMsg = 0; }
        if( ( noteMsg < 0 ) || ( noteMsg > 99 ) ) break;  // Only 88 notes of keyboard are supported

        if( type == midi::NoteOn ){
          notes[ noteMsg ] = true;
        } else {
          notes[ noteMsg ] = false;
        }

        if( notePriority == NP_LAST ) {  
          if( notes[ noteMsg ] ) {  
            // If note is on, add to ordered list
            order_i = ( order_i + 1 ) % 5;
            noteOrder[ order_i ] = noteMsg;
          }
          playLastNote();
        } else if ( notePriority == NP_LOW ) {  
          playBottomNote();
        } else {                 
          playTopNote();
        }
        break;

      case midi::Start:
        
        step_i = STEP_START;
        playing = 1;        // we are playing a sequence
        starting = 1;       // set so we wait for next tick to actually start the seq

        clock_count = 0;    // number of ticks since last quarter note (24 ppqn)
        tick_count = 0;     // one step is 1/16 note - so tick_count up to 6 (6 ppstep)
        step_length = 0;    // decrements, based on sequencer step length
        tick_per_step = 6;  // MIDI 24 PPQN
        slide_div = isSlidePossible(); // slide_type, for how many sub-steps to spread slide over
        break;

      case midi::Stop:

        playing = 0; // we stop playing a sequence
        dac.setVoltageA( 0 );
        dac.updateDAC();
        digitalWrite( GATE, LOW );
        displayCleared = 1;
        display.clear();
        break;

      case midi::Clock:

        if( clock_count == 0 ) {
          digitalWrite( CLOCK, HIGH );  // Start clock pulse
          clock_timer = millis();
        }

        if( starting ) {
          starting = 0; // we got the first tick after start command, lets go!
          displayMsgPlaying( cv_playing );
          playNextNote();
        }

        // inc/dec counters for this tick
        clock_count++;
        tick_count++;
        step_length--;

        // MIDI timing clock sends 24 pulses per quarter note.  
        // Sent pulse only once every 24 pulses
        if( clock_count == my_div( 24, cvClockOutPPQN ) ) {
          clock_count = 0;
        }

        // note count has reached the end of a step (1/16th note)
        if( tick_count == 6 ) { 
          tick_count = 0; // reset
          if( playing ) {
            if( step_length == 0 ) {
              //we have reached the end of this note
              // turn off accent as we are on to a new step, will get turned on if needed by playNextNote
              digitalWrite( ACCENT, LOW );
              playNextNote();  // increments step_i at end of function
            } else {
              // we haven't reach the end of this note
              // because step_length is more than one step
              // keep things moving on however
              step_i++; 
            }
          }
        }
        
        // turn off gate one tick before next step if we aren't sliding
        if( playing && !sliding && step_length == 1 ) {
          digitalWrite( GATE, LOW );

        // if we are sliding, check if we are into the slide ticks 
        // (based on slide_type) and inc/dec voltage to slide up/down
        } else if( playing && sliding && slideVoltStart != 0 && ( slide_div >= step_length ) ) {
          unsigned int mV = slideVoltStart; //safety
          uint8_t deltaFactor = slide_div - step_length; 
          
          if( deltaUp ) {
            mV = ( unsigned int )( ( float )( slideVoltStart + ( deltaFactor * delta ) ) );
          } else {
            mV = ( unsigned int )( ( float )( slideVoltStart - ( deltaFactor * delta ) ) );
          }

          dac.setVoltageA( mV );
          dac.updateDAC();
        }
        break;

      case midi::ControlChange:

        uint8_t midi_cc = MIDI.getData1();
        uint8_t value   = MIDI.getData2();  // d2 from 0 to 127, mid point = 64

        // Default is mod wheel as mod source CC == 1
        if( midi_cc == midiCCnumber ) {
          unsigned int mV = 2 * ( value << 4 );
          dac.setVoltageB( mV );
          dac.updateDAC();
          // if modDuration not infinite (0) then start timer, 
          //else leave mod_timer as zero so we never stop the CV output
          if( modDuration > 0 ) { mod_timer = millis(); };
        }
        break; 

      case midi::ActiveSensing: 
        break;

      default:
        uint8_t d1 = MIDI.getData1();
        uint8_t d2 = MIDI.getData2();
    }
  }
  
}

/* ==========================================================================================
 * NOTE SELECT FUNCTIONS
 * ========================================================================================== */

void playTopNote() {
  uint8_t topNote = 0;
  bool noteActive = false;

  for( uint8_t i = 0; i < 99; i++ ) {
    if( notes[i] ) {
      topNote = i;
      noteActive = true;
    }
  }

  if( noteActive ) {
    playNote( topNote );
  } else {
    // All notes are off, turn off gate
    digitalWrite( GATE, LOW );
  }
}

void playBottomNote() {
  uint8_t bottomNote = 0;
  bool noteActive = false;

  for( uint8_t i = 99; i >= 0; i-- ) {
    if( notes[i] ) {
      bottomNote = i;
      noteActive = true;
    }
  }

  if( noteActive ) {
    playNote( bottomNote );
  } else {
    // All notes are off, turn off gate
    digitalWrite( GATE, LOW );
  }
}

void playLastNote() {
  uint8_t note_i;

  for( uint8_t i = 0; i < 5; i++ ) {
    note_i = noteOrder[mod( order_i - i, 5 )];
    if( notes[note_i]) {
      playNote( note_i );
      return;
    }
  }
  digitalWrite( GATE, LOW );  // All notes are off
}


// play next note from activeSeq

void playNextNote() {

  if( step_i > STEP_END || activeSeq.note[step_i] == LAST_STEP_NOTE )  { step_i = STEP_START; }

  uint8_t noteMsg = activeSeq.note[step_i];
  uint8_t rest = activeSeq.rest[step_i];
  uint8_t accent = activeSeq.accent[step_i];
  uint8_t slide = activeSeq.slide[step_i];

  step_length = activeSeq.length[step_i];                           // global, used to track the number of ticks for this note
  cv_step_length = ( my_div( step_length, 6 ) * tick_per_step );    // same, but for CV ticks

  if( !rest ) {
    if( accent ) { digitalWrite( ACCENT, HIGH ); }
  
    if( slide ) {
      sliding = 1;
      if( slide_div > 1 ) {
        // slide requested, so save the start, end and delta values
        
        slideVoltStart = ( float )( ( activeSeq.note[step_i] * NOTE_SF ) - OCT_SF );
    
        if( activeSeq.note[step_i+1] == LAST_STEP_NOTE ) {
          slideVoltEnd = (float)( ( activeSeq.note[0] * NOTE_SF ) - OCT_SF );
        } else {
          uint8_t steps = my_div( step_length, 6 ); // adjust end if step_length > 1 step
          slideVoltEnd = (float)( ( activeSeq.note[ step_i + steps ] * NOTE_SF ) - OCT_SF );
        }

        if( slideVoltStart > slideVoltEnd ) {
          // slide down
          delta = (slideVoltStart - slideVoltEnd ) / slide_div;
          deltaUp = 0;
        } else if( slideVoltStart < slideVoltEnd ) {
          //slide up
          delta = (slideVoltEnd - slideVoltStart ) / slide_div;
          deltaUp = 1;
        } else {
          // same note
          slideVoltStart = 0;
          slideVoltEnd = 0;
          delta = 0;
        }
      }
    } else {
      slideVoltStart = 0;
      slideVoltEnd = 0;
      delta = 0;
      sliding = 0;
    } 
    playNote( noteMsg );

  } else { // rest

    digitalWrite( GATE, LOW );
    delayMicroseconds( gateDelay );
    playNote( 0 );
  }

  step_i++; // move on to next step

}


// DAC output will be (4095/87) = 47.069 mV per note, and 564.9655 mV per octive
// Note that DAC output will need to be amplified by 1.77X for the standard 1V/octave
// OpAMP may struggle to recieve very low voltages, starts around A0 note 8

void playNote( uint8_t noteMsg ) {
  unsigned int mV;

  if( noteMsg != 0 ) {
    digitalWrite( GATE, HIGH );
    digitalWrite( TRIG, HIGH );
    trigTimer = millis();
  }

  if((float)( noteMsg * NOTE_SF ) - OCT_SF < 0 ) {
    mV = 0;
  } else {
    mV = ( unsigned int )( ( float )(  ( noteMsg * NOTE_SF * sfAdj ) - ( OCT_SF * sfAdj )  ) );
  }
  
  dac.setVoltageA( mV );
  dac.updateDAC();

}


/* Slide types possible will vary based on MIDI or CV PPQN
 *
 * PPQN         4     8     12    16    20    24 (MIDI)
 * TICK/STEP    1     2     3     4     5     6 
 * MIDI                                       *
 * CV           *     *     *     *     *     *
 *
 * SlideType Possible
 * FULL         n/a   2     3     4     5     6     // == tick_per_step -> if > 1
 * HALF         n/a   n/a   n/a   2     2     3     // == my_div( tick_per_step, 2 ) -> if result > 1
 * QART         n/a   n/a   n/a   n/a   n/a   2     // == my_div( tick_per_step, 3 ) -> if result > 1
 */

uint8_t isSlidePossible() {

  uint8_t slide_divider = 0;

  if( tick_per_step > 1 ) {
    switch( activeSeq.slide_type ) {
      
      case FULL:
        slide_divider = tick_per_step;
        break;

      case HALF:
        slide_divider = my_div( tick_per_step, 2 );
        break;

      case QART:
        slide_divider = my_div( tick_per_step, 3 );
        break;
    }
  } 
  return slide_divider;
}


/* ==========================================================================================
 * MIDI FUNCTIONS
 * ========================================================================================== */

//Is executed everytime a Sysex message is received
void sysExInterpreter(byte* data, unsigned messageLength) {

  //check if index is a valid Sysex message
  if( data[MANUFACTURER] == GEOSYNC_MANUF_ID ) {
    //check the command byte and process command
    switch( data[COMMAND] ) {

      case SAVE_STEP : {
        //Sets a step in the sequence
        //PARAM 1 : step #
        //PARAM 2 : note
        //PARAM 3 : reset
        //PARAM 4 : accent
        //PARAM 5 : slide
        uint8_t step = data[PARAM1];
        activeSeq.note[step]   = data[PARAM2];
        activeSeq.length[step] = data[PARAM3];
        activeSeq.rest[step]   = data[PARAM4];
        activeSeq.accent[step] = data[PARAM5];
        activeSeq.slide[step]  = data[PARAM6];
        break;
      }
      case SAVE_SEQ : {
        //Saves the current state of the machine to the specified slot
        //PARAM 1 : the slot where the preset will be saved
        displayHdr();   
        display.println(F("SysEx seq data in..."));
        delay(750);
    
        if (data[PARAM1] < 8) {
          writeSequence( data[PARAM1], 1 );
        }
        break;
      }
      case ACTIVATE_SEQ : {
        //Sets the state of the machine according to the specified preset
        //PARAM 1 : number of the preset to load
        displayHdr(); 
        display.println(F("SysEx cmd data in..."));
        delay(750);
        
        if (data[PARAM1] < 8) {
          readSequence( data[PARAM1], 0 );
        }
        break;
      }
      case SET_MIDI_CH : {
        //Changes the global MIDI channel according to the one specified
        //PARAM 1 : MIDI channel  (1 to 16)
        if( data[PARAM1] < 17 && data[PARAM1] >= 0 ) {
          if( data[PARAM1] != midiChannel ) {
            midiChannel = data[PARAM1];
            saveByteSetting( EEPROM_CH_OFFSET, midiChannel, 1 );
          }
        }
        break;
      }
      case SET_NOTE_P : {
        //Changes the note priority setting
        //PARAM 1 : NOTE_P (0,1,2)

        // This is the first message we get in the batch of device updates
        displayHdr();   
        display.println(F("SysEx seq data in..."));
        delay(1000);

        if( data[PARAM1] < 3 && data[PARAM1] >= 0 ) {
          if( data[PARAM1] != notePriority) {
            notePriority = data[PARAM1];
            setNotePriorityText();
            saveByteSetting( EEPROM_NP_OFFSET, notePriority, 0 );
          } else {
            display.clear();
          }
        } else {
          display.clear();
        }
        break;
      }
      case SET_SLIDE_T : {
        //Changes the global slide setting
        //PARAM 1 : SLIDE_T 6,3,2
        if( data[PARAM1] <= 6 && data[PARAM1] >= 2) {
          if( data[PARAM1] != activeSeq.slide_type ) {
            activeSeq.slide_type = data[PARAM1];
          }
        }
        break;
      }
      case SET_PPQN : {
        //Changes the CV PPQN setting
        //PARAM 1 : PPQN (4,8,12,16,20,24)
        if( data[PARAM1] != cvClockPPQN ) {
          cvClockPPQN = data[PARAM1];
          saveByteSetting( EEPROM_PQ_OFFSET, cvClockPPQN, 0 );
        }
        break;
      }
      case SET_OUT_PPQN : {
        //Changes the CV OUT PPQN setting
        //PARAM 1 : PPQN (1,4)
        if( data[PARAM1] != cvClockOutPPQN ) {
          cvClockOutPPQN = data[PARAM1];
          saveByteSetting( EEPROM_CO_OFFSET, cvClockOutPPQN, 0 );
        }
        break;
      }
      case SET_GATE_OFF : {
        if( data[PARAM1] * 100 != gateDelay ) {
          gateDelay = data[PARAM1] * 100;
          saveUlongSetting( EEPROM_GD_OFFSET, gateDelay, 0 );
        }
        break;
      }
      case SET_MOD_DURATION : {
        if( data[PARAM1] * 10 != modDuration ) {
          modDuration = data[PARAM1] * 10;
          saveUlongSetting( EEPROM_MD_OFFSET, modDuration, 0 );
        }
        break;
      }
      case SET_TRIG_DURATION : {
        if( data[PARAM1] * 10 != trigDuration ) {
          trigDuration = data[PARAM1] * 10;
          saveByteSetting( EEPROM_TD_OFFSET, trigDuration, 0 );
        }
        break;
      }
      case SET_CLOCK_DURATION : {
        if( data[PARAM1] * 10 != clockDuration ) {
          clockDuration = data[PARAM1] * 10;
          saveByteSetting( EEPROM_CD_OFFSET, clockDuration, 0 );
        }
        // last of batch so make sure we got back to menu
        // or else reboot being called by SET_MIDI_CH
        displayCleared = 1;
        break;
      }
      case SET_MOD_CC : {
        if( data[PARAM1] != midiCCnumber ) {
          midiCCnumber = data[PARAM1];
          saveByteSetting( EEPROM_CC_OFFSET, midiCCnumber, 0 );
        }
        break;
      }
      case TEST_COMMS : {
        if( data[PARAM1] == MAJOR_VERSION_INT &&
            data[PARAM2] == MINOR_VERSION_INT &&
            data[PARAM3] == FIX_VERSION_INT ) {
          // Just check first 3, the build version shouldn't invalidate the settings etc
          // If things are changed in the comms / settings, increment at least FIX version
          displayTestComms( 1 );
        } else {
          displayTestComms( 0 );
        }
        break;
      }

    } // switch
  } // if
}
