/* func_L01_002F14E8, 156 bytes, 5 places.  ABANDON - same $gp wall as func_L01_00367F78.
   Read in full: arms a timer (func_3022C8(0x002F1888, 0x002F1588, 0) when the gp word is
   zero), sets it to 5, stores the clamped incoming float in a gp float and the two
   arguments in two gp words. With _gp = 0x001AEFF0 the four slots are 0x001A90C8 (int, test
   and 5), 0x001A90C4 (float, clamped to [0,1]), 0x001A90D0 (a1) and 0x001A90CC (a0) - the
   retail stores in that order, with the unclamped store first and the 0 and 1 arms sharing
   the following stores. The candidate is 176 bytes because all four come out as absolute
   lui/lw pairs (20 bytes over) where retail reads and writes them `-0x5F28($gp)` etc.
   Tried: `__attribute__((sda))`, `__attribute__((section(".sdata")))`, and both in a -G8
   unit; every one of them still emitted the absolute pair, so the blocker is the GPREL
   decision, not the body - the body's own shapes are all measured. */

#include "include_asm.h"
