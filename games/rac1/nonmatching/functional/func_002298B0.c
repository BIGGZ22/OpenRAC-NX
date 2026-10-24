/* functional: high. fcheck: unchecked. Appends the shadow-volume GIF packets for one pass (emit_selected_graphics_dma_packets; mode = 2 or 1, tex = the word for the GIF tag's first register) to the DMA list at D_00161000, from the projected ring func_00228D20 left in the scratchpad (D_001604A0 pairs at 0x70000000, side bytes at 0x70003000, per-point clip bytes at 0x70003800). Sides: every edge i < count-1 whose side byte equals mode gets a 0x50-byte quad (a 0/0x54 register-pair qword and the two pairs i, i+1), behind a 0x40-byte header (DMA cnt + DIRECT, GIF tags with the quad count | 0x8000, 0x5252520F register list) that is written even when no side matches. Caps: the first ring (pair element 0) is drawn as one fan when the sign of D_001604A8[0] (the ring's winding) times -1 for mode 2, +1 otherwise is clear; the second ring (element 1) when D_001604A8[1] times that sign is <= 0 (zero, a VU0-flushed denormal, or negative); each copies the unclipped points (clip byte 0) behind a 0x40-byte header and is dropped entirely when none are left. Nothing is done when D_001604A0 is 1. Differential test: 80 cases agree with retail run in an EE emulator (unicorn with an R5900 MMI/VU0 instruction model). */
extern int D_001604A0;
extern float D_001604A8[2];
extern unsigned int *D_00161000;

static unsigned int fbits_002298B0(float f)
{
    union {
        float f;
        unsigned int u;
    } u;
    u.f = f;
    return u.u;
}

static void copy_qw_002298B0(unsigned int *d, const unsigned int *s)
{
    d[0] = s[0];
    d[1] = s[1];
    d[2] = s[2];
    d[3] = s[3];
}

/* One cap: the ring element `elem` (0 or 1) of every pair whose clip byte is 0, as one GIF fan. */
static void cap_002298B0(int elem, unsigned int tex)
{
    unsigned int *h = D_00161000;
    unsigned int *d = h + 16;
    const unsigned int *src = (const unsigned int *)0x70000000 + elem * 4;
    const signed char *clip = (const signed char *)0x70003800 + elem;
    int n = D_001604A0;
    int count = 0;

    do {
        if (*clip == 0) {
            copy_qw_002298B0(d, src);
            d += 4;
            count++;
        }
        clip += 2;
        src += 8;
    } while (--n != 0);
    if (count == 0) {
        return;
    }
    h[0] = 0x10000000 | (count + 3);
    h[1] = 0;
    h[2] = 0;
    h[3] = 0x50000000 | (count + 3);
    h[4] = 1;
    h[5] = 0x24000000;
    h[6] = 1;
    h[7] = 0;
    h[8] = tex;
    h[9] = 0x3F800000;
    h[10] = 0x55;
    h[11] = 0;
    h[12] = count | 0x8000;
    h[13] = 0x24000000;
    h[14] = 0x52;
    h[15] = 0;
    D_00161000 = h + (count + 4) * 4;
}

void func_002298B0(int mode, int tex)
{
    unsigned int *h;
    unsigned int *d;
    const unsigned int *src = (const unsigned int *)0x70000000;
    const signed char *side = (const signed char *)0x70003000;
    int n = D_001604A0 - 1;
    int count = 0;
    unsigned int flip;
    unsigned int w;
    int qwc;

    if (n == 0) {
        return;
    }
    h = D_00161000;
    d = h + 16;
    do {
        if (*side == mode) {
            d[0] = 0;
            d[1] = 0;
            d[2] = 0x54;
            d[3] = 0;
            copy_qw_002298B0(d + 4, src);
            copy_qw_002298B0(d + 8, src + 4);
            copy_qw_002298B0(d + 12, src + 8);
            copy_qw_002298B0(d + 16, src + 12);
            d += 20;
            count++;
        }
        side++;
        src += 8;
    } while (--n != 0);
    qwc = (int)(((unsigned int)((char *)d - (char *)h) >> 4) - 1);
    h[0] = 0x10000000 | qwc;
    h[1] = 0;
    h[2] = 0;
    h[3] = 0x50000000 | qwc;
    h[4] = 1;
    h[5] = 0x14000000;
    h[6] = 0xF1;
    h[7] = 0;
    h[8] = tex;
    h[9] = 0x3F800000;
    h[10] = 0;
    h[11] = 0;
    h[12] = count | 0x8000;
    h[13] = 0xA4000000;
    h[14] = 0x5252520F;
    h[15] = 0x52;
    D_00161000 = d;

    /* The sign of D_001604A8[k] * (mode == 2 ? -1 : 1), as VU0 computes it: the sign bits xor, and a
       zero or denormal (flushed) value is a signed zero. */
    flip = (mode == 2) ? 0x80000000u : 0;
    w = fbits_002298B0(D_001604A8[0]) ^ flip;
    if ((w & 0x80000000u) == 0) {
        cap_002298B0(0, tex);
    }
    w = fbits_002298B0(D_001604A8[1]) ^ flip;
    if ((w & 0x80000000u) != 0 || (w & 0x7F800000u) == 0) {
        cap_002298B0(1, tex);
    }
}
