/* NON_MATCHING func_L14_002DFA70 -- src/overlays/shared/vendor_002B2A28.c
 * Best so far: SIZE ours 1108 / retail 1064, checked 2026-10-12.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Class 325 moby update (levels 14 and 15): clamps the data block, moves the moby's three coordinates, and on an
 *   Left: the position pointer is $s2 in ours and $s3 in retail (pos is the call argument copied through $a1); ret
 *   Unblock: the source's declaration order for the position pointer and the loop constants; the float order (f20 
 */
extern char D_0013E633[];
extern float D_0015EE60 MACRO_ADDR;
extern float D_0015EE64 MACRO_ADDR;
extern float D_L14_0015F660[] MACRO_ADDR;
extern short D_L14_00161BE8;
extern short D_L14_00161BEC;
extern short D_L14_00161BF0;
extern int func_002140B0(int);
extern void func_L01_0030D380(void);
extern void func_001F49B0(void (*)(void), void *);
extern void func_001F9BD8(void *, void *, void *);
extern float func_001F9B88(float);
extern float func_001FA748(float, float);
extern int func_001F9908(void *);
extern int func_L00_001F10E0(float, void *, int, void *);
extern void func_L00_0025F4A8(void *, void *, void *, float, float, int, int, int, float, float, float, float, int, float, int, int, int, int);
extern void func_0020D678(void *);
extern int func_001F9850(int);
extern int func_L00_00258BC8(int, int);
extern void func_L00_0026DA50(void *, void *, int, int, int, int, float);
extern float func_002140F8(float, float);

/* Class 325 moby update (levels 14 and 15): clamps the data block's vectors, moves the moby's three coordinates, and spawns effect objects when it is in range. */
void func_L14_002DFA70(char *moby)
{
    char *data;
    char *pos;
    char *dp;
    int i;
    int r1;
    int r2;
    int r3;
    int r4;
    int r5;
    int r6;
    int r7;
    int w;
    float f20;
    float f21;

    f21 = 0.0f;
    data = *(char **)(moby + 0x78);
    if (func_002140B0(2) != 0) {
        func_001F49B0(func_L01_0030D380, moby);
    }
    pos = moby + 0x10;
    func_001F9BD8(pos, pos, data);
    f20 = func_001F9B88(*(float *)data);
    if (*(float *)&D_L14_00161BF0 * D_0015EE60 < f20) {
        *(float *)data = *(float *)data * ((*(float *)&D_L14_00161BEC - 1.0f) * D_0015EE60 + 1.0f);
    }
    f20 = func_001F9B88(*(float *)(data + 4));
    if (*(float *)&D_L14_00161BF0 * D_0015EE60 < f20) {
        *(float *)(data + 4) = *(float *)(data + 4) * ((*(float *)&D_L14_00161BEC - 1.0f) * D_0015EE60 + 1.0f);
    }
    if (f21 < *(float *)(data + 8)) {
        *(float *)(data + 8) = *(float *)(data + 8) * ((*(float *)&D_L14_00161BEC - 1.0f) * D_0015EE60 + 1.0f);
    }
    *(float *)(data + 8) = *(float *)(data + 8) - *(float *)&D_L14_00161BE8 * D_0015EE64;
    *(float *)(moby + 0x40) = func_001FA748(*(float *)(moby + 0x40), *(float *)(data + 0x10));
    *(float *)(moby + 0x44) = func_001FA748(*(float *)(moby + 0x44), *(float *)(data + 0x14));
    *(float *)(moby + 0x48) = func_001FA748(*(float *)(moby + 0x48), *(float *)(data + 0x18));
    if (!(*(float *)(moby + 0x10) < 2.0f) && !(1022.0f < *(float *)(moby + 0x10))
        && !(*(float *)(moby + 0x14) < 2.0f) && !(1022.0f < *(float *)(moby + 0x14))
        && !(*(float *)(moby + 0x18) < 2.0f) && !(1022.0f < *(float *)(moby + 0x18))) {
        if (func_001F9908(data + 0x20) != 0
            || func_L00_001F10E0(0.5f, pos, 0, *(void **)(data + 0x24)) != 0) {
            f20 = 1.0f;
            func_L00_0025F4A8(moby, data, pos, f21, f21, 5, 2, 4, f20, 0.5f, 9.0f, f20, 0, 15.0f, 1, 5, -1, 0);
            func_L00_001F10E0(f20, pos, 0, *(void **)(D_0013E633 + 0x2E9D));
        } else {
                f21 = 1.0f;
                r1 = func_001F9850(10);
                r2 = func_001F9850(20);
                r3 = func_L00_00258BC8(r1, r2);
                func_L00_0026DA50(pos, D_L14_0015F660, 0x4F007FFF, 0x1FFFFFFF, r3, 1, 59936.0f);
                i = 1;
                do {
                    r4 = func_002140B0(6);
                    r5 = func_002140B0(2);
                    if (r5 != 0) r4 = -r4;
                    f20 = func_002140F8(40000.0f, 100000.0f);
                    r6 = func_L00_00258BC8(48, 255);
                    w = (r6 << 16) | ((r6 << 8) | 0x60000000) | r6;
                    dp = func_L00_0026DEA0(pos, r4, D_L14_0015F660, w, 0.1f, f21, f21, f20);
                    if (dp != 0) {
                        ((unsigned char *)dp)[3] = 0x44;
                        r7 = func_001F9850(60);
                        *(short *)(dp + 0xA) = r7;
                        *(int *)(dp + 0x24) = 2;
                        ((unsigned char *)dp)[0x2A] = 0x60;
                        dp[0x2B] = dp[0xA];
                    }
                    i--;
                } while (i >= 0);
                return;
        }
    }
    func_0020D678(moby);
}
