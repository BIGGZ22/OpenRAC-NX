#include "include_asm.h"

extern float func_00283C80_3(float, float);

extern float func_00283B98_12(float, float);

extern float func_00283BE0_7(float, float);

extern void func_002833F0_2(void *, void *);

extern void func_002E5DA0(int, int, void *, int);

struct Sub {
    float f0;
    int f4;
    int p8;
    int p12;
    int p16;
    int f20;
};

struct Elem {
    char pad[104];
    struct Sub *sub;
    char pad2[140];
    float f248;
};

struct G {
    char pad[192];
    struct Elem *arr[22];
    int count;
};

struct Outer {
    char pad[104];
    struct G *g;
};

typedef int BatchA_A17_s32;

typedef short BatchA_A17_BatchAs16;

INCLUDE_ASM("asm/levels", func_L02_003C43F0);

/* Walks an inline array of element pointers; for each element whose sub-object flag at +4 is zero it rewrites a 270-degree angle to -90, scales it by pi/180, compares a helper result against 0.001 and routes it through one of two pi/2 helpers, toggles the flag, then calls a two-pointer helper with element+192 and element+240. */
void func_L02_003C46E0(struct Outer *p)
{
    struct G *g = p->g;
    int i;

    for (i = 0; i < g->count; i++) {
        struct Sub *s = g->arr[i]->sub;

        if (s->f4 == 0) {
            if (s->f0 == 270.0f)
                s->f0 = -90.0f;
            if (func_00283C80_3(g->arr[i]->f248, s->f0 * 1.7453294247388839721680e-02f)
                < 1.0000000474974513053894e-03f)
                g->arr[i]->f248 = func_00283B98_12(g->arr[i]->f248, 1.5707964897155761718750e+00f);
            else
                g->arr[i]->f248 = func_00283BE0_7(g->arr[i]->f248, 1.5707964897155761718750e+00f);
            s->f20 = !s->f20;
            func_002833F0_2((char *)g->arr[i] + 192, (char *)g->arr[i] + 240);
        }
    }
    if (g->count != 0)
        func_002E5DA0(1, 0, g->arr[0], 2198);
}

INCLUDE_ASM("asm/levels", func_L02_003C4848);

INCLUDE_ASM("asm/levels", func_L02_003C49A0);

INCLUDE_ASM("asm/levels", func_L02_003C4C80);

INCLUDE_ASM("asm/levels", func_L02_003C4E18);

INCLUDE_ASM("asm/levels", func_L02_003C4ED8);

INCLUDE_ASM("asm/levels", func_L02_003C5098);

INCLUDE_ASM("asm/levels", func_L02_003C51F8);

INCLUDE_ASM("asm/levels", func_L02_003C5438);

INCLUDE_ASM("asm/levels", func_L02_003C9D78);

INCLUDE_ASM("asm/levels", func_L02_003C9ED0);

INCLUDE_ASM("asm/levels", func_L02_003CA0A8);

INCLUDE_ASM("asm/levels", func_L02_003CA280);

INCLUDE_ASM("asm/levels", func_L02_003CC7B0);

INCLUDE_ASM("asm/levels", func_L02_003CCCC8);

INCLUDE_ASM("asm/levels", func_L02_003CCEA0);

INCLUDE_ASM("asm/levels", func_L02_003CCFE8);

INCLUDE_ASM("asm/levels", func_L02_003CD1E8);

INCLUDE_ASM("asm/levels", func_L02_003CD298);

INCLUDE_ASM("asm/levels", func_L02_003CD4A8);

INCLUDE_ASM("asm/levels", func_L02_003CD758);

INCLUDE_ASM("asm/levels", func_L02_003CD8C0);

INCLUDE_ASM("asm/levels", func_L02_003CD970);

INCLUDE_ASM("asm/levels", func_L02_003CE4D0);

INCLUDE_ASM("asm/levels", func_L02_003CE8B0);

INCLUDE_ASM("asm/levels", func_L02_003CE908);

INCLUDE_ASM("asm/levels", func_L02_003CE9A8);

INCLUDE_ASM("asm/levels", func_L02_003CEA28);

