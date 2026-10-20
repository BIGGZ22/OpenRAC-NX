/* 0 of 27 places match - last line: 00_aranos_tutorial: 32 bytes, expected 44
   Retail materialises the block base into $at (lui + addiu, the +0x1C left as the
   load's literal offset), loads the mask 0x60 with ADDI into $v1 and ANDs through it,
   and keeps an explicit nop in the first jr's delay slot; here gcc loads through the
   destination register (lui $v0 / lw $v0,0x1C($v0)), folds the mask into ANDI and
   fills the delay slot with the shift. Flat, base-pointer and named-symbol spellings
   all agree; s64/unsigned forms change the shift to dsrl, which retail does not use.
   Needs a lever that keeps the address in $at (a scratch register) and the mask in a
   register. */

typedef int s32;

extern unsigned char D_001BF200[];

s32 func_L00_002E2048(void) {
 s32 v=*(s32 *)(D_001BF200+0x1C);
 if(v<0) return -1;
 return (v&0x60)>>5;
}