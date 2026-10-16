/* NON_MATCHING func_L15_002CC858 -- src/overlays/shared/vendor_00298BB8.c
 * Best so far: SIZE ours 1512 / retail 1520, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Sentrybot alarm update (class 408, levels 15 and 17): an 11-state switch on m->state, with a shared tail that 
 *   Best so far p1.c: size 1512 against retail 1520 (8 bytes short). Differences left:
 *   - The gp int stores `*(int *)&D_L15_00161B40 = 0` compile as lui + sw (`0x1B40($at)`) where retail uses `-0x51
 *   - Case 4 (state 4, mode != 1 path): retail has `bnel` with `lbu $3,0x53` in its delay slot; ours is a plain `b
 *   - The B98 block (anim != 8 call, state 10), shared by cases 4 and 5, is laid out inline after case 5 in retail
 *   Runs: 8 of 10 used (runs 2 and 5 re-ran an unchanged candidate by mistake). Stopped on budget, not on a wall.
 */
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

extern short D_L15_00161B3C;
extern short D_L15_00161B40;
extern short D_L15_00161B44;
extern int D_L15_0015F6A8 MACRO_ADDR;
extern int D_L15_0015F6B0 MACRO_ADDR;
extern float D_0015EE6C MACRO_ADDR;
extern char D_0013E633[];
extern void func_00213DE0(void *, int, int, int);
extern float func_001F9FA8(float);
extern int func_001FA8A8(int, int, float);
extern float func_001FA748(float, float);
extern int func_001F9850(int);
extern int func_001F9908(void *);
extern void func_L15_002CCE50(char *m, int a);
extern float func_001F9D48(float *, float *);
extern int func_L00_001EFFF0(void *, void *, int, void *, void *);
extern int func_L15_002A3A48(int idx, void *pos);
extern void func_L15_002CCFC0(void *arg);

/* Sentrybot alarm update (moby class 408, levels 15 and 17): turn, watch for the hero, raise the alarm. */
void func_L15_002CC858(L15AlarmMoby *m) {
    L15AlarmData *d = m->data;
    float f;
    int r;

    switch (m->state) {
    case 0: {
        int mode = d->mode;

        *(int *)&D_L15_00161B3C = 0;
        *(int *)&D_L15_00161B40 = 0;
        *(int *)&D_L15_00161B44 = 0;
        if (mode == 2) {
            m->state = 6;
            if (m->animation != 4)
                func_00213DE0(m, 4, 0, 0);
        } else {
            m->state = 3;
            if (m->animation != 2)
                func_00213DE0(m, 2, 0, 0);
        }
        break;
    }
    case 1:
        f = func_001F9FA8(d->f14);
        r = func_001FA8A8(0x801414BC, 0x801414DC, (f + 1.0f) * 0.5f);
        m->w90 = r;
        d->f14 = func_001FA748(d->f14, D_0015EE6C * 6.2831855f);
        if (!d->f8)
            goto tail;
        if (D_L15_0015F6A8 == 2)
            break;
        if (d->mode != 1) {
            m->state = 5;
            break;
        }
        m->state = 6;
        if (m->animation != 4)
            func_00213DE0(m, 4, 0, func_001F9850(30));
        break;
    case 2:
        if (m->status & 2) {
            m->state = 3;
            if (m->animation != 2)
                func_00213DE0(m, 2, 0, func_001F9850(10));
        }
        break;
    case 3:
        if (!d->f8)
            goto tail;
        if (D_L15_0015F6A8 == 2)
            break;
        m->state = 4;
        if (m->animation != 3)
            func_00213DE0(m, 3, 0, func_001F9850(10));
        break;
    case 4:
        if (!(m->status & 2))
            break;
        if (!d->f8 || D_L15_0015F6A8 == 2)
            goto b98;
        if (d->mode != 1) {
            if (m->animation != 8)
                func_00213DE0(m, 8, 0, func_001F9850(10));
            m->state = 5;
        } else {
            m->state = 7;
            if (m->animation != 5)
                func_00213DE0(m, 5, 0, func_001F9850(10));
        }
        break;
    case 5:
        if ((m->status & 2) && m->animation != 0)
            func_00213DE0(m, 0, 0, func_001F9850(10));
        f = func_001F9FA8(d->f14);
        r = func_001FA8A8(0x801414DC, 0x8004048C, (f + 1.0f) * 0.5f);
        m->w90 = r;
        d->f14 = func_001FA748(d->f14, D_0015EE6C * 12.566371f);
        if (func_001F9908(d->blk18) == 0 && D_L15_0015F6A8 != 2)
            break;
        func_L15_002CCE50((char *)m, 0);
        d->f8 = 0;
        goto b98;
    case 6:
        if (!d->f8)
            goto tail;
        m->state = 7;
        if (m->animation != 5)
            func_00213DE0(m, 5, 0, func_001F9850(10));
        break;
    case 7:
        if (!(m->status & 2))
            break;
        m->state = 8;
        if (m->animation != 6)
            func_00213DE0(m, 6, 0, func_001F9850(10));
        break;
    case 8: {
        float *pos = m->position;
        char *h2 = D_0013E633 + 0xE9D;
        float tmp[4];

        if (func_001F9D48(pos, (float *)h2) < 16.0f && D_L15_0015F6A8 != 2 && func_001F9908(&d->c)) {
            qcopy(tmp, pos);
            tmp[2] = tmp[2] + 1.0f;
            if (func_L00_001EFFF0(tmp, h2 + 0x50, 2, 0, 0) == 0 && func_L15_002A3A48(d->idx, pos))
                d->c = d->d10;
        }
        r = func_001F9908(d->blk18);
        if (r != 0 || D_L15_0015F6A8 == 2) {
            func_L15_002CCE50((char *)m, 0);
            d->f8 = 0;
            m->state = 9;
            if (m->animation != 7)
                func_00213DE0(m, 7, 0, func_001F9850(10));
        }
        break;
    }
    case 9:
        if (!(m->status & 2))
            break;
        if (d->mode == 2) {
            m->state = 6;
            if (m->animation != 4)
                func_00213DE0(m, 4, 0, func_001F9850(10));
        } else {
            m->state = 10;
            if (m->animation != 8)
                func_00213DE0(m, 8, 0, func_001F9850(10));
        }
        break;
    case 10:
        if (!(m->status & 2))
            break;
        m->state = 2;
        if (m->animation != 1)
            func_00213DE0(m, 1, 0, func_001F9850(10));
        break;
    }
    goto done8;
b98:
    if (m->animation != 8)
        func_00213DE0(m, 8, 0, func_001F9850(10));
    m->state = 10;
done8:
    if (d->f8)
        *(int *)&D_L15_00161B40 = 1;
tail:
    if (*(int *)&D_L15_00161B3C != D_L15_0015F6B0) {
        *(int *)&D_L15_00161B3C = D_L15_0015F6B0;
        func_L15_002CCFC0(m);
        *(int *)&D_L15_00161B40 = 0;
    }
}
