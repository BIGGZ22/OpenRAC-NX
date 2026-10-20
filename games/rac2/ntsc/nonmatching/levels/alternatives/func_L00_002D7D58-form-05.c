/* Alternative source form for func_L00_002D7D58.
   Retained from 2D7D58_f.c, entry func_L00_002D7D58; declarations preserved.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
typedef unsigned char u8;
typedef struct { u8 pad[8]; void (*f)(void *); u8 rest[8]; } Ent;
extern Ent D_002A3A00[];

void func_L00_002D7D58(void *a0) {
 void *a1=a0;
 int off=*(short *)((u8 *)a1+0x8C)*20;
 Ent *e=(Ent *)((u8 *)D_002A3A00+off);
 if(e->f) e->f(a1);
}
