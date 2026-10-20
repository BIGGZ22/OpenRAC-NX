/* 0 of 12 places match - last line: 00_aranos_tutorial: 80 bytes, expected 88
   Both conditions, the branch-likely into the shared body and the 0x1B constant in the
   beq delay slot are understood, but this cc1 if-converts the shared body: it duplicates
   the load and emits beql + xor/sltiu where retail keeps one plain beq, one bnel and a
   branchy body (bne/jr/addiu $v0,1). Short-circuit ||, if/return and ==-return forms all
   give the branchless body. What unblocks it: a spelling if-conversion refuses for the
   body block of a shared || (retail's body has two predecessors). */

typedef int s32;

typedef unsigned char u8;

extern u8 D_00189E20[];

s32 func_L00_00324120(s32 a0) {
 u8 *b=D_00189E20;
 if(*(s32 *)(b+0x229C)==3||*(s32 *)(b+0x2294)==0x1B) {
  if(*(s32 *)(b+0x588)==a0) return 1;
  return 0;
 }
 if(*(short *)(b+0x34E)!=0) return 0;
 if(*(s32 *)(b+0x33C)==a0) return 1;
 return 0;
}