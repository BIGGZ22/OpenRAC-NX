/* func_002B4C18 (boot 0x2B4C18, 984 bytes, 28 places): `python3 tools/check.py func_002B4C18 --diff` prints
 * `func_002B4C18: 28 of 28 places match` for the C below (it lives in src/game/002B3868-g8.c: the retail
 * accesses D_001AA0F4/F8/FC/104 through $gp in ordinary flow, so the file needs -G8 and those four are `sda`),
 * but tools/build.py cannot place it: the two 10-entry `switch` statements compile to jump tables in the object's
 * own .rodata and Placement.derive() therefore needs `.rodata` at 0x26CBB0 ("needs .rodata at its place, which
 * is not C"; same wall as func_00118CC8). Retail keeps the tables as data at 0x26CBB0 and 0x26CBE0. Per the repo
 * rule the exact C stays here, the INCLUDE_ASM marker is back in src/game/002B3868.c and the -g8 file is as it
 * was.
 *
 * Things that were needed to get the bytes: D_001A8BB8 is volatile (otherwise the first load is shared with the
 * compare in case 3 of the first switch, and the second switch loads the index twice); the second switch takes
 * its index from a local (`sel`); case 2 of the second switch tests `(unsigned)(D_001A8BB0 - 1) >= 2` first and
 * keeps the func_002F53C8 arm last; the cases come in retail's order (3, 4, 1, 2, 6, 5, 7, 8, 9 in the first
 * switch, 3, 0, 4, 1, 2, 6, 5, 7, 8, 9 in the second; case 3 of the second falls into case 0); the constant
 * 0x3DCCCCCE is `0.10000001f`; D_001A8BB0/BB4/BB8/8C7C/15A0/1694 are plain externs without `sda` (bare form: absolute in
 * ordinary flow, $gp only inside a delay slot) while D_0018C2EC and D_001B90F0 are `nosda` (explicit lui/%lo).
 *
 * An untested way round, kept in the #if 0 block at the end: dispatch with GNU computed gotos through the
 * retail tables (D_0026CBB0 and D_0026CBE0 are named boot symbols like any D_ name) and keep the labels alive
 * with unused tables of label addresses. check.py also says `28 of 28` for it, the section need is gone and the
 * code is byte-identical. tools/build.py was NOT run on it (not allowed for the worker); it is plain C but a
 * new mechanism, so it needs a decision before it goes into src/. */

extern int D_001A8BB0;
extern int D_001A8BB4;
extern volatile int D_001A8BB8;
extern int D_0018C2EC __attribute__((nosda));
extern int D_001AA0F4 __attribute__((sda));
extern int D_001AA0F8 __attribute__((sda));
extern int D_001AA0FC __attribute__((sda));
extern char *D_001AA104 __attribute__((sda));
extern float D_001A8C7C;
extern float D_001B15A0;
extern int D_001B1694;
extern float D_001B90F0 __attribute__((nosda));
extern int D_001ABD80;
extern void func_002CA790(void);
extern void func_00285860(void);
extern void func_002F5EF8(void);
extern void func_002F57A8(void);
extern void func_0026FF08(int, int, float, float, float, float);
extern void func_0027AB40(void);
extern void func_002E7248(void);
extern void func_002F8768(void);
extern void func_002DF380(void);
extern void func_002FF288(void);
extern void func_00291128(void);
extern void func_00282A28(void);
extern void func_002B49B0(int);
extern int func_002857B0(void);
extern void func_002C9CF8(int);
extern void func_002858C0(int, int);
extern void func_002F5D40(int);
extern float func_002AA2E8(float *, float, float);
extern void func_002F53C8(int);
extern void func_00288CB0(int, int);
extern void func_002E71B8(void);
extern void func_002E7090(int);
extern void func_002E7018(void);
extern void func_002F8178(int);
extern void func_002DF2E0(int, int);
extern void func_002FF0D8(void);
extern void func_00291108(void);

void func_002B4C18(void)
{
    int mode = D_001A8BB0;
    int sel;
    int done;
    D_001A8BB4 = mode;
    if (D_001A8BB8 == -2)
        return;
    if (D_001AA0F4 > 0) {
        D_001AA0F4--;
        return;
    }
    if (D_0018C2EC == 0)
        return;
    switch (mode) {
    case 3:
        if (D_001A8BB8 != 4 || D_001AA0F8 == 8)
            func_002CA790();
        break;
    case 4:
        func_00285860();
        break;
    case 1:
        func_002F5EF8();
        goto shared;
    case 2:
        func_002F57A8();
    shared:
        D_001B90F0 = 0.62f;
        func_0026FF08(0, 3, 1.1243411f, 0.005f, 0.2f, 0.0f);
        func_0027AB40();
        break;
    case 6:
        if (D_001AA0F8 != 4)
            func_002E7248();
        break;
    case 5:
        func_002F8768();
        break;
    case 7:
        func_002DF380();
        break;
    case 8:
        func_002FF288();
        break;
    case 9:
        func_00291128();
        break;
    case 0:
        break;
    default:
        func_00282A28();
        break;
    }
    func_002B49B0(10);
    done = 0;
    sel = D_001A8BB8;
    switch (sel) {
    case 3:
        if (D_001A8BB4 != 4 || func_002857B0() == 8)
            func_002C9CF8(D_001AA0F8);
    case 0:
        done = 0;
        break;
    case 4:
        func_002858C0(D_001AA0F8, D_001AA0FC);
        break;
    case 1:
        func_002F5D40(D_001AA0FC);
        done = 0;
        break;
    case 2:
        if ((unsigned)(D_001A8BB0 - 1) >= 2) {
            float f;
            func_002AA2E8(&D_001A8C7C, 1.2f, 0.10000001f);
            f = D_001A8C7C;
            D_001B15A0 = f;
            if (1.0f < f)
                D_001B15A0 = 1.0f;
            if (1.2f <= f) {
                done = 0;
                D_001A8C7C = 1.0f;
                func_002F53C8(D_001AA0F8);
            } else {
                done = 1;
            }
        } else {
            func_002F53C8(D_001AA0F8);
        }
        break;
    case 6:
        if (D_001AA0F8 == 5) {
            func_00288CB0(D_001AA0FC, 1);
        } else if (D_001AA0F8 == 6) {
            func_002E71B8();
        } else {
            if (D_001A8BB4 == 3)
                func_002E7090(D_001AA0FC);
            else
                func_002E7018();
            D_001B1694 = 1;
        }
        break;
    case 5:
        func_002F8178(D_001AA0F8);
        break;
    case 7:
        {
            int fallback = D_001ABD80;
            func_002DF2E0(D_001AA0F8 ? D_001AA0F8 : fallback, D_001AA0FC);
        }
        break;
    case 8:
        func_002FF0D8();
        break;
    case 9:
        func_00291108();
        break;
    default:
        func_00282A28();
        break;
    }
    if (done == 0) {
        int state;
        if (D_001AA104 != 0)
            *D_001AA104 = 1;
        state = D_001A8BB8;
        D_001A8BB8 = -2;
        D_001A8BB0 = state;
        D_001AA104 = 0;
    }
}

