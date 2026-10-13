/* 108 bytes: prologue, field loads and register use match retail; about 7 loop instructions differ (retail: beql on *w with the s+=0x150 in its delay slot, bgezl with the next *w load in its delay slot). Missing: the source shape that makes GCC duplicate the s+=0x150 into both paths and put i--/w++ in the loop tail. */
typedef struct OBJ4 {
    char pad0[8];
    int f8;
    int fC;
} OBJ4;

extern void func_322EC8(void *p);

void func_L00_003B60E8(OBJ4 *a0)
{
    char *base = (char *)a0 + 0x20;
    char *s = base + a0->f8;
    int *w = (int *)(base + a0->fC);
    int i;

    for (i = 24; i >= 0; i--, w++) {
        if (*w == 0) {
            s += 0x150;
            continue;
        }
        func_322EC8(s);
        s += 0x150;
    }
}
