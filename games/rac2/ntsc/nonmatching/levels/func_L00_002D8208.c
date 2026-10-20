/* 0 of 27 places match - same shape as func_L00_002D7D58 (same file, the loaded field
   is +0x10 instead of +0x8); the draft below is the twin of the recorded candidate and
   was not separately re-measured. One addu operand order. */

typedef unsigned char u8;

typedef struct { u8 pad[0x10]; void (*f)(void *); u8 rest[0]; } Ent;

extern Ent D_002A3A00[];

void func_L00_002D8208(void *a0) {
 void *a1=a0;
 Ent *e=D_002A3A00;
 e+=*(short *)((u8 *)a1+0x8C);
 if(e->f) e->f(a1);
}