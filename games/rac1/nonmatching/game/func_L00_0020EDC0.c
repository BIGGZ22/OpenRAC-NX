/* Hero animation slots (two entries at 0xD08 of the hero block): keeps each slot's object
   animated from the table D_L00_00179BC0 for the current state, and returns early when a
   slot's distance check (byte 0x23 / float at +8) says the object is done. Finishes with
   the 0x640..0x6FF block (a copy of the object's 0xC0..0xEF data or the 0x90 block). */
extern int func_L00_00234718(int);
extern void func_001F9BC0(void *);
extern void func_001F9EC0(void *, void *, void *);
extern void func_001FA1F8(void *, void *);

void func_L00_0020EDC0(void) {
    char *g = (char *)&D_0013F450_0EB60;
    int k;

    if (*(unsigned char *)(g + 0x20A8) != 0 && *(unsigned char *)(g + 0x20AA) != 0) {
        if (func_L00_00234718(-1) == 0) {
            func_L00_0020ED30();
        }
    }

    for (k = 0; k < 2; k++) {
        unsigned char **slotp = (unsigned char **)(g + 0xD08) + k;
        unsigned char *p = *slotp;
        int ret;
        int d14;

        if (p == 0) {
            continue;
        }
        ret = func_L00_0020DB30(0);
        if (ret == *(int *)(g + 0x2298)) {
            int v5;
            if (*(int *)(g + 0x208C) == 0xC) {
                v5 = D_L00_00179BC0_0EB60[ret].f2C;
            } else {
                v5 = D_L00_00179BC0_0EB60[ret].f28;
            }
            if (v5 != -1 && *(unsigned char *)(p + 0x23) != v5) {
                int r;
                *(int *)(g + 0x2294) = v5;
                r = func_001F9850(8);
                func_L00_00250418(*(int *)(g + 0x2080), p, *(int *)(g + 0x2294), 0, r, 1);
            }
        }

        func_L00_002501C8(*(void **)(g + 0x2080), p);

        d14 = *(int *)(g + 0xD14);
        if (d14 == 0) {
            func_00214D28((float *)(p + 8), 1.0f, D_0015EE60 * 0.2f);
            if ((p[0x5] & 2) == 0) {
                continue;
            }
            if (*(unsigned char *)(g + 0x20AA) == 0) {
                *(int *)(g + 0xD14) = 1;
                *(int *)(p + 0x28) = 0;
                *(unsigned char *)(g + 0x20A8) = 0;
                return;
            }
            continue;
        }
        if (d14 == 2) {
            func_00214D28((float *)(p + 8), 0.0f, D_0015EE60 * 0.25f);
        } else {
            func_00214D28((float *)(p + 8), 0.0f, D_0015EE60 * 0.07f);
        }
        if (*(float *)(p + 8) != 0.0f) {
            continue;
        }
        func_L00_00250120(*(void **)(g + 0x2080), slotp);
        return;
    }

    if (*(int *)(g + 0x208C) != 0xF) {
        char *obj = *(char **)(g + 0x2080);
        char *a = g + 0x640;
        char *b = g + 0x670;
        int i;
        for (i = 0; i < 16; i++) {
            a[i] = obj[0xC0 + i];
        }
        for (i = 0; i < 16; i++) {
            a[0x10 + i] = obj[0xD0 + i];
        }
        for (i = 0; i < 16; i++) {
            a[0x20 + i] = obj[0xE0 + i];
        }
        func_001F9BC0(b);
        *(float *)(g + 0x670) = 1.0f;
        func_001F9EC0(b, b, a);
        for (i = 0; i < 16; i++) {
            (g + 0x680)[i] = (g + 0x90)[i];
        }
        return;
    } else {
        char *ptr = *(char **)(g + 0x5D8);
        float f0;
        float f1;
        int i;
        char *h;

        if (ptr != 0) {
            float a1 = *(float *)(g + 0x80);
            float a2 = *(float *)(g + 0x84);
            float c12;
            float c13;
            for (i = 0; i < 16; i++) {
                (g + 0x680)[i] = (g + 0x90)[i];
            }
            c12 = *(float *)(ptr + 0x10) - a1;
            c13 = *(float *)(ptr + 0x14) - a2;
            f0 = func_L00_001FF860(c12, c13);
            f1 = *(float *)(g + 0x580);
        } else {
            for (i = 0; i < 16; i++) {
                (g + 0x680)[i] = (g + 0x90)[i];
            }
            f0 = func_001FA748(*(float *)(g + 0x57C), *(float *)(g + 0x5D0));
            f1 = *(float *)(g + 0x580);
        }
        *(float *)(g + 0x688) = f0;
        *(float *)(g + 0x684) = -f1;

        h = g + 0x640;
        func_001FA1F8(h, h + 0x40);
        func_001F9BC0(h + 0x30);
        *(float *)(h + 0x30) = 1.0f;
        func_001F9EC0(h + 0x30, h + 0x30, h);
        return;
    }
}
