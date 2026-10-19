/* functional: medium. fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). libmpeg motion-compensation fetch setup (one 16x(h) luma and one chroma fetch descriptor for the current macroblock): splits the block by the 16-pixel (8 chroma) cache lines, fills the two fetch records at d+0x590+idx*0x140 (+0x48/+0xB8 + n*0x1C), the handler pointers (D_00132F30 / D_00132F50 by half-pel code) and the three reference addresses, then bumps the fetch count at d+0x6BC+idx*0x140. Written from the assembly; not run in the capture (hand-checked). */
extern int D_00132F30_25F88[] __asm__("D_00132F30");
extern int D_00132F50_25F88[] __asm__("D_00132F50");

void func_00125F88(struct MpegDecoder *d, struct MpegRefImage *ref, int sfield, int dfield, int yofs, int h,
                   int bx, int by, int dx, int dy, int fieldpred, int avg) {
    char *dd = (char *)d;
    int idx = *(int *)(dd + 0x810);
    int off = idx * 0x140;
    int mc_work = *(int *)(dd + 0x81C);
    int n = *(int *)(dd + 0x6BC + off);
    char *E = dd + 0x590 + off;
    int *fa = (int *)(E + n * 0x1C + 0x48);
    int *fb = (int *)(E + n * 0x1C + 0xB8);
    int t, r6, r8, r20, r21, S, low, bit8, bit9, code, p, base, v;
    int a8s, a9s, c6, c8, c10, c7, lowc, bitc8, bitc9, codeC, h2, r4c;
    int ent0;

    t = dy >> 1;
    if (fieldpred != 0) {
        t = t << 1;
    }
    r6 = t + by + (yofs + sfield);
    r8 = (dx >> 1) + bx;
    r20 = r8 >> 4;
    r21 = r6 >> 4;
    S = ref->unk10 * r20 + r21;
    fa[1] = r8 - (r20 << 4);
    fa[0] = mc_work + ((dfield + yofs) << 5);
    low = r6 - (r21 << 4);
    bit9 = dy & 1;
    bit8 = dx & 1;
    if (bit9 != 0) {
        if (low + (h << fieldpred) < 0x10) {
            fa[2] = h;
            fa[3] = 0;
        } else {
            v = ((0x10 >> fieldpred) - (low >> fieldpred)) - 1;
            fa[2] = v;
            fa[3] = h - v;
        }
    } else {
        if (low + (h << fieldpred) < 0x11) {
            fa[2] = h;
            fa[3] = 0;
        } else {
            v = (0x10 >> fieldpred) - (low >> fieldpred);
            fa[2] = v;
            fa[3] = h - v;
        }
    }
    ent0 = *(int *)(dd + 0x590 + off);
    base = ent0 + n * 0x600;
    a8s = dx + (int)((unsigned int)dx >> 31);
    a9s = dy + (int)((unsigned int)dy >> 31);
    code = (avg << 2) | (bit8 << 1) | bit9;
    fa[5] = base + (low << 4);
    fa[4] = 0x10 << fieldpred;
    fa[6] = base + (low << 4) + 0x300;

    /* chroma */
    h2 = h >> 1;
    c8 = (a8s >> 2) + (bx >> 1);
    bitc8 = (a8s >> 1) & 1;
    c7 = a9s >> 1;
    c6 = ((a9s >> 2) << (fieldpred != 0 ? 1 : 0)) + (by >> 1) + ((yofs >> 1) + sfield);
    c10 = c6 >> 3;
    p = c8 >> 3;
    fb[0] = mc_work + (((dfield + (yofs >> 1)) << 4) + 0x200);
    lowc = c6 - (c10 << 3);
    fb[1] = c8 - (p << 3);
    bitc9 = c7 & 1;
    if (bitc9 != 0) {
        if (lowc + (h2 << fieldpred) < 8) {
            fb[2] = h2;
            fb[3] = 0;
        } else {
            v = ((8 >> fieldpred) - (lowc >> fieldpred)) - 1;
            fb[2] = v;
            fb[3] = h2 - v;
        }
    } else {
        if (lowc + (h2 << fieldpred) < 9) {
            fb[2] = h2;
            fb[3] = 0;
        } else {
            v = (8 >> fieldpred) - (lowc >> fieldpred);
            fb[2] = v;
            fb[3] = h2 - v;
        }
    }
    fb[4] = 8 << fieldpred;
    r4c = ((p - r20) << 1) + (c10 - r21);
    codeC = (avg << 2) | (bitc8 << 1) | bitc9;
    *(int *)(dd + n * 4 + off + 0x5B8) = D_00132F30_25F88[code];
    base = r4c * 0x180 + base;
    fb[5] = base + (lowc << 3) + 0x100;
    fb[6] = base + (lowc << 3) + 0x400;
    *(int *)(dd + n * 4 + off + 0x598) = (int)ref->data + S * 0x180;
    *(int *)(dd + n * 4 + off + 0x5C8) = D_00132F50_25F88[codeC];
    *(int *)(dd + n * 4 + off + 0x5A8) = (int)ref->data + (S + ref->unk10) * 0x180;
    *(int *)(dd + 0x6BC + off) = *(int *)(dd + 0x6BC + off) + 1;
}