INCLUDE_ASM("asm/levels", func_L02_003CEBB8);

INCLUDE_ASM("asm/levels", func_L02_003CEC70);

INCLUDE_ASM("asm/levels", func_L02_003CED70);

INCLUDE_ASM("asm/levels", func_L02_003CEE40);

INCLUDE_ASM("asm/levels", func_L02_003CEFD8);

INCLUDE_ASM("asm/levels", func_L02_003CF1B8);

INCLUDE_ASM("asm/levels", func_L02_003D1588);

INCLUDE_ASM("asm/levels", func_L02_003D1610);

INCLUDE_ASM("asm/levels", func_L02_003D1820);

INCLUDE_ASM("asm/levels", func_L02_003D1FA0);

INCLUDE_ASM("asm/levels", func_L02_003D28E0);

INCLUDE_ASM("asm/levels", func_L02_003D29D0);

INCLUDE_ASM("asm/levels", func_L02_003D6BC0);

INCLUDE_ASM("asm/levels", func_L02_003D6DF8);

INCLUDE_ASM("asm/levels", func_L02_003D6EE0);

INCLUDE_ASM("asm/levels", func_L02_003D7040);

INCLUDE_ASM("asm/levels", func_L02_003D7140);

INCLUDE_ASM("asm/levels", func_L02_003D7300);

INCLUDE_ASM("asm/levels", func_L02_003D7310);

INCLUDE_ASM("asm/levels", func_L02_003D7320);

INCLUDE_ASM("asm/levels", func_L02_003D77B8);

INCLUDE_ASM("asm/levels", func_L02_003D77F8);

INCLUDE_ASM("asm/levels", func_L02_003D7830);

INCLUDE_ASM("asm/levels", func_L02_003D7868);

INCLUDE_ASM("asm/levels", func_L02_003D78A8);

INCLUDE_ASM("asm/levels", func_L02_003D7B88);

INCLUDE_ASM("asm/levels", func_L02_003DBEC8);

INCLUDE_ASM("asm/levels", func_L02_003DED08);

INCLUDE_ASM("asm/levels", func_L02_003E1A40);

INCLUDE_ASM("asm/levels", func_L02_003E2150);

INCLUDE_ASM("asm/levels", func_L02_003EB8F8);

INCLUDE_ASM("asm/levels", func_L02_003F0628);

INCLUDE_ASM("asm/levels", func_L02_003F0788);

INCLUDE_ASM("asm/levels", func_L02_003FC248);

INCLUDE_ASM("asm/levels", func_L02_003FCDF8);

INCLUDE_ASM("asm/levels", func_L02_003FCF08);

INCLUDE_ASM("asm/levels", func_L02_003FE768);

INCLUDE_ASM("asm/levels", func_L02_003FF698);

INCLUDE_ASM("asm/levels", func_L02_003FFC48);

INCLUDE_ASM("asm/levels", func_L02_004006D8);

INCLUDE_ASM("asm/levels", func_L02_00400B00);

INCLUDE_ASM("asm/levels", func_L02_00400D08);

INCLUDE_ASM("asm/levels", func_L02_004012A0);

INCLUDE_ASM("asm/levels", func_L02_004013C0);

INCLUDE_ASM("asm/levels", func_L02_00401480);

INCLUDE_ASM("asm/levels", func_L02_00401530);

INCLUDE_ASM("asm/levels", func_L02_00401ED8);

INCLUDE_ASM("asm/levels", func_L02_00402300);

INCLUDE_ASM("asm/levels", func_L02_00402440);

INCLUDE_ASM("asm/levels", func_L02_004024A0);

/* A17-ptr-store-16 freshly exact-verified under family reservation */
void func_L02_004024B0(char *a0) {
    BatchA_A17_s32 p = (BatchA_A17_s32)*(BatchA_A17_s32 *)(a0 + 0x68);
    *(BatchA_A17_BatchAs16 *)((char *)p + 0x30) = 1;
}

INCLUDE_ASM("asm/levels", func_L02_004024C0);

INCLUDE_ASM("asm/levels", func_L02_00404410);

INCLUDE_ASM("asm/levels", func_L02_00404778);
