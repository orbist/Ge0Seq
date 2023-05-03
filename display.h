#ifndef _SEQ_DISPLAY_H_
#define _SEQ_DISPLAY_H_

// ------ prototypes ------ //

// Show Ge0sync splash, pause 5s then clear screen
void displayInit( void );

// Clear screen, display header
void displayHdr( void );

// Show Test Comms result
void displayTestComms( bool match );

// Display the activeSeq information
void displayHomePage( void );

// Clear screen and display playing info
void displayMsgPlaying( bool cv_play );

// Display the settings button
void displaySettingsButton( void );

// Write DONE message and pause 2s, then clear screen
// if home is true, set displayCleared so we display home page
void displayDONE( bool home );

// Display About Info
void displayAbout( void );

#endif
