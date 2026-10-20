/* 0 of 27 places match - last line: 00_aranos_tutorial: instruction at +0x20 differs
   Right size (68), one instruction apart: retail accumulates into the base register
   (addu $v0,$v0,$v1) and this cc1 puts the scaled index first (addu $v0,$v1,$v0);
   every following instruction is identical because the destination register agrees.
   The dead mult $v1,$a0 at +0x1c IS retail too - this compiler keeps the scaled
   index in the multiplier register, so the product is never moved out of HI/LO.
   [idx], flat, byte-offset and in-place  += i spellings all give the same order
   (the in-place form is the closest: only the two source operands of one addu swap). */

typedef unsigned char u8;

typedef struct { u8 pad[8]; void (*f)(void *); u8 rest[8]; } Ent;

extern Ent D_002A3A00[];

void func_L00_002D7D58(void *a0) {
 void *a1=a0;
 Ent *e=D_002A3A00;
 e+=*(short *)((u8 *)a1+0x8C);
 if(e->f) e->f(a1);
}