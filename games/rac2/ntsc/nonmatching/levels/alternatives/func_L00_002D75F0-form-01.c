/* Alternative source form for func_L00_002D75F0.
   Retained from 2D75F0_a.c, entry func_L00_002D75F0; declarations preserved.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
typedef unsigned char u8;
extern u8 D_L00_001C8C80[];
extern float D_L00_001BD370;
extern float func_00283198(float);
extern float func_00283180(float);
extern void func_0027AB40(void);

void func_L00_002D75F0(void) {
 u8 *b=D_L00_001C8C80;
 float v=1.1243411f;
 float s=0.56217057f;
 float x,y;
 *(float *)(b+0x3D8)=v;
 b[0x3E9]=0;
 *(float *)(b+0x3D4)=v;
 x=func_00283198(s);
 y=func_00283180(s);
 D_L00_001BD370=x/y;
 func_0027AB40();
 *(float *)(b+0x3DC)=0.005f;
 *(float *)(b+0x3E0)=0.2f;
 *(int *)(b+0x3E4)=0;
 *(int *)(b+0x3D0)=0;
}
