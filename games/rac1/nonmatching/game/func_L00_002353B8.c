/* Per-frame update of one object (obj = the call's first argument). A float scale f20 (1.33 at
   most, when the mode at D_L00_0015F6A8 is 6) is stored in the entry at D_L00_0017C440 the first
   time it is seen, and for the modes 2, 9, 0xB and 0xD it scales the 0x2C float of each entry in
   the table at D_L00_0016C960 whose type is 0x1B1, 0x509 or 0x50A. Then it runs a float
   smoothing step on the value at +0x1628 (func_001FA748), and when the mode is 0 it may run a
   second step with other constants. Finally it writes obj+0x90 and calls func_L00_002352D0. */
extern char D_L00_0017C440[];
extern s32 D_0015EE84 MACRO_ADDR;
extern short D_0015EF14;
extern float func_001FA748_2353(float, float) __asm__("func_001FA748");

void func_L00_002353B8(int obj) {
    char *g = D_0013F450;
    char *tbl = D_L00_0016C960;
    char *c = D_L00_0017C440;
    float f20 = *(f32 *)&D_0015EF14;
    int r17 = (int)0x806E8C6Eu;
    int r16 = (int)0x805A3232u;
    float fx;
    float r;
    int mode;
    int i;

    if (D_L00_0015F6A8 == 6) {
        if (1.33f < f20) {
            f20 = 1.33f;
        }
    }

    if (*(unsigned char *)(c + 1) == 0) {
        func_0020D960((void *)obj, 0x18, c);
        *(float *)(c + 0x28) = f20;
        *(float *)(c + 0x20) = f20;
        *(float *)(c + 0x24) = f20;
    }

    mode = D_0015EE84;
    if (mode == 2 || mode == 9 || mode == 0xB || mode == 0xD) {
        int count = *(short *)(tbl + 0x44);
        for (i = 0; i < count; i++) {
            char *q = *(char **)(tbl + 0x178 + i * 4);
            if (q != 0) {
                short t = *(short *)(q + 0xA6);
                if (t == 0x1B1 || t == 0x50A || t == 0x509) {
                    char *w = *(char **)(q + 0x24);
                    *(float *)(q + 0x2C) = *(float *)(w + 0x24) * f20;
                }
            }
        }
    }

    fx = *(f32 *)&D_0015EE6C;
    r = func_001FA748_2353(*(float *)(g + 0x1628), fx * 2.0943952f);
    *(float *)(g + 0x1628) = r;

    if (D_0015EE84 == 0) {
        if (*(int *)(tbl + 0x30) == 0) {
            int a = func_001F9850(0x343);
            if (!(*(int *)(tbl + 0x34) < a)) {
                int b = func_001F9850(0x438);
                if (!(b < *(int *)(tbl + 0x34))) {
                    fx = *(f32 *)&D_0015EE6C;
                    r17 = (int)0x806EAAFAu;
                    r16 = (int)0x80326E5Au;
                    *(float *)(g + 0x1628) = func_001FA748_2353(*(float *)(g + 0x1628), fx * 5.93411922f);
                }
            }
        }
    }

    {
        float f0 = func_001F9FA8(*(float *)(g + 0x1628));
        float f12 = 0.5f;
        int ret;
        f0 = f0 * f12;
        ret = func_001FA8A8(r17, r16, f0 + f12);
        *(int *)(obj + 0x90) = ret;
        func_L00_002352D0(obj);
    }
}
