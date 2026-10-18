/* functional: high. Checked against the US assembly (globals, offsets, calls and arguments, constants, branches, loop bounds); the VU1 sync after the first pass now goes to the level's FUN_L00_002a09d8, which retail calls, not the executable's FUN_002337b0. */
/* Carried from the PAL decompilation's src/overlays/shared/pause_00277208.c (matched C there) (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* What the PAL file this was written in declares before it. */
typedef struct PageMenu_c2772c0 {
    int state;
    char *owner;
    int unk8;
    int unkC;
    int unk10;
    char pad14[0xC4];
    int unkD8;
} PageMenu_c2772c0;
typedef struct HEnt_c2772c0 HEnt_c2772c0;
extern void func_00200E08(int, int, int, int, long, long);
extern int D_L00_00160270 SDATA(D_L00_00160270);
extern void func_001F7888(int a, int b, int flag, float f);
extern void func_00200F90(int arg0, int arg1, int arg2, int arg3, long arg4, int arg5, int arg6);
extern void func_001F7978(void);
extern void func_001F5450(int, int, int, int, int, int, int, int, long, long);
extern long D_0015EED0 MACRO_ADDR;
extern void func_0020D248(void);
extern void func_0020D3B0(void);
extern void func_L00_002A09D8(int); /* the level's own VU1 sync (it polls its own flag word), not the executable's FUN_002337b0 */
extern void func_0020CEF8(void);
extern void func_001F4280(int);
extern void func_00223E28(int);
extern void func_001F4398(void);
typedef int u128_c2772c0 __attribute__((mode(TI)));
extern void func_001FB368(void);
extern void func_L00_002A0FC8(int a, int b, int n);
extern PageMenu_c2772c0 PM __asm__("D_L00_001B9CF0");
extern int D_0015EE84 MACRO_ADDR;
extern int D_0013E05A[];
extern void func_001F3868(void);
extern void func_001F5210(int, int, int, int);
extern void func_00233980(int, long);
extern void func_0020D278(void);
extern void func_0020D1F0(void);
extern void func_0020D218(void);
extern int D_L00_001B23C0[];
extern HEnt_c2772c0 *D_L00_001B9E90[];
extern void func_0020D330(int, int);
typedef struct PageMobyData_c2772c0 {
    u128_c2772c0 row[4];
    char pad40[0x10];
    int x;
    int y;
    int w;
    int h;
} PageMobyData_c2772c0;
extern void func_00237A78(void *, void *, int *, int *, int *, int *);

