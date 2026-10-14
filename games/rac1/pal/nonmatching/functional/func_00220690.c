/* functional: high. A pause-style list screen: title, the menu's rows (box size at +0x20/+0x24, rows at +0x30) with the selection highlighted, an optional footer; returns 2. Not a match (46 of 612 bytes off, register choices only); equiv.py FUNCTIONAL. */
extern int D_001A01F0[];
extern void func_001F4630(int);
extern void func_001F4748(void);
extern int func_0020CA50(void *, void *, void *, int);
extern void func_00234C98(int, int);
extern void func_001F6598(void);
extern void func_001F65A8(void);
extern void func_001153FC(void *, int, int);
extern void func_001F75D0_b(unsigned short *, long, void *, int) __asm__("func_001F75D0");
extern void func_002208F8(int, int, int);
extern void func_001F68E8_c(int, int, long, void *, int) __asm__("func_001F68E8");
extern void *func_001FE540_id(int) __asm__("func_001FE540");

typedef struct { unsigned short f[12]; } TextBox_p;

/* Draws a pause-style list of text rows with a selection marker and a footer line; returns 2. */
int func_00220690(char *arg) {
    TextBox_p t;
    TextBox_p s;
    int mask;
    int n;
    int i;
    int y;
    int *list;
    int *ids;

    y = 0x18;
    i = 0;
    mask = 0;
    func_001F4630(0);
    n = func_0020CA50((void *)0x70000000, &mask, (void *)0, 0);
    func_00234C98(0x42, 0x44);
    func_00234C98(0x47, 0x10B);
    func_001F68E8_c(4, 4, 0x80FFA888L, func_001FE540_id(0x4F59), -1);
    func_001153FC(s.f, 0, 0x18);
    s.f[1] = *(unsigned short *)(arg + 0x24);
    s.f[3] = *(unsigned short *)(arg + 0x20);
    s.f[8] = 0x10;
    t = s;
    if (n > 0) {
        list = (int *)(arg + 0x30);
        ids = (int *)0x70000000;
        do {
            int sel = (list[D_001A01F0[0x89]] ^ i) == 0;
            int color = 0x80FFA888;
            int h;
            int alt = y + 0xA;
            if (sel) color = 0x8020FFFF;
            if (sel) func_001F65A8();
            t.f[5] = y;
            t.f[4] = 0x10;
            func_001F75D0_b(t.f, color, func_001FE540_id(*ids), -1);
            if (sel) func_001F6598();
            h = (short)t.f[7];
            h = h / (short)t.f[8];
            ids++;
            func_002208F8(9, (h == 2) ? y + 0x10 : alt, (mask >> i) & 1);
            y += h * 16;
            i++;
        } while (i < n);
    }
    if (mask < 0) {
        t.f[9] = 1;
        t.f[5] = y + 8;
        t.f[4] = *(int *)(arg + 0x24) >> 1;
        func_001F75D0_b(t.f, 0x8020FFFFL, func_001FE540_id(0x5242), -1);
    }
    func_001F4748();
    return 2;
}
