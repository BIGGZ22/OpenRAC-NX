/* Transition setup and frame loop: carves the blocks at D_001941C8 out of the table at
   D_00137C80, writes the record table at D_001863B0 and then runs the per-frame loop until
   D_0015F690 is set. */
extern void func_00209DC0_EBB48(void) __asm__("func_00209DC0");
extern void func_00233FF8_EBB48(void) __asm__("func_00233FF8");
extern void func_00234018_EBB48(void) __asm__("func_00234018");
extern void func_001FF6B8_EBB48(void) __asm__("func_001FF6B8");
extern void func_002032D0_EBB48(void) __asm__("func_002032D0");
extern void func_001FFA90_EBB48(void) __asm__("func_001FFA90");
extern void func_002349B8_EBB48(void) __asm__("func_002349B8");
extern void func_00234948_EBB48(void) __asm__("func_00234948");
extern void func_001FB598_EBB48(void) __asm__("func_001FB598");
extern void func_001FB8A8_EBB48(void) __asm__("func_001FB8A8");
extern void func_001FB498_EBB48(void) __asm__("func_001FB498");
extern void func_00209E68_EBB48(void) __asm__("func_00209E68");
extern void func_00209070_EBB48(void) __asm__("func_00209070");
extern void func_00218908_EBB48(void) __asm__("func_00218908");
extern void func_002348E8_EBB48(void) __asm__("func_002348E8");
extern void func_001207B8_EBB48(void) __asm__("func_001207B8");
extern void func_001F3890_EBB48(void) __asm__("func_001F3890");
extern void func_00234AC8_EBB48(int) __asm__("func_00234AC8");
extern void func_00205220_EBB48(int) __asm__("func_00205220");
extern void func_001F4E08_EBB48(int) __asm__("func_001F4E08");
extern void func_00122598_EBB48(int) __asm__("func_00122598");
extern void func_0012EC40_EBB48(void) __asm__("func_0012EC40");
extern void func_0012EC30_EBB48(void) __asm__("func_0012EC30");
extern int func_0012DDC0_EBB48(void) __asm__("func_0012DDC0");
extern int func_001F98C0_EBB48(int) __asm__("func_001F98C0");
extern int func_002176C8_EBB48(void *, int, int) __asm__("func_002176C8");
extern int func_0022EEB8_EBB48(int, int, void *) __asm__("func_0022EEB8");
extern void func_0022EAB0_EBB48(int) __asm__("func_0022EAB0");
extern void func_001E9808_EBB48(int) __asm__("func_001E9808");
extern void func_00121B78_EBB48(int, int, int, int) __asm__("func_00121B78");

extern s32 D_0015EE84_EBB48 __asm__("D_0015EE84") MACRO_ADDR;
extern s32 D_0015EE80_EBB48 __asm__("D_0015EE80") MACRO_ADDR;
extern s32 D_0015EE88_EBB48 __asm__("D_0015EE88") MACRO_ADDR;
extern s32 D_0015EEE8_EBB48 __asm__("D_0015EEE8") MACRO_ADDR;
extern char *D_001941C8_EBB48 __asm__("D_001941C8") MACRO_ADDR;
extern char *D_0015EF4C_EBB48 __asm__("D_0015EF4C") MACRO_ADDR;
extern char D_00137C80_EBB48[] __asm__("D_00137C80");
extern s32 D_001863B0_EBB48[] __asm__("D_001863B0");
extern s32 D_0015F690_EBB48 __asm__("D_0015F690") MACRO_ADDR;
extern s32 D_0015F6E8_EBB48 __asm__("D_0015F6E8") MACRO_ADDR;
extern s32 D_0015F6FC_EBB48 __asm__("D_0015F6FC") MACRO_ADDR;
extern s32 D_0015F538_EBB48 __asm__("D_0015F538") MACRO_ADDR;
extern s32 D_0015F050_EBB48 __asm__("D_0015F050") MACRO_ADDR;
extern s32 D_0015F054_EBB48 __asm__("D_0015F054") MACRO_ADDR;
extern s32 D_0015F058_EBB48 __asm__("D_0015F058") MACRO_ADDR;
extern float D_0015F53C_EBB48 __asm__("D_0015F53C") MACRO_ADDR;
extern s32 D_0015EF74_EBB48 __asm__("D_0015EF74") MACRO_ADDR;
extern s32 D_0015EF78_EBB48 __asm__("D_0015EF78") MACRO_ADDR;
extern s32 D_0015EF8C_EBB48 __asm__("D_0015EF8C") MACRO_ADDR;
extern u8 D_0016044C_EBB48[] __asm__("D_0016044C") MACRO_ADDR;

