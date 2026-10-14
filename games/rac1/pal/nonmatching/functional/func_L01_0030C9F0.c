/* functional: high. Same reads, writes, calls and arguments in the same order as retail; the three divisions are guarded with the EE's FLT_MAX (PS2 floats never trap), the quad copies are written as four-word copies, and the registers and branch layout are the compiler's own. */
/* func_L01_0030C9F0 -- src/overlays/shared/vendor_002F7700.c (functional C for the port, not a match)
 * A camera/path moby of Novalis. m = the moby, d = m[0x78] its data: d[0..n) are n key points (16 bytes each,
 * n = d+0x58), d+0x50/0x54 the random-pick arguments, d+0x5C the trigger radius, d+0x60 a float stored
 * with the effect, d+0x68 the effect handle (-1 none), d+0x6C/0x70 the current segment length counters,
 * d+0x74/0x78 the two key indexes being blended, d+0x7C their direction flag, d+0x80/0x84 the clip volume
 * id and the id of a linked moby (state byte negative = it is gone).
 *  State 0: fewer than 2 points deletes the moby; otherwise scales every point by 1/256, clears the handle,
 *    raises m[0x30] to the truncated radius + 4 if lower, and goes to state 1.
 *  State 1: inside the radius (and clip volume) it runs the blend: when the timer (func_001F9908) fires it
 *    picks the next key point at random, advances d+0x78 modulo n and flips d+0x7C; the progress ratio
 *    drives a sine through func_L00_0025C710 to interpolate d+0x40 between the two points (and toward zero
 *    in the last quarter), then starts the effect (func_L00_0023F0D0, handle in d+0x68) or updates its
 *    record in the table at D_L01_001807C0. Outside the radius it ends the effect. A linked moby that is
 *    gone, or no points, ends the effect and deletes the moby. */

extern char D_L01_001672C0_30C9F0[] __asm__("D_L01_001672C0");
extern char D_L01_001807C0_30C9F0[] __asm__("D_L01_001807C0");
extern char *D_L01_00160058_30C9F0 __asm__("D_L01_00160058");

extern int func_001FA898_30C9F0(float) __asm__("func_001FA898");
extern float func_001F9D48_30C9F0(void *, void *) __asm__("func_001F9D48");
extern int func_00215570_30C9F0(void *, int) __asm__("func_00215570");
extern void func_L00_0023F1D0_30C9F0(int) __asm__("func_L00_0023F1D0");
extern void func_0020D678_30C9F0(void *) __asm__("func_0020D678");
extern int func_001F9908_30C9F0(void *) __asm__("func_001F9908");
extern int func_L00_00258BC8_30C9F0(int, int) __asm__("func_L00_00258BC8");
extern float func_001FA888_30C9F0(int) __asm__("func_001FA888");
extern float func_001F9FA8_30C9F0(float) __asm__("func_001F9FA8");
extern void func_L00_0025C710_30C9F0(void *, void *, void *, float) __asm__("func_L00_0025C710");
extern int func_L00_0023F0D0_30C9F0(void *, float, float, float, float, float) __asm__("func_L00_0023F0D0");

static float div_ee_30C9F0(float a, float b) {
    if (b == 0.0f) {
        return a < 0.0f ? -3.4028234663852886e38f : 3.4028234663852886e38f;
    }
    return a / b;
}

static void copy16_30C9F0(void *dst, const void *src) {
    int *o = (int *)dst;
    const int *i = (const int *)src;
    o[0] = i[0];
    o[1] = i[1];
    o[2] = i[2];
    o[3] = i[3];
}

