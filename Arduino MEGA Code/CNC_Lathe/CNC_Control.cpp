#include "CNC_Control.h"

//global vars
volatile boolean command_completed=1, x_command_completed=1, z_command_completed=1; //0=command in progress, 1=command_completed
volatile boolean pause=1; //0=programm running, 1=pause
struct cnc_code_block cnc_code[CNC_CODE_NMAX+1]; //Array of CNC-Code-Blocks, fixed length should be replaced
int jumpback_N = CNC_CODE_NMAX+1; //default Return address for CNC after array end
volatile int N_Offset=0, N_MAX = 0;


//functions
void programm_start(int N) { //start at block N
  STATE_N = N;
  STATE1 &= ~(_BV(STATE1_MANUAL_BIT)); //set STATE1_bit2 = 0
  pause=0;
  STATE1 &= ~(_BV(STATE1_PAUSE_BIT)); //delete STATE1_bit3 = STATE_PAUSE
  STATE2 &= ~(_BV(STATE2_CNC_CODE_NEEDED_BIT));
  if (!((STATE1>>STATE1_STEPPER_BIT)&1)) stepper_on();
}

void programm_pause() { //intermediate stop
  STATE1 |= _BV(STATE1_MANUAL_BIT); //set STATE1_bit2 = 1
  pause=1;
  STATE1 |= _BV(STATE1_PAUSE_BIT); //set STATE1_bit3 = STATE_PAUSE
}

void programm_stop() { //stop and jump back to block 0
  programm_pause();
  STATE_N=0;
  max_revolutions = REVOLUTIONS_MAX;
  absolute=0;
  feed_modus=0;
  interpolationmode=0;
}

void programm_abort() {
  programm_stop();
  //immediate stop of all engines needed!!!
  //disable all Timer
}

boolean process_cnc_listing() {
	boolean success=0; //0=success, 1=failure    
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
        case 25: G25(cnc_code[STATE_N].FTLK);
                break;
        case 26: G26(cnc_code[STATE_N].XI, cnc_code[STATE_N].ZK, cnc_code[STATE_N].FTLK);
                break;
        case 27: G27(cnc_code[STATE_N].FTLK);
                break;
        case 33: G33(cnc_code[STATE_N].ZK, cnc_code[STATE_N].FTLK);
                break;
        case 64: G64();
                break;
        case 73: G73(cnc_code[STATE_N].ZK, cnc_code[STATE_N].FTLK);
                break;
        case 78: G78(cnc_code[STATE_N].XI, cnc_code[STATE_N].ZK, cnc_code[STATE_N].FTLK, cnc_code[STATE_N].HS);
                break;
        case 81: G81(cnc_code[STATE_N].ZK, cnc_code[STATE_N].FTLK);
                break;
        case 82: G82(cnc_code[STATE_N].ZK, cnc_code[STATE_N].FTLK);
                break;
        case 83: G83(cnc_code[STATE_N].ZK, cnc_code[STATE_N].FTLK);
                break;
        case 84: G84(cnc_code[STATE_N].XI, cnc_code[STATE_N].ZK, cnc_code[STATE_N].FTLK, cnc_code[STATE_N].HS);
                break;
        case 85: G85(cnc_code[STATE_N].ZK, cnc_code[STATE_N].FTLK);
                break;
        case 86: G86(cnc_code[STATE_N].XI, cnc_code[STATE_N].ZK, cnc_code[STATE_N].FTLK, cnc_code[STATE_N].HS);
                break;
        case 88: G88(cnc_code[STATE_N].XI, cnc_code[STATE_N].ZK, cnc_code[STATE_N].FTLK, cnc_code[STATE_N].HS);
                break;
        case 89: G89(cnc_code[STATE_N].ZK, cnc_code[STATE_N].FTLK);
                break;
        case 90: G90();
                break;
        case 91: G91();
                break;
        case 92: G92(cnc_code[STATE_N].XI, cnc_code[STATE_N].ZK);
                break;
        case 94: G94();
                break;
        case 95: G95();
                break;
        case 96: G96();
                break;
        case 97: G97(cnc_code[STATE_N].HS);
                break;
        case 196: G196(cnc_code[STATE_N].HS);
                break;
        default:  //Error "G-Code unkown"
                  success=1;       
      }
    } else if (cnc_code[STATE_N].GM =='M' ) {
      switch(cnc_code[STATE_N].GM_NO) {
        case 0: M00();
                break;
        case 3: M03();
                break;
        case 4: M04();
                break;
        case 5: M05();
                break;
        case 6: M06(cnc_code[STATE_N].XI, cnc_code[STATE_N].ZK, cnc_code[STATE_N].FTLK);
                break;
        case 17: M17();
                break;
        case 30: M30();
                break;
        case 98: M98(cnc_code[STATE_N].XI, cnc_code[STATE_N].ZK);
                break;
        case 99: M99(cnc_code[STATE_N].XI, cnc_code[STATE_N].ZK);
                break;
        default:  //Error "M-Code unkown"
                  success=1;       
      }
    } else success=1; //Error "Code Type unkown" 
    //if (command_completed) STATE_N++; //else STATE_N++ in ISRs
	return success;
}


