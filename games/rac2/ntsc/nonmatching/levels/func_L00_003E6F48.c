/* 0 of 27 exact: the loop, the call and its arguments match; the base-pointer load is hoisted above the saves (retail: after s1=a0) and the back-edge reload uses v1 where retail uses v0 (lw v0,0x3B64(gp) in the bgez-like delay slot). */
extern char *D_L00_001B2B54;

extern char *D_L00_001B2B54_1 __attribute__((sda));

extern void func_L00_003E70B8(char *rec, int a, int b, int c, float d, float e, float f, int g);

void func_L00_003E6F48(char *rec)
{
    int i;
    char *base;
    char *p;

    i = 0;
    base = D_L00_001B2B54;
    do {
        p = base + (i << 7);
        i++;
        if (*(char **)(p + 0x7C) == rec)
            func_L00_003E70B8(p, *(int *)(p + 0x68), *(int *)(p + 0x60), 46,
                              *(float *)(p + 0x48), *(float *)(p + 0x34), 0.25f, *(int *)(p + 0x70));
        base = D_L00_001B2B54_1;
    } while (i < 320);
}

