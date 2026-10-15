/* functional: medium. Draws a HUD info panel (frame box, two lines of text with a one pixel shadow, right-aligned counters, and on level-index 0x10 an extra icon box with a number) for the hud element whose word 0x6C (frames left) is positive; returns 0 when it is not, else the element's word 0x58. Not checked on real calls. */
extern int D_0015EE80_66D10 __asm__("D_0015EE80") __attribute__((section(".sdata")));
extern int D_0015EE84_66D10 __asm__("D_0015EE84") __attribute__((section(".sdata")));
extern char D_0013E15A_66D10[] __asm__("D_0013E15A");
extern char D_0013D355_66D10[] __asm__("D_0013D355");
extern char D_0013F450_66D10[] __asm__("D_0013E633");
extern int D_L05_0015FA5C_66D10 __asm__("D_L05_0015FA5C") __attribute__((section(".sdata")));
extern int D_L05_0015FA60_66D10 __asm__("D_L05_0015FA60") __attribute__((section(".sdata")));
extern int D_L05_0015FA64_66D10 __asm__("D_L05_0015FA64") __attribute__((section(".sdata")));
extern int D_L05_0015FA68_66D10 __asm__("D_L05_0015FA68") __attribute__((section(".sdata")));
extern int D_L05_0015FA6C_66D10 __asm__("D_L05_0015FA6C") __attribute__((section(".sdata")));
extern int D_L05_0015FA70_66D10 __asm__("D_L05_0015FA70") __attribute__((section(".sdata")));
extern int D_L05_0015FA74_66D10 __asm__("D_L05_0015FA74") __attribute__((section(".sdata")));
extern int D_L05_0015FA78_66D10 __asm__("D_L05_0015FA78") __attribute__((section(".sdata")));
extern int D_L05_0015FA7C_66D10 __asm__("D_L05_0015FA7C") __attribute__((section(".sdata")));
extern int D_L05_0015FA88_66D10 __asm__("D_L05_0015FA88") __attribute__((section(".sdata")));
extern int D_L05_0015FA98_66D10 __asm__("D_L05_0015FA98") __attribute__((section(".sdata")));
extern int D_L05_0015FA9C_66D10 __asm__("D_L05_0015FA9C") __attribute__((section(".sdata")));
extern int D_L05_0015FAA0_66D10 __asm__("D_L05_0015FAA0") __attribute__((section(".sdata")));
extern char D_L05_0015FAB0_66D10[] __asm__("D_L05_0015FAB0");
extern char D_L05_0015FAB8_66D10[] __asm__("D_L05_0015FAB8");
extern char D_L05_0015F9B8_66D10[] __asm__("D_L05_0015F9B8");
extern char D_L05_0015FAC0_66D10[] __asm__("D_L05_0015FAC0");
extern char D_L05_0015F940_66D10[] __asm__("D_L05_0015F940");
extern void func_00201960_66D10(int, int, int, int, int) __asm__("func_00201960");
extern char *func_001FE540_66D10(int) __asm__("func_001FE540");
extern int func_00116248_66D10(char *, char *, ...) __asm__("func_00116248");
extern void func_L00_001FB7F8_66D10(int, int, int, char *, int) __asm__("func_L00_001FB7F8");
extern int func_001F6600_66D10(char *, int) __asm__("func_001F6600");
extern int func_00200198_66D10(int, int) __asm__("func_00200198");
extern void func_00200468_66D10(int, int, int, int, int, int) __asm__("func_00200468");

int func_L05_00266D10(char *e) {
    char buf[0x80] __attribute__((aligned(16)));
    int x, y, y1, tx, px, py, w;
    int pal;
    char *hd;
    char *t;

    if (*(int *)(e + 0x6C) <= 0)
        return 0;
    pal = D_0015EE80_66D10;
    x = *(int *)(D_0013E15A_66D10 + 0x4A6 + 4) - (D_L05_0015FA60_66D10 + (pal ? 0xA : 0x12));
    y = *(int *)(D_0013E15A_66D10 + 0x4A6 + 4) - (D_L05_0015FA7C_66D10 + (pal ? 0xA : 0x12));
    hd = D_0013F450_66D10 + 0xE1D;
    y1 = y + 1;
    func_00201960_66D10(D_L05_0015FA5C_66D10, x, D_L05_0015FA64_66D10, D_L05_0015FA68_66D10, D_L05_0015FA6C_66D10);
    tx = D_L05_0015FA78_66D10;
    t = func_001FE540_66D10(0x5244);
    func_00116248_66D10(buf, D_L05_0015FAB0_66D10, t);
    func_L00_001FB7F8_66D10(tx - 1, y1, 0x80000000, buf, -1);
    func_L00_001FB7F8_66D10(tx, y, D_L05_0015FA70_66D10, buf, -1);
    w = func_001F6600_66D10(buf, -1);
    tx = tx + w;
    {
        int n = *(short *)(hd + 0x89A) + 1;
        if (!(n < 4)) n = 3;
        func_00116248_66D10(buf, D_L05_0015FAB8_66D10, n);
    }
    func_L00_001FB7F8_66D10(tx - 1, y + 2, 0x80000000, buf, -1);
    func_L00_001FB7F8_66D10(tx, y, D_L05_0015FA74_66D10, buf, -1);
    {
        int n = *(int *)(hd + 0x8C0);
        int base;
        tx = D_L05_0015FA88_66D10;
        n = n - 1;
        if (!(n < 5)) n = 4;
        t = func_001FE540_66D10(n + 0x5229);
        func_00116248_66D10(buf, D_L05_0015F9B8_66D10, t);
    }
    w = func_001F6600_66D10(buf, -1);
    tx = tx - w;
    func_L00_001FB7F8_66D10(tx + 1, y1, 0x80000000, buf, -1);
    func_L00_001FB7F8_66D10(tx, y, D_L05_0015FA74_66D10, buf, -1);
    t = func_001FE540_66D10(0x5245);
    func_00116248_66D10(buf, D_L05_0015FAC0_66D10, t);
    w = func_001F6600_66D10(buf, -1);
    tx = tx - w;
    func_L00_001FB7F8_66D10(tx + 1, y1, 0x80000000, buf, -1);
    func_L00_001FB7F8_66D10(tx, y, D_L05_0015FA70_66D10, buf, -1);
    if (D_0015EE84_66D10 != 0x10)
        return *(int *)(e + 0x58);
    func_00116248_66D10(buf + 0x40, D_L05_0015F940_66D10, *(unsigned char *)(hd + 0x8CE));
    if (*(unsigned char *)(D_0013D355_66D10 + 0x13B) == 0) {
        px = *(int *)(e + 0x50);
        py = D_0015EE80_66D10 ? 0xA : 0x12;
    } else {
        px = D_L05_0015FA98_66D10;
        py = D_L05_0015FA9C_66D10;
    }
    func_00201960_66D10(px - 0x18, py, D_L05_0015FAA0_66D10, 0x20, 0x60);
    w = func_00200198_66D10(0x7558, 2);
    func_00200468_66D10(w, px, py, 0x20, 0x20, 0x80);
    func_L00_001FB7F8_66D10(px + 0x29, py + 8, 0x80000000, buf + 0x40, -1);
    if (*(unsigned char *)(hd + 0x8CE) != 0)
        func_L00_001FB7F8_66D10(px + 0x28, py + 7, 0x80E08060, buf + 0x40, -1);
    else
        func_L00_001FB7F8_66D10(px + 0x28, py + 7, 0x80202080, buf + 0x40, -1);
    return *(int *)(e + 0x58);
}
