#include "include_asm.h"

extern char D_00189EA0[];
extern char D_00233A60[];
extern char D_L30_00235560[];

extern void func_00115484(char *, int, int);
extern int func_002ACBC8(char *, int);
extern void func_002A7470(char *, int, int, int);
extern void func_L00_0042A500(int);
extern char D_001A63A8[];
extern char D_001AEC98[];
extern char D_001B9DC0[];
extern char D_0018C0DC[];
extern char D_00295BF0[];
extern int func_2FB3C8(int);
extern void func_3132D8(void *, unsigned int, int, int, int, int, int, int, int, int);
extern void func_316DB8(char *);
extern int func_32C0D0(char *, int);
extern void func_320FD0(char *, int, int);

typedef struct {
    float x;
    float y;
    float z;
    float u;
    float v;
    float w;
} Vec6;

typedef struct {
    unsigned char pad[0x1E0];
    unsigned short mode;
    unsigned char pad2[0x1FB0 - 0x1E2];
    float f1FB0;
    float f1FB4;
    unsigned char pad3[0x22BC - 0x1FB8];
    unsigned char flag;
} D00189E20_View;

extern D00189E20_View D_00189E20;

typedef struct {
    unsigned char pad[0x124];
    float f124;
    float f128;
    float f12C;
} D00138180_View;

extern D00138180_View D_00138180;

extern float func_321018(float);

INCLUDE_ASM("asm/levels", func_L30_0032AAF0);

INCLUDE_ASM("asm/levels", func_L30_00342FF8);

INCLUDE_ASM("asm/levels", func_L30_003557F0);

float func_L30_00382EF0(float a0, float a1, float a2) {
    return a0 * (1.0f - a2) + a1 * a2;
}

typedef struct {
    float x;
    float y;
} L30Pair2f;

/* Clears the 0x1B00-byte block and lays a 24-by-24 grid of two-float cells over the table at
   0x235560: every cell's first float comes from its row index and its second from its column,
   both on a one-sixth step starting at -2. */
void func_L30_00382F10(void) {
    int i;
    int j;

    func_00115484(D_00233A60, 0, 0x1B00);
    for (i = 0; i < 24; i++) {
        L30Pair2f *row = (L30Pair2f *)(D_L30_00235560 + i * 0xC0);

        for (j = 0; j < 24; j++) {
            row[j].x = i * (1.0f / 6.0f) + -2.0f;
            row[j].y = j * (1.0f / 6.0f) + -2.0f;
        }
    }
}

INCLUDE_ASM("asm/levels", func_L30_00382FB8);

INCLUDE_ASM("asm/levels", func_L30_003830D8);

INCLUDE_ASM("asm/levels", func_L30_003831E8);

INCLUDE_ASM("asm/levels", func_L30_00383398);

INCLUDE_ASM("asm/levels", func_L30_00383928);

INCLUDE_ASM("asm/levels", func_L30_003840E0);

void func_L30_003842E0(char *a0) {
    char *st = *(char **)(a0 + 0x68);

    switch (*(unsigned char *)(a0 + 0x20)) {
    case 0:
        if (*(int *)st == -1) {
            func_316DB8(a0);
        } else {
            func_320FD0(a0, 1, -1);
        }
        break;
    case 1:
        if (func_32C0D0(D_001B9DC0, *(int *)st) != 0) {
            *(unsigned char *)D_0018C0DC = 1;
            func_320FD0(a0, 2, -1);
        }
        break;
    case 2:
        if (func_32C0D0(D_001B9DC0, *(int *)st) != 0) {
            *(unsigned char *)D_0018C0DC = 1;
        } else {
            *(unsigned char *)D_0018C0DC = 0;
            func_320FD0(a0, 1, -1);
        }
        break;
    }
}

INCLUDE_ASM("asm/levels", func_L30_003844C8);

/* Walks the context's 27-word handle table and adopts the first live one the shared probe
   accepts, stamping its index at +0x144 and the handle at +0x140; -1 when none is live. */
