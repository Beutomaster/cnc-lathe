#include <Arduino.h>
#include "CNC_Lathe.h"
#include "CNC_Control.h"
#include "Motion_Control.h"

//global vars
volatile char command_completed=1; //0=command in progress, 1=command_completed
volatile char pause=1; //0=programm running, 1=pause
struct cnc_code_block cnc_code[CNC_CODE_NMAX]; //Array of CNC-Code-Blocks, fixed length should be replaced


//functions
void programm_start(int N) { //start at block N
  STATE_N = N;
  pause=0;
  STATE &= ~(_BV(STATE_PAUSE_BIT)); //delete STATE_bit1 = STATE_PAUSE
}

void programm_pause() { //intermediate stop
  pause=1;
  STATE |= _BV(STATE_PAUSE_BIT); //set STATE_bit1 = STATE_PAUSE
}

void programm_stop() { //stop and jump back to block 0
  programm_pause();
  STATE_N=0;
}

void programm_abort() {
  programm_stop();
  //immediate stop of all engines needed!!!
}


char process_cnc_listing() {
	char success=0; //0=success, 1=failure
  if (command_completed && !pause) {    
    //next_cnc_code
    if (cnc_code[STATE_N].GM =='G') {
      switch(cnc_code[STATE_N].GM_NO) {
        case 0: G00(cnc_code[STATE_N].XI, cnc_code[STATE_N].ZK);
                break;
        case 1: G01(cnc_code[STATE_N].XI, cnc_code[STATE_N].ZK, cnc_code[STATE_N].FTLK);
                break;
        case 2: G02(cnc_code[STATE_N].XI, cnc_code[STATE_N].ZK, cnc_code[STATE_N].FTLK);
                break;
        case 3: G03(cnc_code[STATE_N].XI, cnc_code[STATE_N].ZK, cnc_code[STATE_N].FTLK);
                break;
        case 4: G04(cnc_code[STATE_N].XI);
                break;
        case 20: G20();
                break;
        case 21: G21();
                break;
        case 22: G22();
                break;
        case 24: G24();
                break; 
        default:  //Error "G-Code unkown"
                  success=1;       
      }
    } else if (cnc_code[STATE_N].GM =='M' ) {
      switch(cnc_code[STATE_N].GM_NO) {
        default:  //Error "M-Code unkown"
                  success=1;       
      }
    } else success=1; //Error "Code Type unkown"   
	}
 
	return success;
}


//G-/M-/S-/T-Codes:

//Rapid traverse
void G00(int X, int Z) {
  
}

//Linear interpolation
void G01(int X, int Z, int F){
  set_xz_move(X, Z, F, INTERPOLATION_LINEAR); 
}

//Circular interpolation Clockwise
void G02(int X, int Z, int F){
   set_xz_move(X, Z, F, INTERPOLATION_CIRCULAR_CLOCKWISE);
}

//Circular interpolation Counter clockwise
void G03(int X, int Z, int F){
  set_xz_move(X, Z, F, INTERPOLATION_CIRCULAR_COUNTERCLOCKWISE); 
}

//Dwell (X = Dwell)
void G04(int X) {
  command_running(X);
}

//Intermediate Stop (obsolete, backward compatibility for M00)
void G20() {M00();}

//Empty Line
void G21() {}

//End of program (obsolete, backward compatibility for M30)
void G22() {M30();}

void G24() {} //Radius programing
void G25(int L) {} //Sub-routine call-up (L = Jump address)
void G26(int X, int Z, char T) {} //Tool correction and tool call-up (obsolete, backward compatibility for M06)

//Jump instruction
void G27(int L) {STATE_N = L;}

void G33(int Z, char K) {} //Threading with constant pitch (K = Thread Pitch)
void G64() {} //Feed motors currentless
//void G65() {} //Cassette operation (obsolete)
//void G66() {} //RS 232 operation (obsolete)
void G73(int Z, int F) {} //Chip breakage cycle
void G78(int X, int Z, char K, int H) {} //Threading cycle
void G81(int Z, int F) {} //Drilling cycle
void G82(int Z, int F) {} //Drilling cycle with dwell
void G83(int Z, int F) {} //Drilling cycle, deep hole with withdrawal
void G84(int X, int Z, int F, int H) {} //Longitudinal turning
void G85(int Z, int F) {} //Reaming cycle
void G86(int X, int Z, int F, int H) {} //Grooving with division of cut
void G88(int X, int Z, int F, int H) {} //Facing with division of cut
void G89(int Z, int F) {} //Reaming and drilling with dwell
void G90() {incremental=1;} //Absolute value programing
void G91() {incremental=0;} //Incremental value programing
void G92(int X, int Z) {set_xz_coordinates(X, Z);} //Set register (zero point offset)
void G94() {} //Feed in mm/min
void G95() {} //Feed in mm/rev.
void G96() {} //new: set cutting speed in m/min (increasing revolutions)
void G97(int S) {} //new: set const. revolutions in 1/min
void G196(int S) {} //new: set max. rev. in 1/min for G96

//Programmed stop
void M00() {programm_pause();};

void M03() {} //Main spindle ON. right hand direction run
void M05() {} //Main Spindle OFF
void M06(int X, int Z, char T) {} //Tool length compensation (T = Tool address)
void M17() {} //return command to the main program

//End of Program
void M30() {programm_stop();}

void M98(int X, int Z) {} //Automatic compensation of play
void M99(int I, char K) {} //Circle parameter (I, K = Center point coordinates)

//Timer-Interrupt-isr:
void command_complete() {
  command_completed=1;
}

