/* 0 of 27 places match - last line: 00_aranos_tutorial: instruction at +0x74 differs
   35 of 37 instructions identical: right size (148), $gp read through the sda symbol,
   bnez with the tick load in its delay slot, both calls and the 0x1e stamp all exact.
   This cc1 only swaps the two closing ADDU: it builds ((tick + second) + (first + 0x14))
   where retail builds ((tick + (first + 0x14)) + second). Every spelling tried (nested
   parentheses, results in variables, += chains, second term first, reversed comparison)
   either keeps that association or loses an instruction.
   This unit must be compiled -G8 for the sda attribute to bite: a sibling
   <unit>-g8.c source restores it (see src/levels/shared/L00_00316F60-g8.c), or a
   -O2 -G8 -ffunction-sections line in config/cflags.txt. Under -G8 a plain
   `extern int` still comes out absolute, which is what the two stamped reads need. */

typedef unsigned char u8;

typedef int s32;

extern u8 D_00189E20[];

extern int D_001B19C8 __attribute__((sda));

extern int D_001B19C8_1;

extern int func_002A77E0(int);

void func_L00_002A5480(void) {
 u8 *b=D_00189E20;
 if(*(int *)(b+0x1180)!=0) return;
 if(*(int *)(b+0x1188)==0) {
  *(int *)(b+0x1184)=D_001B19C8_1+0x1e;
  return;
 }
 if(D_001B19C8>*(int *)(b+0x1184)) {
  int t;
  int t2;
  *(int *)(b+0x1180)=1;
  t=func_002A77E0(*(int *)(b+0x1188));
  t2=func_002A77E0(*(int *)(b+0x1188));
  *(int *)(b+0x1184)=D_001B19C8_1+(t+0x14)+t2;
 }
}
