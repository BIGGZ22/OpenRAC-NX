/* Carried from the PAL decompilation's src/game/pause.c (matched C there) (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's.
 * The two versions' code differs here: func_00227100 (near) was paired by place; read it against NTSC-U before trusting it. */
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
/* What the PAL file this was written in declares before it (its unit, and its own file). */
extern void func_001F97B0(void);
extern void func_0020DEF8(void *);
struct Hdr16_1CE60_c225e70 {
    u8 pad0[0x8];
    s16 unk8;
};
struct PreviewItemDefinition_c225e70 {
    u8 pad0[8];
    s32 item_type;
    u8 padC[4];
    s32 oclass;
    u8 pad14[0x38];
};
extern struct PreviewItemDefinition_c225e70 D_001864D0_1F238[] __asm__("D_001863D0");
/*
 * Exact once the short-loop padding retail's assembler adds is reproduced
 * (tools/fix_short_loops.py): both loops are five instructions and get one
 * nop each. g is unsigned char so 0xFF is built as 0xFF, not -1. Two
 * separate loop counters are load-bearing: with one variable reused, both
 * loops take $s2 and q takes $s1.
 */
extern int func_00225530(int);
extern void func_002267B8(void);
extern void func_00212F90(void *, int, int, int);
void *func_00225490(int arg0);
extern int func_00225D88(int handle);
int func_00225DD8(int arg0);
int func_00225E20(int arg0);
typedef struct {
    int unk00;
    int arg[13];
} Rec38_q_c225e70;
extern Rec38_q_c225e70 D_001D6250_q[] __asm__("D_001D5EC0");
typedef struct {
    u8 pad0[0x30];
    s32 items[3];
    u8 pad3C[0x64];
    s32 buf[2];
    u8 padA8[0x14];
    s32 readOff;
    u8 padC0[8];
    u8 loaded[2];
    u8 idx;
    u8 pending;
    u32 base;
    u8 padD0[0x7C];
    s32 streamed;
} Pv_q_c225e70;
extern Pv_q_c225e70 D_001D5F70_q[] __asm__("D_001D5BF0");
extern struct Hdr16_1CE60_c225e70 D_001517D0_q[] __asm__("D_001516D0");
extern Rec38_q_c225e70 D_001D6410_q[] __asm__("D_001D6080");
extern int D_00160350 MACRO_ADDR;
typedef struct {
    u8 pad0[0x48];
    int anim[1];
} Cls_q_c225e70;
typedef struct {
    u8 pad0[0x18];
    struct {
        int a;
        int b;
    } e[1];
} Tbl_q_c225e70;
extern Tbl_q_c225e70 D_00137C80_q[] __asm__("D_00137B80");
extern char *D_001B3200[] NOT_SDA;
extern void func_0020B618(int, int);
extern int func_00216788(int, int, int);
extern void func_002032E0(char *, int);
extern int func_00226670(void);
extern void func_00226848(s32, s32);
extern void func_002130D8(void *, int, int, int, int);
extern void func_00213D28_q(void *, int, int) __asm__("func_00212ED8");
extern void func_00224B68(void);

int func_00227100_r(char *moby, char *m1, char *m2, char **o0, char **o1, char **o2) __asm__("FUN_00225e70");



