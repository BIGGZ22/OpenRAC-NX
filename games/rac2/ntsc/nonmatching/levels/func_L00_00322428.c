/* 0 of 27 places match - last line: 00_aranos_tutorial: 268 bytes, expected 288
   Every constant and global is identified (1.0f, 1/60, 1/3600, 1/216000, 5, 1.2f, 1.44f,
   5/6, 0.0004f, 0.02f, the 1-ulp pair 0x3CA3D70A/0x3CA3D70B, 8e-6; the addresses
   0x1A790C..0x1A7B90 and the two  slots at 0x1A7918/0x1A7928), and the two-name sda
   split works, but this cc1 lays the a0 == 0 block out as the branch target and
   materialises the constants up front: 5 instructions short. */

extern float D_001A790C;
extern float D_001A7910;
extern float D_001A7914;
extern float D_001A7918;
extern float D_001A791C;
extern float D_001A7920;
extern int D_001A7924;
extern float D_001A7928;
extern int D_001A7B90;
extern float D_L00_001A7918 __attribute__((sda));
extern float D_L00_001A7928 __attribute__((sda));

void func_L00_00322428(int a0) {
 float one=1.0f;
 int v=1;
 if(a0!=0) {
  float f6=1.2f;
  float f4=1.2f*1.2f;
  float g0=5.0f/6.0f;
  float g1=0.0004f*50.0f;
  float g2=0.0004f;
  float g3=0.0004f/50.0f;
  float g5=0.02f;
  D_001A7B90=v;
  D_001A790C=f6;
  D_001A7910=f4;
  D_001A7914=g0;
  D_001A7918=g1;
  D_001A791C=g2;
  D_001A7920=g3;
  D_001A7924=6;
  D_L00_001A7928=g5;
  return;
 }
 D_001A7914=one;
 D_001A791C=1.0f/3600.0f;
 D_001A7920=1.0f/216000.0f;
 D_001A7924=5;
 D_001A7928=1.0f/60.0f;
 D_001A7B90=0;
 D_001A790C=one;
 D_001A7910=one;
 D_L00_001A7918=1.0f/60.0f;
}