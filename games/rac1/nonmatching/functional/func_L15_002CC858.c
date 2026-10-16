typedef struct L15AlarmMoby {
    char pad0[0x10];
    float position[3];
    char pad1[4];
    unsigned char state;
    char pad2[0x32];
    unsigned char animation;
    char pad3[0x1C];
    unsigned char status;
    char pad4[7];
    void *data;
    char pad5[0x14];
    int w90;
} L15AlarmMoby;

typedef struct L15AlarmData {
    int mode;
    int idx;
    int f8;
    int c;
    int d10;
    float f14;
    char blk18[16];
} L15AlarmData;

extern short D_L15_00161B3C_2CC858 __asm__("D_L15_00161B3C");
extern short D_L15_00161B40_2CC858 __asm__("D_L15_00161B40");
extern short D_L15_00161B44_2CC858 __asm__("D_L15_00161B44");
extern int D_L15_0015F6A8_2CC858 __asm__("D_L15_0015F6A8") MACRO_ADDR;
extern int D_L15_0015F6B0_2CC858 __asm__("D_L15_0015F6B0") MACRO_ADDR;
extern float D_0015EE6C_2CC858 __asm__("D_0015EE6C") MACRO_ADDR;
extern char D_0013E633_2CC858[] __asm__("D_0013E633");
extern void func_00213DE0_2CC858(void *, int, int, int) __asm__("func_00213DE0");
extern float func_001F9FA8_2CC858(float) __asm__("func_001F9FA8");
extern int func_001FA8A8_2CC858(int, int, float) __asm__("func_001FA8A8");
extern float func_001FA748_2CC858(float, float) __asm__("func_001FA748");
extern int func_001F9850_2CC858(int) __asm__("func_001F9850");
extern int func_001F9908_2CC858(void *) __asm__("func_001F9908");
extern void func_L15_002CCE50_2CC858(char *m, int a) __asm__("func_L15_002CCE50");
extern float func_001F9D48_2CC858(float *, float *) __asm__("func_001F9D48");
extern int func_L00_001EFFF0_2CC858(void *, void *, int, void *, void *) __asm__("func_L00_001EFFF0");
extern int func_L15_002A3A48_2CC858(int idx, void *pos) __asm__("func_L15_002A3A48");
extern void func_L15_002CCFC0_2CC858(void *arg) __asm__("func_L15_002CCFC0");

/* func_L15_002CC858 -- src/overlays/shared/vendor_00298BB8.c (functional C for the port, not a match)
 * Sentrybot alarm update (moby class 408, levels 15 and 17): an 11-state machine that idles, pulses
 * its colour, raises (anim 3..6) when alerted (pvars +8), watches for Ratchet within 16 (line of sight,
 * func_L15_002A3A48) and re-arms its timer, lowers when its alarm timer (pvars +0x18) runs out or the
 * level's alarm is off (D_L15_0015F6A8 == 2); while alerted it sets D_L15_00161B40, and once a frame
 * (D_L15_00161B3C vs the frame counter) runs func_L15_002CCFC0.
 * From the staged near miss (nonmatching/shared/func_L15_002CC858.c), made self-contained; its
 * logic matched retail case by case on reading.
 * equiv: DIFFERENT by shape only. This compiler merges identical tails that retail keeps separate:
 * state 6 and state 4's mode-1 path are the same code (state 7, anim 5), and state 10's
 * func_00213DE0(m, 1, 0, ...) call joins the shared call site. That is why the tool sees one
 * "missing" func_00213DE0 call, +0x53 load and +0x20 store; every path still makes them. */
