/* func_002D5B58 (boot 0x2D5B58, 732 bytes, 28 places): `python3 tools/check.py func_002D5B58 --diff` prints
 * `func_002D5B58: 28 of 28 places match` for the C below, but tools/build.py cannot place it. The 14-entry
 * `switch (type)` compiles to a jump table in the object's own .rodata, the code reaches it through
 * `%hi/%lo(.rodata)`, and Placement.derive() therefore needs `.rodata` at 0x26CD60 ("needs .rodata at its place,
 * which is not C"; same wall as func_00118CC8). Retail keeps the table as data at 0x26CD60 (jtbl_0026CD60: 14
 * words, entries 0 and 1 point at the common return). Per the repo rule the exact C stays here and the
 * INCLUDE_ASM marker is back in src/game/002D5540.c.
 *
 * Shape: a dispatcher on `type` (cases 2..13; 0, 1 and anything above 13 only return 0). The cases write fields of
 * the block at D_001F2840, the flags word D_001A7424 or the byte D_001A7BB4 and call func_002E5E58 / func_002B49E0 /
 * func_002DF190; cases 6..11 also store 0 through `result` and return 1. Case 7 has no break and falls into case 8
 * (retail runs both bodies). The statement order inside cases 6, 7, 8, 10 and 11 was found by brute force (all
 * permutations, one case at a time): the scheduler's store order depends on it.
 *
 * An untested way round, kept in the #if 0 block at the end: dispatch with a GNU computed goto through the
 * retail table (D_0026CD60 is a named boot symbol like any D_ name) and keep the labels alive with an unused
 * table of label addresses. check.py also says `28 of 28` for it, the section need is gone (no `.rodata`
 * relocation from the code; the unused label table sits in a section the link discards), and the code is
 * byte-identical. tools/build.py was NOT run on it (not allowed for the worker); it is plain C but a new
 * mechanism, so it needs a decision before it goes into src/. */

extern char D_001F2840[];
extern int func_002E5E58(int, unsigned int, void *);
extern void func_002B49E0(int, int, int, int, int);

typedef struct {
    char pad0[8];
    int f08;
    char pad1[8];
    int f14;
    int f18;
    int f1C;
    char pad2[0xF4 - 0x20];
    int fF4;
    int fF8;
    int fFC;
    int f100;
    int f104;
} Func002D5B58Root;
extern int D_001A7424 __attribute__((sda));
extern char D_001A7BB4 __attribute__((sda));
extern int D_001C1220[];
extern int D_001F29A0;
extern void func_00287EA8(void);
extern void func_00285828(int, int, int, int);
extern void func_00285788(int, int);
extern void func_002DF190(int, int);

int func_002D5B58(int type, int value, int *result)
{
    int ret = 0;
    switch (type) {
    case 0:
        break;
    case 3:
        ((Func002D5B58Root *)D_001F2840)->f18 = value;
        break;
    case 4:
        func_002E5E58(4, 0, 0);
        func_00287EA8();
        D_001A7424 = (D_001A7424 | 2) & ~4;
        D_001F29A0 = 0;
        func_002B49E0(4, 1, 1, value, 0);
        break;
    case 5:
        func_002E5E58(4, 0, 0);
        D_001A7424 = (D_001A7424 | 4) & ~2;
        func_002B49E0(4, 1, 1, value, 0);
        D_001F29A0 = 1;
        break;
    case 6: {
        Func002D5B58Root *root;
        int index = value >> 16;
        if (index != 0) {
            Func002D5B58Root *first = (Func002D5B58Root *)D_001F2840;
            first->fFC = D_001C1220[index];
        }
        root = (Func002D5B58Root *)D_001F2840;
        root->f100 = root->f14;
        root->f104 = root->f08;
        root->f1C = 5;
        root->fF4 = value & 0xFFFF;
        ret = 1;
        func_002DF190(0, 0x11);
        *result = 0;
        break;
    }
    case 7: {
        Func002D5B58Root *root = (Func002D5B58Root *)D_001F2840;
        root->fF4 = value;
        root->f100 = root->f14;
        root->f1C = 3;
        root->f104 = root->f08;
        func_002DF190(0, 0x11);
        *result = 0;
    }
    case 8: {
        Func002D5B58Root *root = (Func002D5B58Root *)D_001F2840;
        root->f100 = root->f14;
        root->f104 = root->f08;
        root->f1C = 4;
        root->fF4 = value;
        ret = 1;
        func_002DF190(0, 0x11);
        *result = 0;
        break;
    }
    case 10: {
        Func002D5B58Root *root = (Func002D5B58Root *)D_001F2840;
        root->f100 = root->f14;
        root->f104 = root->f08;
        root->f1C = 6;
        root->fF4 = value;
        ret = 1;
        func_002DF190(0, 0x11);
        *result = 0;
        break;
    }
    case 11: {
        Func002D5B58Root *root = (Func002D5B58Root *)D_001F2840;
        root->f100 = root->f14;
        root->f104 = root->f08;
        root->f1C = 7;
        ret = 1;
        func_002DF190(0, 0x11);
        *result = 0;
        break;
    }
    case 9:
        D_001A7BB4 = value;
        ret = 1;
        *result = 0;
        break;
    case 2:
        func_002DF190(2, 0x11);
        break;
    case 12:
        func_002E5E58(4, 0, 0);
        func_00285828(0, 0, 0, 0);
        func_00285788(0, 0);
        func_002B49E0(4, 1, 9, ((Func002D5B58Root *)D_001F2840)->f14, 0);
        break;
    case 13:
        func_002B49E0(4, 1, 11, ((Func002D5B58Root *)D_001F2840)->f14, 0);
        break;
    }
    return ret;
}

