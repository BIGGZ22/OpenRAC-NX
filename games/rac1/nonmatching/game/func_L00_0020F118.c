/* HeroItemsCreate: creates whichever of Ratchet's attached item mobys do not exist yet (the
   hero block D_0013E633 + 0xE1D keeps their moby pointers): the held weapon (slot record at
   +0x1090 + link * 0x50, item index from +0x20D4 or the saved one, 8 by default; also clears
   the three 0xB0-byte records at D_0013E633 + 0x270D), the pack (+0x1180), Clank (+0x1184),
   the helmet/boots kinds (+0x1130, +0x10E0/+0x10E4) and the three upgrade pieces
   (+0x11D0, +0x11D4, +0x1220). Item records are 0x4C bytes in D_L00_00179BC0 (+0x8 slot link,
   +0x10 and +0x14 class ids, +0x18 a byte for +0x20AB). Each new moby gets +0x32 = 0x20,
   +0x31 = 1, Ratchet's +0x38 light words, and usually +0x73 = 0x18 when its class byte +6 is
   set. */
typedef struct {
    char pad0[0x8];
    int link;
    char pad0C[0x4];
    int cls;
    int cls2;
    unsigned char b18;
    char pad19[0x33];
} HeroItemRec;
extern HeroItemRec D_L00_00179BC0_i[] __asm__("D_L00_00179BC0");
extern int D_L00_0015F4F8_i[] __asm__("D_L00_0015F4F8");
extern int D_L00_00179C1C_i[] __asm__("D_L00_00179C1C");
extern int D_L00_0017A59C_i[] __asm__("D_L00_0017A59C");
extern int D_L00_0017A5E8_i[] __asm__("D_L00_0017A5E8");
extern int D_L00_0017A634_i[] __asm__("D_L00_0017A634");
extern char D_0014171B_i[] __asm__("D_0014171B");
extern int D_0015EE88_i[] __asm__("D_0015EE88");
extern unsigned char D_0013D5E9_i[] __asm__("D_0013D5E9");
extern unsigned char D_0013D5EB_i[] __asm__("D_0013D5EB");
extern char *func_0020D348_i(int) __asm__("func_0020D348");
extern void func_00205270_i(int, int) __asm__("func_00205270");
extern void func_001F99B0_i(void *, int, int) __asm__("func_001F99B0");
extern void func_00213DE0_i(void *, int, int, int) __asm__("func_00213DE0");
extern void func_L00_002B5428_i(int) __asm__("func_L00_002B5428");

