#include "include_asm.h"

INCLUDE_ASM("asm/levels", func_L02_002B5F50);

INCLUDE_ASM("asm/levels", func_L02_002DCEA0);

INCLUDE_ASM("asm/levels", func_L02_002EF388);

INCLUDE_ASM("asm/levels", func_L02_002EF968);

INCLUDE_ASM("asm/levels", func_L02_002EFB60);

INCLUDE_ASM("asm/levels", func_L02_002F1650);

INCLUDE_ASM("asm/levels", func_L02_002F1FB8);

INCLUDE_ASM("asm/levels", func_L02_002F21C0);

INCLUDE_ASM("asm/levels", func_L02_0031E918);

extern float func_L02_0031E918(float scale);
extern float func_002A7940(void);
extern float func_00283180(float x);
extern float func_00283198(float x);
extern void func_00282C88(void *target);

void func_L02_0031E9E8(float *arg0, float arg1) {
    float a = func_002A7940();
    float b = func_L02_0031E918(arg1);

    func_00282C88(arg0);
    arg0[0] = func_00283180(a) * b;
    arg0[1] = func_00283198(a) * b;
}

extern unsigned int D_L02_001FC060[];
extern char *D_L02_001B359C __attribute__((section(".sdata")));
extern void func_L00_003294F0(unsigned char *entity, int enabled);

/* Walk the 16-bit stream the table entry points at; a negative word ends it. */
void func_L02_0031F858(int arg0, int arg1) {
    short *p;

    if (D_L02_001FC060[arg0] != 0) {
        p = (short *)D_L02_001FC060[arg0];
        do {
            func_L00_003294F0((unsigned char *)(D_L02_001B359C + ((*(unsigned short *)p & 0x7FFF) << 8)), arg1);
        } while (*p++ >= 0);
    }
}

INCLUDE_ASM("asm/levels", func_L02_00321C30);

extern void func_002A04B8(char *object, int *out0, int *out4, int *out8);

/* Pack the three words the callee fills in behind a marker bit. */
int func_L02_003272D8(char *arg0) {
    int w0;
    int w4;
    int w8;
    int v;

    func_002A04B8(arg0, &w0, &w4, &w8);
    v = (w4 << 8) | 0x80000000;
    return (w8 << 16) | v | w0;
}

extern char *D_L02_001B368C __attribute__((section(".sdata")));
extern void func_L02_00327390(char *arg0, char *arg1, int arg2);

/* Index -1 means "no record". */
void func_L02_00327350(int arg0, int arg1) {
    char *base;

    if (arg0 != -1) {
        base = D_L02_001B368C + (arg0 << 7);
        func_L02_00327390(base + 0x30, base + 0x70, arg1);
    }
}

extern unsigned char D_0018C0D4;
extern void func_L00_002B9D60(int a0, int a1, int a2, int a3);
extern void func_L00_0042A500(int arg0);

void func_L02_00327390(char *arg0, char *arg1, int arg2) {
    int kind = 0;

    switch (D_0018C0D4) {
    case 1:
        kind = 0x3F;
        break;
    case 2:
        kind = 0x56;
        break;
    }
    func_L00_002B9D60((int)arg0, (int)arg1, kind, 1);
    if (arg2 != -1)
        func_L00_0042A500(arg2);
}

INCLUDE_ASM("asm/levels", func_L02_00329410);

extern void func_00282C88(void *target);
extern void func_00283098(float *out, const float *a, const float *b);

void func_L02_003294D8(char *arg0, float *arg1, float arg2) {
    float *p = arg1;
    char *q = arg0;

    func_00282C88(p);
    *p = arg2;
    func_00283098(p, p, (const float *)(q + 0xC0));
}

typedef int TI128 __attribute__((mode(TI)));

extern float func_002B04E0(void *arg0, void *arg1);
extern float func_00282C48(float t);

/* Copy the incoming 16-byte record to a local, hand both to the two calls. */
void func_L02_00329C48(void *arg0, TI128 *arg1) {
    TI128 buf = *arg1;

    func_00282C48(func_002B04E0(arg0, &buf));
}

INCLUDE_ASM("asm/levels", func_L02_0032B468);

extern void func_L02_0032E3B0(void *a0, void *a1);

/* Forward the call with the second pointer advanced by one record. */
void func_L02_0032E390(void *a0, void *a1) {
    func_L02_0032E3B0(a0, (char *)a1 + 0x10);
}

INCLUDE_ASM("asm/levels", func_L02_0032E3B0);

INCLUDE_ASM("asm/levels", func_L02_0032E508);

INCLUDE_ASM("asm/levels", func_L02_0032E998);