void FUN_L00_002772c0(int arg0) {
    u128_c2772c0 c[4];
    u128_c2772c0 pad[5];
    u128_c2772c0 box[2];
    struct {
        int w;
        int h;
        int x;
        int y;
    } rect;
    char **pp;
    int i;
    int k;
    int j;
    int n;
    int pass;
    int flags;

    func_001FB368();
    pp = 0;
    func_L00_002A0FC8(PM.unk10, D_0015EE84, *(int *)((char *)D_0013E05A + 0x4AA) << 11);
    if (arg0 != 0) {
        return;
    }
    func_001F3868();
    func_001F5210(0, 0, 0, 0x30);
    if (PM.state == 0x14) {
        return;
    }
    func_00233980(0x47, 0x5360B);
    func_0020D278();
    func_0020D1F0();
    func_0020D218();
    for (i = 0; i < 14; i++) {
        if (D_L00_001B23C0[i] != 0 && D_L00_001B9E90[i] != 0 && (i != 6 || PM.unkD8 != 0)) {
            func_0020D330((int)D_L00_001B9E90[i], 1);
        }
    }
    if (PM.owner != 0) {
        pp = (char **)(PM.owner + 0x44);
    }
    for (k = 0; k < 14; k++) {
        PageMobyData_c2772c0 *d;
        if (D_L00_001B9E90[k] == 0 || D_L00_001B23C0[k] == 0) {
            continue;
        }
        if (k == 6 && PM.unkD8 == 0) {
            continue;
        }
        d = *(PageMobyData_c2772c0 **)((char *)D_L00_001B9E90[k] + 0x78);
        qcopy(&c[0], d);
        qcopy(&c[1], &d->row[1]);
        qcopy(&c[2], &d->row[2]);
        qcopy(&c[3], &d->row[3]);
        box[0] = c[0];
        box[1] = c[3];
        func_00237A78(&box[0], &box[1], &rect.w, &rect.h, &rect.x, &rect.y);
        flags = rect.w;
        rect.x++;
        rect.y++;
        if (pp != 0 && pp[k] != 0) {
            *(int *)(pp[k] + 0x20) = flags;
            *(int *)(pp[k] + 0x24) = rect.h;
            *(int *)(pp[k] + 0x18) = rect.x;
            *(int *)(pp[k] + 0x1C) = rect.y;
        }
        d->x = rect.x;
        d->y = rect.y;
        d->w = rect.w;
        d->h = rect.h;
        func_00200E08(rect.x + 1, rect.y + 1, rect.x + rect.w - 1, rect.y + rect.h - 1, D_L00_00160270, 0);
    }
    for (pass = 0; pass < 2; pass++) {
        HEnt_c2772c0 **mobys = D_L00_001B9E90;
        for (j = 0; j < 14; j++) {
            char *m = (char *)mobys[j];
            int pw, ph, px, py;
            int tw, th, su, sv;
            int r, u, v, u1, v1;
            char *d;
            if (m == 0 || pp == 0 || pp[j] == 0) {
                continue;
            }
            flags = *(int *)(pp[j] + 0x10);
            if (flags & 4) {
                continue;
            }
            if (D_L00_001B23C0[j] == 0 || *(int *)(pp[j] + 4) == 0) {
                continue;
            }
            if (j == 6 && PM.unkD8 == 0) {
                continue;
            }
            if (pass == 0 && !(flags & 2)) {
                continue;
            }
            if (pass == 1 && (flags & 2)) {
                continue;
            }
            if (flags & 1) {
                (*(int (**)(char *))(pp[j] + 4))(pp[j]);
                continue;
            }
            d = *(char **)(m + 0x78);
            px = *(int *)(d + 0x50);
            py = *(int *)(d + 0x54);
            pw = *(int *)(d + 0x58);
            ph = *(int *)(d + 0x5C);
            tw = 7;
            while ((1 << tw) < pw) {
                tw++;
            }
            th = 7;
            while ((1 << th) < ph) {
                th++;
            }
            while (tw + th >= 18) {
                th--;
            }
            func_001F7888(tw, th, pass != 0, 1.0f);
            su = 1 << tw;
            sv = 1 << th;
            func_00200F90(0, 0, su, sv, D_L00_00160270, 0, 0);
            r = (*(int (**)(char *))(pp[j] + 4))(pp[j]);
            func_001F7978();
            func_00233980(0x42, 0x8000000064L);
            func_00233980(0x47, 0x43);
            if (r & 1) {
                continue;
            }
            u = 0;
            u1 = su;
            v = 0;
            v1 = sv;
            if (r & 2) {
                u1 = pw;
                v1 = ph;
            } else if (r & 8) {
                u = (u1 - pw) / 2;
                v = (v1 - ph) / 2;
                u1 -= u;
                v1 -= v;
                if (u < 0) {
                    u = 0;
                }
                if (v < 0) {
                    v = 0;
                }
                if (u1 > su) {
                    u1 = su;
                }
                if (v1 > sv) {
                    v1 = sv;
                }
            } else if (r & 4) {
                if (pw < ph) {
                    u = u1 / 2 - (pw * u1) / (ph * 2);
                    u1 -= u;
                } else {
                    v = v1 / 2 - (ph * v1) / (pw * 2);
                    v1 -= v;
                }
            } else if (!(r & 0x10)) {
                continue;
            }
            func_001F5450(px, py, pw, ph, u, v, u1 - u, v1 - v, 0x80808080L, D_0015EED0);
        }
        if (pass == 0) {
            func_0020D248();
            func_0020D3B0();
            func_L00_002A09D8(0x10);
            func_0020CEF8();
        }
    }
    func_001F4280(0);
    for (n = 0; n < 14; n++) {
        if (D_L00_001B23C0[n] != 0 && (n != 6 || PM.unkD8 != 0)) {
            func_00223E28((int)D_L00_001B9E90[n]);
        }
    }
    func_001F4398();
}
