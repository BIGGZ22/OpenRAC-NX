/* 8 instructions; 3 differ: the table-base load comes out as `lui $v1,0; lw $v0,0($v1)` (retail: `lui $v0,0x1B; lw $v0,0x1844($v0)`) and the sll/lw order differs. The two stores already match. */
extern char *D_L00_001B1844;

void func_L00_0042A500(int i)
{
    char *t = D_L00_001B1844;
    char *v = *(char **)(t + 28 + (i << 5));

    *(short *)(v + 38) = 0;
    *(int *)(v + 32) = 0;
}
