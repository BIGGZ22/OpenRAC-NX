/* functional: medium (by reading; checked by call capture or static compare). Menu item preview animation driver: loads/streams the next queued preview animation, and when the head request is ready (trigger rule 0 immediate / 1 after anim ends / 2 after delay) copies it to the active slot, starts the animations on the source and item mobys and recreates the three attachment mobys. Returns 0. */
extern int D_00160450_27100 __asm__("D_00160450");
extern char D_001D5F70_27100[] __asm__("D_001D5F70");
extern char D_001D6250_27100[] __asm__("D_001D6250");
extern char D_001D6410_27100[] __asm__("D_001D6410");
extern short D_001517D0_27100[] __asm__("D_001517D0");
extern char D_00137C80_27100[] __asm__("D_00137C80");
extern char D_001864D0_27100[] __asm__("D_001864D0");
extern char *D_001B3580_27100 __asm__("D_001B3580");
extern void func_0020C468_27100(int, int) __asm__("func_0020C468");
extern void func_002270B0_27100(int) __asm__("func_002270B0");
extern void func_00203B18_27100(char *, int) __asm__("func_00203B18");
extern int func_00227018_27100(int) __asm__("func_00227018");
extern int func_00217628_27100(int, int, int) __asm__("func_00217628");
extern void func_001F9978_27100(void) __asm__("func_001F9978");
extern void func_00227068_27100(int) __asm__("func_00227068");
extern void func_00227928_27100(void) __asm__("func_00227928");
extern void func_00227A70_27100(void) __asm__("func_00227A70");
extern void func_00227B00_27100(int, int) __asm__("func_00227B00");
extern void func_00213F28_27100(char *, int, int, int, int) __asm__("func_00213F28");
extern void func_00213D28_27100(char *, int, int) __asm__("func_00213D28");
extern void func_00213DE0_27100(char *, int, int, int) __asm__("func_00213DE0");
extern char *func_002267C0_27100(char *) __asm__("func_002267C0");
extern char *func_00226720_27100(void) __asm__("func_00226720");
extern void func_0020ED48_27100(char *) __asm__("func_0020ED48");
extern void func_00225DF8_27100(void) __asm__("func_00225DF8");

int func_00227100_27100(char *, char *, char *, char **, char **, char **) __asm__("func_00227100");
#define MS D_001D5F70_27100
#define RQ D_001D6250_27100
#define AC D_001D6410_27100
#define W(p, o) (*(int *)((p) + (o)))
#define U(p, o) (*(unsigned int *)((p) + (o)))
#define B(p, o) (*(unsigned char *)((p) + (o)))

