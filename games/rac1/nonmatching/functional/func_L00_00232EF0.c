/* RatchetAnimAdvance: one tick of Ratchet's animation (the moby at
   D_0013F450 + 0x2080). Clears the key flags (+0xA98 bits 0/1), advances
   the blend t (+0x54) by speed * rate on the same sequence, by the next
   value of the eased curve table (+0xAA0 curve, +0xAA4 position) or by
   the rate during a blend; snaps t near 1 and near 0; then steps keys
   while t >= 1 (queued jumps +0xAB8 / +0xAB0..AB4, wrap at the sequence's
   frame count, the new key's rate word becomes +0xA94). Afterwards plays
   the sequence's frame sounds crossed this tick, keeps or releases the
   loop sound (+0x7C / +0x7D), syncs the two helper animations, updates
   the key time (+0xAA8) and its step (+0xAAC) and sets +0xA9C to
   "blending".
   Port copy of nonmatching/game/func_L00_00232EF0.c.
   Adapted from ReRAC (crates/rc-game/src/hero/anim.rs, Ratchet's advance
   0x247d48; ISC License, Copyright (c) 2026 ReRAC contributors). */
extern float D_L00_0017BEF0[];
extern float D_L00_0015F7E8[];
extern int D_L00_0015F6A8_ea[] __asm__("D_L00_0015F6A8");
extern int func_0022ED80(int, int, void *);
extern void func_L00_0028EBF0(int);
extern float func_0020D830(void *);
extern void func_L00_00232B78(void);
extern void func_L00_00232980(void);
extern void func_L00_00232B20(void);

