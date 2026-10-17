/* functional: medium. initAll(a, b, c): same reads, writes and calls in the same order; the thread-parameter block is filled the way retail does (word 0 is left unset). Argument roles of the callees are taken from the registers only. */
extern int D_00161308;
extern char *D_0016130C;
extern int D_00161310;
extern char D_001E8E20[];
extern char D_00166D00[];

extern void func_0023CD10(char *);
extern void func_0012B870(void);
extern void func_0023DE98(char *, int, int, char *, int, int, char *, int);
extern void func_0023BFA0(char *, char *, int, int);
extern void func_0023DFA0(char *, int, int, void *, char *);
extern void func_0023E560(char *, int, int, int);
extern int func_00118B50(void *);
extern void func_00118B70(int, char *);
extern int func_0023CE18(char *, int, int);
extern void func_001E9730(char *, ...);
extern int func_00118A90(int, void *, int);
extern void func_00119390(int);
extern int func_00118AB0(int, void *, int);
extern void func_00119460(int);
extern void func_0023C9C0(void);
extern void func_0023CAF8(void);
extern void func_0023E1F8(void);
extern void func_0023C7A8(void);
extern void func_0023C910(void);

int func_0023BB90(int a, int b, int c) {
    int thread[9] = {0};
    int ok;
    int id;
    char *s;
    int g;

    *(volatile unsigned int *)0x1000E000 |= 3;
    *(volatile unsigned int *)0x1000E010 = 4;

    func_0023CD10(D_0016130C);
    func_0012B870();

    s = D_0016130C;
    g = D_00161308;
    func_0023DE98(s + 0xD9048, g + 0x1C85C0, 0xEB768, s + 0x52040,
                  g + 0x1C7180, 0x100, s + 0xD6040, 0x200);

    s = D_0016130C;
    g = D_00161308;
    func_0023BFA0(s + 0xD9100, s + 0x50040, 0x2000, g + 0x1C8190);

    s = D_0016130C;
    func_0023DFA0(s + 0xD9048, 0, 0, (void *)func_0023C9C0, s);

    s = D_0016130C;
    func_0023DFA0(s + 0xD9048, 3, c, (void *)func_0023CAF8, s);

    g = D_00161308;
    s = D_0016130C;
    func_0023E560(s + 0xD9168, (g & 0xFFFFFFF) | 0x20000000, g + 0x1A0000, 2);

    s = D_0016130C;
    thread[1] = (int)func_0023E1F8;
    thread[2] = (int)(s + 0xD2040);
    thread[3] = 0x4000;
    thread[4] = (int)D_00166D00;
    thread[5] = 1;
    thread[8] = 0;
    id = func_00118B50(thread);
    D_00161310 = id;
    s = D_0016130C;
    func_00118B70(id, s + 0xD9048);

    s = D_0016130C;
    ok = 1;
    if (func_0023CE18(s + 0xD9040, a, b) == 0) {
        ok = 0;
        func_001E9730(D_001E8E20);
    }

    id = func_00118A90(2, (void *)func_0023C7A8, 0);
    s = D_0016130C;
    *(int *)(s + 0xD90FC) = id;
    func_00119390(2);

    id = func_00118AB0(2, (void *)func_0023C910, 0);
    s = D_0016130C;
    *(int *)(s + 0xD90F8) = id;
    func_00119460(2);
    return ok;
}
