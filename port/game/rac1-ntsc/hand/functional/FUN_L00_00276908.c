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
/* What the PAL file this was written in declares before it (its unit, and its own file). */
extern char D_L00_001B9CF0[] NOT_SDA;
extern int D_L00_0015F5D8 MACRO_ADDR;
extern unsigned char D_0014161B NOT_SDA;
extern unsigned char D_0013E533 NOT_SDA;
extern int D_0015ED98 MACRO_ADDR;
extern short D_0015EE78;
extern struct HEnt *D_L00_001B9E90[];
extern char *D_L00_00197300[] NOT_SDA;
extern unsigned char D_L00_0017E4D8[];
extern unsigned char D_L00_00173E40[];
extern void func_002337B0(int);
extern void func_002335D0(void);
extern void func_001FF128(int, int, int);
extern void func_L00_00235310(int);
extern struct HEnt *func_00225530(struct HEnt *);
extern int func_L00_00234E00(void);
extern int func_001F9688(int);
extern int func_L00_00235E18(int, int);
extern int func_001FF308(int, int, void *, void *, void *, void *, int);
extern void func_002336A0(void);
extern void func_L00_00239CC8(void);
extern void func_L00_00239D00(void);
extern void func_L00_00239DF8(void);



void FUN_L00_00276908(void) {
    int i;
    int value;
    int handle;
    char *obj;
    void (*callback)(char *, int);
    unsigned char *table;
    char *st = D_L00_001B9CF0;

    if (*(int *)(st + 0x110) >= 10) {
        func_002337B0(1);
        func_002335D0();
        if (*(int *)(st + 4) != 0) {
            {
                for (i = 0; i < 14; i++) {
                    char *s = D_L00_001B9CF0;
                    obj = (*(struct { char p[0x44]; char *a[14]; } **)(s + 4))->a[i];
                    if (obj != 0) {
                        callback = *(void (**)(char *, int))(obj + 0xC);
                        if (callback != 0) {
                            callback(obj, 0);
                        }
                    }
                }
            }
            {
                char *s = D_L00_001B9CF0;
                *(int *)(s + 4) = 0;
            }
        }
        {
            int i;
            int value = 0x26;
            for (i = 3; i >= 0; i--) {
                char *s = D_L00_001B9CF0;
                char *h = (char *)&D_0013E533 + 0xE1D;
                if (((int *)(&D_0014161B + 0x45))[3 - i] != ((int *)s)[15 - i]) {
                    ((int *)(h + 0x20B8))[3 - i] =
                        ((int *)s)[15 - i] != 0
                            ? ((int *)s)[15 - i]
                            : value;
                }
            }
        }
        {
            char *state = D_L00_001B9CF0;
            int value;
            table = D_L00_0017E4D8;
            value = *(int *)(*(char **)(table + 0x18) + 0x74);
            *(int *)&D_0015EE78 = *(int *)(state + 0x18);
            func_001FF128(0, value, 0);
        }
        func_L00_00235310(2);
        if (*(int *)(*(char **)(table + 0x18) + 0x80) != 0) {
            func_L00_00235310(3);
        }
        if (*(int *)(*(char **)(table + 0x18) + 0x84) != 0) {
            func_L00_00235310(4);
        }
        {
            int k;
            for (k = 0; k < 14; k++) {
                D_L00_001B9E90[k] = func_00225530(D_L00_001B9E90[k]);
            }
        }
        handle = func_L00_00234E00();
        func_L00_00235E18(handle, func_001F9688(0xB4));
        handle = func_001FF308(2, 0x754E, func_L00_00239CC8, func_L00_00239D00,
                               func_L00_00239DF8, &D_0015ED98, 9999999);
        func_L00_00235E18(handle, func_001F9688(0xB4));
        {
            char *s = D_L00_001B9CF0;
            *(int *)s = 0x14;
            *(int *)(s + 0x14) = 2;
            value = *(int *)(s + 0xCC);
            while (value < ((unsigned char *)D_L00_00197300[0])[0xC]) {
                ((struct { char p[0x48]; char *a[1]; } *)D_L00_00197300[0])->a[value] = 0;
                value++;
            }
        }
        {
            char *s = D_L00_001B9CF0;
            ((unsigned char *)D_L00_00197300[0])[0xC] = *(unsigned char *)(s + 0xCC);
        }
        D_L00_0015F5D8 = 1;
        {
            int *tbl = (int *)D_L00_00173E40;
            ((struct { int p[4]; int f; } *)tbl)->f &= 0x7FFFFFFF;
        }
        func_002336A0();
        func_002337B0(1);
    }
}
