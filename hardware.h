#ifndef _SEQ_HARDWARE_H_
#define _SEQ_HARDWARE_H_

// ------ prototypes ------ //

// initialise pins as required
void pinInit( void );

// pin state transition helper - only react if state has changed from last known state
inline bool pinHasChanged( uint8_t pin );

// initialise activeSeq on boot
void activeSeqInit( void );

// initialise MIDI call backs, settings and channel
void midiInit( void );

// initialise DAC channels and gain, and set outputs to 0mV
void dacInit( void );

// led flash helper
void do_some_flash( uint8_t nof_flash );

#endif

