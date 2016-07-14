#include "CNC_Lathe.h"
#include "CNC_Control.h"
#include "Motion_Control.h"

//global vars
volatile char command_completed=0;
volatile char pause=0;

//functions
void next_cnc_code() {
}

void programm_abort() {
}

char process_cnc_listing() {
	char success=0;
    if (command_completed && !pause) {
        next_cnc_code();
		success=1;
	}
	return success;
}


//G-/M-/S-/T-Codes:
void G00(int X, int Z); //Rapid traverse

//Linear interpolation
void G01(int X, int Z, int F){
  set_xz_move(X, Z, F, INTERPOLATION_LINEAR); 
}

//Circular interpolation Clockwise
void G02(int X, int Z, int F){
   set_xz_move(X, Z, F, INTERPOLATION_CIRCULAR);
}
void G03(int X, int Z, int F); //Circular interpolation Counter clockwise
void G04(int X); //Dwell (X = Dwell)
void G20(); //Intermediate Stop (obsolete, backward compatibility for M00)
void G21(); //Empty Line
void G22(); //End of program (obsolete, backward compatibility for M30)
void G24(); //Radius programing
void G25(int L); //Sub-routine call-up (L = Jump address)
void G26(int X, int Z, char T); //Tool correction and tool call-up (obsolete, backward compatibility for M06)
void G27(int L); //Jump instruction
void G33(int Z, char K); //Threading with constant pitch (K = Thread Pitch)
void G64(); //Feed motors currentless
//void G65(); //Cassette operation (obsolete)
//void G66(); //RS 232 operation (obsolete)
void G73(int Z, int F); //Chip breakage cycle
void G78(int X, int Z, char K, int H); //Threading cycle
void G81(int Z, int F); //Drilling cycle
void G82(int Z, int F); //Drilling cycle with dwell
void G83(int Z, int F); //Drilling cycle, deep hole with withdrawal
void G84(int X, int Z, int F, int H); //Longitudinal turning
void G85(int Z, int F); //Reaming cycle
void G86(int X, int Z, int F, int H); //Grooving with division of cut
void G88(int X, int Z, int F, int H); //Facing with division of cut
void G89(int Z, int F); //Reaming and drilling with dwell
void G90(); //Absolute value programing
void G91(); //Incremental value programing
void G92(int X, int Z); //Set register (zero point offset)
void G94(); //Feed in mm/min
void G95(); //Feed in mm/rev.
void G96(); //new: set cutting speed in m/min (increasing revolutions)
void G97(int S); //new: set const. revolutions in 1/min
void G196(int S); //new: set max. rev. in 1/min for G96
void M00(); //Programmed stop
void M03(); //Main spindle ON. right hand direction run
void M05(); //Main Spindle OFF
void M06(int X, int Z, char T); //Tool length compensation (T = Tool address)
void M17(); //return command to the main program
void M30(); //End of Program
void M98(int X, int Z); //Automatic compensation of play
void M99(int I, char K); //Circle parameter (I, K = Center point coordinates)

//Timer-Interrupt-isr:
void command_complete() {
}

