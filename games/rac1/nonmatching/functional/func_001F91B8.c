/* functional: medium. fcheck: unchecked. Hand-written first half of the optional extra pass (draw_debug_font): starts the toSPR DMA (channel 9) of 16 quadwords from D_0019C2C0 to scratchpad 0x2400 (the light list func_001F8B6C reads; MADR, SADR, QWC, then CHCR = 0x100, as retail; the transfer itself is the DMA controller's), advances the texture-page counter D_0015EF74 by 0x400, then for each of the D_0015F578 0x20-byte records at D_0015F57C (with the matching pointer of the array D_0015F580): clears rec+0x14, asks func_001FA8F0(rec, rec[4] + 16 - rec[3]) whether it is on screen; when it is (>= 0) it stores in rec+0x14 an alpha (255 when the camera-space depth z = (rec.xyz - camera position) . column 2 of the camera matrix D_00187040 is <= rec[4], else (int)(255 - (z - rec[4]) * 255/16)) with bit 31 set for a partial (0) result, the 64-bit TEX0 at rec+0x18 for the texture header at ptr->4 (TBP from D_0015EF74 >> 8, TBW 1 as retail's delay-slot zeroing leaves it, PSM 0x13, TW/TH from the header, TCC, CBP from the counter's earlier value, bit 63) after bumping the counter by 1 << (tw + th), and, while D_0015F558 >= -0x40, appends a texture upload entry {D_0018E840, 0, cbp, header + 0x10, tw, th, tbp} to D_0018D140 and bumps D_0015F558. Not reproduced: in the near branch retail ORs 0xFF into the caller's $s6 instead of $zero (taken as 0 here, as in the test); it also leaves vf30 = (1,1,1) and its jump returns through $t6. The record count must be non-zero (the caller checks it; retail's loop is a do-while). Differential test: 300 cases agree with retail run in an EE emulator (unicorn with an R5900 MMI/VU0 instruction model) to 1e-5 relative. */

extern unsigned int D_0015EF74;
extern int D_0015F578;
extern unsigned char *D_0015F57C;
extern unsigned char **D_0015F580;
extern int D_0015F558;
extern unsigned char D_00187040[];
extern unsigned char D_0018D140[];
extern int D_0018E840[];
extern unsigned char D_0019C2C0[];
extern int func_001FA8F0(float *p, float f);

#define ADD64_1F91B8(lo, hi, al, ah) do { unsigned int _s = (lo) + (al); (hi) += (ah) + (_s < (lo)); (lo) = _s; } while (0)

void func_001F91B8(void)
{
    unsigned int cnt;
    unsigned int cbp;
    unsigned int tbp;
    int n;
    unsigned char *rec;
    unsigned char **pp;
    const float *cam = (const float *)D_00187040;
    float step = 255.0f / 16.0f;

    *(volatile unsigned int *)0x1000D410 = (unsigned int)D_0019C2C0;
    *(volatile unsigned int *)0x1000D480 = 0x2400;
    *(volatile unsigned int *)0x1000D420 = 0x10;
    *(volatile unsigned int *)0x1000D400 = 0x100;

    cnt = D_0015EF74;
    cbp = cnt >> 8;
    D_0015EF74 = cnt + 0x400;

    n = D_0015F578;
    rec = D_0015F57C;
    pp = D_0015F580;
    do {
        unsigned char *ent = *pp;
        float *r = (float *)rec;
        float lim = r[4];
        int vis;

        vis = func_001FA8F0(r, (lim + 16.0f) - r[3]);
        *(int *)(rec + 0x14) = 0;
        if (vis >= 0) {
            float dx = r[0] - cam[0x50];
            float dy = r[1] - cam[0x51];
            float dz = r[2] - cam[0x52];
            float z = ((cam[2] * dx + cam[6] * dy) + cam[10] * dz);
            unsigned int a;
            int *hdr;
            unsigned int lo, hi;
            int k;

            if (z <= lim) {
                a = 0xFF;
            } else {
                a = (unsigned int)(int)(255.0f - (z - lim) * step);
            }
            if (vis == 0) {
                a |= 0x80000000u;
            }
            *(unsigned int *)(rec + 0x14) = a;

            cnt = D_0015EF74;
            tbp = cnt >> 8;
            hdr = *(int **)(ent + 4);
            D_0015EF74 = cnt + (1u << ((hdr[1] + hdr[0]) & 31));

            /* TEX0 as retail's 64-bit sum: tbp + (1 << 14) + (0x13 << 20) + (s64)tw << 26
               + (s64)th << 30 + (1 << 34) + cbp << 37 + (1 << 63), in two 32-bit halves. */
            lo = tbp;
            hi = 0;
            ADD64_1F91B8(lo, hi, 1u << 14, 0);
            ADD64_1F91B8(lo, hi, 0x13u << 20, 0);
            ADD64_1F91B8(lo, hi, (unsigned int)hdr[0] << 26, (unsigned int)(hdr[0] >> 6));
            ADD64_1F91B8(lo, hi, (unsigned int)hdr[1] << 30, (unsigned int)(hdr[1] >> 2));
            hi += 4u;
            hi += cbp << 5;
            hi += 0x80000000u;
            *(unsigned int *)(rec + 0x18) = lo;
            *(unsigned int *)(rec + 0x1C) = hi;

            k = D_0015F558;
            if (k + 0x40 >= 0) {
                unsigned char *e = D_0018D140 + (k << 4);

                *(unsigned int *)(e + 0) = (unsigned int)D_0018E840;
                *(unsigned short *)(e + 4) = 0;
                *(unsigned short *)(e + 6) = (unsigned short)cbp;
                *(unsigned int *)(e + 8) = (unsigned int)hdr + 0x10;
                e[0xC] = (unsigned char)hdr[0];
                e[0xD] = (unsigned char)hdr[1];
                *(unsigned short *)(e + 0xE) = (unsigned short)tbp;
                D_0015F558 = k + 1;
            }
        }
        rec += 0x20;
        pp++;
    } while (--n != 0);
}