//G-/M-/S-/T-Codes:

//Rapid traverse
void G00(int X, int Z) {
  set_xz_move(X, Z, 499, RAPID_LINEAR_MOVEMENT);
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

//Sub-routine call-up (L = Jump address)
void G25(int L) {
  jumpback_N = STATE_N + N_Offset + 1; //next block
  STATE_N = L-1;
}

void G26(int X, int Z, byte T) { M06(X, Z, T);} //Tool correction and tool call-up (obsolete, backward compatibility for M06)

//Jump instruction
void G27(int L) {STATE_N = L - N_Offset - 1;}

void G33(int Z, int K) {} //Threading with constant pitch (K = Thread Pitch)
void G64() {stepper_off();} //Feed motors currentless
//void G65() {} //Cassette operation (obsolete)
//void G66() {} //RS 232 operation (obsolete)
void G73(int Z, int F) {} //Chip breakage cycle
void G78(int X, int Z, int K, int H) {} //Threading cycle
void G81(int Z, int F) {} //Drilling cycle
void G82(int Z, int F) {} //Drilling cycle with dwell
void G83(int Z, int F) {} //Drilling cycle, deep hole with withdrawal
void G84(int X, int Z, int F, int H) {} //Longitudinal turning
void G85(int Z, int F) {} //Reaming cycle
void G86(int X, int Z, int F, int H) {} //Grooving with division of cut
void G88(int X, int Z, int F, int H) {} //Facing with division of cut
void G89(int Z, int F) {} //Reaming and drilling with dwell
void G90() {absolute=1;} //Absolute value programing
void G91() {absolute=0;} //Incremental value programing
void G92(int X, int Z) {set_xz_coordinates(X, Z);} //Set register (zero point offset)
void G94() {feed_modus=FEED_IN_MM_PER_MIN;} //Feed in mm/min
void G95() {feed_modus=FEED_IN_MM_PER_REVOLUTION;} //Feed in mm/rev.
void G96() {feed_modus=FEED_IN_M_PER_MIN_AT_INCR_REVOLUTIONS;} //new: set cutting speed in m/min (increasing revolutions)
void G97(int S) {set_revolutions(S);} //new: set const. revolutions in 1/min

//new: set max. rev. in 1/min for G96
void G196(int S) {
  if (S>=0 && S<=REVOLUTIONS_MAX) max_revolutions=S;
  else ERROR_NO |= _BV(ERROR_CNC_CODE_BIT);
}

//Programmed stop
void M00() {programm_pause();};

//Main spindle ON. right hand direction run
void M03() {
  spindle_direction(LOW);
  spindle_on();
}

//Main spindle ON. left hand direction run
void M04() {
  spindle_direction(HIGH);
  spindle_on();
}

void M05() {spindle_off();} //Main Spindle OFF

//Tool length compensation (T = Tool address)
void M06(int X, int Z, byte T) {
  get_Tool_X(X);
  get_Tool_Z(Z);
  set_tool_position(T);
}
  
void M17() {STATE_N = jumpback_N-1;} //return command to the main program

//End of Program
void M30() {programm_stop();}

void M98(int X, int Z) {} //Automatic compensation of play
void M99(int I, int K) {} //Circle parameter (I, K = Center point coordinates)

