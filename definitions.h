/*
 * This file contains all the definitions of the constats used throughout the code
 */

#ifndef INC_GUARD_DEFINITIONS
#define INC_GUARD_DEFINITIONS


// Update these when releasing new version
#define MAJOR_VERSION_STR "1"
#define MINOR_VERSION_STR "5"
#define FIX_VERSION_STR   "0"
#define BUILD_VERSION_STR "2"

#define MAJOR_VERSION_INT 1
#define MINOR_VERSION_INT 5
#define FIX_VERSION_INT   0
#define BUILD_VERSION_INT 2

#define BUILD_DATE "230504"

#define GEOSYNC_MANUF_ID 99

//#define PRODUCT_NAME "      TB-Valhalla     "
#define PRODUCT_NAME "     TB-Sequencer     "
#define BOOT_BANNER  "      BOOTING...      "

// ------ Arduino pin numbers ------ //

// 0 - RX - arduino hw / MIDI default
// 1 - TX - arduino hw
#define ACCENT 2      // 4
#define GATE 3        // 5
#define TRIG 4        // 6
#define CLOCK 5       // 11
#define CLOCK_IN 6    // 12
#define RUN_IN 7      // 13
// 8 unused
#define LED_PIN 9     // 15 
#define DAC_CS 10     // 16  
// 11 MOSI
// 12 unused
// 13 SCK
#define ENC_A 15      // 23  A0
#define ENC_B 14      // 24  A1
#define ENC_BUT 16    // 25  A2
// 17 unused
#define DISP_SDA 18   // 27  A4
#define DISP_SCL 19   // 28  A5


// ------ Note Priority values ------ //

#define NP_LAST 0
#define NP_HIGH 1
#define NP_LOW  2


// ------ Step sequence params ------ //

#define STEP_START  0    // byte offset
#define STEP_END   15     // byte offset
#define STEP_LEN   16

#define LAST_STEP_NOTE 127          // Used to denote sequences less than 16 steps

#define NUM_SEQ 8

#define SEQ_ADDR 64                 // seqeucnces offset eeprom start byte
#define SEQ_LENGTH 88               // full sequence data byte length

#define SEQ_SLIDE_T_OFFSET 80       // offset into sequence data for slide_type
#define SEQ_END ( SEQ_ADDR + ( SEQ_LENGTH * NUM_SEQ ))  // last byte we use in eeprom

// ------ EEPROM params ------ //

#define EEPROM_INIT_TRUE 74         //flag to denote eeprom has been inited

#define EEPROM_INITED_OFFSET 0      //offset into eeprom for init byte

#define SEQ_ACTIVE_SLOT_OFFSET 16    //offset into eeprom for lastActive
#define EEPROM_NP_OFFSET 17          //offset into eeprom for notePriority
#define EEPROM_CH_OFFSET 18          //offset into eeprom for MIDI channel
#define EEPROM_PQ_OFFSET 19         //offset into eeprom for CV Clock in PPQN
#define EEPROM_CO_OFFSET 20         //offset into eeprom for CV Clock out PPQN
#define EEPROM_TD_OFFSET 21         //offset into eeprom for CV trigDuration (ulong 4 bytes)
#define EEPROM_CD_OFFSET 22         //offset into eeprom for CV clockDuration (ulong 4 bytes)
#define EEPROM_CC_OFFSET 23         //offset into eeprom for MIDI CC number
#define EEPROM_BYTE_NEXT_OFFSET 24  // unused 24-31

// 4bytes each below here
#define EEPROM_SF_OFFSET 32          //offset into eeprom for Scale Factor (float 4 bytes)
#define EEPROM_GD_OFFSET 36         //offset into eeprom for CV gateDelay (ulong 4 bytes)
#define EEPROM_MD_OFFSET 40         //offset into eeprom for CV modDuration (ulong 4 bytes)
#define EEPROM_NEXT_OFFSET 44       // unused 44 up to SEQ_ADDR


// ------ DAC note voltages ------ //

//CV values to output to DAC - needs multiplied by 1.77 for 1V/Oct range

#define NOTE_SF 47.069f  // This value can be tuned if CV output isn't exactly 1V/octave
#define OCT_SF 564.9655f


// ------ Screen definitions ------ //

#define SSD1306_NO_SPLASH 1
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C

// ------ Enumerations ------ //

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
 * NB: if not possible, it won't slide gradually, but we will not turn off gate between steps
 */


enum slide_t {
  FULL = 6,
  HALF = 3,
  QART = 2
};

enum SysexByteNumber_t {
  MANUFACTURER = 1,
  COMMAND = 2,
  PARAM1 = 3,
  PARAM2 = 4,
  PARAM3 = 5,
  PARAM4 = 6,
  PARAM5 = 7,
  PARAM6 = 8
};

// the numbers associated with every command in the sysex interpreter
// must match what GS editor encodes as SysEx commands
enum Command_t {
  SAVE_STEP = 1,
  SET_PPQN = 2,
  SET_OUT_PPQN = 3,   // new
  SET_GATE_OFF = 4,   // gui sends between 1 and 10 (x100) us
  SAVE_SEQ = 5,
  ACTIVATE_SEQ = 6,
  SET_SLIDE_T = 7,
  SET_NOTE_P = 8,
  SET_MIDI_CH = 9,
  SET_MOD_CC = 10, // new
  SET_TRIG_DURATION = 11,  // new - gui sends 1 to 10 (x10) ms
  SET_CLOCK_DURATION = 12, // new - gui sends 1 to 10 (x10) ms
  SET_MOD_DURATION = 14,   // new - gui sends 0 to 100 (x10) ms ( 0 always on )
  TEST_COMMS = 15
};

// the menu states
enum Menu {
  SETTINGS,                     // top level list
  CV_SETTINGS,
  MIDI_SETTINGS,
  SEQ_SETTINGS,
  FACTORY_RESET,
  ABOUT,
  CV_CLOCK_PPQN_SET,            // CV settings 1
  CV_OUT_PPQN_SET,
  CV_GATE_OFF_DELAY_SET,
  SCALE_FACTOR_SET,
  CV_SETTINGS2,
  CV_TRIG_DURATION_SET,         // CV settings 2
  CV_CLOCK_DURATION_SET,
  CV_MOD_DURATION_SET,
  MIDI_CHANNEL_SET,             // MIDI settings
  NOTE_PRIORITY_SET,
  MIDI_CC_MOD_SET,
  ACTIVATE_SEQ_SET,             // Sequence settings
  CLEAR_SEQ_SET,
  SAVE_ACTIVE_SEQ_SET,
  MODIFY_SEQ,
  FACTORY_RESET_CONFIRM,
  EXIT
} menu;

// ------ LED Definitions ------ //
// LED flash rates & clear timeout
#define FAST_FLASH 300
#define SLOW_FLASH 750
#define PAUSE_FLASH 1500

// ------- Durations ------ //
#define HIGHLIGHT_TIMEOUT 20000       // Highlight disappears after 20 seconds.  Timer resets whenever encoder turned or button pushed
#define FACTORY_RESET_TIMEOUT 3000    // 
#define PULSE_DURATION 20
#define PULSE_DURATION_MIN 10
#define PULSE_DURATION_MAX 200 // uint so only 255 max
#define MOD_DURATION_MIN 0      // 
#define MOD_DURATION_MAX 5000 // 5s




#endif