void func_L00_00232EF0(void) {
    char *g = D_0013E633 + 0xE1D;
    char *obj = *(char **)(g + 0x2080);
    unsigned char frameA0;
    int same;
    float t0;
    float t;
    float delta;
    float keytime;
    float old;

    /* Retail's first call is to the 20-byte routine at 0x232A00, which the
       function list folds into func_L00_00211F68 by its fingerprint but
       which clears +0x2278 and +0x227C, not the target speed and speed. */
    *(int *)(g + 0x227C) = 0;
    *(int *)(g + 0x2278) = 0;
    *(int *)(g + 0xA98) &= ~3;
    frameA0 = *(unsigned char *)(obj + 0x50);
    t0 = *(float *)(obj + 0x54);
    same = *(unsigned char *)(obj + 0x52) == *(unsigned char *)(obj + 0x53);
    if (same) {
        *(float *)(obj + 0x54) = t0 + *(float *)(g + 0xA90) * *(float *)(g + 0xA94);
    } else if (*(int *)(g + 0xAA0) >= 0) {
        int pos = *(int *)(g + 0xAA4);
        *(float *)(obj + 0x54) = *(float *)((char *)D_L00_0017BEF0 + pos * 4 + *(int *)(g + 0xAA0) * 100);
        *(int *)(g + 0xAA4) = pos + 1;
    } else {
        *(float *)(obj + 0x54) = t0 + *(float *)(g + 0xA94);
    }

    t = *(float *)(obj + 0x54);
    if (0.99f < t && t < 1.01f) {
        *(float *)(obj + 0x54) = 1.0f;
        t = *(float *)(obj + 0x54);
    }
    if (-0.01f < t && t < 0.01f) {
        *(float *)(obj + 0x54) = 0.0f;
    }
    t = *(float *)(obj + 0x54);
    delta = t - t0;

    if (1.0f <= t) {
        float qrate = D_L00_0015F7E8[0];
        do {
            unsigned char sb = *(unsigned char *)(obj + 0x53);
            unsigned char fb;
            char *bank;

            if (*(unsigned char *)(obj + 0x52) != sb) {
                *(unsigned char *)(obj + 0x52) = sb;
                *(unsigned char *)(obj + 0x7E) =
                    *(unsigned char *)(*(char **)(*(char **)(obj + 0x24) + 0x48 + sb * 4) + 0x12);
            }
            *(int *)(g + 0xAA0) = -1;
            *(int *)(g + 0xA98) |= 1;
            fb = *(unsigned char *)(obj + 0x51);
            *(unsigned char *)(obj + 0x50) = fb;
            *(int *)(obj + 0x68) = *(int *)(obj + 0x6C);
            *(unsigned char *)(obj + 0x51) = fb + 1;
            if (*(int *)(g + 0xAB8) != 0) {
                unsigned char nf;
                *(int *)(g + 0xAB8) = 0;
                *(float *)(g + 0xA94) = qrate;
                *(float *)(obj + 0x54) = 0.0f;
                nf = *(unsigned char *)(g + 0xAB4);
                *(unsigned char *)(obj + 0x51) = nf;
                bank = *(char **)(*(char **)(obj + 0x24) + 0x48 + *(unsigned char *)(obj + 0x53) * 4);
                *(int *)(obj + 0x6C) = *(int *)(bank + 0x1C + nf * 4);
                func_L00_00232EA8();
                func_L00_00232B78();
            } else if (*(int *)(g + 0xAB0) != -1
                       && *(int *)(g + 0xAB4) < (int)*(unsigned char *)(obj + 0x51)) {
                unsigned char nf;
                *(float *)(g + 0xA94) = 0.33333334f;
                *(float *)(obj + 0x54) = 0.0f;
                nf = *(unsigned char *)(g + 0xAB0);
                *(unsigned char *)(obj + 0x51) = nf;
                bank = *(char **)(*(char **)(obj + 0x24) + 0x48 + *(unsigned char *)(obj + 0x53) * 4);
                *(int *)(obj + 0x6C) = *(int *)(bank + 0x1C + nf * 4);
                func_L00_00232B78();
            } else {
                float nt;
                char *prev;
                float r;

                bank = *(char **)(*(char **)(obj + 0x24) + 0x48 + *(unsigned char *)(obj + 0x53) * 4);
                if (*(unsigned char *)(obj + 0x51) >= *(unsigned char *)(bank + 0x10)) {
                    *(unsigned char *)(obj + 0x51) = 0;
                    *(int *)(g + 0xA98) |= 2;
                }
                bank = *(char **)(*(char **)(obj + 0x24) + 0x48 + *(unsigned char *)(obj + 0x53) * 4);
                nt = *(float *)(obj + 0x54) - 1.0f;
                prev = *(char **)(obj + 0x68);
                *(float *)(obj + 0x54) = nt;
                *(int *)(obj + 0x6C) = *(int *)(bank + 0x1C + *(unsigned char *)(obj + 0x51) * 4);
                nt = nt / *(float *)(g + 0xA94);
                *(float *)(obj + 0x54) = nt;
                r = *(float *)prev;
                *(float *)(g + 0xA94) = r;
                *(float *)(obj + 0x54) = *(float *)(obj + 0x54) * r;
            }
            t = *(float *)(obj + 0x54);
        } while (1.0f <= t);
    }

    /* Frame sounds of the sequence whose key time passed this tick. */
    if (same && *(unsigned char *)(obj + 0x7E) != 0) {
        int cur;
        int prev;
        int n;
        int i;
        char *bank;
        int *p;

        cur = func_001FA898(t * 16.0f);
        cur = *(unsigned char *)(obj + 0x50) * 16 + cur;
        prev = frameA0 * 16 + func_001FA898(t0 * 16.0f);
        bank = *(char **)(*(char **)(obj + 0x24) + 0x48 + *(unsigned char *)(obj + 0x52) * 4);
        p = (int *)(bank + 0x1C + *(unsigned char *)(bank + 0x10) * 4);
        for (i = 0; i < *(unsigned char *)(bank + 0x12); i++, p++) {
            int w = *p;
            int k = w >> 16;
            if (prev < k && !(cur < k)) {
                func_0022ED80(w & 0xFFFF, 0, obj);
                break;
            }
        }
    }

    /* Loop sound: release it when the sequence wants another one, start
       the one it wants when none is playing. */
    {
        unsigned char v = *(unsigned char *)(obj + 0x7D);
        if (v != 0xFF) {
            char *e = D_0013E633 + 0x1D + v * 0x70;
            if (*(char **)(e + 0x88) != obj) {
                *(unsigned char *)(obj + 0x7D) = 0xFF;
            } else if (*(short *)(e + 0x7E) != *(unsigned char *)(obj + 0x7C)) {
                func_L00_0028EBF0(v);
                *(unsigned char *)(obj + 0x7D) = 0xFF;
            }
        } else if (*(unsigned char *)(obj + 0x7C) != 0xFF
                   && D_L00_0015F6A8_ea[0] != 2 && D_L00_0015F6A8_ea[0] != 6) {
            *(unsigned char *)(obj + 0x7D) = func_0022ED80(*(unsigned char *)(obj + 0x7C), 4, obj);
        }
    }

    func_L00_00232980();
    func_L00_00232B20();
    old = *(float *)(g + 0xAA8);
    keytime = func_0020D830(obj);
    *(float *)(g + 0xAA8) = keytime;
    if (old <= keytime) {
        *(float *)(g + 0xAAC) = keytime - old;
    } else {
        *(float *)(g + 0xAAC) = delta;
    }
    *(int *)(g + 0xA9C) = *(unsigned char *)(obj + 0x52) != *(unsigned char *)(obj + 0x53);
}
