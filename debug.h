#ifndef _SEQ_DEBUG_H_
#define _SEQ_DEBUG_H_

#ifdef DEBUG
// only include if DEBUG defined

#define DEBUG_FULL 1          // uncomment for full debug
#define DEBUG_SEQ_SLOTS 1     // uncomment for EEPROM read at start - uses 82bytes ~4% of memory!

// set to run these at startup
#define CLEAR_EEPROM 0
#define WRITE_SEQUENCE 0
#define CLEAR_SEQUENCE 0

// ------ prototypes ------ //

// serial display functions mirroring I2C display contents
void debugStart( void );
void debugHome( void );
#endif



#ifdef DEBUG_FULL
// only include if DEBUG_FULL  defined

// ------ prototypes ------ //

// show all eeprom settings on serial console
void debugSettings( void );
#endif



#ifdef DEBUG_SEQ_SLOTS
// only include if DEBUG_SEQ_SLOTS  defined

// ------ prototypes ------ //

// so we don't mess up the activeSeq, tmpSeq is used to read in all sequences from eeprom
// when called by debugShowAllSeq() - this requires about 4% of memory and so shouldn't be
// left enabled in released builds

Seq_t tmpSeq;

// Show all contents of sequences in EEPROM on serial console
void debugShowAllSeq( void );
#endif

#endif
