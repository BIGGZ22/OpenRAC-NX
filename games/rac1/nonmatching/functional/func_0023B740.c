/* functional: medium. Movie player main loop (a = video decoder, b = movie state base, c = pointer to the byte count of the stream). Same reads, writes and calls in the same order, including the progress marker written to D_00161314 before each step. Callee argument roles come from the registers only. */
extern int D_00161314;
extern int D_00161318;
extern char *D_0016130C;
extern int D_0015EFD8;
extern int D_0015EFA0;
extern int D_0015EF20;
extern int D_0015EE84;
extern char D_0013CA40[];
extern char D_0013E650[];

extern void func_00118D80(int);
extern void func_00122598(int);
extern void func_00218908(void);
extern void func_0023E040(char *);
extern int func_0023CD30(char *, int *);
extern int func_0023CE30(int *, int, int, int);
extern void func_0023CD60(char *, int);
extern void func_0023BF70(void);
extern void func_0023BB40(void);
extern int func_0023CDA8(char *, int *);
extern int func_0012AD10(char *, int, int, char *, int);
extern void func_0023CDF0(char *, int);
extern int func_0023E5C8(char *);
extern int func_0023BB60(void);
extern void func_0023C960(int);
extern void func_0023C088(char *);
extern int func_0023E050(char *);
extern int func_0023E0D8(char *);
extern int func_0023E1B0(char *);
extern void func_0023C9B0(void);
extern void func_0023C0E0(char *);
extern void func_0012E348(int, int);
extern void func_0012DDC0(void);

int func_0023B740(char *a, char *b, int *c) {
    int local[2];
    int ret = 0;
    int started = 0;
    int left = c[0];
    int toread = left;
    int cond;
    int v, v2, v3;

    D_00161314 = 3;
    func_00118D80(0);
    func_00118D80(2);
    func_00122598(0);
    D_00161314 = 4;

    while (left >= 5 && func_0023E050(a) != 3) {
        D_00161314 = 5;
        D_00161318 = D_00161318 + 1;
        func_00218908();
        v = D_0015EFD8;
        D_00161314 = 6;
        if (v != -1) {
            D_00161314 = 7;
            if (v == 2 && *(int *)(D_0013CA40 + 0x1A4) != 0) {
                cond = 1;
            } else {
                int t1;
                if (D_0015EFA0 != 0 || D_0015EF20 != 0 || D_0015EFD8 != 0)
                    t1 = 1;
                else
                    t1 = !(D_0015EE84 > 0);
                if (t1 && (*(int *)(D_0013CA40 + 0x1A4) & 0x800)) {
                    cond = 1;
                } else {
                    u64 m = (0x8000ULL << 28) | 0xF;
                    cond = ((*(u64 *)(D_0013CA40 + 0x1A0) & m) == m);
                }
            }
            if (cond) {
                ret = 1;
                func_0023E040(D_0016130C + 0xD9048);
            }
            D_00161314 = 8;
        }
        D_00161314 = 9;
        v3 = func_0023CD30(b, local);
        D_00161314 = 10;
        if (toread > 0 && v3 > 0xFFFF) {
            D_00161314 = 11;
            v2 = func_0023CE30(c, local[0], 0x10000, 0);
            D_00161314 = 12;
            func_0023CD60(b, v2);
            toread -= v2;
            D_00161314 = 13;
        }
        D_00161314 = 14;
        func_0023BF70();
        D_00161314 = 15;
        func_0023BB40();
        D_00161314 = 16;
        v3 = func_0023CDA8(b, &local[1]);
        D_00161314 = 17;
        if (v3 > 0) {
            D_00161314 = 18;
            v2 = func_0012AD10(a, local[1], v3, b, *(int *)(b + 0x50008));
            D_00161314 = 19;
            func_0023CDF0(b, v2);
            left -= v2;
            D_00161314 = 20;
        }
        D_00161314 = 21;
        func_0023BF70();
        D_00161314 = 22;
        if (started == 0) {
            if (func_0023E5C8(D_0016130C + 0xD9168) != 0) {
                if (func_0023BB60() != 0) {
                    D_00161314 = 23;
                    func_0023C960(1);
                    started = 1;
                    D_00161314 = 24;
                    func_0023C088(D_0016130C + 0xD9100);
                    D_00161314 = 25;
                }
            }
        }
    }

    while (func_0023E0D8(a) == 0) {
        func_0023BF70();
        func_0023BB40();
    }
    while (func_0023E1B0(a) == 0 && func_0023E050(a) != 3) {
        func_0023BF70();
        func_0023BB40();
    }
    func_0023C9B0();
    func_0023C0E0(D_0016130C + 0xD9100);
    func_0012E348(5, *(int *)(D_0013E650 + 0x5C));
    func_0012DDC0();
    D_00161314 = 0;
    D_00161318 = 0;
    return ret;
}
