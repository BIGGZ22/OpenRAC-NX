/* Alternative source form for func_L00_002D7D58.
   Retained from 2D7D58_c.c, entry func_L00_002D7D58; declarations preserved.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
typedef unsigned char u8;
typedef int s32;
extern u8 D_002A3A00[];

void func_L00_002D7D58(void *a0) {
 void *a1=a0;
 void (*f)(void *);
 f=*(void (**)(void *))((u8 *)D_002A3A00+(s32)*(short *)((u8 *)a1+0x8C)*20+8);
 if(f!=0) f(a1);
}
