#ifndef _VARIABLES_H_
#define _VARIABLES_H_

// ------ Hardware ------ //

// Define the MCP4822 instance and proivide the CS pin
MCP4822 dac(DAC_CS);

// Define the display instance and provide the pins etc
SSD1306AsciiAvrI2c display;

// Need to edit midi_Settings.h and reduce SysEx buffer to 32 --- i.e.  static const unsigned SysExMaxSize = 32;
MIDI_CREATE_DEFAULT_INSTANCE();

// Encoder
uint8_t encoderPos, encoderPosPrev;
Bounce encButton = Bounce(); 

// ------ Sequencer Structures and Globals ------ //

struct Seq_t {
  uint8_t note[STEP_LEN]   = {36,36,36,36,34,34,34,34,36,36,36,34,34,36,36,34};
  //uint8_t note[STEP_LEN]   = {1,2,3,4,5,6,7,8,127,10,11,12,13,14,15,16};
  uint8_t length[STEP_LEN] = {6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6};
  uint8_t accent[STEP_LEN] = {0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,0};
  uint8_t rest[STEP_LEN]   = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  uint8_t slide[STEP_LEN]  = {1,0,1,0,1,1,1,0,1,1,0,0,0,1,0,0};
  slide_t slide_type = FULL;          // the slide type for this sequence
};

Seq_t activeSeq;                      // active seq that will play
uint8_t seq_stored[NUM_SEQ+1];        // array of what sequences are stored indexed from 1, iniited via EEPROMinit()
uint8_t activeSlot = 0;               // the last active slot (persists over reboot)
uint8_t step_i = STEP_START;          // current step in playing activeSeq
uint8_t step_length = 6;              // current step length
uint8_t cv_step_length = 1;           // the cv step length based on cv ppqn
uint8_t shiftNumber = 0;              // the number of steps to shift in menu

// ------ Sliding Variables ------ //


float slideVoltStart = 0.0f;             // sliding from this voltage to...
float slideVoltEnd = 0.0f;               // ...this voltage
float delta = 0.0f;                   // the delta mv between start and end slide
uint8_t slide_div = 1;                // is slide possible - calculate divider as needed



// ------ MIDI Variables ------ //

bool notes[88] = { 0 };     
uint8_t noteOrder[5] = { 0 }, order_i = { 0 };
unsigned long trigTimer = 0;                      // Needs to be global as used outside loop

// MIDI settings - set by editor, by menu and stored in eeprom
uint8_t midiChannel = 10; // default is 0, but need a non zero here or gets optimised out of loop
uint8_t notePriority = 0;
uint8_t midiCCnumber = 1; // default to mod wheel
uint8_t midiStartEnable = 1; // default to enabled
uint8_t midiCCaccent = 80; // default CC for accent input

// CV settings - set by editor, by menu and stored in eeprom
uint8_t       cvClockPPQN = 4;        // CV Clock In PPQN
uint8_t       cvClockOutPPQN = 1;     // CV Clock Out PPQN
float         sfAdj = 1.0f;           // Adjustment for scaling of v/oct
unsigned long gateDelay = 500;        // Adjustment for delay after gate off when tick_per_step = 1
unsigned long modDuration = 0;        // Timeout for mod CV to turn off (0 = stay on)
uint8_t       trigDuration = 20;      // Timeout for trigger CV to turn off
uint8_t       clockDuration = 20;     // Timeour for clock CV out to turn off

uint8_t tick_per_step = 1;            // how many ticks do we have per step - based on CV Clock In PPQN

// ------ Menu Variables ------ //
//bool highlightEnabled = 0;            
unsigned long int highlightTimer = 0;
unsigned long int encoderTimer = 0;


// globals where text used in menu, but encoded to numbers in eeprom
uint8_t activeSlotMenu;
uint8_t clearSlotMenu;
uint8_t saveSlotMenu;
char priorityMenu[4];
char fresetMenu[3];

// ------ Packed global booleans - bitmap ------ //
volatile uint8_t bitmap = 0b00000010;
volatile uint8_t bitmap2 = 0b00000000;

inline bool   is_true( uint8_t *bitmap, uint8_t bit ) { return( *bitmap & ( 1 << bit )); }
inline void  set_true( uint8_t *bitmap, uint8_t bit ) { *bitmap |= ( 1 << bit ); }
inline void set_false( uint8_t *bitmap, uint8_t bit ) { *bitmap &= ~( 1 << bit ); }

// ------ Inline helper functions ------ //

inline int mod( int a, int b ) {
  int r = a % b;
  return r < 0 ? r + b : r;
}

inline uint8_t my_div( uint8_t a, uint8_t b ) {
  div_t d = div( a, b );
  return( d.quot );
}

// call the reset function to reboot
inline void( *resetFunc ) ( void ) = 0;

// Set the menu text for the note priority
inline void setNotePriorityText( void ) {
  if( notePriority == NP_LAST ) { strcpy_P( priorityMenu, PSTR( "Last" ) ); }
  else if( notePriority == NP_LOW ) { strcpy_P( priorityMenu, PSTR( "Botm" ) ); }
  else { strcpy_P( priorityMenu, PSTR( "Top" ) ); }
}

#endif
