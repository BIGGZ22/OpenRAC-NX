/* Clank's glow and blink (US 0x2278c0): eases the head scale
   (D_0015EEF0 + 0x28) toward 1.0, or 1.8 with the big-head cheat; then,
   on the hero moby in bodies 1/2 or on Clank (D_0013F450 + 0x1184) while
   he is shown, writes the glow colour into +0x90 (a scale_ticks(110) sine
   pulse, base red 0x88 at one health point; turned red while the hit
   flash +0x1EE runs). In Ratchet mode only: re-arms the blink timer
   (+0xFFC) when it runs out and Clank is on sequence 1, steps the blink
   frame (+0xFFE, back to 0 at 22), detaches the four eyelid nodes
   (+0xEF0 + 0x40 k) on frame 0, otherwise attaches the missing ones with
   their poses and sets the nodes' weight from the blink table.
   Adapted from ReRAC (crates/rc-game/src/hero/idle.rs clank_glow_blink,
   clank_glow_word; ISC License, Copyright (c) 2026 ReRAC contributors). */
extern unsigned char D_0013E633_ea[] __asm__("D_0013E633");
extern unsigned char D_0015EEB0_ea[] __asm__("D_0015EEB0");
extern int D_0015EEF0_ea[] __asm__("D_0015EEF0");
extern int D_L00_0015F7D8[];
extern char D_L00_0017C2E0[];
extern char D_L00_0017C320[];
extern char D_L00_0017C360[];
extern float D_L00_0017C3A0[];
extern float func_00214D28(float *p, float target, float maxstep);
extern float func_001FA888(int);
extern float func_001F9FA8(float);
extern int func_001FA898(float);
extern int func_001F9938(void *);
extern int func_L00_00258BC8(int, int);
extern void func_0020D9D8(void *, void *);
extern void func_0020D960(char *, int, void *);
extern void func_001F9C30(void *, void *, float);

void func_L00_00205FF0(void) {
    unsigned char *g;
    char *obj;
    int period;
    float fp;
    float a;
    float s;
    float s24;
    int t;
    int r;
    int gc;
    int b;
    int r12;
    int g24;
    int b12;
    int i;

    if (D_0015EEB0_ea[3] != 0) {
        func_00214D28((float *)((char *)D_0015EEF0_ea + 0x28), 1.8f, 0.05f);
    } else {
        func_00214D28((float *)((char *)D_0015EEF0_ea + 0x28), 1.0f, 0.05f);
    }

    g = D_0013E633_ea + 0xE1D;
    if ((unsigned int)(g[0x20A4] - 1) < 2) {
        obj = *(char **)(g + 0x2080);
    } else {
        if (*(short *)(g + 0x22D8) != 0) {
            return;
        }
        obj = *(char **)(g + 0x1184);
    }
    if (obj == 0) {
        return;
    }

    /* The glow colour. */
    *(unsigned short *)(obj + 0x34) |= 0x10;
    r = (*(int *)(g + 0x22A8) == 1) ? 0x88 : 0x38;
    period = func_001F9850(0x6E);
    fp = func_001FA888(period);
    a = (float)(D_L00_0015F6B0 % period) / fp;
    a = a + a;
    a = a * 3.1415927f;
    s = func_001F9FA8(a + -3.1415927f);
    s24 = s * 24.0f;
    r12 = func_001FA898(s24) + 0xC;
    r = r + r12;
    t = func_001FA898(s * 48.0f);
    g24 = t + 0x18;
    gc = t + 0xA0;
    t = func_001FA898(s24);
    b12 = t + 0xC;
    b = t + 0x4C;
    if (*(short *)(g + 0x1EE) != 0) {
        float k = 1.0f;
        int t5 = func_001F9850(5);
        int t20 = func_001F9850(20);
        int h = *(short *)(g + 0x1EE);

        if (func_001F9850(45) - t5 < h) {
            k = func_001FA888(func_001F9850(45) - *(short *)(g + 0x1EE));
            k = k / func_001FA888(t5);
        }
        if (*(short *)(g + 0x1EE) < t20) {
            k = func_001FA888(*(short *)(g + 0x1EE));
            k = k / func_001FA888(t20);
        }
        r = r - func_001FA898((float)r12 * k);
        gc = gc - func_001FA898((float)g24 * k);
        b = b - func_001FA898((float)b12 * k);
        r = r + func_001FA898(k * 112.0f);
        gc = gc - func_001FA898(k * 72.0f);
    }
    *(int *)(obj + 0x90) = (0x80 << 24) | (b << 16) | (gc << 8) | r;

    /* Clank's blink, Ratchet mode only. */
    g = D_0013E633_ea + 0xE1D;
    if (g[0x20A4] != 0) {
        return;
    }
    if (func_001F9938(g + 0xFFC) != 0) {
        unsigned char seq = *(unsigned char *)(obj + 0x53);
        if (seq == 1) {
            int lo = func_001F9850(50);
            int v = func_L00_00258BC8(lo, func_001F9850(200));
            *(short *)(g + 0xFFE) = seq;
            *(short *)(g + 0xFFC) = v;
        }
    }
    if (*(short *)(g + 0xFFE) == 0) {
        return;
    }
    {
        short f = *(unsigned short *)(g + 0xFFE) + 1;
        *(short *)(g + 0xFFE) = f;
        if (f >= 0x16) {
            *(short *)(g + 0xFFE) = 0;
        }
    }
    if (*(short *)(g + 0xFFE) == 0) {
        unsigned char *node = g + 0xEF0;
        for (i = 3; i >= 0; i--) {
            if (node[1] != 0) {
                func_0020D9D8(obj, node);
            }
            node += 0x40;
        }
        return;
    }
    {
        unsigned char *e = g;
        char *quat = D_L00_0017C2E0;
        char *scl = D_L00_0017C320;
        char *trans = D_L00_0017C360;
        int *list = D_L00_0015F7D8;

        for (i = 0; i < 4; i++) {
            if (e[0xEF1] == 0) {
                func_0020D960(obj, *list, e + 0xEF0);
                e[0xEF3] = 1;
                qcopy(e + 0xF00, quat);
                qcopy(e + 0xF10, scl);
                qcopy(e + 0xF20, trans);
                func_001F9C30(e + 0xF20, e + 0xF20, *(float *)(obj + 0x2C));
            }
            e += 0x40;
            quat += 0x10;
            scl += 0x10;
            trans += 0x10;
            list++;
        }
    }
    {
        float *w = (float *)(g + 0xEFC);
        for (i = 3; i >= 0; i--) {
            *w = D_L00_0017C3A0[*(short *)(g + 0xFFE)];
            w += 0x10;
        }
    }
}