int func_L30_00384768(char *a0) {
    char *q = *(char **)(a0 + 0x68);
    int i;

    for (i = 0; i < 27; i++) {
        int h = *(int *)(q + i * 4);

        if (h != -1 && func_002ACBC8(D_00189EA0, h) != 0) {
            *(int *)(q + 0x140) = *(int *)(q + i * 4);
            *(int *)(q + 0x144) = i;
            return *(int *)(q + i * 4);
        }
    }
    return -1;
}

void func_L30_003847F8(char *a0) {
    char *st = *(char **)(a0 + 0x68);
    Vec6 buf;
    int v;

    D_00189E20.mode = 5;
    D_00189E20.flag = 1;
    buf = *(Vec6 *)D_001AEC98;
    v = func_2FB3C8(*(int *)(D_00295BF0 + *(int *)(st + 0x144) * 4));
    func_3132D8(&buf, 0x80F0F0F0, v, 1, 0, -1, 0, 0, 0, 0);
}

INCLUDE_ASM("asm/levels", func_L30_003848B8);

int func_L30_00384980(char *a0) {
    char *st = *(char **)(a0 + 0x68);
    char *g = (char *)D_001A63A8;
    int idx;

    if (*(int *)(g + 0x68) == 0 && *(int *)(g + 0x24) == -1) {
        char *q;

        idx = *(int *)(st + 0x144);
        if (idx != -1) {
            q = st + idx * 4;
            *(int *)(g + 0x24) = (*(int *)(q + 0x70) & 0x3FFF) + 0xBB8;
            *(int *)(g + 0x28) = 0;
            return 1;
        }
    }
    return 0;
}

/* func_L30_00386300 lives in the -g8 sibling: it reaches its globals through $gp. */

INCLUDE_ASM("asm/levels", func_L30_0038E7E0);

INCLUDE_ASM("asm/levels", func_L30_0038EBF8);

INCLUDE_ASM("asm/levels", func_L30_003941D8);

INCLUDE_ASM("asm/levels", func_L30_003B3C18);

void func_L30_003B3E60(float *a0, float *a1, float *a2, float *a3) {
    *a0 = -D_00189E20.f1FB0;
    *a2 = -D_00189E20.f1FB4;
    *a1 = func_321018(D_00138180.f128) - func_321018(D_00138180.f124);
    *a3 = D_00138180.f12C;
    if (*a0 > 1.0f) {
        *a0 = 1.0f;
    } else if (*a0 < -1.0f) {
        *a0 = -1.0f;
    }
    if (*a1 > 1.0f) {
        *a1 = 1.0f;
    } else if (*a1 < -1.0f) {
        *a1 = -1.0f;
    }
}

INCLUDE_ASM("asm/levels", func_L30_003B3F70);

INCLUDE_ASM("asm/levels", func_L30_003BE478);

void func_L30_003BE678(char *a0) {
    char *st = *(char **)(a0 + 0x68);

    if (*(unsigned char *)(a0 + 0x20) == 1) {
        if (*(unsigned char *)(a0 + 0x43) != 1) {
            func_002A7470(a0, 1, 0, 5);
        }
        *(unsigned char *)(a0 + 0x20) = 2;
    } else {
        if (*(unsigned char *)(a0 + 0x43) != 2) {
            func_002A7470(a0, 2, 0, 5);
        }
        *(unsigned char *)(a0 + 0x20) = 5;
        if (*(int *)(st + 0xC) != -1) {
            func_L00_0042A500(*(int *)(st + 0xC));
        }
    }
}

INCLUDE_ASM("asm/levels", func_L30_003C7180);

INCLUDE_ASM("asm/levels", func_L30_003D9CA8);

INCLUDE_ASM("asm/levels", func_L30_003D9E60);

INCLUDE_ASM("asm/levels", func_L30_003DA040);

INCLUDE_ASM("asm/levels", func_L30_003E1BA0);

INCLUDE_ASM("asm/levels", func_L30_003E1E78);

INCLUDE_ASM("asm/levels", func_L30_00404608);

INCLUDE_ASM("asm/levels", func_L30_0040AE38);

INCLUDE_ASM("asm/levels", func_L30_0040B4E0);

INCLUDE_ASM("asm/levels", func_L30_0040B6B0);

INCLUDE_ASM("asm/levels", func_L30_0040B9D8);

INCLUDE_ASM("asm/levels", func_L30_0040C270);
