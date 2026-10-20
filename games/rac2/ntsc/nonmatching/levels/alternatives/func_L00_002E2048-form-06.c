/* Alternative source form for func_L00_002E2048.
   Retained from 2E2048_g.c, entry func_L00_002E2048; declarations preserved.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
typedef long s64;
extern int D_001BF21C;

s64 func_L00_002E2048(void) {
 s64 v=D_001BF21C;
 if(v<0) return -1;
 return (v&0x60)>>5;
}