void func_L01_0030C9F0(char *m) {
    char *d = *(char **)(m + 0x78);
    char *pos = m + 0x10;
    float dist, f20;
    int v;

    if (m[0x20] == 0) {
        int n = *(int *)(d + 0x58);
        int i;
        if (n < 2) {
            func_0020D678_30C9F0(m);
            return;
        }
        for (i = 0; i < n; i++) {
            float *p = (float *)(d + i * 16);
            p[0] = p[0] * 0.00390625f;
            p[1] = p[1] * 0.00390625f;
            p[2] = p[2] * 0.00390625f;
            p[3] = p[3] * 0.00390625f;
        }
        *(int *)(d + 0x68) = -1;
        {
            unsigned char c = (unsigned char)m[0x30];
            if (c < func_001FA898_30C9F0(*(float *)(d + 0x5C)) + 4) {
                m[0x30] = func_001FA898_30C9F0(*(float *)(d + 0x5C)) + 4;
            }
        }
        m[0x20] = 1;
        return;
    }
    if (m[0x20] != 1) {
        return;
    }

    dist = func_001F9D48_30C9F0(pos, D_L01_001672C0_30C9F0);
    f20 = 0.0f;
    if (dist < *(float *)(d + 0x5C)) {
        f20 = 1.0f;
    }
    if (f20 != 0.0f && *(int *)(d + 0x80) != -1) {
        if (func_00215570_30C9F0(D_L01_001672C0_30C9F0, *(int *)(d + 0x80)) == 0) {
            f20 = 0.0f;
        }
    }
    if (*(int *)(d + 0x84) != -1
        && D_L01_00160058_30C9F0[(*(int *)(d + 0x84) << 8) + 0x20] < 0) {
        if (*(int *)(d + 0x68) != -1) {
            func_L00_0023F1D0_30C9F0(*(int *)(d + 0x68));
        }
        func_0020D678_30C9F0(m);
        return;
    }

    if (f20 == 0.0f) {
        int h = *(int *)(d + 0x68);
        if (h != -1) {
            func_L00_0023F1D0_30C9F0(h);
            *(int *)(d + 0x68) = -1;
        }
        return;
    }

    if (func_001F9908_30C9F0(d + 0x6C) != 0) {
        int r = func_L00_00258BC8_30C9F0(*(int *)(d + 0x50), *(int *)(d + 0x54));
        int s;
        *(int *)(d + 0x74) = *(int *)(d + 0x78);
        *(int *)(d + 0x6C) = r;
        *(int *)(d + 0x70) = r;
        s = func_L00_00258BC8_30C9F0(1, *(int *)(d + 0x58) - 1);
        {
            int n = *(int *)(d + 0x58);
            int sum = *(int *)(d + 0x78) + s;
            *(int *)(d + 0x7C) = (*(int *)(d + 0x7C) == 0);
            *(int *)(d + 0x78) = n != 0 ? sum % n : 0;
        }
    }

    {
        float t, sn, f0, f1;
        char *out = d + 0x40;
        int a, b;
        unsigned int qa[4], qb[4];

        t = func_001FA888_30C9F0(*(int *)(d + 0x70) - *(int *)(d + 0x6C));
        t = div_ee_30C9F0(t, func_001FA888_30C9F0(*(int *)(d + 0x70)));
        t = t * 3.14159274101257f;
        v = *(int *)(d + 0x7C);
        sn = t * 0.5f;
        if (v != 0) {
            sn = sn + 1.57079637050629f;
        }
        sn = func_001F9FA8_30C9F0(sn);

        if (v != 0) {
            a = *(int *)(d + 0x78);
            b = *(int *)(d + 0x74);
        } else {
            a = *(int *)(d + 0x74);
            b = *(int *)(d + 0x78);
        }
        copy16_30C9F0(qa, d + a * 16);
        copy16_30C9F0(qb, d + b * 16);
        func_L00_0025C710_30C9F0(out, qa, qb, sn);

        f1 = *(float *)(d + 0x5C);
        f0 = f1 * 0.75f;
        if (f0 < dist) {
            f1 = f1 - f0;
            sn = div_ee_30C9F0(dist - f0, f1);
            qa[0] = qa[1] = qa[2] = qa[3] = 0;
            copy16_30C9F0(qb, out);
            func_L00_0025C710_30C9F0(out, qb, qa, sn);
        }
    }

    {
        int h = *(int *)(d + 0x68);
        if (h == -1) {
            *(int *)(d + 0x68) = func_L00_0023F0D0_30C9F0(pos, *(float *)(d + 0x60), *(float *)(d + 0x4C),
                                                         *(float *)(d + 0x40), *(float *)(d + 0x44),
                                                         *(float *)(d + 0x48));
        } else {
            char *e = D_L01_001807C0_30C9F0 + h * 32;
            copy16_30C9F0(e + 0x10, pos);
            *(float *)(e + 0x1C) = *(float *)(d + 0x60);
            copy16_30C9F0(e, d + 0x40);
        }
    }
}
