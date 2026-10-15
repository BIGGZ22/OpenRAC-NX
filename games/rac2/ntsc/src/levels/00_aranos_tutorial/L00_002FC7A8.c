#include "include_asm.h"

INCLUDE_ASM("asm/levels", func_L00_002FC7A8);

INCLUDE_ASM("asm/levels", func_L00_002FCBE8);

INCLUDE_ASM("asm/levels", func_L00_00338E90);

extern void func_00282CC0(char *, char *, char *);
extern float func_00282C48(float);
extern float func_00283CE0(int);
extern int func_00283D38(char *, char *, float);
extern int func_00282978(char *);
extern void func_002B9180(void *);

void func_L00_00338FB8(char *obj) {
    char *tail = obj + 0x20;
    obj[8] += tail[0x1C];
    func_00282CC0(obj + 0x10, obj + 0x10, tail);
    *(int *)(obj + 4) = func_00283D38(*(char **)(tail + 0x18), *(char **)(tail + 0x14),
        func_00282C48(func_00283CE0(*(short *)(obj + 0x0A)) * *(float *)(tail + 0x10) - 1.0f));
    if (func_00282978(obj + 0x0A) != 0) {
        func_002B9180(obj);
    }
}

INCLUDE_ASM("asm/levels", func_L00_00354DF8);

extern unsigned int D_001A7AF0 __attribute__((sda));
extern void func_002E76C0(void *, int, int);
extern void func_002ED4B8(void);

void func_L00_003618D8(void *obj) {
    int count;
    int i;
    count = 1;
    if (((D_001A7AF0 >> 4) & 1) != 0) count = 3;
    i = 0;
    while (i < count) {
        func_002E76C0(obj, i, 1);
        i++;
    }
    func_002ED4B8();
}

INCLUDE_ASM("asm/levels", func_L00_003827E0);

INCLUDE_ASM("asm/levels", func_L00_00383300);

INCLUDE_ASM("asm/levels", func_L00_00383958);

INCLUDE_ASM("asm/levels", func_L00_0038D750);

INCLUDE_ASM("asm/levels", func_L00_0038DB68);

INCLUDE_ASM("asm/levels", func_L00_00393148);

INCLUDE_ASM("asm/levels", func_L00_003B9CD8);

INCLUDE_ASM("asm/levels", func_L00_003C5B88);

INCLUDE_ASM("asm/levels", func_L00_003CD160);

INCLUDE_ASM("asm/levels", func_L00_003CD658);

INCLUDE_ASM("asm/levels", func_L00_003CD8E0);

INCLUDE_ASM("asm/levels", func_L00_003CE278);

INCLUDE_ASM("asm/levels", func_L00_003CF438);

INCLUDE_ASM("asm/levels", func_L00_003CF530);

INCLUDE_ASM("asm/levels", func_L00_003CF610);

extern float func_002A78D8(float, float);
extern int func_002A7820(int, int);
extern void func_002E5CF8(int, int, char *);
extern void func_002AA3A8(float *, float *, float, float, float, float);
extern void func_002833F0(char *, char *);
extern int func_00282948(char *);

void func_L00_003CF9B0(char *obj) {
    char *ctx;
    float limit;
    ctx = *(char **)(obj + 0x68);
    if (*(unsigned char *)(obj + 0x20) == 0) {
        *(float *)(ctx + 8) = func_002A78D8(5.0f, 15.0f);
        *(unsigned char *)(obj + 0x20) = 1;
        *(int *)(ctx + 0x0C) = func_002A7820(0x3C, 0x78);
        limit = *(float *)(ctx + 8) * 0.017453294f;
        if (*(float *)(obj + 0xF0) < limit) {
            func_002E5CF8(0, 0, obj);
        } else {
            func_002E5CF8(1, 0, obj);
        }
    }
    if (*(unsigned char *)(obj + 0x20) != 1) return;
    limit = *(float *)(ctx + 8) * 0.017453294f;
    func_002AA3A8((float *)(obj + 0xF0), (float *)ctx, limit,
        0.0008726647356525064f, 0.0008726647356525064f, 0.004363324f);
    func_002833F0(obj + 0xC0, obj + 0xF0);
    if (func_00282948(ctx + 0x0C) != 0) {
        *(unsigned char *)(obj + 0x20) = 0;
    } else if (*(float *)(obj + 0xF0) == *(float *)(ctx + 8) * 0.017453294f) {
        *(unsigned char *)(obj + 0x20) = 0;
    }
}

INCLUDE_ASM("asm/levels", func_L00_003CFB00);

INCLUDE_ASM("asm/levels", func_L00_003D0008);

INCLUDE_ASM("asm/levels", func_L00_003D0910);
