#ifndef _SEQ_EEPROM_H_
#define _SEQ_EEPROM_H_

// The format of the EEPROM is
//
// Byte       0 - Inited or not (EEPROM_INIT_TRUE = inited = 74 )
//            1 - Sequence 1 stored (1 = true)
//            2 - Sequence 2 stored (1 = true)
//            3 - Sequence 3 stored (1 = true)
//            4 - Sequence 4 stored (1 = true)
//            5 - Sequence 4 stored (1 = true)
//            6 - Sequence 4 stored (1 = true)
//            7 - Sequence 4 stored (1 = true)
//            8 - Sequence 4 stored (1 = true)
//
//         9-15 - unused
//
//           16 - Last activated seq number (1-4)
//           17 - Note priority (HIGH,LOW,LAST)
//           18 - Device MIDI Channel (0-15) 0 = OMNI
//           19 - CV Clock in PPQN (24 = 6 pulse per step,
//                                  12 = 3 pulse per step,
//                                   4 = 1 pulse per step)
//           20 - CV Clock out PPQN ( 1 = 4 step per pulse,
//                                    4 = 1 step per pulse )
//           21 - CV Trigger Duration - how long the trigger pulse lasts (20ms default)
//           22 - CV Clock Duration - how long the clock out pulse lasts (20ms default)
//           23 - MIDI CC number that is used as input to CV mod output
//           24 - MIDI start/stop behaviour - enable or disable
//           25 - MIDI CC to recieve accent triggers - default CC80 - values <64 off, >63 on
//
//        26-31 - unused
//
//                (4 bytes each after here)
//
//        32-35 - Scaling Factor (float) for CV 
//        36-39 - CV Gate Delay - how many us to delay after gate goes low
//                                Only matters when CV Clock in = 1 pulse per step
//        40-43 - CV Modulation Duration - how long the mod value stays emitting (0 = infinite)
//
//        44-63 - Unused
//
// Byte  64-151 - Sequence 1
// Byte 152-239 - Sequence 2
// Byte 240-327 - Sequence 3
// Byte 328-415 - Sequence 4
// Byte 416-503 - Sequence 5
// Byte 514-591 - Sequence 6
// Byte 592-679 - Sequence 7
// Byte 680-767 - Sequence 8
//
// When reading/writing/clearing sequence EEPROM locations the location starts from 0 offset
// When displaying sequence locations to users, location starts from 1 offset !!!

// The format of a sequence is 

// Bytes  0-15  The note value for nth step.
// Bytes 16-31  The length value for nth step.
// Bytes 32-47  The accent value for nth step.
// Bytes 48-63  The rest value for nth step.
// Bytes 64-79  The slide value for nth step.
// Byte     80  The slide_type for this sequence.
// Bytes 82-87  Unused

// ------ prototypes ------ //

// eeprom functions
void eepromInit( void );
void eepromClear( void );

// eeprom update settings functions
void saveByteSetting(  uint8_t offset, uint8_t value, bool reboot );
void saveUlongSetting( uint8_t offset, unsigned long value, bool reboot );
void saveFloatSetting( uint8_t offset, float value, bool reboot );

//eeprom sequence functions
void clearSequence( uint8_t location, bool hdr );
void writeSequence( uint8_t location, bool hdr );
void readSequence(  uint8_t location, bool hdr );

//memory sequence functions
void shiftSequence( uint8_t delta, bool dir, bool all );


#endif