void func_L15_002CC858(L15AlarmMoby *m) {
    L15AlarmData *d = m->data;
    float f;
    int r;

    switch (m->state) {
    case 0: {
        int mode = d->mode;

        *(int *)&D_L15_00161B3C_2CC858 = 0;
        *(int *)&D_L15_00161B40_2CC858 = 0;
        *(int *)&D_L15_00161B44_2CC858 = 0;
        if (mode == 2) {
            m->state = 6;
            if (m->animation != 4)
                func_00213DE0_2CC858(m, 4, 0, 0);
        } else {
            m->state = 3;
            if (m->animation != 2)
                func_00213DE0_2CC858(m, 2, 0, 0);
        }
        break;
    }
    case 1:
        f = func_001F9FA8_2CC858(d->f14);
        r = func_001FA8A8_2CC858(0x801414BC, 0x801414DC, (f + 1.0f) * 0.5f);
        m->w90 = r;
        d->f14 = func_001FA748_2CC858(d->f14, D_0015EE6C_2CC858 * 6.2831855f);
        if (!d->f8)
            goto tail;
        if (D_L15_0015F6A8_2CC858 == 2)
            break;
        if (d->mode != 1) {
            m->state = 5;
            break;
        }
        m->state = 6;
        if (m->animation != 4)
            func_00213DE0_2CC858(m, 4, 0, func_001F9850_2CC858(30));
        break;
    case 2:
        if (m->status & 2) {
            m->state = 3;
            if (m->animation != 2)
                func_00213DE0_2CC858(m, 2, 0, func_001F9850_2CC858(10));
        }
        break;
    case 3:
        if (!d->f8)
            goto tail;
        if (D_L15_0015F6A8_2CC858 == 2)
            break;
        m->state = 4;
        if (m->animation != 3)
            func_00213DE0_2CC858(m, 3, 0, func_001F9850_2CC858(10));
        break;
    case 4:
        if (!(m->status & 2))
            break;
        if (!d->f8 || D_L15_0015F6A8_2CC858 == 2)
            goto b98;
        if (d->mode != 1) {
            if (m->animation != 8)
                func_00213DE0_2CC858(m, 8, 0, func_001F9850_2CC858(10));
            m->state = 5;
        } else {
            m->state = 7;
            if (m->animation != 5)
                func_00213DE0_2CC858(m, 5, 0, func_001F9850_2CC858(10));
        }
        break;
    case 5:
        if ((m->status & 2) && m->animation != 0)
            func_00213DE0_2CC858(m, 0, 0, func_001F9850_2CC858(10));
        f = func_001F9FA8_2CC858(d->f14);
        r = func_001FA8A8_2CC858(0x801414DC, 0x8004048C, (f + 1.0f) * 0.5f);
        m->w90 = r;
        d->f14 = func_001FA748_2CC858(d->f14, D_0015EE6C_2CC858 * 12.566371f);
        if (func_001F9908_2CC858(d->blk18) == 0 && D_L15_0015F6A8_2CC858 != 2)
            break;
        func_L15_002CCE50_2CC858((char *)m, 0);
        d->f8 = 0;
        goto b98;
    case 6:
        if (!d->f8)
            goto tail;
        m->state = 7;
        if (m->animation != 5)
            func_00213DE0_2CC858(m, 5, 0, func_001F9850_2CC858(10));
        break;
    case 7:
        if (!(m->status & 2))
            break;
        m->state = 8;
        if (m->animation != 6)
            func_00213DE0_2CC858(m, 6, 0, func_001F9850_2CC858(10));
        break;
    case 8: {
        float *pos = m->position;
        char *h2 = D_0013E633_2CC858 + 0xE9D;
        float tmp[4];

        if (func_001F9D48_2CC858(pos, (float *)h2) < 16.0f && D_L15_0015F6A8_2CC858 != 2 && func_001F9908_2CC858(&d->c)) {
            qcopy(tmp, pos);
            tmp[2] = tmp[2] + 1.0f;
            if (func_L00_001EFFF0_2CC858(tmp, h2 + 0x50, 2, 0, 0) == 0 && func_L15_002A3A48_2CC858(d->idx, pos))
                d->c = d->d10;
        }
        r = func_001F9908_2CC858(d->blk18);
        if (r != 0 || D_L15_0015F6A8_2CC858 == 2) {
            func_L15_002CCE50_2CC858((char *)m, 0);
            d->f8 = 0;
            m->state = 9;
            if (m->animation != 7)
                func_00213DE0_2CC858(m, 7, 0, func_001F9850_2CC858(10));
        }
        break;
    }
    case 9:
        if (!(m->status & 2))
            break;
        if (d->mode == 2) {
            m->state = 6;
            if (m->animation != 4)
                func_00213DE0_2CC858(m, 4, 0, func_001F9850_2CC858(10));
        } else {
            m->state = 10;
            if (m->animation != 8)
                func_00213DE0_2CC858(m, 8, 0, func_001F9850_2CC858(10));
        }
        break;
    case 10:
        if (!(m->status & 2))
            break;
        m->state = 2;
        if (m->animation != 1)
            func_00213DE0_2CC858(m, 1, 0, func_001F9850_2CC858(10));
        break;
    }
    goto done8;
b98:
    if (m->animation != 8)
        func_00213DE0_2CC858(m, 8, 0, func_001F9850_2CC858(10));
    m->state = 10;
done8:
    if (d->f8)
        *(int *)&D_L15_00161B40_2CC858 = 1;
tail:
    if (*(int *)&D_L15_00161B3C_2CC858 != D_L15_0015F6B0_2CC858) {
        *(int *)&D_L15_00161B3C_2CC858 = D_L15_0015F6B0_2CC858;
        func_L15_002CCFC0_2CC858(m);
        *(int *)&D_L15_00161B40_2CC858 = 0;
    }
}
