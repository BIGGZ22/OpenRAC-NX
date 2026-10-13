/* functional: high. The NTSC decomp's matched C for the same instructions (FUN_L14_002ed530), carried to PAL's
   names by OpenRAC's tools/port.py. Not a PAL match: PAL's compiler gives our 1756 bytes for retail's 1768 bytes. */
typedef u32 u128_EE990 __attribute__((mode(TI), aligned(16)));
typedef union {
    u128_EE990 q;
    f32 f[4];
    s32 i[4];
} Vec4_EE990;
extern f32 D_0015EE60 MACRO_ADDR;
extern float func_L00_00258C80(float, float);
extern float D_L14_00161D90[] MACRO_ADDR;
extern int D_L14_00161D84;
extern short D_L14_00161D28;
extern short D_L14_00161D38;
extern short D_L14_00161D3C;
extern short D_L14_00161D40_EE990 __asm__("D_L14_00161D40");
extern short D_L14_00161D44;
extern short D_L14_00161D5C;
extern short D_L14_00161D60;
extern short D_L14_00161D64;
extern short D_L14_00161D68;
extern short D_L14_00161D6C;
extern u8 D_L14_00161DB0_EE990 __asm__("D_L14_00161DB0") MACRO_ADDR;
extern s16 D_L14_00161DB2 MACRO_ADDR;
extern f32 D_L14_00161DB4 MACRO_ADDR;
extern char * D_L14_00161DB8 MACRO_ADDR;
extern s32 D_L14_00161DBC MACRO_ADDR;
extern f32 D_L14_00161DC0 MACRO_ADDR;
extern s32 D_L14_00161DC4 MACRO_ADDR;
extern s32 D_L14_00161DC8[] MACRO_ADDR;
extern f32 D_L14_001E00D0_EE990[40][4] __asm__("D_L14_001E00D0");
extern u16 D_L14_001E0350_EE990[40] __asm__("D_L14_001E0350");
extern f32 D_L14_001E03A0_EE990[3][16][4] __asm__("D_L14_001E03A0");
extern u16 D_L14_001E06A0_EE990[3][16] __asm__("D_L14_001E06A0");
extern void func_001F9BF0(void *, void *, void *);
extern float func_001F9CB8(void *);
extern float func_001FA888(int);
extern int func_001FA898(float);
extern int func_001F9850(int);
extern void func_001F9C08(void *, void *, void *, float);
extern int func_L00_00258BC8(int, int);
extern float func_002140F8(float, float);
extern int func_002140B0(int);
extern float func_001F9F90(float);
extern float func_001F9FA8(float);
extern float func_001FA748(float, float);
extern float func_001F9B88(float);
extern int func_001F9908(void *);
extern void func_0022EEB8(int, int, void *);
extern void func_001F49B0_EE990(void (*)(char *), char *) __asm__("func_001F49B0");
void func_L14_002EFB10(float *pos, int n);
void func_L14_002EF078();
extern float D_L14_001E0330[];
extern float D_L14_00161DA0[] MACRO_ADDR;

/* Lightning arc between the two camera points: builds the bolt and its three branches once, then ages and fades them.
   Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/overlays/l14/gameplay/entities/002df080.c, FUN_L14_002ed530. */
