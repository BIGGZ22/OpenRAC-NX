/* 0 of 27 places match - last line: 00_aranos_tutorial: instruction at +0x3c differs
   Right size (164) and every constant is exact (1.1243411f, 0.56217057f, 0.005f, 0.2f and
   the divide feeding the 0x1BD370 stamp); only two independent stores are swapped: retail
   emits swc1 0x3D8, sb 0x3E9, then the 0x3D4 store in the first call's delay slot, this
   cc1 emits both stores first and pushes the sb into the delay slot. The three statement
   orders tried (0x3D8/sb/0x3D4, 0x3D4/0x3D8/sb, sb/0x3D8/0x3D4) all reorder. */

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
 *(float *)(b+0x3D4)=v;
 *(float *)(b+0x3D8)=v;
 b[0x3E9]=0;
 x=func_00283198(s);
 y=func_00283180(s);
 D_L00_001BD370=x/y;
 func_0027AB40();
 *(float *)(b+0x3DC)=0.005f;
 *(float *)(b+0x3E0)=0.2f;
 *(int *)(b+0x3E4)=0;
 *(int *)(b+0x3D0)=0;
}