#if 0
/* The computed-goto variant (check.py 28 of 28, no .rodata need); also needs: extern void *D_0026CD60[]; */
int func_002D5B58(int type, int value, int *result)
{
    int ret = 0;
    static void *const keep[] = {&&L3, &&L4, &&L5, &&L6, &&L7, &&L8, &&L10, &&L11, &&L9, &&L2, &&L12, &&L13};
    if ((unsigned)type >= 14)
        goto done;
    goto *D_0026CD60[type];
    L3:
        ((Func002D5B58Root *)D_001F2840)->f18 = value;
        goto done;
    L4:
        func_002E5E58(4, 0, 0);
        func_00287EA8();
        D_001A7424 = (D_001A7424 | 2) & ~4;
        D_001F29A0 = 0;
        func_002B49E0(4, 1, 1, value, 0);
        goto done;
    L5:
        func_002E5E58(4, 0, 0);
        D_001A7424 = (D_001A7424 | 4) & ~2;
        func_002B49E0(4, 1, 1, value, 0);
        D_001F29A0 = 1;
        goto done;
    L6: {
        Func002D5B58Root *root;
        int index = value >> 16;
        if (index != 0) {
            Func002D5B58Root *first = (Func002D5B58Root *)D_001F2840;
            first->fFC = D_001C1220[index];
        }
        root = (Func002D5B58Root *)D_001F2840;
        root->f100 = root->f14;
        root->f104 = root->f08;
        root->f1C = 5;
        root->fF4 = value & 0xFFFF;
        ret = 1;
        func_002DF190(0, 0x11);
        *result = 0;
        goto done;
    }
    L7: {
        Func002D5B58Root *root = (Func002D5B58Root *)D_001F2840;
        root->fF4 = value;
        root->f100 = root->f14;
        root->f1C = 3;
        root->f104 = root->f08;
        func_002DF190(0, 0x11);
        *result = 0;
    }
    L8: {
        Func002D5B58Root *root = (Func002D5B58Root *)D_001F2840;
        root->f100 = root->f14;
        root->f104 = root->f08;
        root->f1C = 4;
        root->fF4 = value;
        ret = 1;
        func_002DF190(0, 0x11);
        *result = 0;
        goto done;
    }
    L10: {
        Func002D5B58Root *root = (Func002D5B58Root *)D_001F2840;
        root->f100 = root->f14;
        root->f104 = root->f08;
        root->f1C = 6;
        root->fF4 = value;
        ret = 1;
        func_002DF190(0, 0x11);
        *result = 0;
        goto done;
    }
    L11: {
        Func002D5B58Root *root = (Func002D5B58Root *)D_001F2840;
        root->f100 = root->f14;
        root->f104 = root->f08;
        root->f1C = 7;
        ret = 1;
        func_002DF190(0, 0x11);
        *result = 0;
        goto done;
    }
    L9:
        D_001A7BB4 = value;
        ret = 1;
        *result = 0;
        goto done;
    L2:
        func_002DF190(2, 0x11);
        goto done;
    L12:
        func_002E5E58(4, 0, 0);
        func_00285828(0, 0, 0, 0);
        func_00285788(0, 0);
        func_002B49E0(4, 1, 9, ((Func002D5B58Root *)D_001F2840)->f14, 0);
        goto done;
    L13:
        func_002B49E0(4, 1, 11, ((Func002D5B58Root *)D_001F2840)->f14, 0);
        goto done;
done:
    return ret;
}
#endif
