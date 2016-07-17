#ifndef CNC_Control_h 
#define CNC_Control_h

//includes
#include <Arduino.h>
#include "CNC_Lathe.h"
#include "Motion_Control.h"

//defines
#define CNC_CODE_NMAX 500

struct cnc_code_block { //not finished, Format von Siemens NX???
   char GM; //G or M-Code
   char GM_NO; //G/M-Code-Number
   int XI; //X/I-Parameter
   int ZK; //Z/K-Parameter (K for M99)
   int FTLK; //F/T/L/K-Parameter (K for G33 and G78)
   int HS; //H/S-Parameter
};

//global vars
extern volatile boolean command_completed;
extern volatile boolean pause;
extern struct cnc_code_block cnc_code[CNC_CODE_NMAX]; //Array of CNC-Code-Blocks, fixed length should be replaced

//functions
void programm_start(int);
void programm_pause();
void programm_stop();
void programm_abort();
boolean process_cnc_listing();

//G-/M-/S-/T-Codes:
void G00(int, int); //Rapid traverse
void G01(int, int, int); //Linear interpolation
void G02(int, int, int); //Circular interpolation Clockwise
void G03(int, int, int); //Circular interpolation Counter clockwise
void G04(int); //Dwell (X = Dwell)
void G20(); //Intermediate Stop (obsolete, backward compatibility for M00)
void G21(); //Empty Line
void G22(); //End of program (obsolete, backward compatibility for M30)
void G24(); //Radius programing
void G25(int); //Sub-routine call-up (L = Jump address)
void G26(int, int Z, char T); //Tool correction and tool call-up (obsolete, backward compatibility for M06)
void G27(int); //Jump instruction
void G33(int, char); //Threading with constant pitch (K = Thread Pitch)
void G64(); //Feed motors currentless
//void G65(); //Cassette operation (obsolete)
//void G66(); //RS 232 operation (obsolete)
void G73(int, int); //Chip breakage cycle
void G78(int, int, char, int); //Threading cycle
void G81(int, int); //Drilling cycle
void G82(int, int); //Drilling cycle with dwell
void G83(int, int); //Drilling cycle, deep hole with withdrawal
void G84(int, int, int, int); //Longitudinal turning
void G85(int, int); //Reaming cycle
void G86(int, int, int, int); //Grooving with division of cut
void G88(int, int, int, int); //Facing with division of cut
void G89(int, int); //Reaming and drilling with dwell
void G90(); //Absolute value programing
void G91(); //Incremental value programing
void G92(int, int); //Set register (zero point offset)
void G94(); //Feed in mm/min
void G95(); //Feed in mm/rev.
void G96(); //new: set cutting speed in m/min (increasing revolutions)
void G97(int); //new: set const. revolutions in 1/min
void G196(int); //new: set max. rev. in 1/min for G96
void M00(); //Programmed stop
void M03(); //Main spindle ON. right hand direction run
void M05(); //Main Spindle OFF
void M06(int, int, char); //Tool length compensation (T = Tool address)
void M17(); //return command to the main program
void M30(); //End of Program
void M98(int, int); //Automatic compensation of play
void M99(int, char); //Circle parameter (I, K = Center point coordinates)

//Timer-Interrupt-isr:
void command_complete();

#endif

