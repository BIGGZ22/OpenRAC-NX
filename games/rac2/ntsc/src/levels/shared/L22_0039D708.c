#include "include_asm.h"

extern void ref_L22_003A4FF8_00(void);
extern int func_L22_003A2DF8(int, float);

typedef struct Rec_003A2D58 {
    int count;
    unsigned char pad04[0x18];
    float angle;
} Rec_003A2D58;

typedef union F32Bits {
    float f;
    unsigned int u;
} F32Bits;

extern Rec_003A2D58 *D_L22_001FE940[];

INCLUDE_ASM("asm/levels", func_L22_0039D708);

INCLUDE_ASM("asm/levels", func_L22_003A15D0);

INCLUDE_ASM("asm/levels", func_L22_003A1EC8);

INCLUDE_ASM("asm/levels", func_L22_003A1ED8);

INCLUDE_ASM("asm/levels", func_L22_003A2160);

INCLUDE_ASM("asm/levels", func_L22_003A2780);

INCLUDE_ASM("asm/levels", func_L22_003A2988);

INCLUDE_ASM("asm/levels", func_L22_003A2A28);

INCLUDE_ASM("asm/levels", func_L22_003A2B90);

extern int func_L22_003A48F8(int);

/* Collects the set bit positions of VALUE, keeping those a null FLAG lets
 * through and, otherwise, those the shared table scan accepts. */
int func_L22_003A2CB8(float value, int unused, int *out, int flag) {
    int pos = 0;
    int count = 0;
    unsigned int mask;
    int *o;
    F32Bits bits;
    bits.f = value;
    mask = bits.u;
    o = out;
    while (pos < 32) {
        if (mask & 1) {
            if (flag == 0 || func_L22_003A48F8(pos) != 0) {
                *o = pos;
                o++;
                count++;
            }
        }
        pos++;
        mask >>= 1;
    }
    return count;
}

int func_L22_003A2D58(char *a0, int a1, int a2) {
    char *base = *(char **)(a0 + 0x68) + 0x2A0;
    int *slot = (int *)(base + a1 * 4);
    Rec_003A2D58 *rec;
    int n;
    rec = D_L22_001FE940[*slot];
    if (func_L22_003A2DF8(a2, rec->angle) != 0) return 0;
    rec = D_L22_001FE940[*slot];
    n = rec->count - 1;
    rec = (Rec_003A2D58 *)((char *)rec + n * 16);
    func_L22_003A2DF8(a2, rec->angle);
    return n;
}

INCLUDE_ASM("asm/levels", func_L22_003A2DF8);

void func_L22_003A2E10(char *a0, unsigned short a1) {
    char *p = *(char **)(a0 + 0x68);
    unsigned short old = *(unsigned short *)(p + 0x354);
    int i = (short)*(short *)(p + 0x35E);
    char *w;
    i = (i + 1) % 3;
    *(short *)(p + 0x35E) = (short)i;
    w = p + i * 2;
    *(unsigned short *)(w + 0x358) = old;
    *(unsigned short *)(p + 0x354) = a1;
}

INCLUDE_ASM("asm/levels", func_L22_003A2E50);

INCLUDE_ASM("asm/levels", func_L22_003A3368);

INCLUDE_ASM("asm/levels", func_L22_003A3550);

INCLUDE_ASM("asm/levels", func_L22_003A36A0);

INCLUDE_ASM("asm/levels", func_L22_003A39C8);

INCLUDE_ASM("asm/levels", func_L22_003A3B50);

INCLUDE_ASM("asm/levels", func_L22_003A3CA0);

INCLUDE_ASM("asm/levels", func_L22_003A3D50);

INCLUDE_ASM("asm/levels", func_L22_003A4128);

INCLUDE_ASM("asm/levels", func_L22_003A42E8);

INCLUDE_ASM("asm/levels", func_L22_003A4490);

INCLUDE_ASM("asm/levels", func_L22_003A45F0);

typedef struct Ctl_003A47E8 {
    int f0;
    int f4;
    int f8;
} Ctl_003A47E8;

extern Ctl_003A47E8 *D_L22_001B2AB4[1] __attribute__((sda));
extern void func_00316B30(void *, int, void *);
extern void func_0039CDC0(void *, void *, int, int, int);

/* Runs the two optional child passes while the controller's counter lasts. */
void func_L22_003A47E8(char *a0, int flags, int a2, int a3, int a4) {
    char *s0 = *(char **)(a0 + 0x68);
    char buf[16];
    if (D_L22_001B2AB4[0]->f8 == 0) return;
    if (flags & 1) {
        func_00316B30(*(void **)(s0 + 0x440), 3, buf);
        func_0039CDC0(*(void **)(s0 + 0x440), buf, a2, a3,
                      a4 != 0 || D_L22_001B2AB4[0]->f0 != 0);
    }
    if (flags & 2) {
        func_00316B30(*(void **)(s0 + 0x440), 4, buf);
        func_0039CDC0(*(void **)(s0 + 0x440), buf, a2, a3,
                      a4 != 0 || D_L22_001B2AB4[0]->f0 != 0);
    }
    D_L22_001B2AB4[0]->f8 = D_L22_001B2AB4[0]->f8 - 1;
}

INCLUDE_ASM("asm/levels", func_L22_003A4938);

INCLUDE_ASM("asm/levels", func_L22_003A4CA8);

extern float D_L22_001B2AA4 __attribute__((sda));
extern char *D_0018C0B0;
extern void func_002F3AB0(void *, void *, void *);
extern void func_002F3D60(void *, void *, float);
extern void func_002F3AF0(void *, void *, float);
extern void func_002F3A80(void *, void *, void *);
extern void func_L22_003A2B90(void *, void *, void *, int);
extern void func_L22_003A2988(void *, float);

/* Builds the sub-object's three local matrices around its fixed offset and
 * hands the result to the placement helper. */
void func_L22_003A4EE8(char *a0) {
    char b0[16];
    char b1[16];
    char b2[16];
    char *o = *(char **)(a0 + 0x68);
    func_002F3AB0(b0, a0 + 0x10, o + 0x3F0);
    func_002F3D60(b0, b0, D_L22_001B2AA4 * 4.0f);
    func_002F3AF0(b1, D_0018C0B0 + 0xE0, -D_L22_001B2AA4 * 4.0f);
    func_002F3A80(b2, b0, b1);
    func_002F3AF0(b2, b2, 0.5f);
    func_002F3A80(b2, b2, o + 0x3F0);
    func_L22_003A2B90(a0, o + 0x3B0, b2, 1);
    func_L22_003A2988(o + 0x3B0, 0.0f);
    *(char **)(o + 0x3AC) = o + 0x3B0;
}

int func_L22_003A4FF8(void) {
    ref_L22_003A4FF8_00();
    return 1;
}
