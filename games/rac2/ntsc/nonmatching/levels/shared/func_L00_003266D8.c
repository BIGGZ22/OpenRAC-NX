/* 0 of 27 places match - last line: 00_aranos_tutorial: 208 bytes, expected 212
   The whole logic is recovered (it is a three-stage countdown/reset state machine):
     if (D_001A8FB4 == 0) { n = D_001A8FB8 + 1; D_gp1 = n < 11 ? n : 10; }
     else D_001A8FB8 = 0;
     if (D_001A8FB0 != 0) { D_gp2 = D_001A8FB0 - 1; if (D_gp2 == 0) D_001A8FB4 = 0; }
     if (D_001A8FB4 == 0 || D_gp3 == 0) D_gp4 = -1;
     else if (D_gp4 == -1) D_gp4 = func_0028B3D0(12, 0, 0x002F7530, 0x002F7568,
                                                 0x002F7588, 0, 0);
     else func_L00_002F7298(10);
   Every branch, the delay-slot stores and the seven-argument call (5-7 in $t0-$t2) are
   byte-identical; the four $gp globals need the -g8 sibling, the three 0x001A8Fxx ones
   are ABSOLUTE (`nosda`).
   The 4 bytes are the clamp only: retail emits slti / bnez / sw(delay) / addiu / b / sw
   (six words, two stores), the ternary gives slti / movn (208, one word short) and the
   if/else gives an extra `beql` + `b` (216, one word long). Four spellings measured
   (`n < 11 ? n : 10`, if/else both polarities, the else arm carrying `n = 10;`). */

typedef int s32;

extern s32 D_001A8FB4;
extern s32 D_001A8FB8;
extern s32 D_001A8FB0;
extern s32 D_L00_00000001 __attribute__((sda));
extern s32 D_L00_00000002 __attribute__((sda));
extern s32 D_L00_00000003 __attribute__((sda));
extern s32 D_L00_00000004 __attribute__((sda));
extern unsigned char D_002F7530[];
extern unsigned char D_002F7568[];
extern unsigned char D_002F7588[];
extern s32 func_0028B3D0(s32, s32, void *, void *, void *, s32, s32);
extern void func_L00_002F7298(s32);

void func_L00_003266D8(void) {
    if (D_001A8FB4 == 0) {
        s32 n = D_001A8FB8 + 1;

        D_L00_00000001 = n < 11 ? n : 10;
    } else {
        D_001A8FB8 = 0;
    }
    if (D_001A8FB0 != 0) {
        D_L00_00000002 = D_001A8FB0 - 1;
        if (D_L00_00000002 == 0)
            D_001A8FB4 = 0;
    }
    if (D_001A8FB4 == 0 || D_L00_00000003 == 0) {
        D_L00_00000004 = -1;
    } else {
        if (D_L00_00000004 == -1)
            D_L00_00000004 = func_0028B3D0(12, 0, D_002F7530, D_002F7568,
                                           D_002F7588, 0, 0);
        else
            func_L00_002F7298(10);
    }
}
