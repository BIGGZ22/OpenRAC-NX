/* Alternative source form for func_L00_002A5480.
   Retained from 5480_g8b.c, entry func_L00_002A5480; declarations preserved.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
typedef unsigned char u8;
typedef int s32;
extern u8 D_00189E20[];
extern int D_001B19C8;
extern int D_001B19C8_1 __attribute__((nosda));
extern int func_002A77E0(int);

void func_L00_002A5480(void) {
 u8 *b=D_00189E20;
 if(*(int *)(b+0x1180)!=0) return;
 if(*(int *)(b+0x1188)==0) {
  *(int *)(b+0x1184)=D_001B19C8_1+0x1e;
  return;
 }
 if(*(int *)(b+0x1184)<D_001B19C8) {
  int t;
  *(int *)(b+0x1180)=1;
  t=func_002A77E0(*(int *)(b+0x1188));
  *(int *)(b+0x1184)=D_001B19C8_1+(t+0x14)+func_002A77E0(*(int *)(b+0x1188));
 }
}