INCLUDE_ASM("asm/levels", func_L02_0032EC98);

INCLUDE_ASM("asm/levels", func_L02_0032EDD8);

INCLUDE_ASM("asm/levels", func_L02_0032EF10);

INCLUDE_ASM("asm/levels", func_L02_0032F008);

INCLUDE_ASM("asm/levels", func_L02_00331440);

extern void func_00282CF0(float *out, const float *a, const float *b);
extern int func_L02_00332668(char *arg0, char *arg1);

void func_L02_00332630(char *arg0, void *arg1) {
    int tmp[4];

    func_00282CF0((float *)tmp, (const float *)arg1, (const float *)(arg0 + 0x10));
    func_L02_00332668(arg0, tmp);
}

extern int D_L02_001A8FF0 __attribute__((sda));
extern float func_00283248(float a, float b);
extern int func_L01_00322288(char *a0, float f12);
extern void *func_L00_0032C638(char *a0);
extern float func_L00_003BFF68(char *a0, void *a1, char *a2, int a3);

/* Pick a target from the shape, or answer whether its height still fits. */
int func_L02_00332668(char *arg0, char *arg1) {
    void *rec;
    float h;

    if (D_L02_001A8FF0 == 0)
        return func_L01_00322288(arg0, func_00283248(*(float *)(arg1), *(float *)(arg1 + 4)));

    rec = func_L00_0032C638(arg0);
    if (rec == 0)
        return 0;
    h = func_L00_003BFF68(arg0, rec, arg1, 0);
    if (*(float *)((char *)rec + 0x30) < h)
        return 0;
    return 1;
}


INCLUDE_ASM("asm/levels", func_L02_00332900);

extern void func_00282F20(float *out, const float *in, float s);
extern void *func_L00_0032C638(char *a0);
extern float func_L00_0031CDD0(char *a0, char *a1, int a2, float f12, float f13);

/* Aim the record at the shape the caller found. */
int func_L02_003329C8(char *a0, char *a1, float f12, float f13) {
    char *rec = func_L00_0032C638(a0);

    if (rec == 0)
        return 0;
    *(float *)(rec + 0x2C) = f13;
    func_00282CF0((float *)(rec + 0xA0), (const float *)a1, (const float *)(a0 + 0x10));
    *(int *)(rec + 0xA8) = 0;
    func_00282F20((float *)(rec + 0xA0), (const float *)(rec + 0xA0), f12);
    *(float *)(rec + 0xA8) = func_L00_0031CDD0(a0 + 0x10, a1, 0, f12, -f13);
    return 1;
}


INCLUDE_ASM("asm/levels", func_L02_00332A80);

INCLUDE_ASM("asm/levels", func_L02_00332D28);

INCLUDE_ASM("asm/levels", func_L02_00332EF0);

INCLUDE_ASM("asm/levels", func_L02_00333218);

INCLUDE_ASM("asm/levels", func_L02_00333300);

extern void func_L02_00333480(char *arg0, char *arg1, void *arg2);

void func_L02_00333438(char *arg0, char *arg1) {
    int tmp[4];

    func_00282CF0((float *)tmp, (const float *)arg1, (const float *)(arg0 + 0x10));
    func_L02_00333480(arg0, arg1, tmp);
}

INCLUDE_ASM("asm/levels", func_L02_00333480);

extern void *func_002B9030(int kind);
extern int func_00283CF0(float f);
extern float func_002A7878(float a, float b);
extern float func_00283CE0(int n);
extern int func_L00_0031C998(int a0, int a1, float f12);
extern void func_00282D30(void *a0, void *a1, float f12);
extern unsigned char *D_L02_002001D8;

INCLUDE_ASM("asm/levels", func_L02_003449D8);

INCLUDE_ASM("asm/levels", func_L02_00348420);

INCLUDE_ASM("asm/levels", func_L02_00349BD8);

INCLUDE_ASM("asm/levels", func_L02_00389F70);

INCLUDE_ASM("asm/levels", func_L02_0038A008);

INCLUDE_ASM("asm/levels", func_L02_0038BA28);

INCLUDE_ASM("asm/levels", func_L02_0038BD00);

INCLUDE_ASM("asm/levels", func_L02_0038C998);

INCLUDE_ASM("asm/levels", func_L02_0038CDE8);

extern int func_00282978(void *p);
extern int func_00283D38(int a0, int a1, float f12);

