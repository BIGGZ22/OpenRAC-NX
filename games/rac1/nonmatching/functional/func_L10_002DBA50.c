/* functional: medium. Draws a moby's grid of d->4 (at most 10) animated quads: builds one transform per element, then for each of 4 rows and 19 columns fills a GS packet (uv scrolled by D_L10_00161DE0, colour blended from the heights in the neighbouring moby's data by D_L10_00161DE4/8, alpha faded with distance to D_L10_001672C0) and draws it with func_L00_001FD1D8. Not run against real calls; the packet header words are rebuilt from the globals gp-0x5010..-0x5004 and func_001F4868(14). */
typedef struct Pkt_DBA50 {
    f32 uv1[8];
    u64 hdr1[4];
    f32 pos[16];
    u32 col[4];
    f32 uv2[8];
    u64 hdr2[4];
} Pkt_DBA50;

extern u8 *D_L10_00160058_DBA50 __asm__("D_L10_00160058") MACRO_ADDR;
extern u8 D_L10_001672C0_DBA50[] __asm__("D_L10_001672C0");
extern f32 D_L10_00161DE0_DBA50 __asm__("D_L10_00161DE0") MACRO_ADDR;
extern s32 D_L10_00161DE4_DBA50 __asm__("D_L10_00161DE4") MACRO_ADDR;
extern s32 D_L10_00161DE8_DBA50 __asm__("D_L10_00161DE8") MACRO_ADDR;
extern f32 D_L10_001DA2C0_DBA50[] __asm__("D_L10_001DA2C0");
extern f32 D_L10_001DA2E0_DBA50[] __asm__("D_L10_001DA2E0");
extern s32 D_L10_00161CF0_DBA50 SDATA(D_L10_00161CF0);
extern s32 D_L10_00161CF4_DBA50 SDATA(D_L10_00161CF4);
extern s32 D_L10_00161CF8_DBA50 SDATA(D_L10_00161CF8);
extern s32 D_L10_00161CFC_DBA50 SDATA(D_L10_00161CFC);
extern s32 D_L10_00161D10_DBA50 SDATA(D_L10_00161D10);
extern s32 D_L10_00161D14_DBA50 SDATA(D_L10_00161D14);
extern f32 D_L10_00161D18_DBA50 SDATA(D_L10_00161D18);
extern f32 func_001F9D10_DBA50(void *, void *) __asm__("func_001F9D10");
extern void func_001FA1C0_DBA50(void *, f32) __asm__("func_001FA1C0");
extern void func_001FA218_DBA50(void *, void *) __asm__("func_001FA218");
extern void func_001FA540_DBA50(void *, void *, void *) __asm__("func_001FA540");
extern f32 func_001FA888_DBA50(s32) __asm__("func_001FA888");
extern s32 func_001FA898_DBA50(f32) __asm__("func_001FA898");
extern s32 func_001FA8A8_DBA50(s32, s32, f32) __asm__("func_001FA8A8");
extern f32 func_001F9878_DBA50(f32) __asm__("func_001F9878");
extern u64 func_001F4868_DBA50(s32) __asm__("func_001F4868");
extern void func_001F9BD8_DBA50(void *, void *, void *) __asm__("func_001F9BD8");
extern void func_L00_001FD1D8_DBA50(void *, void *, s32) __asm__("func_L00_001FD1D8");

