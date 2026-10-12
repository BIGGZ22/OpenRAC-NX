#include "common.h"
#include "structs.h"

/* func_0012C508 returns a short: its epilogue sign-extends $v0. */
extern short func_0012C508(int, int);

extern char r1_D_001538A8[];

extern int func_0012C680(void *, int);

extern void func_0012C788(void *, int);

extern void r1_func_0012C430(void *, char *, int);

extern int func_0012B568(void *);

extern void func_0012C3B0(void *);

extern void r1_func_0012C468(void *, void *);

extern char r1_D_001538E0[];

extern int func_0012B8B0(void *, int);

extern int func_0012B1C0(void *);

extern int r1_func_001263A8(void *, int);

extern int func_0012B678(void *);

/* rac1-decomp src/core/00125630.c: func_00127748 */
int func_0012B568(void *arg0) {
    char *s = (char *)arg0;
    unsigned int code;
    int val = 0;
    int cont;
    int ret;

    do {
        code = func_0012C508((int)s, 0);
        switch (code) {
        case 35:
            cont = 1;
            val += 33;
            break;
        case 0:
            ret = func_0012C680(s, 11);
            if (*(int *)(s + 0x848) != 0 && ret == 15) {
                func_0012C788(s, 11);
                cont = 1;
            } else {
                r1_func_0012C430(s, r1_D_001538A8, code);
                *(int *)(s + 0x11C) = 1;
                return 1;
            }
            break;
        case 34:
            cont = 1;
            break;
        default:
            val += code;
            cont = 0;
            break;
        }
    } while (cont);

    return val;
}

/* rac1-decomp src/core/00125630.c: func_00127858 */
int func_0012B678(void *arg0) {
    char *s = (char *)arg0;
    int count;
    int ret;

    *(int *)(s + 0x810) = 0;
    *(int *)(s + 0x814) = 0;

    count = *(int *)(s + 0x12C) * *(int *)(s + 0x130);
    if (*(int *)(s + 0x174) != 3) {
        count >>= 1;
    }

    do {
        ret = func_0012B8B0(s, count);
    } while (ret == 1 || ret == 3);

    func_0012C3B0(s);
    if (func_0012B1C0(s) == 0) {
        ret = 2;
    }

    while (((*(volatile unsigned int *)0x1000D400 >> 8) & 1) != 0)
        ;

    if (ret == 0) {
        r1_func_001263A8(s, (unsigned int)*(int *)(s + 0x810) < 1);
    }

    if ((unsigned int)(ret - 1) < 2) {
        r1_func_0012C468(s, r1_D_001538E0);
    }

    return (unsigned int)ret < 1;
}