#if 0
/* The computed-goto variant (check.py 28 of 28, no .rodata need); also needs:
   extern void *D_0026CBB0[];  extern void *D_0026CBE0[]; */
void func_002B4C18(void)
{
    int mode = D_001A8BB0;
    int sel;
    int done;
    {
        static void *const keep1[] = {&&L1_3, &&L1_4, &&L1_1, &&L1_2, &&L1_6, &&L1_5, &&L1_7, &&L1_8, &&L1_9, &&L1_0, &&L1_def, &&shared};
        static void *const keep2[] = {&&L2_3, &&L2_0, &&L2_4, &&L2_1, &&L2_2, &&L2_6, &&L2_5, &&L2_7, &&L2_8, &&L2_9, &&L2_def};
    }
    D_001A8BB4 = mode;
    if (D_001A8BB8 == -2)
        return;
    if (D_001AA0F4 > 0) {
        D_001AA0F4--;
        return;
    }
    if (D_0018C2EC == 0)
        return;
    if ((unsigned)mode >= 10)
        goto L1_def;
    goto *D_0026CBB0[mode];
    L1_3:
        if (D_001A8BB8 != 4 || D_001AA0F8 == 8)
            func_002CA790();
        goto E1;
    L1_4:
        func_00285860();
        goto E1;
    L1_1:
        func_002F5EF8();
        goto shared;
    L1_2:
        func_002F57A8();
    shared:
        D_001B90F0 = 0.62f;
        func_0026FF08(0, 3, 1.1243411f, 0.005f, 0.2f, 0.0f);
        func_0027AB40();
        goto E1;
    L1_6:
        if (D_001AA0F8 != 4)
            func_002E7248();
        goto E1;
    L1_5:
        func_002F8768();
        goto E1;
    L1_7:
        func_002DF380();
        goto E1;
    L1_8:
        func_002FF288();
        goto E1;
    L1_9:
        func_00291128();
        goto E1;
    L1_0:
        goto E1;
    L1_def:
        func_00282A28();
        goto E1;
    E1:
    func_002B49B0(10);
    done = 0;
    sel = D_001A8BB8;
    if ((unsigned)sel >= 10)
        goto L2_def;
    goto *D_0026CBE0[sel];
    L2_3:
        if (D_001A8BB4 != 4 || func_002857B0() == 8)
            func_002C9CF8(D_001AA0F8);
    L2_0:
        done = 0;
        goto E2;
    L2_4:
        func_002858C0(D_001AA0F8, D_001AA0FC);
        goto E2;
    L2_1:
        func_002F5D40(D_001AA0FC);
        done = 0;
        goto E2;
    L2_2:
        if ((unsigned)(D_001A8BB0 - 1) >= 2) {
            float f;
            func_002AA2E8(&D_001A8C7C, 1.2f, 0.10000001f);
            f = D_001A8C7C;
            D_001B15A0 = f;
            if (1.0f < f)
                D_001B15A0 = 1.0f;
            if (1.2f <= f) {
                done = 0;
                D_001A8C7C = 1.0f;
                func_002F53C8(D_001AA0F8);
            } else {
                done = 1;
            }
        } else {
            func_002F53C8(D_001AA0F8);
        }
        goto E2;
    L2_6:
        if (D_001AA0F8 == 5) {
            func_00288CB0(D_001AA0FC, 1);
        } else if (D_001AA0F8 == 6) {
            func_002E71B8();
        } else {
            if (D_001A8BB4 == 3)
                func_002E7090(D_001AA0FC);
            else
                func_002E7018();
            D_001B1694 = 1;
        }
        goto E2;
    L2_5:
        func_002F8178(D_001AA0F8);
        goto E2;
    L2_7:
        {
            int fallback = D_001ABD80;
            func_002DF2E0(D_001AA0F8 ? D_001AA0F8 : fallback, D_001AA0FC);
        }
        goto E2;
    L2_8:
        func_002FF0D8();
        goto E2;
    L2_9:
        func_00291108();
        goto E2;
    L2_def:
        func_00282A28();
        goto E2;
    E2:
    if (done == 0) {
        int state;
        if (D_001AA104 != 0)
            *D_001AA104 = 1;
        state = D_001A8BB8;
        D_001A8BB8 = -2;
        D_001A8BB0 = state;
        D_001AA104 = 0;
    }
}
#endif
