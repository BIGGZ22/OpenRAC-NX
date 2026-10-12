/* func_L00_003F0130: 104 bytes, 27 places; the loop, prologue and call match, but the final read of the base global (0x1B2BB4) is "lw $v0,0x3BC4($gp)" in retail, which -G0 (this file) cannot emit: needs -G8 for L00_003EE100.c. Missing: the file flags and the order of the entry-address computation. */
extern char *D_L00_001B2BB4 __attribute__((sda));

extern void func_L00_003F09A0(char *entry);

char *func_L00_003F0130(int a0)
{
    int off = 0;
    int n = 0;

    do {
        char *e = D_L00_001B2BB4 + off;
        if (*(int *)(e + 0x30) == a0) {
            func_L00_003F09A0(e);
        }
        off += 0x50;
    } while (--n >= 0);
    return D_L00_001B2BB4;
}