void func_L14_002EE990(char *moby) {
    Vec4_EE990 delta;
    float len;
    float t;
    float sign;
    float ang;
    float r;
    int i;
    int j;
    int n;
    float *p;
    s32 *starts;
    u16 *e;

    if (*(int *)&D_L14_00161D28 == 0) {
        *(int *)&D_L14_00161D28 = 1;
        D_L14_00161DC4 = 1;
        D_L14_00161DB8 = moby;
        i = 0;
        starts = D_L14_00161DC8;
        *(int *)&(*(f32 *)&D_L14_00161D3C) = 0;
        func_001F9BF0(&delta, D_L14_00161DA0, D_L14_00161D90);
        len = func_001F9CB8(&delta);
        (*(f32 *)&D_L14_00161D44) = len / func_001FA888(40);
        D_L14_00161DB0_EE990 = 0;
        for (; i < 40; i++) {
            e = &D_L14_001E0350_EE990[i];
            p = D_L14_001E00D0_EE990[i];
            p[1] = p[2] = 0.0f;
            p[3] = 1.0f;
            p[0] = (*(f32 *)&D_L14_00161D44) * (float)i;
            if (i < 39) {
                p[2] = p[-2] + func_L00_00258C80((*(f32 *)&D_L14_00161D5C), (*(f32 *)&D_L14_00161D60));
                if ((*(f32 *)&D_L14_00161D64) < (*&p)[2]) {
                    p[2] = (*(f32 *)&D_L14_00161D64);
                } else if (p[2] < -(*(f32 *)&D_L14_00161D64)) {
                    p[2] = -(*(f32 *)&D_L14_00161D64);
                }
            }
            t = func_001FA888(i) / func_001FA888(40);
            *e = func_001FA898((float)-func_001F9850((*(s32 *)&D_L14_00161D40_EE990)) * t + 0.0f);
        }
        func_001F9C08(D_L14_001E0330, D_L14_001E0330 - 4, D_L14_001E0330 + 4, 0.5f);
        sign = -1.0f;
        D_L14_00161D84 = D_L14_00161DBC = func_001F9850(func_L00_00258BC8((*(s32 *)&D_L14_00161D68), (*(s32 *)&D_L14_00161D6C)));
        D_L14_00161DC0 = 1.0f / func_001FA888(D_L14_00161D84 - func_001F9850((*(s32 *)&D_L14_00161D40_EE990)));
        if (func_002140B0(2) != 0) {
            sign = 1.0f;
        }
        for (j = 0; j < 3; j++) {
            int k = func_L00_00258BC8(5, 22);
            int m;
            float (*row)[4] = D_L14_001E03A0_EE990[j];
            u16 *sq = D_L14_001E06A0_EE990[j];
            starts[j] = k;
            sq[0] = D_L14_001E0350_EE990[k];
            row[0][0] = 0.0f;
            row[0][2] = 0.0f;
            r = func_002140F8(1.0f, 10.0f);
            ang = func_002140F8(0.5235988f, 1.3962634f) * sign;
            row[0][0] = r * func_001F9F90(ang);
            row[0][2] = r * func_001F9FA8(ang);
            for (m = 1; m < 16; m++) {
                float d;
                float na;
                r = func_002140F8(1.0f, 10.0f);
                d = func_L00_00258C80(0.0f, 40.0f) * 0.017453292f;
                na = func_001FA748(ang, d);
                if (func_001F9B88(na) > 1.3962634f || (ang > 0.0f && na < 0.0f) || (ang < 0.0f && na > 0.0f)) {
                    ang = func_001FA748(ang, -d);
                } else {
                    ang = na;
                }
                D_L14_001E03A0_EE990[j][m][0] = r * func_001F9F90(ang);
                D_L14_001E03A0_EE990[j][m][2] = r * func_001F9FA8(ang);
                D_L14_001E06A0_EE990[j][m] = D_L14_001E0350_EE990[starts[j] + m];
            }
            sign = -sign;
        }
    }
    if (moby == D_L14_00161DB8) {
        func_001F9908(&D_L14_00161D84);
        for (i = 0; i < 40; i++) {
            s16 v = ++D_L14_001E0350_EE990[i];
            if (func_001F9850((*(s32 *)&D_L14_00161D40_EE990)) < v) {
                D_L14_001E0350_EE990[i] = func_001F9850((*(s32 *)&D_L14_00161D40_EE990));
            }
            if (i != 0) {
                if (i >= 35) {
                    float k5 = 5.0f;
                    float *pp = D_L14_001E00D0_EE990[i];
                    pp[2] = pp[2] + (0.0f - pp[2]) * (func_001FA888(i - 35) / k5);
                }
            }
        }
        func_001F9850((*(s32 *)&D_L14_00161D40_EE990));
        for (j = 0; j < 3; j++) {
            for (n = 0; n < 16; n++) {
                s16 v = ++D_L14_001E06A0_EE990[j][n];
                if (func_001F9850((*(s32 *)&D_L14_00161D40_EE990)) < v) {
                    D_L14_001E06A0_EE990[j][n] = func_001F9850((*(s32 *)&D_L14_00161D40_EE990));
                }
            }
        }
        if (D_L14_00161D84 < D_L14_00161DBC - func_001F9850((*(s32 *)&D_L14_00161D40_EE990))) {
            if (func_001F9850(60) < D_L14_00161DBC) {
                if ((*(f32 *)&D_L14_00161D3C) < 6.2831855f) {
                    D_L14_00161DB2 = func_001FA898(func_001F9FA8(D_L14_00161DB4) * 128.0f);
                    D_L14_00161DB4 = func_001FA748(D_L14_00161DB4, (*(f32 *)&D_L14_00161D38));
                    (*(f32 *)&D_L14_00161D3C) += (*(f32 *)&D_L14_00161D38) * D_0015EE60;
                } else {
                    D_L14_00161DB2 = 0;
                }
            } else {
                D_L14_00161DB2 = 0;
            }
            D_L14_00161DB0_EE990 = (u8)D_L14_00161DB2 + func_001FA898(func_001FA888(D_L14_00161D84) * D_L14_00161DC0 * -255.0f + 255.0f);
            if (D_L14_00161DC4 != 0) {
                func_0022EEB8(0, 0, moby);
                D_L14_00161DC4 = 0;
            }
            if (D_L14_00161DB0_EE990 < 50) {
                func_L14_002EFB10(D_L14_00161D90, 3);
            }
        } else {
            D_L14_00161DB0_EE990 = 0;
        }
        func_001F49B0_EE990(func_L14_002EF078, moby);
    }
}
