/* func_L00_003B8420 (204 bytes, 4 places) -- NEAR MISS, not integrated.
   Attach/detach probe for the sub-object at +0x68: queries the resident list
   D_00189EA0 for the entry's id twice, and depending on the +0x28 flag and the
   func_002A8508(+0x40, -1) result either allocates the +0x2C block through
   func_L00_002D70A0(5), stamps +0x28 = 1 and runs func_L00_003B81D0, or frees the
   +0x2C block through func_L00_002D7140 and clears +0x28.  Every call, argument
   and flag store is identified and the tail is shared by two paths as in retail.
   Two upstream shapes measured: the early-return spelling collapses retail's
   two-step exit (beqz to the shared `beql $v0,$zero` epilogue test) into one
   branch and is 196 bytes; the nested single-return spelling keeps retail's
   structure but swaps the two callee-saved pointers (this cc1 gives s1 to the
   0x00190000 constant and s2 to the parameter, retail the other way).  Needs the
   allocation lever, not a new spelling. */
extern char D_00189EA0[];
extern int func_002ACBC8(char *, int);
extern int func_002A8508(int, int);
extern void *func_L00_002D70A0(int);
extern void func_L00_002D7140(void *);
extern void func_L00_003B81D0(char *);

void func_L00_003B8420(char *a0)
{
    char *s0 = *(char **)(a0 + 0x68);

    if (func_002ACBC8(D_00189EA0, *(int *)s0) != 0) {
        if (*(int *)(s0 + 0x28) == 0) {
            if (func_002A8508(*(int *)(s0 + 0x40), -1) != 0) {
                *(int *)(s0 + 0x2C) = (int)func_L00_002D70A0(5);
                *(int *)(s0 + 0x28) = 1;
                func_L00_003B81D0(a0);
            }
        } else if (func_002ACBC8(D_00189EA0, *(int *)s0) != 0) {
            if (func_002A8508(*(int *)(s0 + 0x40), -1) == 0) {
                func_L00_002D7140(*(void **)(s0 + 0x2C));
                *(int *)(s0 + 0x28) = 0;
            }
        } else {
            func_L00_002D7140(*(void **)(s0 + 0x2C));
            *(int *)(s0 + 0x28) = 0;
        }
    }
}
