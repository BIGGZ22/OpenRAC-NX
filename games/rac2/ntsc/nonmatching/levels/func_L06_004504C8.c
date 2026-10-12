/* func_L06_004504C8 (120 bytes) -- ABANDON, 3 words (see func_L06_0044B250.c).
 * Everything after the -1 test matches exactly, including the branch-likely
 * `beql v0,zero` for the second guard and the shared epilogue targets.
 * Best shape tried: named `char *p` + *(int *)p.
 */
extern char D_L06_00189EA0[];
extern char D_L06_001395D0;
extern int func_333898();
extern void func_2E3EF0(char *);
extern int func_360F58(char *);
extern void func_35A550(char *, int, int);

void func_L06_004504C8(char *a0)
{
    char *p;

    if (*(unsigned char *)(a0 + 0x20) != 0)
        return;
    p = *(char **)(a0 + 0x68);
    if (*(int *)p == -1)
        return;
    if (func_360F58(D_L06_00189EA0) == 0)
        return;
    if (func_333898() == 0)
        return;
    D_L06_001395D0 = 1;
    func_35A550(a0, 1, -1);
}
