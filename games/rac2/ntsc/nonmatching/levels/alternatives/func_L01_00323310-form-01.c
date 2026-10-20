/* Alternative source form for func_L01_00323310.
   Retained from 323310_a.c, entry func_L01_00323310; declarations preserved.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
typedef unsigned char u8;
extern u8 D_001A63A8[];

void func_L01_00323310(int a0,int a1) {
 u8 *b=D_001A63A8;
 int v;
 if(*(short *)(b+0x48)==a0&&*(short *)(b+0x3C)==-1) return;
 v=a0;
 if(v!=-1) v&=0xFF;
 if(a1!=-1) v&=0xFF;
 if(*(short *)(b+0x4E)!=0||*(short *)(b+0x96)!=0) {
  *(short *)(b+0x3E)=a1;
  *(short *)(b+0x3C)=v;
  return;
 }
 *(short *)(b+0x48)=v;
}