void func_L10_002DBA50(u8 *m) {
    u8 R[16 * 0x40] __attribute__((aligned(16)));
    f32 rot[16] __attribute__((aligned(16)));
    Pkt_DBA50 pkt __attribute__((aligned(16)));
    u8 *d;
    u8 *W;
    f32 dist;
    f32 fade;
    f32 f21;
    f32 f20;
    f32 t;
    u64 w0;
    u64 g;
    u64 h;
    s32 i;
    s32 j;
    s32 k;
    s32 n;
    s32 o;
    s32 idx;
    s32 v;
    s32 color;
    s32 alpha;
    s32 count;

    d = *(u8 **)(m + 0x78);
    W = *(u8 **)(D_L10_00160058_DBA50 + (*(s32 *)(d + 0xC) << 8) + 0x78);
    count = *(s32 *)(d + 4);
    dist = func_001F9D10_DBA50(m + 0x10, D_L10_001672C0_DBA50);
    if (48.0f < dist)
        fade = (80.0f - dist) * 0.03125f;
    else
        fade = 1.0f;

    func_001FA1C0_DBA50(rot, *(f32 *)d * 0.5f);
    i = 0;
    if (count > 0) {
        do {
            u8 *Ri = R + i * 0x40;
            func_001FA218_DBA50(Ri, m + 0x40);
            func_001FA540_DBA50(Ri, Ri, rot);
            qcopy(Ri + 0x30, m + 0x10);
            i++;
            t = func_001FA888_DBA50(i);
            *(f32 *)(Ri + 0x38) = *(f32 *)(Ri + 0x38) + t * *(f32 *)(d + 8);
        } while (i < *(s32 *)(d + 4));
    }

    w0 = func_001F4868_DBA50(0xE);
    g = (u64)(s64)D_L10_00161CF0_DBA50;
    g |= (u64)(s64)D_L10_00161CF4_DBA50 << 2;
    g |= (u64)(s64)D_L10_00161CF8_DBA50 << 4;
    g |= (u64)(s64)D_L10_00161CFC_DBA50 << 6;
    g |= (u64)0x8000 << 24;
    h = ((u64)0xFF90 << 32) | 0x260;
    pkt.hdr1[1] = w0;
    pkt.hdr2[1] = w0;
    pkt.hdr1[2] = h;
    pkt.hdr2[2] = h;
    pkt.hdr1[3] = g;
    pkt.hdr2[3] = g;
    pkt.hdr2[0] = 0;
    pkt.hdr1[0] = 0;
    for (j = 0; j < 4; j++) {
        f32 u = D_L10_001DA2C0_DBA50[j * 2 + 1];
        pkt.uv2[j * 2 + 1] = u;
        pkt.uv1[j * 2 + 1] = u;
    }

    for (o = 0; o < 4; o++) {
        f21 = 0.0f;
        for (n = 0; n < 19; n++) {
            f21 = f21 + D_L10_00161DE0_DBA50;
            if (1.0f < f21)
                f21 = f21 - 1.0f;
            for (j = 0; j < 4; j++) {
                f32 a = D_L10_001DA2C0_DBA50[j * 2] + f21;
                a = a + *(f32 *)(W + 0x12DE0);
                pkt.uv2[j * 2] = a;
                pkt.uv1[j * 2] = a;
            }
            if (*(s32 *)(d + 4) > 0) {
                k = 0;
                do {
                    idx = *(u8 *)(d + 0x10 + k);
                    v = *(s32 *)(W + 0x12C00 + idx * 4 + o * 0x78);
                    if (D_L10_00161DE8_DBA50 < v) {
                        f20 = func_001FA888_DBA50(v - D_L10_00161DE8_DBA50);
                        t = f20 / func_001FA888_DBA50(D_L10_00161DE4_DBA50 - D_L10_00161DE8_DBA50);
                    } else {
                        f20 = func_001FA888_DBA50(v);
                        f20 = f20 / func_001FA888_DBA50(D_L10_00161DE8_DBA50);
                        t = 1.0f - f20;
                    }
                    color = func_001FA8A8_DBA50(D_L10_00161D10_DBA50, D_L10_00161D14_DBA50, t);
                    t = func_001FA888_DBA50(color >> 24);
                    alpha = func_001FA898_DBA50(fade * fade * t);
                    color = (color & 0xFFFFFF) + (s32)((u32)alpha << 24);
                    if (*(u8 *)(m + 0x20) == 2) {
                        f20 = func_001FA888_DBA50(*(s32 *)(d + 0x34));
                        t = func_001F9878_DBA50(D_L10_00161D18_DBA50 * 60.0f);
                        f20 = f20 * 255.0f;
                        alpha = func_001FA898_DBA50(f20 / t);
                        color = (s32)((u32)alpha << 24) | 0xFFFFFF;
                    }
                    pkt.col[0] = color;
                    pkt.col[1] = color;
                    pkt.col[2] = color;
                    pkt.col[3] = color;
                    for (j = 0; j < 4; j++) {
                        func_001F9BD8_DBA50(&pkt.pos[j * 4], &D_L10_001DA2E0_DBA50[j * 4],
                                            W + o * 0x2580 + (n + (j >> 1)) * 0x140 + idx * 16);
                    }
                    func_L00_001FD1D8_DBA50(pkt.pos, R + k * 0x40, 0);
                    k++;
                } while (k < *(s32 *)(d + 4));
            }
        }
    }
}