void func_001EBB48(void) {
    char *base;
    char *cur;
    char *tb;
    char *cc;
    s32 *R;
    s32 cnt;
    s32 s19;
    s32 k;
    s32 c21;
    s32 v2;
    s32 t;
    s32 t2;
    s32 s5;
    s32 m3;
    float one;

    one = 1.0f;
    D_0015EE84_EBB48 = 0;
    *(u32 *)0x10000010 = 0x83;
    *(s32 *)0x10000000 = 0;
    t = D_0015EEE8_EBB48;
    func_00209DC0_EBB48();
    D_0015EEE8_EBB48 = t;
    func_001EABE8();

    base = D_001941C8_EBB48;
    tb = D_00137C80_EBB48;
    cc = (char *)&D_0018CC20_EABE8;
    R = D_001863B0_EBB48;
    cur = base + 0x60;
    D_0015EF4C_EBB48 = base;

    t = func_002176C8_EBB48(cur, *(s32 *)(tb + 0x1500), *(s32 *)(tb + 0x1504));
    *(s32 *)(base + 0x2C) = t;
    *(s32 *)(base + 0x28) = (s32)(cur - base);
    cur += (t + 15) & ~15;

    t = func_002176C8_EBB48(cur, *(s32 *)(tb + 0x1508), *(s32 *)(tb + 0x150C));
    *(s32 *)(base + 0x34) = t;
    *(s32 *)(base + 0x30) = (s32)(cur - base);
    cur += (t + 15) & ~15;

    t = func_002176C8_EBB48(cur, *(s32 *)(tb + 0x1510), *(s32 *)(tb + 0x1514));
    *(s32 *)(base + 0x3C) = t;
    *(s32 *)(base + 0x38) = (s32)(cur - base);
    cur += (t + 15) & ~15;

    t = func_002176C8_EBB48(cur, *(s32 *)(tb + 0x1518), *(s32 *)(tb + 0x151C));
    *(s32 *)(base + 0x44) = t;
    *(s32 *)(base + 0x40) = (s32)(cur - base);
    cur += (t + 15) & ~15;

    t = func_002176C8_EBB48(cur, *(s32 *)(tb + 0x1520), *(s32 *)(tb + 0x1524));
    *(s32 *)(base + 0x4C) = t;
    *(s32 *)(base + 0x48) = (s32)(cur - base);
    cur += (t + 15) & ~15;

    t = func_002176C8_EBB48(cur, *(s32 *)(tb + 0x14F8), *(s32 *)(tb + 0x14FC));
    *(s32 *)(base + 0x24) = t;
    *(s32 *)(base + 0x20) = (s32)(cur - base);
    func_001FFA90_EBB48();

    func_00233FF8_EBB48();
    func_00234018_EBB48();
    func_001FF6B8_EBB48();
    func_002032D0_EBB48();

    cnt = (s32)((u32)*(s32 *)0x10000000 / 0x109u);
    while (cnt < func_001F98C0_EBB48(0xB4)) {
        func_00122598_EBB48(0);
        cnt++;
    }
    func_0012EC40_EBB48();
    while (func_0012DDC0_EBB48() != 0) {
    }
    s19 = 1;
    func_0012EC30_EBB48();

    c21 = 0;
    if (D_0015F690_EBB48 == 0) {
        do {
            func_002349B8_EBB48();
            func_00234948_EBB48();
            func_001FB598_EBB48();
            func_001FB8A8_EBB48();
            func_001FB498_EBB48();
            func_00209E68_EBB48();
            func_00209070_EBB48();
            func_00218908_EBB48();
            func_001EB300_EABE8(D_0015EE88_EBB48);
            D_0015F6FC_EBB48 = 0;

            /* Records of 16 bytes: a, b, c, d. Each frame, records whose b is -1 are
               checked against the timer at D_0018CC20+0x34; the rest are handled
               through func_001EBAF0. */
            if (R[0] != -1) {
                k = 0;
                do {
                    s32 *rec = R + 4 * k;
                    s32 now = *(s32 *)(cc + 0x34);

                    if (rec[1] == -1) {
                        if (now < rec[0]) {
                            rec[3] = -1;
                        } else if (rec[3] == -1) {
                            rec[3] = func_0022EEB8_EBB48(rec[2], 0, 0);
                        }
                    } else {
                        if (now < rec[0] || rec[1] < now) {
                            if (func_001EBAF0(rec[3], rec[2]) != 0) {
                                func_0022EAB0_EBB48(rec[3]);
                            }
                            rec[3] = -1;
                        } else {
                            if (func_001EBAF0(rec[3], rec[2]) == 0) {
                                rec[3] = func_0022EEB8_EBB48(rec[2], 0, 0);
                            }
                        }
                    }
                    k++;
                } while (R[4 * k] != -1);
            }

            func_001EB458();
            func_001EB7C0(0);

            if (D_0015F6E8_EBB48 != 0) {
                c21 = 0;
            } else {
                c21++;
                if (c21 >= func_001F98C0_EBB48(0x5DC)) {
                    func_001E9808_EBB48(s19);
                    c21 = 0;
                    s5 = s19 + 1;
                    t2 = s19 + 4;
                    if (-1 < s5) {
                        t2 = s5;
                    }
                    D_0015F53C_EBB48 = one;
                    *(s32 *)(cc + 0x34) = 0;
                    *(s32 *)(cc + 0x3C) = 0;
                    s19 = t2 >> 2;
                    m3 = s19 << 2;
                    s19 = s5 - m3;
                    t2 = s19 + 1;
                    if (s19 == 0) {
                        s19 = t2;
                    }
                    D_0015F050_EBB48 = 0;
                    D_0015F054_EBB48 = 0;
                    func_00205220_EBB48(0);
                    D_0015F058_EBB48 = 0;
                    func_002348E8_EBB48();
                    continue;
                }
            }

            v2 = D_0016044C_EBB48[0];
            if (v2 != D_0015EE80_EBB48) {
                t2 = (D_0015EE80_EBB48 == 0);
                D_0015EE80_EBB48 = t2;
                func_001F4E08_EBB48(4);
                func_001207B8_EBB48();
                func_00121B78_EBB48(0, 1, (D_0015EE80_EBB48 == 0) ? 2 : 3, 0);
                t = D_0015EF78_EBB48;
                D_0015F53C_EBB48 = one;
                func_001F3890_EBB48();
                D_0015EF78_EBB48 = t;

                func_00234AC8_EBB48(1);
                func_00122598_EBB48(0);
                D_0015F538_EBB48 += 1;
            }
        } while (D_0015F690_EBB48 == 0);
    }

    if (D_0015EE80_EBB48 == 0) {
        D_0015EF8C_EBB48 = 0x280000;
    }
    t = D_0015EF8C_EBB48;
    D_0015EF74_EBB48 = t;
    D_0015EF78_EBB48 = t;
}
