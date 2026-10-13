/* func_L06_0044B250 (136 bytes) -- ABANDON. Same family as 0044B2D8/0044B350/004504C8.
 * Only three words differ (first-difference at +0x20):
 *   retail: lw v0,0x68(s0) ; addiu v1,zero,-1 ; lw a1,0(v0) ; beq a1,v1
 *   mine  : lw v0,0x68(s0) ; addiu a0,zero,-1 ; lw v1,0(v0) ; beq v1,a0
 * Everything else (frame, all four address materialisations, both call sites,
 * the -1 argument, the branch to the shared epilogue) is byte-identical.
 * Tried: direct `**(int **)`; named `int v`; named `char *p`; reversed operands
 * `-1 == x`. All four produce the same allocation. The -1 constant is allocated
 * one step earlier than the loaded value here, and the value skips $a0 in retail.
 */
extern char D_L06_00189EA0[];
extern char D_L06_001B1330[];
extern char D_L06_001B1370[];
extern char D_L06_001395B8[];
extern void func_2E3EF0(char *);
extern int func_360F58(char *);
extern void func_35A550(char *, int, int);

void func_L06_0044B250(char *a0)
{
    if (*(unsigned char *)(a0 + 0x20) != 0)
        return;
    if (**(int **)(a0 + 0x68) == -1)
        return;
    if (func_360F58(D_L06_00189EA0) == 0)
        return;
    func_2E3EF0(D_L06_001B1330);
    func_2E3EF0(D_L06_001B1370);
    D_L06_001395B8[0x14] = 1;
    D_L06_001395B8[0x13] = 1;
    func_35A550(a0, 1, -1);
}