/* Runs the item-preview animation stream queue: finishes a pending buffer, starts the next read, and on a ready entry swaps in the preview mobys. */
int func_00227100_r(char *moby, char *m1, char *m2, char **o0, char **o1, char **o2) {
    int flagA = 0;
    int flagB = 0;
    int result = 0;
    char *p;

    if (D_00160350 == 0) {
        return 0;
    }
    {
        if (*(unsigned char *)(moby + 0x70) & 2) {
            flagA = 1;
            if (D_001D6410_q[0].arg[2] == 0 || --D_001D6410_q[0].arg[2] == 0) {
                flagB = 1;
            }
        }
        {
            if (D_001D5F70_q[0].pending != 0) {
                if (D_001517D0_q[0].unk8 == 0) {
                    int n = D_001D5F70_q[0].pending - 1;
                    int *slot = D_001D5F70_q[0].buf + n;
                    int buf = *slot;
                    func_0020B618(buf + D_001D5F70_q[0].readOff, buf);
                    D_001D5F70_q[0].readOff = 0;
                    func_00225E20(buf);
                    D_001D5F70_q[0].loaded[n] = D_001D5F70_q[0].streamed;
                    if (D_001D6250_q[0].arg[0] == D_001D5F70_q[0].streamed) {
                        D_001D6250_q[0].unk00 = 2;
                    }
                    ((Cls_q_c225e70 *)D_001B3200[0])->anim[D_001D5F70_q[0].streamed] = *slot;
                    func_002032E0(D_001B3200[0], D_001D5F70_q[0].streamed);
                    D_001D5F70_q[0].pending = 0;
                }
            }
        }
        if (D_001D5F70_q[0].pending == 0 && D_001517D0_q[0].unk8 == 0 &&
            D_00160350 > 0 && D_001D6250_q[0].unk00 == 0) {
            int idx = D_001D5F70_q[0].idx;
            unsigned char *ld = D_001D5F70_q[0].loaded;
            unsigned int want = D_001D6250_q[0].arg[0];
            if (ld[idx] == want || ld[idx == 0] == want) {
                D_001D6250_q[0].unk00 = 2;
            } else if (want < D_001D5F70_q[0].base) {
                D_001D6250_q[0].unk00 = 3;
            } else {
                int k = want - D_001D5F70_q[0].base;
                int buf = D_001D5F70_q[0].buf[idx];
                int sz = D_00137C80_q[0].e[k].b << 11;
                int addr = buf + func_00225D88(buf) - sz;
                D_001D5F70_q[0].readOff = addr - buf;
                if (func_00216788(addr, D_00137C80_q[0].e[k].a, D_00137C80_q[0].e[k].b) == 0) {
                    func_001F97B0();
                }
                func_00225DD8(buf);
                D_001D5F70_q[0].pending = D_001D5F70_q[0].idx + 1;
                ld[D_001D5F70_q[0].idx] = 0xFF;
                D_001D6250_q[0].unk00 = 1;
                D_001D5F70_q[0].streamed = D_001D6250_q[0].arg[0];
            }
        }
        if (D_00160350 > 0 && (unsigned int)(D_001D6250_q[0].unk00 - 2) < 2) {
            int item = D_001D6250_q[0].arg[3];
            if (item != 0 && D_001D5F70_q[0].items[0] != item &&
                D_001D5F70_q[0].items[2] != item && D_001D5F70_q[0].items[1] != item) {
                result = 0;
                func_00226670();
            } else if (D_001D6250_q[0].arg[1] == 0) {
                result = 1;
            } else if (D_001D6250_q[0].arg[1] == 1 && flagA != 0) {
                result = 1;
            } else if (D_001D6250_q[0].arg[1] == 2 && flagB != 0) {
                result = 1;
            } else if (D_001D6250_q[0].arg[1] == 3) {
                result = 0;
                func_00226670();
            }
        }
        if (result != 0) {
            D_001D6410_q[0] = D_001D6250_q[0];
            func_00226670();
            func_002267B8();
            func_00226848(D_001D6410_q[0].arg[11], D_001D6410_q[0].arg[12]);
            func_002130D8(moby, D_001D6410_q[0].arg[0], 0, 10, 5);
            if (D_001D6410_q[0].arg[0] == D_001D5F70_q[0].loaded[0]) {
                D_001D5F70_q[0].idx = 1;
            } else if (D_001D6410_q[0].arg[0] == D_001D5F70_q[0].loaded[1]) {
                D_001D5F70_q[0].idx = 0;
            }
            if (D_001864D0_1F238[D_001D6410_q[0].arg[3]].item_type == 2) {
                if (m2 != 0) {
                    func_002130D8(m2, D_001D6410_q[0].arg[4], 0, 10, 5);
                }
                if (m1 != 0) {
                    func_00213D28_q(m1, 1, 0);
                }
            } else if (m1 != 0) {
                func_002130D8(m1, D_001D6410_q[0].arg[4], 0, 10, 5);
            }
            *o0 = (char *)func_00225530((int)*o0);
            *o1 = (char *)func_00225530((int)*o1);
            *o2 = (char *)func_00225530((int)*o2);
            if (D_001D6410_q[0].arg[5] != -1) {
                if ((*o0 = func_00225490(D_001D6410_q[0].arg[5])) != 0) {
                    func_00213D28_q(*o0, D_001D6410_q[0].arg[6], 0);
                    func_00212F90(*o0, D_001D6410_q[0].arg[6], 0, 10);
                    *(short *)(*o0 + 0x34) = 0;
                    p = *o0;
                    qcopy(p + 0x10, moby + 0x10);
                    qcopy(p + 0x40, moby + 0x40);
                    if (*(short *)(p + 0xA6) == 0x4A) {
                        *(float *)(p + 0x2C) = *(float *)(p + 0x2C) * 3.0f;
                    }
                    func_0020DEF8(*o0);
                    *(void **)(*o0 + 0x74) = func_00224B68;
                }
            }
            if (D_001D6410_q[0].arg[7] != -1) {
                if ((*o1 = func_00225490(D_001D6410_q[0].arg[7])) != 0) {
                    func_00213D28_q(*o1, D_001D6410_q[0].arg[8], 0);
                    func_00212F90(*o1, D_001D6410_q[0].arg[8], 0, 10);
                    *(short *)(*o1 + 0x34) = 0;
                    p = *o1;
                    qcopy(p + 0x10, moby + 0x10);
                    qcopy(p + 0x40, moby + 0x40);
                    if (*(short *)(p + 0xA6) == 0x4A) {
                        *(float *)(p + 0x2C) = *(float *)(p + 0x2C) * 3.0f;
                    }
                    func_0020DEF8(*o1);
                    *(void **)(*o1 + 0x74) = func_00224B68;
                }
            }
            if (D_001D6410_q[0].arg[9] != -1) {
                if ((*o2 = func_00225490(D_001D6410_q[0].arg[9])) != 0) {
                    func_00213D28_q(*o2, D_001D6410_q[0].arg[10], 0);
                    func_00212F90(*o2, D_001D6410_q[0].arg[10], 0, 10);
                    *(short *)(*o2 + 0x34) = 0;
                    p = *o2;
                    qcopy(p + 0x10, moby + 0x10);
                    qcopy(p + 0x40, moby + 0x40);
                    if (*(short *)(p + 0xA6) == 0x4A) {
                        *(float *)(p + 0x2C) = *(float *)(p + 0x2C) * 3.0f;
                    }
                    func_0020DEF8(*o2);
                    *(void **)(*o2 + 0x74) = func_00224B68;
                }
            }
        }
    }
    return 0;
}
