/* NON_MATCHING func_L00_002862E0 -- src/overlays/shared/pause_00277208.c
 * Best so far: SIZE ours 456 / retail 436, checked 2026-10-12.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   p4.c is the closest (size 436 = retail; BYTES 259): qcopy dst written as D_0013E633 + 0xE9D (not via a g varia
 *   2-case `switch (k)` after `if (k == 0 || k == 3) goto done;`, `s = 0` before it, and D_L00_001BB5C0[0x11]/[0x1
 *   directly in the found path (p is dead by then).
 *   Remaining difference: gcc derives D_0013E633+0x1D (b) from D+0xE9D with `addu $4,-0xE80` and keeps D+0xE1D in 
 *   reg, so ours uses $s0-$s3 (frame 0x50) where retail uses $s0-$s2 (frame 0x40) and builds b with its own lui/ad
 *   before/after the qcopy and a second alias array for b; no change. Would need the wording that stops that cse d
 *   w02/q30: with the file's own declarations (D_0013E633/D_0014171B/D_L00_001BB5C0 reached through __asm__ aliase
 *   Round hq3/s04: best.c as stored does not compile (its declarations clash with the file's); p10 form is the com
 */
extern char D_L00_001BA960[] NOT_SDA;
extern int D_L00_00160098 MACRO_ADDR;
extern int D_L00_0016009C MACRO_ADDR;
extern void func_001F99B0(void *, int, int);
extern void func_L00_001FF040(int, void *, unsigned short);
extern void func_001F9978(void);
extern void func_L00_002110C0(int, int);
extern void func_L00_00251E30(void *);
extern void func_001FA1F8(void *, void *);
extern void func_L00_001ED600(void);
extern void func_00216270(void);

/* loads the saved pause-menu state block into the game globals */
void func_L00_002862E0(void) {
    char *p = (char *)D_L00_001BB5C0;
    unsigned char *g;
    unsigned char *b;
    char *base;
    char *end;
    char *e;
    int k, s, i, o;
    if (D_L00_001BB5C0[0] == 0) {
        func_001F99B0(D_L00_001BA960, 0, 0xC60);
        return;
    }
    func_L00_001FF040((int)D_L00_001BA960, p, 0xC60);
    g = D_0013E633 + 0xE1D;
    qcopy(g + 0x80, p + 0x10);
    qcopy(g + 0x90, p + 0x20);
    *(int *)(*(char **)(g + 0x2080) + 0x38) = *(int *)(p + 0x30);
    *(int *)(*(char **)(g + 0x2080) + 0x3C) = *(int *)(p + 0x34);
    *(int *)(*(char **)(g + 0x2080) + 0x80) = *(int *)(p + 0x38);
    b = D_0013E633 + 0x1D;
    b[0x6B] |= 7;
    *(unsigned short *)(D_0014171B + 0x100ED) = *(unsigned short *)(p + 0xC54);
    *(int *)(b + 0x64) = *(int *)(p + 0x3C);
    b[0x68] = p[0x40];
    b[0x69] = p[0x41];
    b[0x6A] = p[0x42];
    k = *(int *)(p + 0x44);
    if (k == 0 || k == 3) goto done;
    s = 0;
    if (k == 1) s = 0x57;
    else if (k == 2) s = 0x1A3;
    else func_001F9978();
    base = (char *)D_L00_00160098;
    end = (char *)D_L00_0016009C;
    for (i = 0; ; i++) {
        o = i << 8;
        e = base + o;
        if (e >= end) goto done;
        if (*(short *)(e + 0xA6) == s) break;
    }
    func_L00_002110C0(D_L00_001BB5C0[0x11], D_L00_001BB5C0[0x13]);
done:
    g = D_0013E633 + 0xE1D;
    func_L00_00251E30(*(void **)(g + 0x2080));
    func_001FA1F8(*(char **)(g + 0x2080) + 0xC0, g + 0x90);
    func_L00_001ED600();
    func_00216270();
}