/* Toggle the lamp and hand the new state to the colour routine. */
void func_L02_0038D978(char *arg0) {
    char *s0 = *(char **)(arg0 + 0x68);

    if (*(short *)(arg0 + 0xAA) != 0x9B0)
        return;
    if (func_00282978(s0 + 0x74) != 0) {
        *(unsigned short *)(s0 + 0x74) = 0x32;
        *(unsigned int *)(s0 + 0x8C) = (*(unsigned int *)(s0 + 0x8C) == 0);
    }
    *(int *)(arg0 + 0x78) = func_00283D38(*(int *)(arg0 + 0x78),
        *(unsigned int *)(s0 + 0x8C) != 0 ? 0x50787878 : 0x50141414, 0.15f);
}


extern void func_L02_0038DA60(void *arg0);
extern void func_L02_0038DC68(unsigned char *arg0);
extern void func_L02_0038DE58(unsigned char *arg0);
extern void func_L02_0038E048(void *arg0);
extern void func_L02_0038E250(unsigned char *arg0);
extern void func_L02_0038E430(unsigned char *arg0);
extern void func_L02_0038E610(void *arg0);
extern void func_L02_0038E838(unsigned char *arg0);

void func_L02_0038DA00(void *arg0) {
    func_L02_0038DA60(arg0);
    func_L02_0038DC68(arg0);
    func_L02_0038DE58(arg0);
    func_L02_0038E048(arg0);
    func_L02_0038E250(arg0);
    func_L02_0038E430(arg0);
    func_L02_0038E610(arg0);
    func_L02_0038E838(arg0);
}

INCLUDE_ASM("asm/levels", func_L02_0038DA60);

extern void func_00282D60(void *buf, void *value, float scale);
extern int func_00283DB8(void *buf, float value);
extern void func_L00_002ECFE8(void *a0, void *a1, int a2, unsigned int a3,
                              float f12, float f13, float f14, int t0, int t1);
extern int D_L02_001B35B8 __attribute__((sda));
extern char *D_L02_001FBFC0[];
extern int func_L02_002F1658(void);

INCLUDE_ASM("asm/levels", func_L02_0038DC68);

INCLUDE_ASM("asm/levels", func_L02_0038DE58);

INCLUDE_ASM("asm/levels", func_L02_0038E048);

INCLUDE_ASM("asm/levels", func_L02_0038E250);

INCLUDE_ASM("asm/levels", func_L02_0038E430);

INCLUDE_ASM("asm/levels", func_L02_0038E610);

INCLUDE_ASM("asm/levels", func_L02_0038E838);

/* Raise the byte field to eight when it is below. */
void func_L02_00392C70(unsigned char *a0) {
    if (a0[0x20] < 8) {
        a0[0x20] = 8;
    }
}

/* Raise the byte field to eight when it is below. */
void func_L02_003948D0(unsigned char *a0) {
    if (a0[0x20] < 8) {
        a0[0x20] = 8;
    }
}

extern void func_L02_003AA650(char *arg0);

void func_L02_003A6F78(char *arg0) {
    unsigned char *p;
    unsigned char *q;

    arg0[0x20] = 6;
    *(unsigned short *)(arg0 + 0x34) = (*(unsigned short *)(arg0 + 0x34) | 0x41) & 0xEFFF;
    *(int *)(arg0 + 0x98) = 0;
    p = *(unsigned char **)(arg0 + 0x68);
    p[0xC8] = 1;
    p[0xC9] = 0xFF;
    q = *(unsigned char **)(p + 0xFC);
    if (q != 0) {
        if (*(short *)(q + 0xAA) == 0x326) {
            func_L02_003AA650(q);
            *(unsigned char **)(p + 0xFC) = 0;
        } else {
            *(unsigned char **)(p + 0xFC) = 0;
        }
    }
}

INCLUDE_ASM("asm/levels", func_L02_003A6FE8);

INCLUDE_ASM("asm/levels", func_L02_003A8770);

extern void func_002B9180(void *p);
extern void func_0029F338(void *e);
extern void *D_L02_001B31EC __attribute__((sda));

/* Release the eight slots behind the entity, then retire it. */
void func_L02_003AA650(char *arg0) {
    void **p;
    int i;

    p = (void **)(*(char **)(arg0 + 0x68) + 0x30);
    for (i = 7; i >= 0; i--) {
        if (p[0] != 0) {
            func_002B9180(p[0]);
            p[0] = 0;
        }
        p++;
    }
    if (D_L02_001B31EC == arg0)
        arg0[0x20] = 4;
    else
        func_0029F338(arg0);
}


INCLUDE_ASM("asm/levels", func_L02_003B6548);

INCLUDE_ASM("asm/levels", func_L02_003C0890);

INCLUDE_ASM("asm/levels", func_L02_003C26F0);

INCLUDE_ASM("asm/levels", func_L02_003C4068);

INCLUDE_ASM("asm/levels", func_L02_003C4280);
