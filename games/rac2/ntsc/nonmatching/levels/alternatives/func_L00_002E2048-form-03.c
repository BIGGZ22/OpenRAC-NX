/* Alternative source form for func_L00_002E2048.
   Retained from 2E2048_d.c, entry func_L00_002E2048; declarations preserved.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
typedef int s32;
extern unsigned char D_001BF200[];

s32 func_L00_002E2048(void) {
 s32 v;
 if(*(s32 *)(D_001BF200+0x1C)<0) return -1;
 v=*(s32 *)(D_001BF200+0x1C);
 return (v&0x60)>>5;
}
