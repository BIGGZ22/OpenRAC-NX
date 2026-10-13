/* func_L06_0044B350 (116 bytes) -- ABANDON, 3 words (see func_L06_0044B250.c).
 * Best shape tried: named `int v` assigned from *(int *)*(char **)(a0+0x68).
 */
extern char D_L06_00189EA0[];
extern char D_L06_001B13F0[];
extern char D_L06_001395CF;
extern void func_2E3EF0(char *);
extern int func_360F58(char *);
extern void func_35A550(char *, int, int);

void func_L06_0044B350(char *a0)
{
    int v;

    if (*(unsigned char *)(a0 + 0x20) != 0)
        return;
    v = *(int *)*(char **)(a0 + 0x68);
    if (v == -1)
        return;
    if (func_360F58(D_L06_00189EA0) == 0)
        return;
    func_2E3EF0(D_L06_001B13F0);
    D_L06_001395CF = 1;
    func_35A550(a0, 1, -1);
}
