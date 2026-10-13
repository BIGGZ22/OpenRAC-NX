extern float D_L00_001AFFF4 __attribute__((sda));
extern float D_L00_001AFFFC __attribute__((sda));
extern float D_L00_001B0000 __attribute__((sda));
extern int D_L00_001B2AC0[];
extern TI128_Q D_L00_001B2A50[];
extern TI128_Q D_L00_002403C0[];
extern TI128_Q D_L00_002403D0[];
extern char D_L00_002403B0[];

extern float func_002EF240(int mode);
extern void func_002EE178(void *a0, void *a1, float f12);
extern void func_002EE108(void *a0, void *a1, void *a2);

/* Per-frame update: refreshed once, then a rolling pair of records is advanced and three floats are reset. */
void func_L00_003C6A88(char *owner)
{
    TI128_Q local;
    char *inner;
    float rate;
    char *first;
    char *second;
    int i;
    int *clear;
    TI128_Q *target;

    if (D_L00_001B2A00 == 0)
        return;

    inner = *(char **)(owner + 0x68);
    rate = D_L00_001AEFA4 / func_002EF240(12);
    D_L00_001B2A00 = 0;
    func_002EE178(&local, owner + 0xC0, -rate);
    D_L00_001B2A10 = rate;
    D_L00_002403C0[0] = *(TI128_Q *)inner;

    first = D_L00_002403B0 + 0x10;
    second = (char *)D_L00_002403D0;
    for (i = 10; i >= 0; i--) {
        func_002EE108(second, first, &local);
        first += 16;
        second += 16;
    }

    D_L00_001AFFF4 = 0.0f;
    D_L00_001AFFFC = 0.0f;
    D_L00_001B0000 = 0.0f;
    D_L00_001B2A14 = 0;
    D_L00_001B2ACC = 0;
    clear = D_L00_001B2AC0;
    target = D_L00_001B2A50;
    for (i = 2; i >= 0; i--) {
        *clear = 0;
        *target = *(TI128_Q *)&local;
        clear++;
        target++;
    }
}