void func_L00_0020F118(void) {
    char *h = D_0013E633 + 0xE1D;
    char *m;
    char *s;
    char *p;
    int idx;
    int link;

    if (*(int *)(h + 0x1090) == 0 && *(int *)(h + 0x10A4) < D_L00_0015F4F8_i[0] &&
        (*(int *)(h + 0x20B8) == 0 || *(int *)(h + 0x20B8) == 0x24)) {
        idx = *(int *)(h + 0x20D4);
        if (idx == 0) {
            if (D_0015EE88_i[2] != 0) {
                idx = 8;
            } else {
                idx = *(int *)(D_0014171B_i + 0x45);
                if (idx == 0) {
                    idx = 8;
                }
            }
        }
        func_00205270_i(D_L00_00179BC0_i[idx].cls, -1);
        for (p = D_0013E633 + 0x270D; p < D_0013E633 + 0x270D + 0x210; p += 0xB0) {
            func_001F99B0_i(p, 0, 0xB0);
            *(short *)(p + 0xA0) = -1;
        }
        *(int *)(h + 0x10B8) = idx;
        link = D_L00_00179BC0_i[idx].link;
        s = h + link * 0x50;
        m = func_0020D348_i(D_L00_00179BC0_i[*(int *)(s + 0x10B8)].cls);
        if (m != 0) {
            *(short *)(m + 0x32) = 0x20;
            *(unsigned char *)(m + 0x31) = 1;
            *(long *)(m + 0x38) = *(long *)(*(char **)(h + 0x2080) + 0x38);
            if (*(unsigned char *)(*(char **)(m + 0x24) + 6) != 0) {
                *(unsigned char *)(m + 0x73) = 0x18;
            }
            *(char **)(s + 0x1090) = m;
            *(int *)(s + 0x10B4) = 2;
            if (idx == 8) {
                *(int *)(s + 0x10A0) = 0x80;
                func_00213DE0_i(m, 1, 0, 1);
            } else {
                *(int *)(s + 0x10A0) = 0x20;
            }
            s = h + link * 0x50;
            *(unsigned char *)(h + 0x20AB) = D_L00_00179BC0_i[*(int *)(s + 0x10B8)].b18;
        }
    }

    if (*(int *)(h + 0x1180) == 0) {
        int kind = *(int *)(h + 0x20E0);
        if (kind == 0) {
            kind = *(int *)(D_0014171B_i + 0x51);
            if (kind == 0) {
                kind = 2;
                if (D_0015EE88_i[3] != 0) {
                    kind = 3;
                }
            }
        }
        *(int *)(h + 0x11A8) = kind;
        m = func_0020D348_i(D_L00_00179BC0_i[kind].cls);
        if (m != 0) {
            *(short *)(m + 0x32) = 0x20;
            *(unsigned char *)(m + 0x31) = 1;
            *(long *)(m + 0x38) = *(long *)(*(char **)(h + 0x2080) + 0x38);
            if (*(unsigned char *)(*(char **)(m + 0x24) + 6) != 0) {
                *(unsigned char *)(m + 0x73) = 0x18;
            }
            *(int *)(h + 0x11A4) = 2;
            *(char **)(h + 0x1180) = m;
            if (*(short *)(h + 0x22D8) == 0 ||
                (*(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) | 0x41,
                 *(short *)(h + 0x22D8) == 0)) {
                if (*(int *)(h + 0x11A8) == 3) {
                    func_L00_002B5428_i(0);
                    func_L00_002B5428_i(1);
                }
            }
        }
    }

    if (*(int *)(h + 0x1184) == 0) {
        m = func_0020D348_i(D_L00_00179C1C_i[0]);
        if (m != 0) {
            *(short *)(m + 0x32) = 0x20;
            *(unsigned char *)(m + 0x31) = 1;
            *(long *)(m + 0x38) = *(long *)(*(char **)(h + 0x2080) + 0x38);
            if (*(unsigned char *)(*(char **)(m + 0x24) + 6) != 0) {
                *(unsigned char *)(m + 0x73) = 0x18;
            }
            *(int *)(h + 0x11A4) = 2;
            *(char **)(h + 0x1184) = m;
            if (*(short *)(h + 0x22D8) != 0) {
                *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) | 0x41;
            }
        }
    }

    if (*(int *)(h + 0x1130) == 0) {
        int a = *(int *)(D_0014171B_i + 0x4D);
        int b = *(int *)(h + 0x20DC);
        if (a != 0 || b != 0) {
            if (b == 0) {
                *(int *)(h + 0x1158) = a;
            } else {
                *(int *)(h + 0x1158) = b;
            }
            m = func_0020D348_i(D_L00_00179BC0_i[*(int *)(h + 0x1158)].cls);
            if (m != 0) {
                *(short *)(m + 0x32) = 0x20;
                *(unsigned char *)(m + 0x31) = 1;
                *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) | 2;
                *(long *)(m + 0x38) = *(long *)(*(char **)(h + 0x2080) + 0x38);
                *(int *)(m + 0x74) = 0;
                if (*(unsigned char *)(*(char **)(m + 0x24) + 6) != 0) {
                    *(unsigned char *)(m + 0x73) = 0x18;
                }
                *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) | 0x800;
                *(int *)(h + 0x1154) = 2;
                *(char **)(h + 0x1130) = m;
            }
        }
    }

    if (*(int *)(h + 0x10E0) == 0) {
        int k = *(int *)(D_0014171B_i + 0x45 + 4);
        if (k != 0) {
            *(int *)(h + 0x1108) = k;
            m = func_0020D348_i(D_L00_00179BC0_i[k].cls);
            if (m != 0) {
                *(short *)(m + 0x32) = 0x20;
                *(unsigned char *)(m + 0x31) = 1;
                *(long *)(m + 0x38) = *(long *)(*(char **)(h + 0x2080) + 0x38);
                if (*(unsigned char *)(*(char **)(m + 0x24) + 6) != 0) {
                    *(unsigned char *)(m + 0x73) = 0x18;
                }
                *(char **)(h + 0x10E0) = m;
                *(int *)(h + 0x1104) = 2;
            }
            m = func_0020D348_i(D_L00_00179BC0_i[*(int *)(D_0014171B_i + 0x45 + 4)].cls2);
            if (m != 0) {
                *(short *)(m + 0x32) = 0x20;
                *(unsigned char *)(m + 0x31) = 1;
                *(long *)(m + 0x38) = *(long *)(*(char **)(h + 0x2080) + 0x38);
                if (*(unsigned char *)(*(char **)(m + 0x24) + 6) != 0) {
                    *(unsigned char *)(m + 0x73) = 0x18;
                }
                *(char **)(h + 0x10E4) = m;
                *(int *)(h + 0x1104) = 2;
            }
        }
    }

    if (*(int *)(h + 0x11D0) == 0 && D_0013D5E9_i[0] != 0) {
        m = func_0020D348_i(D_L00_0017A59C_i[0]);
        *(int *)(h + 0x11F8) = 0x21;
        if (m != 0) {
            *(short *)(m + 0x32) = 0x20;
            *(unsigned char *)(m + 0x31) = 1;
            *(long *)(m + 0x38) = *(long *)(*(char **)(h + 0x2080) + 0x38);
            if (*(unsigned char *)(*(char **)(m + 0x24) + 6) != 0) {
                *(unsigned char *)(m + 0x73) = 0x18;
            }
            *(char **)(h + 0x11D0) = m;
            *(int *)(h + 0x11F4) = 2;
        }
    }

    if (*(int *)(h + 0x11D4) == 0 && D_0013D5E9_i[1] != 0) {
        m = func_0020D348_i(D_L00_0017A5E8_i[0]);
        *(int *)(h + 0x11F8) = 0x22;
        if (m != 0) {
            *(short *)(m + 0x32) = 0x20;
            *(unsigned char *)(m + 0x31) = 1;
            *(long *)(m + 0x38) = *(long *)(*(char **)(h + 0x2080) + 0x38);
            *(int *)(h + 0x11F4) = 2;
            *(char **)(h + 0x11D4) = m;
        }
    }

    if (*(int *)(h + 0x1220) == 0 && D_0013D5EB_i[0] != 0) {
        m = func_0020D348_i(D_L00_0017A634_i[0]);
        *(int *)(h + 0x1248) = 0x23;
        if (m != 0) {
            *(short *)(m + 0x32) = 0x20;
            *(unsigned char *)(m + 0x31) = 1;
            *(long *)(m + 0x38) = *(long *)(*(char **)(h + 0x2080) + 0x38);
            *(int *)(h + 0x1244) = 2;
            *(char **)(h + 0x1220) = m;
        }
    }
}
