/* functional: high. fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). libmpeg picture-size setup: derives the macroblock counts from the context's width/height (halved and paired when the interlace-style flag at +0x848 is set), and when the 16-aligned size changed, rebuilds the frame buffers: three allocations of w*h*3/2 bytes (64-aligned) from the region at +0x108, the nine plane descriptors, and their stored sizes. Same calls, arguments and order as retail. */
extern void func_0012BD50(void *);
extern unsigned int func_0012BD60(void *, char *, unsigned int, unsigned int);
extern void func_0012C8B0(unsigned int *, unsigned int *, unsigned int *, unsigned int *, unsigned int *, unsigned int *, unsigned int *, unsigned int *, unsigned int *, int, int, int, int, int);
extern int func_0012C4C0(void *, int, int);

void func_0012C608(void *arg0) {
    char *a = (char *)arg0;
    char *s = *(char **)(a + 0x40);
    int flag = *(int *)(s + 0x848);
    int v;
    int w16, h16, h2, size;
    char *reg;

    if (flag == 0) {
        *(int *)(s + 0x174) = 3;
        *(int *)(s + 0x17C) = 1;
        *(int *)(s + 0x144) = 5;
        *(int *)(s + 0x13C) = 1;
        *(int *)(s + 0x140) = 1;
        *(int *)(s + 0x188) = 1;
    }
    *(int *)(s + 0x12C) = (*(int *)(s + 0x124) + 15) >> 4;
    if (flag != 0 && *(int *)(s + 0x13C) == 0) {
        v = ((*(int *)(s + 0x128) + 31) >> 5) << 1;
    } else {
        v = (*(int *)(s + 0x128) + 15) >> 4;
    }
    *(int *)(s + 0x130) = v;
    h16 = v << 4;
    w16 = *(int *)(s + 0x12C) << 4;
    if (w16 == *(int *)(a + 0x0) && h16 == *(int *)(a + 0x4)) {
        return;
    }
    *(int *)(a + 0x4) = h16;
    *(int *)(a + 0x0) = w16;
    size = (int)((unsigned int)(w16 * (h16 * 0x180)) >> 8);
    h2 = h16 >> 1;
    reg = s + 0x108;
    func_0012BD50(reg);
    *(int *)(s + 0xFC) = (int)func_0012BD60(s, reg, size, 0x40);
    *(int *)(s + 0x100) = (int)func_0012BD60(s, reg, size, 0x40);
    *(int *)(s + 0x104) = (int)func_0012BD60(s, reg, size, 0x40);
    func_0012C8B0((unsigned int *)(s + 0x1E8), (unsigned int *)(s + 0x250), (unsigned int *)(s + 0x2B8),
                  (unsigned int *)(s + 0x320), (unsigned int *)(s + 0x388), (unsigned int *)(s + 0x3F0),
                  (unsigned int *)(s + 0x458), (unsigned int *)(s + 0x4C0), (unsigned int *)(s + 0x528),
                  *(int *)(s + 0xFC), *(int *)(s + 0x100), *(int *)(s + 0x104), w16, h16);
    func_0012C4C0(s + 0x1E8, w16, h16);
    func_0012C4C0(s + 0x250, w16, h16);
    func_0012C4C0(s + 0x2B8, w16, h16);
    func_0012C4C0(s + 0x320, w16, h2);
    func_0012C4C0(s + 0x388, w16, h2);
    func_0012C4C0(s + 0x3F0, w16, h2);
    func_0012C4C0(s + 0x458, w16, h2);
    func_0012C4C0(s + 0x4C0, w16, h2);
    func_0012C4C0(s + 0x528, w16, h2);
}