int func_00227100_27100(char *src, char *prim, char *sec, char **att0, char **att1, char **att2)
{
    int complete = 0;
    int expired = 0;
    int act = 0;
    char **atts[3];
    int k;
    int j;

    if (D_00160450_27100 == 0)
        return 0;
    if (B(src, 0x70) & 2) {
        complete = 1;
        if (W(AC, 0xC) == 0) {
            expired = 1;
        } else {
            W(AC, 0xC) = W(AC, 0xC) - 1;
            if (W(AC, 0xC) == 0)
                expired = 1;
        }
    }
    if (B(MS, 0xCB) != 0 && D_001517D0_27100[4] == 0) {
        int idx = B(MS, 0xCB) - 1;
        int buf = W(MS, 0xA0 + idx * 4);
        func_0020C468_27100(buf + W(MS, 0xBC), buf);
        W(MS, 0xBC) = 0;
        func_002270B0_27100(buf);
        B(MS, 0xC8 + idx) = B(MS, 0x14C);
        if (W(RQ, 4) == W(MS, 0x14C))
            W(RQ, 0) = 2;
        W(D_001B3580_27100, 0x48 + W(MS, 0x14C) * 4) = W(MS, 0xA0 + idx * 4);
        func_00203B18_27100(D_001B3580_27100, W(MS, 0x14C));
        B(MS, 0xCB) = 0;
    }
    if (B(MS, 0xCB) == 0 && D_001517D0_27100[4] == 0 && D_00160450_27100 > 0 && W(RQ, 0) == 0) {
        unsigned int id = U(RQ, 4);
        int ri = B(MS, 0xCA);
        if (B(MS, 0xC8 + ri) == id || B(MS, 0xC8 + (ri == 0)) == id) {
            W(RQ, 0) = 2;
        } else if (id < U(MS, 0xCC)) {
            W(RQ, 0) = 3;
        } else {
            unsigned int a = id - U(MS, 0xCC);
            char *e = D_00137C80_27100 + 0x1C + a * 8;
            int buf = W(MS, 0xA0 + ri * 4);
            int rd = func_00227018_27100(buf);
            int raddr = buf + rd - (W(e, 0) << 11);
            W(MS, 0xBC) = raddr - buf;
            if (func_00217628_27100(raddr, W(D_00137C80_27100 + a * 8, 0x18), W(e, 0)) == 0)
                func_001F9978_27100();
            func_00227068_27100(buf);
            B(MS, 0xCA + 1) = ri + 1;
            B(MS, 0xC8 + ri) = 0xFF;
            W(RQ, 0) = 1;
            W(MS, 0x14C) = W(RQ, 4);
        }
    }
    if (D_00160450_27100 > 0 && (unsigned int)(W(RQ, 0) - 2) < 2) {
        int item = W(RQ, 0x10);
        int ok = 1;
        if (item != 0) {
            if (W(MS, 0x30) != item && W(MS, 0x38) != item && W(MS, 0x34) != item) {
                act = 0;
                func_00227928_27100();
                ok = 0;
            }
        }
        if (ok) {
            int t = W(RQ, 8);
            if (t == 0) {
                act = 1;
            } else if (t == 1 && complete != 0) {
                act = 1;
            } else if (t == 2 && expired != 0) {
                act = 1;
            } else if (t == 3) {
                act = 0;
                func_00227928_27100();
            }
        }
    }
    if (act == 0)
        return 0;

    for (k = 0; k < 14; k++)
        W(AC, k * 4) = W(RQ, k * 4);
    func_00227928_27100();
    func_00227A70_27100();
    func_00227B00_27100(W(AC, 0x30), W(AC, 0x34));
    func_00213F28_27100(src, W(AC, 4), 0, 10, 5);
    if (W(AC, 4) == B(MS, 0xC8))
        B(MS, 0xCA) = 1;
    else if (W(AC, 4) == B(MS, 0xC9))
        B(MS, 0xCA) = 0;
    if (W(D_001864D0_27100, W(AC, 0x10) * 0x4C + 8) == 2) {
        if (sec != 0)
            func_00213F28_27100(sec, W(AC, 0x14), 0, 10, 5);
        if (prim != 0)
            func_00213D28_27100(prim, 1, 0);
    } else if (prim != 0) {
        func_00213F28_27100(prim, W(AC, 0x14), 0, 10, 5);
    }
    *att0 = func_002267C0_27100(*att0);
    *att1 = func_002267C0_27100(*att1);
    *att2 = func_002267C0_27100(*att2);
    atts[0] = att0;
    atts[1] = att1;
    atts[2] = att2;
    for (k = 0; k < 3; k++) {
        char *m;
        if (W(AC, 0x18 + k * 8) == -1)
            continue;
        m = func_00226720_27100();
        *atts[k] = m;
        if (m == 0) {
            if (k == 2)
                return 0;
            continue;
        }
        func_00213D28_27100(m, W(AC, 0x1C + k * 8), 0);
        func_00213DE0_27100(*atts[k], W(AC, 0x1C + k * 8), 0, 10);
        *(short *)(m + 0x34) = 0;
        for (j = 0; j < 4; j++)
            W(m, 0x10 + j * 4) = W(src, 0x10 + j * 4);
        for (j = 0; j < 4; j++)
            W(m, 0x40 + j * 4) = W(src, 0x40 + j * 4);
        if (*(short *)(m + 0xA6) == 0x4A)
            *(float *)(m + 0x2C) = *(float *)(m + 0x2C) * 3.0f;
        func_0020ED48_27100(*atts[k]);
        W(*atts[k], 0x74) = (int)func_00225DF8_27100;
    }
    return 0;
}
